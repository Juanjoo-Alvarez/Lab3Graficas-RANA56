#include "obj_loader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
std::size_t vertexIndex(const std::string& token, std::size_t count) {
    // En "42/8/3" o "42//3", solo el primer numero indexa las posiciones.
    const auto text=token.substr(0,token.find('/'));
    std::size_t consumed=0;
    const auto raw=std::stoll(text,&consumed);
    if(consumed != text.size() || raw == 0) throw std::runtime_error("Indice OBJ invalido: " + token);
    // OBJ usa indices positivos desde 1, o negativos desde el ultimo vertice.
    const auto index=raw > 0 ? raw-1 : static_cast<long long>(count)+raw;
    if(index < 0 || static_cast<unsigned long long>(index) >= count)
        throw std::runtime_error("Indice OBJ fuera de rango: " + token);
    return static_cast<std::size_t>(index);
}
void loadMTL(Mesh& mesh, const std::filesystem::path& path) {
    std::ifstream file(path);
    if(!file) throw std::runtime_error("No se pudo abrir MTL: " + path.string());
    std::string line,name;
    while(std::getline(file,line)) {
        std::istringstream stream(line);
        std::string tag;
        stream >> tag;
        if(tag == "newmtl") stream >> name;
        else if(tag == "Kd") {
            double r,g,b;
            if(name.empty() || !(stream >> r >> g >> b) || !std::isfinite(r+g+b))
                throw std::runtime_error("Color Kd invalido en MTL");
            mesh.materials[name]=Color(int(std::lround(std::clamp(r,0.0,1.0)*255)),
                int(std::lround(std::clamp(g,0.0,1.0)*255)),int(std::lround(std::clamp(b,0.0,1.0)*255)));
        }
    }
}
}
Mesh parseOBJ(std::istream& input, const std::filesystem::path& baseDirectory) {
    Mesh mesh;
    std::string line,material;
    std::size_t lineNumber=0;
    while(std::getline(input,line)) {
        ++lineNumber;
        line=line.substr(0,line.find('#'));
        std::istringstream stream(line);
        std::string tag;
        stream >> tag;
        try {
            if(tag == "v") {
                Vec3 v;
                if(!(stream >> v.x >> v.y >> v.z) || !std::isfinite(v.x+v.y+v.z))
                    throw std::runtime_error("Vertice invalido");
                mesh.vertices.push_back(v);
            } else if(tag == "f") {
                std::array<std::string,3> tokens;
                std::string extra;
                if(!(stream >> tokens[0] >> tokens[1] >> tokens[2]) || stream >> extra)
                    throw std::runtime_error("El OBJ debe estar triangulado: se requieren 3 indices por cara");
                mesh.faces.push_back({{vertexIndex(tokens[0],mesh.vertices.size()),
                    vertexIndex(tokens[1],mesh.vertices.size()),
                    vertexIndex(tokens[2],mesh.vertices.size())},material});
            } else if(tag == "usemtl") stream >> material;
            else if(tag == "mtllib") {
                std::string filename;
                while(stream >> filename) loadMTL(mesh,baseDirectory/filename);
            }
        } catch(const std::exception& error) {
            throw std::runtime_error("OBJ linea " + std::to_string(lineNumber) + ": " + error.what());
        }
    }
    if(mesh.vertices.empty() || mesh.faces.empty()) throw std::runtime_error("OBJ sin vertices o caras");
    return mesh;
}
Mesh loadOBJ(const std::filesystem::path& path) {
    std::ifstream file(path);
    if(!file) throw std::runtime_error("No se pudo abrir OBJ: " + path.string());
    return parseOBJ(file,path.parent_path());
}
