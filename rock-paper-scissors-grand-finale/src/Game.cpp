#include "Game.h"

#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>

#include "misc.h"
#include "mathh.h"

void Game::Init() {
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

	SDL_Init(SDL_INIT_VIDEO
			 | SDL_INIT_AUDIO);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Mix_Init(0);

	// SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	window = SDL_CreateWindow("rock-paper-scissors-grand-finale",
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							  GAME_W, GAME_H,
							  SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, -1,
								  SDL_RENDERER_ACCELERATED
								  | SDL_RENDERER_TARGETTEXTURE);

	// SDL_RenderSetLogicalSize(renderer, GAME_W, GAME_H);

	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048);

	Mix_Volume(-1, 32);

	tex_entities = IMG_LoadTexture(renderer, "entities.png");

	snd_rock     = Mix_LoadWAV("rock.wav");
	snd_paper    = Mix_LoadWAV("paper.wav");
	snd_scissors = Mix_LoadWAV("scissors.wav");

	_rdrand64_step(&random.s[0]);
	_rdrand64_step(&random.s[1]);
	_rdrand64_step(&random.s[2]);
	_rdrand64_step(&random.s[3]);

	entities = (Entity*) malloc(MAX_ENTITIES * sizeof(*entities));
	memset(entities, 0, MAX_ENTITIES * sizeof(*entities));

	entity_count = MAX_ENTITIES;
	for (int i = 0; i < entity_count; i++) {
		Entity* e = &entities[i];
		e->x = random.range(0.0f, 1920.0f/*2000.0f*/);
		e->y = random.range(0.0f, 1080.0f/*2000.0f*/);
		e->type = (EntityType) (random.next() % 3);
	}
}

void Game::Quit() {
	free(entities);

	Mix_FreeChunk(snd_scissors);
	Mix_FreeChunk(snd_paper);
	Mix_FreeChunk(snd_rock);

	SDL_DestroyTexture(tex_entities);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

void Game::Run() {
	while (!quit) {
		Frame();
	}
}

void Game::Frame() {
	double t = GetTime();

	double frame_end_time = t + (1.0 / (double)GAME_FPS);

	{
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
				case SDL_QUIT: {
					quit = true;
					break;
				}

				case SDL_KEYDOWN: {
					SDL_Scancode scancode = ev.key.keysym.scancode;
					switch (scancode) {
						case SDL_SCANCODE_ESCAPE: {
							paused ^= true;
							break;
						}

						case SDL_SCANCODE_R: {
							memset(entities, 0, MAX_ENTITIES * sizeof(*entities));

							entity_count = MAX_ENTITIES;
							for (int i = 0; i < entity_count; i++) {
								Entity* e = &entities[i];
								e->x = random.range(0.0f, 1920.0f/*2000.0f*/);
								e->y = random.range(0.0f, 1080.0f/*2000.0f*/);
								e->type = (EntityType) (random.next() % 3);
							}
							break;
						}
					}
				}
			}
		}
	}

	float delta = 60.0f / (float)GAME_FPS;

	if (!paused) {
		Update(delta);
	}

	const Uint8* key = SDL_GetKeyboardState(nullptr);

	float spd = 20.0f;
	if (key[SDL_SCANCODE_LSHIFT]) spd = 10.0f;

	if (key[SDL_SCANCODE_LEFT])  camera_x -= spd * delta;
	if (key[SDL_SCANCODE_RIGHT]) camera_x += spd * delta;
	if (key[SDL_SCANCODE_UP])    camera_y -= spd * delta;
	if (key[SDL_SCANCODE_DOWN])  camera_y += spd * delta;

	Draw(delta);

#ifndef __EMSCRIPTEN__
	t = GetTime();
	double time_left = frame_end_time - t;
	if (time_left > 0.0) {
		SDL_Delay((Uint32) (time_left * (0.95 * 1000.0)));
		while (GetTime() < frame_end_time) {}
	}
#endif
}

Entity* Game::find_closest(float x, float y, EntityType type) {
	Entity* result = nullptr;
	float dist = INFINITY;

	for (int i = 0; i < entity_count; i++) {
		Entity* e = &entities[i];

		if (e->type != type) {
			continue;
		}

		float d = point_distance(x, y, e->x, e->y);
		if (d < dist) {
			dist = d;
			result = e;
		}
	}

	return result;
}

void Game::Update(float delta) {
	for (int i = 0; i < entity_count; i++) {
		Entity* e = &entities[i];

		EntityType target_type = EntityType::SCISSORS;
		if (e->type == EntityType::PAPER) {
			target_type = EntityType::ROCK;
		} else if (e->type == EntityType::SCISSORS) {
			target_type = EntityType::PAPER;
		}

		if (Entity* target = find_closest(e->x, e->y, target_type)) {
			float dx = target->x - e->x;
			float dy = target->y - e->y;
			normalize0(dx, dy, &dx, &dy);
			e->x += dx / 2.0f * delta;
			e->y += dy / 2.0f * delta;
		}
	}

	for (int i = 0; i < entity_count; i++) {
		Entity* e = &entities[i];

		for (int j = 0; j < entity_count; j++) {
			if (i == j) {
				continue;
			}

			Entity* e2 = &entities[j];

			if (!circle_vs_circle(e->x, e->y, 16.0f, e2->x, e2->y, 16.0f)) {
				continue;
			}

			switch (e->type) {
				case EntityType::ROCK: {
					if (e2->type == EntityType::SCISSORS) {
						e2->type = EntityType::ROCK;
						play_sound(snd_rock);
					}
					break;
				}

				case EntityType::PAPER: {
					if (e2->type == EntityType::ROCK) {
						e2->type = EntityType::PAPER;
						play_sound(snd_paper);
					}
					break;
				}

				case EntityType::SCISSORS: {
					if (e2->type == EntityType::PAPER) {
						e2->type = EntityType::SCISSORS;
						play_sound(snd_scissors);
					}
					break;
				}
			}
		}
	}
}

void Game::Draw(float delta) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	for (int i = 0; i < entity_count; i++) {
		Entity* e = &entities[i];

		SDL_Rect src = {
			(int)e->type * 32,
			0,
			32,
			32
		};

		SDL_Rect dest = {
			(int) (e->x - 16.0f - camera_x),
			(int) (e->y - 16.0f - camera_y),
			32,
			32
		};

		SDL_RenderCopy(renderer, tex_entities, &src, &dest);
	}

	SDL_RenderPresent(renderer);
}
