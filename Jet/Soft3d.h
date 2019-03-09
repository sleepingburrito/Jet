#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include "GraphicsSettings.h"


//math
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#define PI 3.1415926f
#define PI_HALF (PI / 2.0f)
#define PI_FORTH (PI_HALF / 2.0f)
#define PI_TWO (PI * 2.0f)
#define SINCOS45 (0.707106781186f)
#define fovToPerspective(fov, screenWidthHalf)  ((screenWidthHalf)/tanf( degreesToRadians(fov / 2.0f)))
#define degreesToRadians(angleDegrees) ( (angleDegrees) * (float)PI / 180.0f )

#define DRAW_DISTANCE 30000

//meshes
typedef struct {
	float x, y, z;
}vector3;

typedef struct {
	float x, y;
}vector2;

typedef struct {
	size_t vertexIndex[3];
	size_t uvIndex[3];
}face;

typedef struct {
	vector3 * vertices;
	size_t verticesCountMax;

	vector2 * uvs;
	size_t uvsCountMax;

	face * faces;
	size_t facesCountMax;
}obj;

obj LoadObj(const char * const fileName) {
	//start//
	obj objBack;

	//load file into ram//
	size_t fileSize = 0;
	FILE *f = fopen(fileName, "rb");
	if (f == NULL)
	{
		puts("Could not open obj file");
		abort();
	}

	fseek(f, 0, SEEK_END);
	fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char * file = (char *)calloc(fileSize + 1, 1);

	if (file == NULL)
	{
		puts("Could get ram for file obj");
		abort();
	}

	if (fileSize != fread(file, sizeof(char), fileSize, f))
	{
		puts("failed to read file obj");
		abort();
	}
	fclose(f);

	++fileSize; //to acknowledge the ending null

	//parse file//
	//first pass
	objBack.uvsCountMax = 0;
	objBack.verticesCountMax = 0;
	objBack.facesCountMax = 0;

	for (size_t i = 0; i < fileSize - 1; i++)
	{
		//counts
		if (file[i] == 'v')
		{
			if (file[i + 1] == 't')
				objBack.uvsCountMax++;
			else if (file[i + 1] == ' ')
				objBack.verticesCountMax++;
		}
		else if (file[i] == 'f' && file[i + 1] == ' ')
			objBack.facesCountMax++;

		if (file[i] == 13 || file[i] == 10) //turn new lines into null term strings
			file[i] = 0; //null term 
	}

	//get mem
	objBack.vertices = calloc(objBack.verticesCountMax, sizeof(vector3));
	objBack.uvs = calloc(objBack.uvsCountMax, sizeof(vector2));
	objBack.faces = calloc(objBack.facesCountMax, sizeof(face));

	if (objBack.vertices == NULL || objBack.uvs == NULL || objBack.faces == NULL)
	{
		puts("calloc fail for making ram obj");
		abort();
	}

	//real parse time
	size_t tmpIndexV = 0;
	size_t tmpIndexUv = 0;
	size_t tmpFaceIndex = 0;
	for (size_t i = 0; i < fileSize; i++)
	{
		while (i < fileSize && file[i] != 'v' && file[i] != 'f') i++;

		if (file[i] == 'v')
		{
			if (file[i + 1] == 't') //uv
			{
				sscanf(&file[i], "vt %f %f", &objBack.uvs[tmpIndexUv].x, &objBack.uvs[tmpIndexUv].y);
				tmpIndexUv++;
			}
			else if (file[i + 1] == ' ') //vert
			{
				sscanf(&file[i], "v %f %f %f", &objBack.vertices[tmpIndexV].x, &objBack.vertices[tmpIndexV].y, &objBack.vertices[tmpIndexV].z);
				tmpIndexV++;
			}
		}
		else if (file[i] == 'f' && file[i + 1] == ' ') //face
		{
			sscanf(&file[i], "f %zu/%zu %zu/%zu %zu/%zu",
				&objBack.faces[tmpFaceIndex].vertexIndex[0], &objBack.faces[tmpFaceIndex].uvIndex[0],
				&objBack.faces[tmpFaceIndex].vertexIndex[1], &objBack.faces[tmpFaceIndex].uvIndex[1],
				&objBack.faces[tmpFaceIndex].vertexIndex[2], &objBack.faces[tmpFaceIndex].uvIndex[2]);

			//objs count from 1 but I need a zero start
			for (size_t i2 = 0; i2 < 3; i2++)
			{
				objBack.faces[tmpFaceIndex].vertexIndex[i2]--;
				objBack.faces[tmpFaceIndex].uvIndex[i2]--;
			}

			tmpFaceIndex++;
		}
	}


	//exit//
	free(file);
	return objBack;
}


//draw 2d//
typedef struct {
	unsigned char r, g, b;
}pixel;

typedef struct {
	pixel * textPixel;
	size_t width, height, size;
}texture;

#define ScreenWidth SCREEN_WIDTH_START
#define camCenterX (ScreenWidth / 2)
#define ScreenHeight SCREEN_HEIGHT_START
#define camCenterY (ScreenHeight / 2)
#define ScreenSize (ScreenWidth * ScreenHeight)

float * zbuffer = NULL;
void AllocateScreen(void){
	if (NULL == (zbuffer = calloc(ScreenSize, sizeof(float))))
	{
		puts("calloc error making zbuffer");
		abort();
	}
}

void ClearScreenBuffer(void) {
	for (size_t i = 0; i < ScreenSize; i++) {
		zbuffer[i] = FLT_MAX;
		((Uint32 *)softwareRenderScreen->pixels)[i] = 0;
	}
}

texture LoadTexture(const char * const fileName, const size_t width, const size_t height)
{
	texture reTex;
	reTex.width = width;
	reTex.height = height;
	reTex.size = width * height;

	FILE *f = fopen(fileName, "rb");
	if (f == NULL)
	{
		puts("Could not open texture file");
		abort();
	}

	if ((reTex.textPixel = calloc(reTex.size, sizeof(pixel))) == NULL)
	{
		puts("failed to calloc a texture");
		abort();
	}

	if (reTex.size != fread(reTex.textPixel, sizeof(pixel), reTex.size, f))
	{
		puts("failed to read file texture");
		abort();
	}
	fclose(f);

	return reTex;
}

//drawing 3d//
typedef struct {
	vector3 position;
	vector3 rotation;
}location;

//cam
location camera;
float camCosX, camSinX;
float camCosY, camSinY;
float camCosZ, camSinZ;


vector3 * vertexCache = NULL;
#define vertexCacheCountMax 2000000
void AllocateVertexCache(void) {
	if (NULL == (vertexCache = calloc(vertexCacheCountMax, sizeof(vector3))))
	{
		puts("calloc error making vertex cache");
		abort();
	}
}

void SetCameraState(void) {
	camCosX = cosf(camera.rotation.x);
	camSinX = sinf(camera.rotation.x);

	camCosY = cosf(camera.rotation.y);
	camSinY = sinf(camera.rotation.y);

	camCosZ = cosf(camera.rotation.z);
	camSinZ = sinf(camera.rotation.z);
}

void ResetCameraStart(void)
{
	memset(&camera, 0, sizeof(location));
	SetCameraState();
}

#define FOV 90
void DrawMesh(const obj * const mesh, const location * const loc, const texture * const image) {
	if (mesh->verticesCountMax > vertexCacheCountMax) {
		puts("verticesCountMax > vertexCacheCountMax");
		abort();
	}

	//local space pre
	float tmpCosX, tmpSinX, tmpCosY, tmpSinY, tmpCosZ, tmpSinZ;

	if (loc != NULL) {
		tmpCosX = cosf(loc->rotation.x);
		tmpSinX = sinf(loc->rotation.x);

		tmpCosY = cosf(loc->rotation.y);
		tmpSinY = sinf(loc->rotation.y);

		tmpCosZ = cosf(loc->rotation.z);
		tmpSinZ = sinf(loc->rotation.z);
	}

	//move vertex
	for (size_t i = 0; i < mesh->verticesCountMax; i++) {
		float tx = mesh->vertices[i].x;
		float ty = mesh->vertices[i].y;
		float tz = mesh->vertices[i].z;
		float tx2, ty2, tz2;

		//local space
		if (loc != NULL) {
			tx += loc->position.x;
			ty += loc->position.y;
			tz += loc->position.z;

			tx2 = tx * tmpCosX - ty * tmpSinX;
			ty = tx * tmpSinX + ty * tmpCosX;
			tx = tx2;

			ty2 = ty * tmpCosY - tz * tmpSinY;
			tz = ty * tmpSinY + tz * tmpCosY;
			ty = ty2;

			tz2 = tz * tmpCosZ - tx * tmpSinZ;
			tx = tz * tmpSinZ + tx * tmpCosZ;
			tz = tz2;
		}

		//world space
		tx += camera.position.x;
		ty += camera.position.y;
		tz += camera.position.z;

		tx2 = tx * camCosX - ty * camSinX;
		ty = tx * camSinX + ty * camCosX;
		tx = tx2;

		ty2 = ty * camCosY - tz * camSinY;
		tz = ty * camSinY + tz * camCosY;
		ty = ty2;

		tz2 = tz * camCosZ - tx * camSinZ;
		tx = tz * camSinZ + tx * camCosZ;
		tz = tz2;

		//perspective correcting
		if (tz >= 0) { //treat geometry behind the camera as orthographic to minimize distortion
			const float percentCloseToCamera = tz / (tz + fovToPerspective(FOV, ScreenWidth));

			tx += (camCenterX - tx) * percentCloseToCamera;
			ty += (camCenterY - ty) * percentCloseToCamera;
		}

		//done
		vertexCache[i].x = tx;
		vertexCache[i].y = ty;
		vertexCache[i].z = tz;
	}

	//draw faces
	for (size_t i = 0; i < mesh->facesCountMax; i++) {
		//bounds checking
		const float z1 = vertexCache[mesh->faces[i].vertexIndex[0]].z;
		const float z2 = vertexCache[mesh->faces[i].vertexIndex[1]].z;
		const float z3 = vertexCache[mesh->faces[i].vertexIndex[2]].z;

		if (z1 < 0 && z2 < 0 && z3 < 0)
			continue;
		if (z1 > DRAW_DISTANCE && z2 > DRAW_DISTANCE && z3 > DRAW_DISTANCE)
			continue;

		const int x1 = (int)vertexCache[mesh->faces[i].vertexIndex[0]].x;
		const int x2 = (int)vertexCache[mesh->faces[i].vertexIndex[1]].x;
		const int x3 = (int)vertexCache[mesh->faces[i].vertexIndex[2]].x;

		int left = min(x1, min(x2, x3));
		if (left >= ScreenWidth) continue;
		int right = max(x1, max(x2, x3));
		if (right < 0) continue;

		const int y1 = (int)vertexCache[mesh->faces[i].vertexIndex[0]].y;
		const int y2 = (int)vertexCache[mesh->faces[i].vertexIndex[1]].y;
		const int y3 = (int)vertexCache[mesh->faces[i].vertexIndex[2]].y;

		int top = min(y1, min(y2, y3));
		if (top >= ScreenHeight) continue;
		int bottom = max(y1, max(y2, y3));
		if (bottom < 0) continue;

		const int y2my3 = y2 - y3;
		const int y3my1 = y3 - y1;
		const int y1my2 = y1 - y2;

		//backface culling / pre comput area
		int totalArea = (x1*(y2my3)+x2 * (y3my1)+x3 * (y1my2));
		if (totalArea > -1) continue;

		totalArea = -totalArea;
		const float totalAreaDbl = (float)totalArea;

		//texture
		const float textW = (float)image->width;
		const float textH = (float)image->height;

		const float uvX1 = mesh->uvs[mesh->faces[i].uvIndex[0]].x * textW;
		const float uvX2 = mesh->uvs[mesh->faces[i].uvIndex[1]].x * textW;
		const float uvX3 = mesh->uvs[mesh->faces[i].uvIndex[2]].x * textW;

		const float uvY1 = mesh->uvs[mesh->faces[i].uvIndex[0]].y * textH;
		const float uvY2 = mesh->uvs[mesh->faces[i].uvIndex[1]].y * textH;
		const float uvY3 = mesh->uvs[mesh->faces[i].uvIndex[2]].y * textH;

		//more bounds keeping
		left = max(left, 0);
		right = min(right, ScreenWidth - 1);
		top = max(top, 0);
		bottom = min(bottom, ScreenHeight - 1);

		//render triangle 
		for (int y = top; y < bottom; ++y) {
			//precompute
			const int apre = x2 * (y3 - y) + x3 * (y - y2);
			const int bpre = x1 * (y - y3) + x3 * (y1 - y);
			const int cpre = x1 * (y2 - y) + x2 * (y - y1);

			const int indexPre = y * ScreenWidth;
			bool inTriangle = false;

			for (int x = left; x < right; ++x) {
				//finding triangle cordiant space
				const int a = abs(x*y2my3 + apre);
				const int b = abs(x*y3my1 + bpre);

				if (a + b + abs(x*y1my2 + cpre) == totalArea) {
					//interpolation (last part of finding cords)
					const float ba = (float)a / totalAreaDbl;
					const float bb = (float)b / totalAreaDbl;
					const float bc = 1.0f - (ba + bb);
					
					//right side optmization scanline
					inTriangle = true;

					//depth testing
					const float depth = z1 * ba + z2 * bb + z3 * bc;
					if (depth > 0) {
						//indexing
						const int index = indexPre + x;
						float * const pixelDepth = &zbuffer[index];

						if (depth < *pixelDepth) {
							//finding texture cords
							const pixel tmpPxSour = image->textPixel[
								((int)(uvY1 * ba + uvY2 * bb + uvY3 * bc) * image->width)
							   + (int)(uvX1 * ba + uvX2 * bb + uvX3 * bc)];
							   
							//filling in the pixel
							*pixelDepth = depth;
							((Uint32 *)softwareRenderScreen->pixels)[index] = (tmpPxSour.r << 16) | (tmpPxSour.g << 8) | tmpPxSour.b;
						}
					}
				} else if (inTriangle) break;
			}
		}

	}
}