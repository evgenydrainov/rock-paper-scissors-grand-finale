#pragma once

#include <SDL.h>
#include <SDL_mixer.h>

#include "xoshiro256plusplus.h"

#define GAME_W 640
#define GAME_H 480
#define GAME_FPS 60

enum struct EntityType {
	ROCK,
	PAPER,
	SCISSORS
};

struct Entity {
	EntityType type;
	float x;
	float y;
};

struct Game {
	Entity* entities;
	int entity_count;

	float camera_x;
	float camera_y;
	float map_w;
	float map_h;
	float entity_speed;
	float entity_run_away_speed;
	float entity_shiver_multiplier = 0.5f;

	xoshiro256plusplus random;

	bool paused;
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool quit;
	int frame;
	double prev_time;

	SDL_Texture* tex_entities;

	Mix_Chunk* snd_rock;
	Mix_Chunk* snd_paper;
	Mix_Chunk* snd_scissors;

	void Init();
	void Quit();
	void Run();
	void Frame();
	void Update(float delta);
	void Draw(float delta);
	void Reset();

	Entity* find_closest(Entity* e);
};
