#pragma once
#include <cmath>

struct Vec3 {
    double x = 0, y = 0, z = 0;
    Vec3 operator+(Vec3 v) const { return {x+v.x,y+v.y,z+v.z}; }
    Vec3 operator-(Vec3 v) const { return {x-v.x,y-v.y,z-v.z}; }
    Vec3 operator*(double s) const { return {x*s,y*s,z*s}; }
};
inline double dot(Vec3 a, Vec3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
inline Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}
inline Vec3 normalized(Vec3 v) {
    const double length = std::sqrt(dot(v,v));
    return length > 1e-12 ? v*(1.0/length) : Vec3{};
}
inline double radians(double degrees) { return degrees*3.14159265358979323846/180.0; }
