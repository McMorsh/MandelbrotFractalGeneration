#include "view.hpp"

#include <algorithm>
#include <cmath>

namespace {

double SanitizeSpan(double span) {
  if (!std::isfinite(span) || span <= 0.0) {
    return kInitialSpan;
  }
  return std::clamp(span, kMinSpan, kMaxSpan);
}

double SanitizeCenter(double value, double fallback) {
  return std::isfinite(value) ? value : fallback;
}

}  // namespace

View::View(int width, int height) {
  Resize(width, height);
}

View::View(double center_x, double center_y, double span, int width, int height)
    : center_x_(SanitizeCenter(center_x, kInitialCenterX)),
      center_y_(SanitizeCenter(center_y, kInitialCenterY)),
      span_(SanitizeSpan(span)) {
  Resize(width, height);
}

double View::GetCenterX() const {
  return center_x_;
}

double View::GetCenterY() const {
  return center_y_;
}

double View::GetSpan() const {
  return span_;
}

int View::GetWidth() const {
  return width_;
}

int View::GetHeight() const {
  return height_;
}

void View::Resize(int width, int height) {
  if (width <= 0 || height <= 0) {
    return;
  }

  width_ = width;
  height_ = height;
}

ComplexPoint View::GetPointAt(double screen_x, double screen_y) const {
  const double scale = span_ / width_;
  return {center_x_ + (screen_x - width_ * 0.5) * scale,
          center_y_ - (screen_y - height_ * 0.5) * scale};
}

bool View::ZoomAt(double factor, double screen_x, double screen_y) {
  // A negative or NaN factor would break the center.
  if (!(factor > 0.0)) {
    return false;
  }
  if (!std::isfinite(screen_x) || !std::isfinite(screen_y)) {
    return false;
  }

  const double new_span = std::clamp(span_ * factor, kMinSpan, kMaxSpan);
  if (new_span == span_) {
    return false;
  }

  const double new_scale = new_span / width_;
  const ComplexPoint anchor = GetPointAt(screen_x, screen_y);

  center_x_ = anchor.real - (screen_x - width_ * 0.5) * new_scale;
  center_y_ = anchor.imag + (screen_y - height_ * 0.5) * new_scale;
  span_ = new_span;
  return true;
}

void View::PanBy(double delta_x, double delta_y) {
  if (!std::isfinite(delta_x) || !std::isfinite(delta_y)) {
    return;
  }

  const double scale = span_ / width_;
  center_x_ -= delta_x * scale;
  center_y_ += delta_y * scale;
}

bool IsSameView(const View& lhs, const View& rhs) {
  return lhs.GetCenterX() == rhs.GetCenterX() &&
         lhs.GetCenterY() == rhs.GetCenterY() && lhs.GetSpan() == rhs.GetSpan();
}
