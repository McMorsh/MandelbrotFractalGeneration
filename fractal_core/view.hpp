#ifndef FRACTALGENERATION_FRACTAL_CORE_VIEW_HPP_
#define FRACTALGENERATION_FRACTAL_CORE_VIEW_HPP_

/// Initial Mandelbrot view: the whole set fits into the window.
inline constexpr double kInitialCenterX = -0.6;
inline constexpr double kInitialCenterY = 0.0;
inline constexpr double kInitialSpan = 3.5;

/// At smaller scales double no longer distinguishes adjacent pixels on a
/// typical window width.
inline constexpr double kMinSpan = 1e-11;
inline constexpr double kMaxSpan = 8.0;

/// Point in the complex plane.
struct ComplexPoint {
  double real = 0.0;
  double imag = 0.0;
};

/// Camera: window center, width in complex units, and screen size.
/// The screen size comes from outside, so the camera can be tested without a
/// window.
class View {
 public:
  /// Creates the initial view for a screen of the given size.
  View(int width, int height);
  /// Creates a view with the given center and scale. Span is clamped to
  /// [kMinSpan, kMaxSpan], and non-finite values are replaced with the initial
  /// ones.
  View(double center_x, double center_y, double span, int width, int height);

  /// Returns the view center on the real axis.
  double GetCenterX() const;
  /// Returns the view center on the imaginary axis.
  double GetCenterY() const;
  /// Returns the window width in complex units.
  double GetSpan() const;
  /// Returns the screen width in pixels.
  int GetWidth() const;
  /// Returns the screen height in pixels.
  int GetHeight() const;

  /// Resizes the screen while keeping the center and scale. An invalid size
  /// is ignored.
  void Resize(int width, int height);

  /// Returns the point in the complex plane under a screen point given in
  /// pixels.
  ComplexPoint GetPointAt(double screen_x, double screen_y) const;

  /// Scales the span by factor while keeping the point under the cursor.
  /// @param factor positive multiplier: greater than 1 zooms in, less than 1
  /// zooms out
  /// @return false if the scale did not change or factor is not positive.
  bool ZoomAt(double factor, double screen_x, double screen_y);

  /// Pans the image under the cursor. delta is given in pixels.
  void PanBy(double delta_x, double delta_y);

 private:
  double center_x_ = kInitialCenterX;
  double center_y_ = kInitialCenterY;
  double span_ = kInitialSpan;
  int width_ = 1;
  int height_ = 1;
};

/// Compares the center and span; screen sizes are ignored.
bool IsSameView(const View& lhs, const View& rhs);

#endif  // FRACTALGENERATION_FRACTAL_CORE_VIEW_HPP_
