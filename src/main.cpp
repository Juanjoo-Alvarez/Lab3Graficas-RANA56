#include <SDL.h>
#include "renderer.h"
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>

constexpr int SCREEN_WIDTH=1200,SCREEN_HEIGHT=900;
constexpr const char* MODEL_NAME="RANA_56_Juan_Jose_Rivas_Alvarez_24856.obj";

struct Options {
    std::filesystem::path model,output="renders/rana_renderizada.png";
    bool headless=false,save=false,help=false;
    int frames=0;
};
Options options(int argc, char** argv) {
    Options result;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg == "--headless") result.headless=true;
        else if(arg == "--help" || arg == "-h") result.help=true;
        else if(arg == "--model" || arg == "--output" || arg == "--frames") {
            if(i+1 >= argc) throw std::runtime_error("Falta valor para " + arg);
            const std::string value=argv[++i];
            if(arg == "--model") result.model=value;
            else if(arg == "--output") { result.output=value; result.save=true; }
            else {
                std::size_t used=0;
                result.frames=std::stoi(value,&used);
                if(used != value.size() || result.frames < 1) throw std::runtime_error("--frames debe ser positivo");
            }
        } else throw std::runtime_error("Opcion desconocida: " + arg);
    }
    return result;
}

struct SDLSession {
    SDLSession() {
        SDL_SetMainReady();
        if(SDL_Init(SDL_INIT_VIDEO) != 0) throw std::runtime_error(SDL_GetError());
    }
    ~SDLSession() { SDL_Quit(); }
};

void renderBuffer(SDL_Renderer* renderer, SDL_Texture* texture, const Framebuffer& buffer) {
    void* pixels=nullptr;
    int pitch=0;
    if(SDL_LockTexture(texture,nullptr,&pixels,&pitch) != 0) throw std::runtime_error(SDL_GetError());
    for(int y=0;y<buffer.height();++y) {
        std::memcpy(static_cast<std::uint8_t*>(pixels)+std::size_t(y)*pitch,
            buffer.pixels().data()+std::size_t(y)*buffer.width(),std::size_t(buffer.width())*4);
    }
    SDL_UnlockTexture(texture);
    if(SDL_RenderClear(renderer) != 0 || SDL_RenderCopy(renderer,texture,nullptr,nullptr) != 0)
        throw std::runtime_error(SDL_GetError());
    SDL_RenderPresent(renderer);
}

int main(int argc,char** argv) {
    try {
        auto config=options(argc,argv);
        if(config.help) {
            std::cout << "RANA-56 | Juan Jose Rivas Alvarez - 24856\n"
                         "Uso: rana_renderer [--model nave.obj] [--output imagen.png] [--headless] [--frames N]\n"
                         "Flechas: rotar | R: vista inicial | P: guardar PNG | Escape: salir\n";
            return 0;
        }
        if(config.model.empty()) {
            config.model=MODEL_NAME;
            if(!std::filesystem::exists(config.model)) {
                std::unique_ptr<char,decltype(&SDL_free)> base(SDL_GetBasePath(),SDL_free);
                if(base) config.model=std::filesystem::path(base.get())/MODEL_NAME;
            }
        }
        const Mesh mesh=loadOBJ(config.model);
        Framebuffer framebuffer(SCREEN_WIDTH,SCREEN_HEIGHT);
        View view;
        const auto stats=render(mesh,framebuffer,view);
        std::cout << mesh.vertices.size() << " vertices, " << stats.submittedTriangles << " triangulos\n"
                  << "Pixeles del modelo: " << stats.visiblePixels << "; limites: ("
                  << stats.minX << ',' << stats.minY << ") - (" << stats.maxX << ',' << stats.maxY << ")\n";
        if(config.headless || config.save) {
            framebuffer.savePNG(config.output);
            std::cout << "PNG guardado: " << config.output.string() << '\n';
        }
        if(config.headless) return 0;

        SDLSession session;
        std::unique_ptr<SDL_Window,decltype(&SDL_DestroyWindow)> window(SDL_CreateWindow(
            "RANA-56 | Renderizador por software | Flechas: rotar, R: reiniciar, P: PNG, Esc: salir",
            SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH,SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),SDL_DestroyWindow);
        if(!window) throw std::runtime_error(SDL_GetError());
        // SDL solo presenta la imagen. Ninguna llamada SDL dibuja los triangulos.
        std::unique_ptr<SDL_Renderer,decltype(&SDL_DestroyRenderer)> renderer(
            SDL_CreateRenderer(window.get(),-1,SDL_RENDERER_SOFTWARE),SDL_DestroyRenderer);
        if(!renderer) throw std::runtime_error(SDL_GetError());
        if(SDL_RenderSetLogicalSize(renderer.get(),SCREEN_WIDTH,SCREEN_HEIGHT) != 0)
            throw std::runtime_error(SDL_GetError());
        std::unique_ptr<SDL_Texture,decltype(&SDL_DestroyTexture)> texture(SDL_CreateTexture(
            renderer.get(),SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,SCREEN_HEIGHT),SDL_DestroyTexture);
        if(!texture) throw std::runtime_error(SDL_GetError());

        bool running=true,dirty=false;
        int frames=0;
        while(running) {
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                if(event.type == SDL_QUIT) running=false;
                if(event.type == SDL_KEYDOWN) {
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE: running=false; break;
                        case SDLK_LEFT: view.yaw-=5; dirty=true; break;
                        case SDLK_RIGHT: view.yaw+=5; dirty=true; break;
                        case SDLK_UP: view.elevation=std::min(80.0,view.elevation+5); dirty=true; break;
                        case SDLK_DOWN: view.elevation=std::max(-80.0,view.elevation-5); dirty=true; break;
                        case SDLK_r: view={}; dirty=true; break;
                        case SDLK_p:
                            if(dirty) { render(mesh,framebuffer,view); dirty=false; }
                            framebuffer.savePNG(config.output);
                            std::cout << "PNG guardado: " << config.output.string() << '\n';
                            break;
                        default: break;
                    }
                }
            }
            if(dirty) { render(mesh,framebuffer,view); dirty=false; }
            renderBuffer(renderer.get(),texture.get(),framebuffer);
            if(config.frames > 0 && ++frames >= config.frames) running=false;
            SDL_Delay(16);
        }
    } catch(const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
