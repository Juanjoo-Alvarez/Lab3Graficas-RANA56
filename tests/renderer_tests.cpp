#include "obj_loader.h"
#include "renderer.h"
#include "triangle.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

void require(bool condition,const char* message) {
    if(!condition) throw std::runtime_error(message);
}
void parserTests() {
    const std::string vertices="v 0 0 0\nv 1 0 0\nv 0 1 0\n";
    for(const char* face : {"f 1 2 3", "f 1/1 2/2 3/3", "f 1//1 2//1 3//1",
                            "f -3/1/1 -2/2/1 -1/3/1 # comentario"}) {
        std::istringstream input(vertices+face);
        auto mesh=parseOBJ(input);
        require(mesh.faces[0].vertexIndices == std::array<std::size_t,3>{0,1,2},"Indices OBJ incorrectos");
    }
    for(const char* face : {"f 0 2 3", "f 1 2 4", "f -4 -2 -1", "f 1x 2 3", "f 1 2", "f 1 2 3 1"}) {
        bool rejected=false;
        try { std::istringstream input(vertices+face); parseOBJ(input); }
        catch(const std::exception&) { rejected=true; }
        require(rejected,"El cargador acepto una cara invalida");
    }
}
void triangleTests() {
    Framebuffer a(16,16),b(16,16);
    const Color red(255,0,0),green(0,255,0),black(0,0,0);
    a.clear(black); b.clear(black);
    triangle(a,{2,2,5},{13,2,5},{2,13,5},red);
    triangle(b,{2,13,5},{13,2,5},{2,2,5},red);
    require(a.pixels() == b.pixels(),"El winding cambio la cobertura");
    require(a.pixels()[3*16+3] == red.packed(),"No se relleno el interior del triangulo");
    require(a.pixels()[14*16+14] == black.packed(),"Se relleno fuera del triangulo");
    triangle(a,{2,2,1},{13,2,1},{2,13,1},green);
    triangle(a,{2,2,8},{13,2,8},{2,13,8},red);
    require(a.pixels()[3*16+3] == green.packed(),"El z-buffer no conserva la superficie cercana");
    b.clear(black);
    triangle(b,{-20,-20,1},{40,-20,1},{-20,40,1},green);
    require(b.pixels()[0] == green.packed(),"El recorte elimino un triangulo visible");
    auto before=b.pixels();
    triangle(b,{1,1,0},{5,5,0},{9,9,0},red);
    require(before == b.pixels(),"El triangulo degenerado modifico el framebuffer");
    a.clear(black);
    triangle(a,{0,0,1},{16,0,1},{0,16,1},green);
    triangle(a,{16,0,1},{16,16,1},{0,16,1},green);
    require(std::all_of(a.pixels().begin(),a.pixels().end(),[&](auto p){return p==green.packed();}),
            "Hay huecos entre triangulos adyacentes");
    a.clear(black);
    triangle(a,{0,0,1},{16,0,5},{0,16,1},green);
    require(a.depths()[1*16+1] < a.depths()[1*16+10],"No se interpola la profundidad");
}
void modelTest(const char* path) {
    const auto mesh=loadOBJ(path);
    require(mesh.vertices.size() == 618 && mesh.faces.size() == 1232,"Modelo de la entrega inesperado");
    require(mesh.materials.size() == 7,"No se cargaron los siete materiales");
    Framebuffer framebuffer(1200,900);
    for(View view : {View{},View{123.7,32},View{33.7,-40}}) {
        auto stats=render(mesh,framebuffer,view);
        require(stats.submittedTriangles == mesh.faces.size(),"No se recorrieron todas las caras");
        require(stats.visiblePixels > 1200*900/6,"El modelo es demasiado pequeno");
        require(stats.minX >= 100 && stats.maxX < 1100 && stats.minY >= 85 && stats.maxY < 815,
                "El modelo sale del margen de pantalla");
        require(std::abs(stats.minX+stats.maxX-1199) <= 2 && std::abs(stats.minY+stats.maxY-899) <= 2,
                "El modelo no esta centrado");
    }
}
int main(int argc,char** argv) {
    try {
        parserTests(); triangleTests();
        require(argc == 2,"Falta ruta al OBJ de integracion");
        modelTest(argv[1]);
        std::cout << "OK: OBJ manual, indices, triangulos, winding, clipping, profundidad y encuadre.\n";
    } catch(const std::exception& error) {
        std::cerr << "FALLO: " << error.what() << '\n';
        return 1;
    }
}
