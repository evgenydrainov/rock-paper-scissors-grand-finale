@echo off
mkdir ..\out
mkdir ..\out\emscripten
emcc -O3 -o ../out/emscripten/index.html^
 -sWASM=1 -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS="[""png""]" -sUSE_SDL_TTF=2 -sUSE_SDL_MIXER=2^
 --preload-file entities.png --preload-file rock.wav --preload-file paper.wav --preload-file scissors.wav^
 src/Game.cpp src/main.cpp src/imgui/imgui.cpp src/imgui/imgui_demo.cpp src/imgui/imgui_draw.cpp src/imgui/imgui_impl_sdl2.cpp src/imgui/imgui_impl_sdlrenderer2.cpp src/imgui/imgui_tables.cpp src/imgui/imgui_widgets.cpp