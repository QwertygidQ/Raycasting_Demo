#include "Raycasting.hpp"

int main()
{
	Raycasting demo;

	if(!demo.init())
		return -1;

	while(demo.isRunning())
	{
		demo.processInput();
		demo.update();
		demo.render();
	}

	demo.end();

	return 0;
}
