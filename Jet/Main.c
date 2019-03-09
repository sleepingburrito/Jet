#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "SDLtimingTools.h"
#include "GraphicsSettings.h"
#include "SDLStartWindow.h"
#include "Soft3d.h"


int main(int argc, char* argv[]){
	//perfrmace satisitcs timer
	unsigned int updateDisplayPref = TimerSet(1000);
	StartWindow();

	//setup 3d
	AllocateScreen();
	ResetCameraStart();
	AllocateVertexCache();

	//loading map
	texture mapTexture = LoadTexture("map.data", 2048, 2048);
	obj map = LoadObj("mapc.obj");

	//loading collision
	obj mapCollision = LoadObj("mapc.obj");

	//test wep
	obj sil = LoadObj("s.obj");


	//movment setup
	const float walkspeedRate = 4.0f;
	const float floatSpeedRate = 3.0f;
	const float turnSpeedRate = 0.001f;
	const float lookUpDownRate = 0.0015f;
	const float lookUpDownMax = 0.523599f;
	const float cameraSize = 1000.0f / 2.0f;
	const float cameraHeight = 1000.0f;

	float walkSpeed = 0.0f;
	float strafingSpeed = 0.0f;
	float leftRightLook = 0.0f;
	float upDownLook = 0.0f;
	
	float hspeed = 0.0f;
	float vspeed = 0.0f;
	float zspeed = 0.0f;

	float left = 0.0f;
	float right = 0.0f;
	float top = 0.0f;
	float bottom = 0.0f;

	while (true){
		//io event
		SDL_Event event;
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) return EXIT_SUCCESS;
		
		/////////////////
		//moving camera//
		/////////////////
		//turning
		if (keyState[SDL_SCANCODE_LEFT]){
			leftRightLook += turnSpeedRate * deltaTime;
		}else if (keyState[SDL_SCANCODE_RIGHT]) {
			leftRightLook -= turnSpeedRate * deltaTime;
		}
		while (leftRightLook > PI_TWO || leftRightLook < 0) { //keep in range
			leftRightLook -= leftRightLook > (PI_TWO) ? PI_TWO : -PI_TWO;
		}
		const float leftRightOffset = leftRightLook + PI_HALF;
		camera.rotation.z = leftRightOffset;

		//walking
		walkSpeed = 0;
		if (keyState[SDL_SCANCODE_W]) {
			walkSpeed = walkspeedRate;
		}
		if (keyState[SDL_SCANCODE_S]) {
			walkSpeed = -walkspeedRate;
		}
		

		//strafing
		strafingSpeed = 0;
		if (keyState[SDL_SCANCODE_A]) {
			strafingSpeed = walkspeedRate;
		}
		if (keyState[SDL_SCANCODE_D]) {
			strafingSpeed = -walkspeedRate;
		}


		//moving and cd
		float hspeed = 0;
		float vspeed = 0;
		if (strafingSpeed != 0 && walkSpeed != 0){
			strafingSpeed *= SINCOS45;
			walkSpeed *= SINCOS45;
		}
		if (strafingSpeed != 0) {
			strafingSpeed *= deltaTime;
			const float leftRightTmp = leftRightLook + PI_HALF;
			hspeed += strafingSpeed * cosf(leftRightTmp);
			vspeed += strafingSpeed * sinf(leftRightTmp);
		}
		if (walkSpeed != 0){
			walkSpeed *= deltaTime;
			hspeed += walkSpeed * cosf(leftRightLook);
			vspeed += walkSpeed * sinf(leftRightLook);
		}
		if (hspeed != 0 || vspeed != 0){
			camera.position.x += hspeed;
			camera.position.z += vspeed;
		}


		//floating
		if (keyState[SDL_SCANCODE_E]) {
			camera.position.y += floatSpeedRate * deltaTime;
		}
		if (keyState[SDL_SCANCODE_Q]) {
			camera.position.y -= floatSpeedRate * deltaTime;
		}
		
		//look up and down
		if (keyState[SDL_SCANCODE_DOWN]) {
			upDownLook += lookUpDownRate * deltaTime;
			if (upDownLook >= lookUpDownMax)
				upDownLook = lookUpDownMax;
		}
		if (keyState[SDL_SCANCODE_UP]) {
			upDownLook -= lookUpDownRate * deltaTime;
			if (upDownLook <= -lookUpDownMax)
				upDownLook = -lookUpDownMax;
		}
		camera.rotation.x = upDownLook * sinf(leftRightOffset);
		camera.rotation.y = upDownLook * cosf(leftRightOffset);

		//moving physics
		left = -camera.position.x - cameraSize;
		right = -camera.position.x + cameraSize;
		top = -camera.position.z - cameraSize;
		bottom = -camera.position.z + cameraSize;

		//map collision
		/*
		camera.position.y = 0;
		float topY = 0;
		for (size_t i = 0; i < mapCollision.verticesCountMax; ++i) {
			if (mapCollision.vertices[i].x > left
				&& mapCollision.vertices[i].x < right
				&& mapCollision.vertices[i].z > top
				&& mapCollision.vertices[i].z < bottom
				&& mapCollision.vertices[i].y < topY) {
				topY = mapCollision.vertices[i].y;
			}
		}
		camera.position.y += 1000 + -topY;
		*/
		


		////////
		//draw//
		////////
		ClearScreenBuffer();
		SetCameraState();
		START_SOFTWARE_RENDER;
		DrawMesh(&map, NULL, &mapTexture);
		DrawMesh(&sil, NULL, &mapTexture);
		END_SOFTWARE_RENDER;
		SoftwareRender();
		
		///////////////
		//fps counter//
		///////////////
		UpdateFpsTimer();
		UpdateDeltaTime(false);
		if (TimerPassed(updateDisplayPref)){
			printf("deltaTime: %f fps: %i\n", deltaTime, fps);
			updateDisplayPref = TimerSet(1000);
		}
	}

	SDL_Quit();
	return EXIT_SUCCESS;
}