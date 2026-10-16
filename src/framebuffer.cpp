#include "framebuffer.h"
#include <fstream>
#include <limits>
#include <stdexcept>

Framebuffer::Framebuffer(int width, int height) : width_(width), height_(height) {
    if(width < 1 || height < 1 || width > 8192 || height > 8192)
        throw std::runtime_error("Dimensiones de framebuffer invalidas");
    pixels_.resize(std::size_t(width)*height);
    depths_.resize(pixels_.size());
    clear({17,24,33});
}
void Framebuffer::clear(Color color) {
    std::fill(pixels_.begin(), pixels_.end(), color.packed());
    std::fill(depths_.begin(), depths_.end(), std::numeric_limits<double>::infinity());
}
void Framebuffer::point(int x, int y, double depth, Color color) {
    if(x < 0 || y < 0 || x >= width_ || y >= height_) return;
    const auto index = std::size_t(y)*width_+x;
    if(depth < depths_[index]) {
        depths_[index] = depth;
        pixels_[index] = color.packed();
    }
}

namespace {
using Bytes = std::vector<std::uint8_t>;
void bigEndian(Bytes& out, std::uint32_t n) {
    for(int shift : {24,16,8,0}) out.push_back(std::uint8_t(n >> shift));
}
std::uint32_t crc32(const std::uint8_t* data, std::size_t size) {
    std::uint32_t crc = 0xffffffffu;
    for(std::size_t i=0;i<size;++i) {
        crc ^= data[i];
        for(int bit=0;bit<8;++bit) crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320u : 0u);
    }
    return crc ^ 0xffffffffu;
}
void chunk(Bytes& out, const char* type, const Bytes& payload) {
    bigEndian(out, static_cast<std::uint32_t>(payload.size()));
    const auto start = out.size();
    out.insert(out.end(),type,type+4);
    out.insert(out.end(),payload.begin(),payload.end());
    bigEndian(out,crc32(out.data()+start,out.size()-start));
}
}

// PNG RGB de 8 bits, sin dependencias: zlib con bloques DEFLATE sin compresion.
// Los bytes son exactamente los pixeles del framebuffer que muestra SDL.
void Framebuffer::savePNG(const std::filesystem::path& path) const {
    Bytes scanlines;
    scanlines.reserve(std::size_t(height_)*(1+3*std::size_t(width_)));
    for(int y=0;y<height_;++y) {
        scanlines.push_back(0); // Filtro PNG "None".
        for(int x=0;x<width_;++x) {
            auto pixel = pixels_[std::size_t(y)*width_+x];
            for(int shift : {16,8,0}) scanlines.push_back(std::uint8_t(pixel >> shift));
        }
    }
    Bytes zlib{0x78,0x01};
    for(std::size_t offset=0;offset<scanlines.size();) {
        auto length = std::min<std::size_t>(65535,scanlines.size()-offset);
        zlib.push_back(offset+length == scanlines.size() ? 1 : 0);
        const auto n = static_cast<std::uint16_t>(length);
        const auto inverse = static_cast<std::uint16_t>(~n);
        zlib.insert(zlib.end(),{std::uint8_t(n),std::uint8_t(n >> 8),
                               std::uint8_t(inverse),std::uint8_t(inverse >> 8)});
        zlib.insert(zlib.end(),scanlines.begin()+offset,scanlines.begin()+offset+length);
        offset += length;
    }
    std::uint32_t a=1,b=0;
    for(auto byte : scanlines) { a=(a+byte)%65521; b=(b+a)%65521; }
    bigEndian(zlib,(b << 16) | a);
    Bytes png{137,80,78,71,13,10,26,10}, header;
    bigEndian(header,std::uint32_t(width_));
    bigEndian(header,std::uint32_t(height_));
    header.insert(header.end(),{8,2,0,0,0});
    chunk(png,"IHDR",header);
    chunk(png,"IDAT",zlib);
    chunk(png,"IEND",{});
    if(path.has_parent_path()) std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path,std::ios::binary);
    file.write(reinterpret_cast<const char*>(png.data()),static_cast<std::streamsize>(png.size()));
    if(!file) throw std::runtime_error("No se pudo guardar PNG: " + path.string());
}
