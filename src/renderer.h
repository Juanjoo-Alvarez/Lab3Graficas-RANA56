#pragma once
#include "framebuffer.h"
#include "obj_loader.h"

struct View { double yaw=33.7, elevation=32.0; };
struct RenderStats {
    std::size_t submittedTriangles=0, visiblePixels=0;
    int minX=0,minY=0,maxX=0,maxY=0;
};
RenderStats render(const Mesh& mesh, Framebuffer& buffer, View view = {});
