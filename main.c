#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define WIN_WIDTH 160
#define WIN_HEIGHT 160

#define MAX_POINTS 10

#define QUADRATIC 0
#define CUBIC 1
#define MAX_TYPE_ID CUBIC

typedef struct {
	int x, y;
} point_t;


typedef struct {
	int r, g, b;
} color_t;

SDL_Window* win;
SDL_Renderer* render;
SDL_Event event;

point_t newPoint_t(int x, int y) {
	point_t p;
	p.x = x;
	p.y = y;
	return p;
}

color_t newColor_t(int r, int g, int b) {
	color_t c;
	c.r = r;
	c.g = g;
	c.b = b;
	return c;
}

void setRenderColor(color_t c) {
	SDL_SetRenderDrawColor(render, c.r, c.g, c.b, 255);
}

point_t lerp(point_t point1, point_t point2, float t) { // where p1 < p2 and 0 < t < 1
	point_t newPoint;
	newPoint.x = point1.x + (point2.x - point1.x) * t;
	newPoint.y = point1.y + (point2.y - point1.y) * t;

	return newPoint;
}

int main() {
	point_t points[MAX_POINTS];
	point_t controlPoints[2] = {
		newPoint_t(WIN_WIDTH / 2, WIN_HEIGHT / 2),
		newPoint_t(0, 0)
	};
	int nPoints = 0, iControlPoint = 0, nControlPoints = 1, type = QUADRATIC;
	float precision = 0.02;
	color_t bgColor = newColor_t(0, 0, 0);
	color_t cpColor = newColor_t(0, 0x88, 0xFF);
	color_t ptColor = newColor_t(0xFF, 0x88, 0);

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Error: Failed to init SDL");
		return 1;
	}

	win = SDL_CreateWindow("Bezier Curves", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIN_WIDTH, WIN_HEIGHT, 0);
	render = SDL_CreateRenderer(win, -1, 0);

	setRenderColor(bgColor);
	SDL_RenderClear(render);
	setRenderColor(cpColor);
	SDL_RenderDrawPoint(render, controlPoints[0].x, controlPoints[0].y);
	SDL_RenderPresent(render);


	while(true) {
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				goto exit;
			} else if(event.type == SDL_KEYUP) {
				if(event.key.keysym.sym == SDLK_c) {
					clear:
					nPoints = 0;
					setRenderColor(bgColor);
					SDL_RenderClear(render);
					
					setRenderColor(cpColor);
					for(int i = 0; i < nControlPoints; i++) {
						SDL_RenderDrawPoint(render, controlPoints[i].x, controlPoints[i].y);
					}

					SDL_RenderPresent(render);
				} else if(event.key.keysym.sym == SDLK_SPACE && nPoints >= 2) {
					setRenderColor(ptColor);
					
					for(float t = 0; t < 1; t += precision) {
						for(int i = 0; i < ((nPoints == 2) ? 1 : nPoints); i++) {
							point_t p2, p;
							if(i + 1 >= nPoints) p2 = points[0];
							else p2 = points[i + 1];

							if(type == QUADRATIC)
								p = lerp(lerp(points[i], controlPoints[0], t), lerp(controlPoints[0], p2, t), t);
							else if(type == CUBIC) {
								point_t cpl = lerp(controlPoints[0], controlPoints[1], t); // control point lerp
								p = lerp(lerp(lerp(points[i], controlPoints[0], t), cpl, t), lerp(cpl, lerp(controlPoints[1], p2, t), t), t);
							}

							SDL_RenderDrawPoint(render, p.x, p.y);
						}
					}

					SDL_RenderPresent(render);
					
				} else {
					if(event.key.keysym.sym == SDLK_LEFT) {
						type--;
						if(type < 0) type = MAX_TYPE_ID;
					} else if(event.key.keysym.sym == SDLK_RIGHT) {
						type++;
						if(type > MAX_TYPE_ID) type = 0;
					} else continue;

					if(type == QUADRATIC) {
						nControlPoints = 1;
						controlPoints[0] = newPoint_t(WIN_WIDTH / 2, WIN_HEIGHT / 2);
					}
					else if(type == CUBIC) {
						nControlPoints = 2;
						controlPoints[0] = newPoint_t(WIN_WIDTH / 3, WIN_HEIGHT / 2);
						controlPoints[1] = newPoint_t(WIN_WIDTH / 3 * 2, WIN_HEIGHT / 2);
					}

					goto clear;
				}		
			} else if(event.type == SDL_MOUSEBUTTONUP && nPoints < MAX_POINTS) {
				if(event.button.button == SDL_BUTTON_LEFT) {
					setRenderColor(ptColor);
					points[nPoints] = newPoint_t(event.button.x, event.button.y);
					nPoints++;
				} else if(event.button.button == SDL_BUTTON_RIGHT) {
					setRenderColor(bgColor);
					SDL_RenderDrawPoint(render, controlPoints[iControlPoint].x, controlPoints[iControlPoint].y);

					setRenderColor(cpColor);
					controlPoints[iControlPoint].x = event.button.x;
					controlPoints[iControlPoint].y = event.button.y;
					SDL_RenderDrawPoint(render, controlPoints[iControlPoint].x, controlPoints[iControlPoint].y);

					iControlPoint++;
					if(iControlPoint > nControlPoints - 1) iControlPoint = 0;
				}

				SDL_RenderDrawPoint(render, event.button.x, event.button.y);
				SDL_RenderPresent(render);

			}
		}

		
	}

	exit:
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}
