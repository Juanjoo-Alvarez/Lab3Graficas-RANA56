#include "renderer.h"
#include "triangle.h"
#include <limits>

RenderStats render(const Mesh& mesh, Framebuffer& buffer, View view) {
    buffer.clear({17,24,33});
    const double yaw=radians(view.yaw),pitch=radians(view.elevation);
    const Vec3 towardCamera{std::sin(yaw)*std::cos(pitch),std::sin(pitch),std::cos(yaw)*std::cos(pitch)};
    const Vec3 right=normalized(cross({0,1,0},towardCamera));
    const Vec3 up=cross(towardCamera,right);
    std::vector<Vec3> projected;
    double minX=std::numeric_limits<double>::infinity(),minY=minX;
    double maxX=-minX,maxY=-minX;
    for(Vec3 v : mesh.vertices) {
        Vec3 p{dot(v,right),dot(v,up),-dot(v,towardCamera)};
        projected.push_back(p);
        minX=std::min(minX,p.x); maxX=std::max(maxX,p.x);
        minY=std::min(minY,p.y); maxY=std::max(maxY,p.y);
    }
    // Ajuste ortografico: 12% de margen minimo y centro del bounding box en pantalla.
    const double scale=std::min(buffer.width()*.76/std::max(maxX-minX,1e-9),
                               buffer.height()*.76/std::max(maxY-minY,1e-9));
    for(Vec3& p : projected) {
        p.x=buffer.width()*.5+(p.x-(minX+maxX)*.5)*scale;
        p.y=buffer.height()*.5-(p.y-(minY+maxY)*.5)*scale;
    }
    RenderStats stats;
    const Vec3 light=normalized({-.35,.8,1.0});
    for(const Face& face : mesh.faces) {
        // Recorrido MANUAL solicitado: cara -> indices -> tres vertices -> triangle.
        const std::size_t i0=face.vertexIndices[0];
        const std::size_t i1=face.vertexIndices[1];
        const std::size_t i2=face.vertexIndices[2];
        const Vec3 v0=mesh.vertices[i0];
        const Vec3 v1=mesh.vertices[i1];
        const Vec3 v2=mesh.vertices[i2];
        const Vec3 normal=normalized(cross(v1-v0,v2-v0));
        const auto material=mesh.materials.find(face.material);
        const Color base=material == mesh.materials.end() ? Color(100,190,65) : material->second;
        const Color currentColor=base.shaded(.45+.55*std::max(0.0,dot(normal,light)));
        triangle(buffer,projected[i0],projected[i1],projected[i2],currentColor);
        ++stats.submittedTriangles;
    }
    stats.minX=buffer.width(); stats.minY=buffer.height();
    stats.maxX=-1; stats.maxY=-1;
    for(int y=0;y<buffer.height();++y) for(int x=0;x<buffer.width();++x) {
        if(std::isfinite(buffer.depths()[std::size_t(y)*buffer.width()+x])) {
            ++stats.visiblePixels;
            stats.minX=std::min(stats.minX,x); stats.maxX=std::max(stats.maxX,x);
            stats.minY=std::min(stats.minY,y); stats.maxY=std::max(stats.maxY,y);
        }
    }
    return stats;
}
