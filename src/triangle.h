#pragma once
#include "framebuffer.h"
#include "math3d.h"
#include <algorithm>
#include <cmath>

inline double edge(Vec3 a, Vec3 b, double x, double y) {
    return (b.x-a.x)*(y-a.y)-(b.y-a.y)*(x-a.x);
}

// Relleno por coordenadas baricentricas; funciona con ambos sentidos de winding.
// Vec3 contiene x,y en pixeles y z en espacio de camara (menor = mas cercano).
inline void triangle(Framebuffer& buffer, Vec3 a, Vec3 b, Vec3 c, Color color) {
    const double area = edge(a,b,c.x,c.y);
    if(std::abs(area) < 1e-9) return;
    const double minX=std::max(0.0,std::floor(std::min({a.x,b.x,c.x})));
    const double maxX=std::min(double(buffer.width()-1),std::ceil(std::max({a.x,b.x,c.x})));
    const double minY=std::max(0.0,std::floor(std::min({a.y,b.y,c.y})));
    const double maxY=std::min(double(buffer.height()-1),std::ceil(std::max({a.y,b.y,c.y})));
    if(minX > maxX || minY > maxY) return;
    for(int y=int(minY);y<=int(maxY);++y) {
        for(int x=int(minX);x<=int(maxX);++x) {
            const double px=x+.5,py=y+.5;
            const double w0=edge(b,c,px,py)/area;
            const double w1=edge(c,a,px,py)/area;
            const double w2=edge(a,b,px,py)/area;
            if(w0 >= -1e-10 && w1 >= -1e-10 && w2 >= -1e-10)
                buffer.point(x,y,w0*a.z+w1*b.z+w2*c.z,color);
        }
    }
}
