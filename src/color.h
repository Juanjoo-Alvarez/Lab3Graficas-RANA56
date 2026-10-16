#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

struct Color {
    std::uint8_t r = 0, g = 0, b = 0;
    Color() = default;
    Color(int red, int green, int blue)
        : r(static_cast<std::uint8_t>(std::clamp(red, 0, 255))),
          g(static_cast<std::uint8_t>(std::clamp(green, 0, 255))),
          b(static_cast<std::uint8_t>(std::clamp(blue, 0, 255))) {}
    std::uint32_t packed() const {
        return 0xff000000u | (std::uint32_t(r) << 16) | (std::uint32_t(g) << 8) | b;
    }
    Color shaded(double intensity) const {
        return {int(std::lround(r*intensity)), int(std::lround(g*intensity)),
                int(std::lround(b*intensity))};
    }
};
