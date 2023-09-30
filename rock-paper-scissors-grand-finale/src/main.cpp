#include "Game.h"

#ifndef __EMSCRIPTEN__

int main(int argc, char* argv[]) {
	Game game{};

	game.Init();
	game.Run();
	game.Quit();

	return 0;
}

#else

#include <emscripten.h>

Game* g;

static void emscripten_main_loop() {
	g->Frame();
}

int main(int argc, char* argv[]) {
	Game game{};
	g = &game;

	game.Init();
	emscripten_set_main_loop(emscripten_main_loop, 60, 1);
	game.Quit();

	return 0;
}

#endif
