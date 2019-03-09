#pragma once
#include <SDL.h>
#include <stdbool.h>
#include "GraphicsSettings.h"

//public, readonly
SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;
Uint8 * keyState = NULL;
SDL_Surface * softwareRenderScreen = NULL;

//false = failed
//run this in SDL's main
bool StartWindow(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH_START, SCREEN_HEIGHT_START, 0, &window, &renderer) == 0)
		{
			keyState = (Uint8*)SDL_GetKeyboardState(NULL);
			if (keyState == NULL)
				abort();
			softwareRenderScreen = SDL_CreateRGBSurface(0, SCREEN_WIDTH_START, SCREEN_HEIGHT_START, 32, 0, 0, 0, 0);
			return true;
		}
	}
	return false;
}


#define START_SOFTWARE_RENDER SDL_LockSurface(softwareRenderScreen)
#define END_SOFTWARE_RENDER SDL_UnlockSurface(softwareRenderScreen)


void SoftwareRender(void) {
	//clear screen
	//SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	//SDL_RenderClear(renderer);
	SDL_Texture * const bufferScreen = SDL_CreateTextureFromSurface(renderer, softwareRenderScreen);
	SDL_RenderCopy(renderer, bufferScreen, NULL, NULL);
	SDL_RenderPresent(renderer);
	SDL_DestroyTexture(bufferScreen);
}