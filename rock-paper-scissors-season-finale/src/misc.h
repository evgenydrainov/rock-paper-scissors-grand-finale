#define ArrayLength(a) (sizeof(a) / sizeof(*a))

static double GetTime() {
	return (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
}

static void stop_sound(Mix_Chunk* chunk) {
	for (int i = 0; i < Mix_AllocateChannels(-1); i++) {
		if (Mix_Playing(i) && Mix_GetChunk(i) == chunk) {
			Mix_HaltChannel(i);
		}
	}
}

static void play_sound(Mix_Chunk* chunk) {
	stop_sound(chunk);
	Mix_PlayChannel(-1, chunk, 0);
}
