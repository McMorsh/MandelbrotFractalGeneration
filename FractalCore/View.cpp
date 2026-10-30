#include "View.h"

#include <algorithm>
#include <cmath>

namespace {
	double sanitizeSpan(double span) {
		if (!std::isfinite(span) || span <= 0.0)
			return kInitialSpan;
		return std::clamp(span, kMinSpan, kMaxSpan);
	}

	double sanitizeCenter(double value, double fallback) {
		return std::isfinite(value) ? value : fallback;
	}

} // namespace

View::View(int width, int height) {
	resize(width, height);
}

View::View(double centerX, double centerY, double span, int width, int height)
	: centerX(sanitizeCenter(centerX, kInitialCenterX)), centerY(sanitizeCenter(centerY, kInitialCenterY)),
	  span(sanitizeSpan(span)) {
	resize(width, height);
}

double View::getCenterX() const {
	return centerX;
}

double View::getCenterY() const {
	return centerY;
}

double View::getSpan() const {
	return span;
}

int View::getWidth() const {
	return width;
}

int View::getHeight() const {
	return height;
}

void View::resize(int width, int height) {
	if (width <= 0 || height <= 0)
		return;

	this->width = width;
	this->height = height;
}

ComplexPoint View::pointAt(double screenX, double screenY) const {
	const double scale = span / width;
	return {centerX + (screenX - width * 0.5) * scale, centerY - (screenY - height * 0.5) * scale};
}

bool View::zoomAt(double factor, double screenX, double screenY) {
	// Отрицательный или NaN factor сломал бы центр
	if (!(factor > 0.0))
		return false;
	if (!std::isfinite(screenX) || !std::isfinite(screenY))
		return false;

	const double newSpan = std::clamp(span * factor, kMinSpan, kMaxSpan);
	if (newSpan == span)
		return false;

	const double newScale = newSpan / width;
	const ComplexPoint anchor = pointAt(screenX, screenY);

	centerX = anchor.real - (screenX - width * 0.5) * newScale;
	centerY = anchor.imag + (screenY - height * 0.5) * newScale;
	span = newSpan;
	return true;
}

void View::panBy(double deltaX, double deltaY) {
	if (!std::isfinite(deltaX) || !std::isfinite(deltaY))
		return;

	const double scale = span / width;
	centerX -= deltaX * scale;
	centerY += deltaY * scale;
}

bool sameView(const View& lhs, const View& rhs) {
	return lhs.getCenterX() == rhs.getCenterX() && lhs.getCenterY() == rhs.getCenterY() &&
		   lhs.getSpan() == rhs.getSpan();
}
