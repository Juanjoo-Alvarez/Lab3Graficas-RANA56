#pragma once
#include "color.h"
#include "math3d.h"
#include <array>
#include <filesystem>
#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

struct Face {
    std::array<std::size_t,3> vertexIndices;
    std::string material;
};
struct Mesh {
    std::vector<Vec3> vertices;
    std::vector<Face> faces;
    std::unordered_map<std::string,Color> materials;
};
Mesh parseOBJ(std::istream& input, const std::filesystem::path& baseDirectory = {});
Mesh loadOBJ(const std::filesystem::path& path);
