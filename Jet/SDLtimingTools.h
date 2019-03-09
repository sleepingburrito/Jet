#pragma once
#include <SDL.h>
#include <stdbool.h>

//pubic members
float deltaTime = 0; //is in milliseconds
unsigned int masterClock = 0;

//needs to be called eatch frame to be updated
//pause master time
void UpdateDeltaTime(const bool pause)
{
	static unsigned int timeLastFrame = 0;

	if (!pause){
		unsigned int tmpDelta = SDL_GetTicks() - timeLastFrame;

		if (tmpDelta == 0) { //frame limit of 1000 fps, max accuracy of SDL_GetTicks
			SDL_Delay(1);
			tmpDelta = 1;
		}

		masterClock += tmpDelta;
		deltaTime = (float)tmpDelta;
	}

	timeLastFrame = SDL_GetTicks();
}


//timer tools
unsigned int TimerSet(const unsigned int time)
{
	return masterClock + time;
}

unsigned int TimerPassed(const unsigned int timer)
{
	return timer <= masterClock;
}


//fps tools
int fps = 0;
void UpdateFpsTimer(void){
	static int fpsCounter = 0;
	static int fpsTimer = 0;

	if (TimerPassed(fpsTimer))
	{
		fps = fpsCounter;
		fpsCounter = 0;
		fpsTimer = TimerSet(1000);
	}

	++fpsCounter;
}
