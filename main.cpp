#include "ColorMap.h"
#include "Fractal.h"
#include "FractalRender.h"

#include <raylib.h>

#include <algorithm>
#include <chrono>
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

constexpr int kIterationStep = 100;
constexpr int kMinIterations = 50;
constexpr int kMaxIterations = 5000;
constexpr int kMinIterationBonus = -2000;
constexpr int kMaxIterationBonus = 3000;

// Чем сильнее приближение, тем больше итераций нужно для проработки деталей
int iterationsForSpan(double span) {
    const double zoom = kInitialSpan / span;
    const double iterations = 150.0 + 100.0 * std::log2(std::max(zoom, 1.0));
    return static_cast<int>(std::clamp(iterations, 150.0, 3000.0));
}

int iterationsForView(double span, int bonus) {
    return std::clamp(iterationsForSpan(span) + bonus, kMinIterations, kMaxIterations);
}

struct ComplexPoint {
    double real;
    double imag;
};

// View хранит окно комплексной плоскости, которое видно на экране
struct View {
    double centerX = kInitialCenterX;
    double centerY = kInitialCenterY;
    double span = kInitialSpan;

    ComplexPoint pointAt(Vector2 mouse) const {
        const double scale = span / kWindowWidth;
        return {
            centerX + (mouse.x - kWindowWidth * 0.5) * scale,
            centerY - (mouse.y - kWindowHeight * 0.5) * scale
        };
    }

    // Приближение колесом мыши: точка под курсором остается на месте.
    // Возвращает false, если масштаб уже уперся в предел и не изменился.
    bool zoomAt(double factor, Vector2 mouse) {
        const double newSpan = std::clamp(span * factor, kMinSpan, kMaxSpan);
        if (newSpan == span) return false;

        const double newScale = newSpan / kWindowWidth;
        const ComplexPoint anchor = pointAt(mouse);

        centerX = anchor.real - (mouse.x - kWindowWidth * 0.5) * newScale;
        centerY = anchor.imag + (mouse.y - kWindowHeight * 0.5) * newScale;
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

std::string makeInfo(const View& view, int maxIter, const char* palette, double renderMs) {
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer),
                  "zoom %.3gx   |   iterations %d   |   %s   |   render %.1f ms",
                  kInitialSpan / view.span, maxIter, palette, renderMs);
    return buffer;
}

std::string makeDetails(const View& view, const ComplexPoint& cursor) {
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer),
                  "center (%.15g, %+.15gi)   |   cursor (%.15g, %+.15gi)",
                  view.centerX, view.centerY, cursor.real, cursor.imag);
    return buffer;
}
} // namespace

int main() {
    FractalRender renderer(kWindowWidth, kWindowHeight, "Mandelbrot Fractal Generation");
    ColorMap colormap;
    View view;

    bool showHelp = true;
    bool needsRender = true;
    bool needsRecolor = false;
    int iterationBonus = 0;
    int maxIter = 0;
    double renderMs = 0.0;

    std::vector<double> data;
    std::string notice;
    double noticeTime = 0.0;

    while (!WindowShouldClose()) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            const Vector2 delta = GetMouseDelta();
            if (delta.x != 0.0f || delta.y != 0.0f) {
                view.panBy(delta);
                needsRender = true;
            }
        }

        const float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            const double factor = std::pow(1.2, -static_cast<double>(wheel));
            needsRender = view.zoomAt(factor, GetMousePosition()) || needsRender;
        }

        if (IsKeyPressed(KEY_LEFT_BRACKET)) {
            iterationBonus = std::clamp(iterationBonus - kIterationStep,
                                        kMinIterationBonus, kMaxIterationBonus);
            needsRender = true;
        }
        if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
            iterationBonus = std::clamp(iterationBonus + kIterationStep,
                                        kMinIterationBonus, kMaxIterationBonus);
            needsRender = true;
        }

        if (IsKeyPressed(KEY_P)) {
            colormap.nextPalette();
            needsRecolor = true;
        }

        if (IsKeyPressed(KEY_R)) {
            view = View{};
            iterationBonus = 0;
            needsRender = true;
        }

        if (IsKeyPressed(KEY_H)) {
            showHelp = !showHelp;
        }

        // Пересчитываем фрактал только при изменении вида,
        // а при смене палитры перекрашиваем готовую карту итераций
        if (needsRender || needsRecolor) {
            const auto start = std::chrono::steady_clock::now();

            if (needsRender) {
                maxIter = iterationsForView(view.span, iterationBonus);
                const Fractal fractal(maxIter);
                data = fractal.generate(
                    kWindowWidth, kWindowHeight, view.centerX, view.centerY, view.span);
                needsRender = false;
            }

            renderer.draw(data, colormap, maxIter);
            needsRecolor = false;
            renderMs = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - start).count();
        }

        // Сохраняем уже пересчитанный кадр, а не предыдущий
        if (IsKeyPressed(KEY_F12)) {
            const std::string path = screenshotPath();
            notice = renderer.saveImage(path) ? "Saved " + path : "Could not save " + path;
            noticeTime = GetTime();
        }

        const ComplexPoint cursor = view.pointAt(GetMousePosition());
        std::string info = makeInfo(view, maxIter, colormap.paletteName(), renderMs);
        std::string details = makeDetails(view, cursor);
        if (!notice.empty() && GetTime() - noticeTime < 3.0) {
            details += "   |   " + notice;
        }
        renderer.display(info, details, showHelp);
    }

    return 0;
}
