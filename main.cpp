#include "ColorMap.h"
#include "Fractal.h"
#include "FractalRender.h"

#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

namespace {
constexpr int kWindowWidth = 1200;
constexpr int kWindowHeight = 800;

// Начальное окно целиком содержит множество Мандельброта
constexpr double kInitialCenterX = -0.6;
constexpr double kInitialCenterY = 0.0;
constexpr double kInitialSpan = 3.5;
// Ниже этого масштаба double перестаёт различать соседние пиксели
constexpr double kMinSpan = 1e-11;
constexpr double kMaxSpan = 8.0;

// Чем сильнее приближение, тем больше итераций нужно для проработки деталей
int iterationsForSpan(double span) {
    const double zoom = kInitialSpan / span;
    const double iterations = 150.0 + 100.0 * std::log2(std::max(zoom, 1.0));
    return static_cast<int>(std::clamp(iterations, 150.0, 3000.0));
}

// View хранит окно комплексной плоскости, которое видно на экране
struct View {
    double centerX = kInitialCenterX;
    double centerY = kInitialCenterY;
    double span = kInitialSpan;

    // Приближение колесом мыши: точка под курсором остается на месте.
    // Возвращает false, если масштаб уже уперся в предел и не изменился.
    bool zoomAt(double factor, Vector2 mouse) {
        const double newSpan = std::clamp(span * factor, kMinSpan, kMaxSpan);
        if (newSpan == span) return false;

        const double oldScale = span / kWindowWidth;
        const double newScale = newSpan / kWindowWidth;

        const double offsetX = (mouse.x - kWindowWidth * 0.5) * oldScale;
        const double offsetY = (mouse.y - kWindowHeight * 0.5) * oldScale;
        const double anchorX = centerX + offsetX;
        const double anchorY = centerY - offsetY;

        centerX = anchorX - (mouse.x - kWindowWidth * 0.5) * newScale;
        centerY = anchorY + (mouse.y - kWindowHeight * 0.5) * newScale;
        span = newSpan;
        return true;
    }

    void panBy(Vector2 delta) {
        const double scale = span / kWindowWidth;
        centerX -= delta.x * scale;
        centerY += delta.y * scale;
    }
};

std::string screenshotPath() {
    const std::time_t now = std::time(nullptr);

    std::tm local{};
#if defined(_WIN32)
    if (localtime_s(&local, &now) != 0) return "mandelbrot.png";
#else
    if (!localtime_r(&now, &local)) return "mandelbrot.png";
#endif

    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "mandelbrot_%Y%m%d_%H%M%S.png", &local);
    return buffer;
}

std::string makeInfo(const View& view, int maxIter) {
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer),
                  "zoom %.3gx   |   iterations %d   |   center (%.15g, %+.15gi)",
                  kInitialSpan / view.span, maxIter, view.centerX, view.centerY);
    return buffer;
}
} // namespace

int main() {
    FractalRender renderer(kWindowWidth, kWindowHeight, "Mandelbrot Fractal Generation");
    ColorMap colormap;
    View view;

    bool showHelp = true;
    bool dirty = true;
    int maxIter = 0;

    std::string notice;
    double noticeTime = 0.0;

    while (!WindowShouldClose()) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            const Vector2 delta = GetMouseDelta();
            if (delta.x != 0.0f || delta.y != 0.0f) {
                view.panBy(delta);
                dirty = true;
            }
        }

        const float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            const double factor = std::pow(1.2, -static_cast<double>(wheel));
            dirty = view.zoomAt(factor, GetMousePosition()) || dirty;
        }

        if (IsKeyPressed(KEY_R)) {
            view = View{};
            dirty = true;
        }

        if (IsKeyPressed(KEY_H)) {
            showHelp = !showHelp;
        }

        // Пересчитываем фрактал только при изменении вида
        if (dirty) {
            maxIter = iterationsForSpan(view.span);
            const Fractal fractal(maxIter);
            const std::vector<double> data = fractal.generate(
                kWindowWidth, kWindowHeight, view.centerX, view.centerY, view.span);
            renderer.draw(data, colormap, maxIter);
            dirty = false;
        }

        // Сохраняем уже пересчитанный кадр, а не предыдущий
        if (IsKeyPressed(KEY_F12)) {
            const std::string path = screenshotPath();
            notice = renderer.saveImage(path) ? "Saved " + path : "Could not save " + path;
            noticeTime = GetTime();
        }

        std::string info = makeInfo(view, maxIter);
        if (!notice.empty() && GetTime() - noticeTime < 3.0) {
            info += "   |   " + notice;
        }
        renderer.display(info, showHelp);
    }

    return 0;
}
