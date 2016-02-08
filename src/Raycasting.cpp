#include "Raycasting.hpp"
#include <math.h>
#include <stdlib.h>
#include <string>

Raycasting::Raycasting()
{
	window = nullptr;
	renderer = nullptr;
	font = nullptr;

	oldTime = newTime = FPS = 0.0;

	moveSpeed = rotSpeed = 0.0;

	running = false;

	up = down = left = right = quit = false;

	posX = 22;
	posY = 12;
	dirX = -1;
	dirY = 0;
	planeX = 0;
	planeY = 0.66;
}

Raycasting::~Raycasting()
{

}

bool Raycasting::init()
{
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) != 0)
		return false;

	if(TTF_Init() == -1)
		return false;

	window = SDL_CreateWindow("raycasting_demo", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, windowW, windowH, SDL_WINDOW_SHOWN);
	if(window == nullptr)
		return false;

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(renderer == nullptr)
		return false;

	font = TTF_OpenFont(fontPath, fontSize);
	if(font == nullptr)
		return false;

	running = true;

	return true;
}

bool Raycasting::isRunning()
{
	return running;
}

void Raycasting::processInput()
{
	up = down = left = right = false;

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	SDL_PumpEvents();
	up = keys[SDL_SCANCODE_W];
	down = keys[SDL_SCANCODE_S];
	left = keys[SDL_SCANCODE_A];
	right = keys[SDL_SCANCODE_D];
	quit = keys[SDL_SCANCODE_ESCAPE];
}

void Raycasting::update()
{
	moveSpeed = baseMoveSpeed/FPS;
	rotSpeed = baseRotSpeed/FPS;

	if(quit)
	{
		running = false;
		return;
	}

	if(up)
	{
		if(worldMap[int(posX+dirX*moveSpeed)][int(posY)] == false) posX += dirX*moveSpeed;
		if(worldMap[int(posX)][int(posY+dirY*moveSpeed)] == false) posY += dirY*moveSpeed;
	}
	if(down)
	{
		if(worldMap[int(posX-dirX*moveSpeed)][int(posY)] == false) posX -= dirX*moveSpeed;
		if(worldMap[int(posX)][int(posY - dirY * moveSpeed)] == false) posY -= dirY*moveSpeed;
	}
	if(right)
	{
		double oldDirX = dirX;

		dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
		dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);

		double oldPlaneX = planeX;

		planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
		planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
	}
	if(left)
	{
		double oldDirX = dirX;

		dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
		dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);

		double oldPlaneX = planeX;

		planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
		planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
	}

	for(int x = 0; x < windowW; x++)
	{
		double cameraX = 2*x/double(windowW) -1;
		double rayPosX = posX, rayPosY = posY;
		double rayDirX = dirX+planeX*cameraX, rayDirY = dirY+planeY*cameraX;

		int mapX = int(rayPosX), mapY = int(rayPosY);
		double sideDistX, sideDistY;
		double deltaDistX = sqrt(1+pow(rayDirY, 2)/pow(rayDirX, 2)), deltaDistY = sqrt(1+pow(rayDirX, 2)/pow(rayDirY, 2));

		double perpWallDist;

		int stepX, stepY;
		bool hit = false;
		int side;

		if(rayDirX < 0)
		{
			stepX = -1;
			sideDistX = (rayPosX - mapX) * deltaDistX;
		}
		else
		{
			stepX = 1;
			sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
		}

		if(rayDirY < 0)
		{
			stepY = -1;
			sideDistY = (rayPosY - mapY) * deltaDistY;
		}
		else
		{
			stepY = 1;
			sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
		}

		while(hit == false)
		{
			if(sideDistX < sideDistY)
			{
				mapX += stepX;
				sideDistX += deltaDistX;
				side = 0;
			}
			else
			{
				mapY += stepY;
				sideDistY += deltaDistY;
				side = 1;
			}
			if(worldMap[mapX][mapY] > 0)
				hit = true;
		}

		if(side == 0)
			perpWallDist = fabs((mapX - rayPosX + (1 - stepX)/2)/rayDirX);
		else
			perpWallDist = fabs((mapY - rayPosY + (1 - stepY)/2)/rayDirY);

		int height = abs(int(windowH/perpWallDist));
		int start = (windowH-height)/2;
		if(start < 0)
			start = 0;
		int end = start+height;
		if(end >= windowH)
			end = windowH-1;

		linesPoints[x][0] = start;
		linesPoints[x][1] = end;

		uint8_t color = 0xFF;
		if(side == 1)
			color /= 2;

		switch(worldMap[mapX][mapY])
		{
		case 1: //red
			linesColors[x][0] = color;
			linesColors[x][1] = 0;
			linesColors[x][2] = 0;
			break;
		case 2: //blue
			linesColors[x][0] = 0;
			linesColors[x][1] = color;
			linesColors[x][2] = 0;
			break;
		case 3: //green
			linesColors[x][0] = 0;
			linesColors[x][1] = 0;
			linesColors[x][2] = color;
			break;
		case 4: //white
			linesColors[x][0] = color;
			linesColors[x][1] = color;
			linesColors[x][2] = color;
			break;
		}
	}

	oldTime = newTime;
	newTime = SDL_GetTicks();
	FPS = 1000/(newTime - oldTime);
}

void Raycasting::render()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
	SDL_RenderClear(renderer);
	for(int x = 0; x < windowW; x++)
	{
		SDL_SetRenderDrawColor(renderer, linesColors[x][0], linesColors[x][1], linesColors[x][2], 0xFF);
		SDL_RenderDrawLine(renderer, x, linesPoints[x][0], x, linesPoints[x][1]);
	}

 	SDL_Color textColor = {0xFF, 0xFF, 0xFF}; //white

 	std::string temp = std::to_string(FPS);
 	SDL_Surface* textSurface = TTF_RenderText_Solid(font, temp.c_str(), textColor);
 	SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
 	const int w = textSurface -> w, h = textSurface -> h;
 	SDL_FreeSurface(textSurface);
 	textSurface = nullptr;

 	SDL_Rect src, dest;

	src.x = src.y = dest.x = dest.y = 0;
	src.w = dest.w = w;
	src.h = dest.h = h;

	SDL_RenderCopyEx(renderer, textTexture, &src, &dest, 0, 0, SDL_FLIP_NONE);

	SDL_DestroyTexture(textTexture);
	textTexture = nullptr;

	SDL_RenderPresent(renderer);
}

void Raycasting::end()
{
	TTF_CloseFont(font);
	font = nullptr;

	SDL_DestroyRenderer(renderer);
	renderer = nullptr;

	SDL_DestroyWindow(window);
	window = nullptr;

	SDL_Quit();
}
