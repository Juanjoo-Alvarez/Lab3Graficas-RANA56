#pragma once
#include "color.h"
#include <filesystem>
#include <vector>

class Framebuffer {
public:
    Framebuffer(int width, int height);
    int width() const { return width_; }
    int height() const { return height_; }
    const std::vector<std::uint32_t>& pixels() const { return pixels_; }
    const std::vector<double>& depths() const { return depths_; }
    void clear(Color color);
    void point(int x, int y, double depth, Color color);
    void savePNG(const std::filesystem::path& path) const;
private:
    int width_, height_;
    std::vector<std::uint32_t> pixels_;
    std::vector<double> depths_;
};
