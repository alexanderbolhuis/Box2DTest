//http://www.youtube.com/user/thecplusplusguy
//The Box2D main program with SDL
#include <iostream>
#include <SDL2/SDL.h>
#include <Box2D/Box2D.h>

const int WIDTH=640;
const int HEIGHT=480;
const float M2P=20;
const float P2M=1/M2P;
b2World* world;
SDL_Surface* screen;
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* sdlTexture;

// Teken pixel op surface
void putPixel(SDL_Surface* dest,int x,int y,int r,int g,int b)
{
	if(x>=0 && x<dest->w && y>=0 && y<dest->h)
		((Uint32*)dest->pixels)[y*dest->pitch/4+x]=SDL_MapRGB(dest->format,r,g,b);
}

// Swap functie
void swapValue(int& a,int& b)
{
	int tmp=a;
	a=b;
	b=tmp;
}

// Teken lijn op surface
void drawLine(SDL_Surface* dest,int x0,int y0,int x1,int y1)
{
	int tmp;
	bool step;

	step=abs(y1-y0)>abs(x1-x0);
	if(step)
	{
		swapValue(x0,y0);
		swapValue(x1,y1);
	}

	if(x0>x1)
	{
		swapValue(x1,x0);
		swapValue(y1,y0);
	}
	float error=0.0;
	float y=y0;
	float roundError=(float)abs(y1-y0)/(x1-x0);
	int ystep=(y1>y0 ? 1 : -1);
	for(int i=x0;i<x1;i++)
	{
		if(step)
			putPixel(dest,y,i,255,255,255);
		else
			putPixel(dest,i,y,255,255,255);
		error+=roundError;
		if(error>=0.5)
		{
			y+=ystep;
			error-=1;
		}
	}
}

// Roteer rectangle
void rotateTranslate(b2Vec2& vector,const b2Vec2& center,float angle)
{
	b2Vec2 tmp;
	tmp.x=vector.x*cos(angle)-vector.y*sin(angle);
	tmp.y=vector.x*sin(angle)+vector.y*cos(angle);
	vector=tmp+center;
}

// Voeg nieuwe rectangle toe op muiscoordinaten
b2Body* addRect(int x,int y,int w,int h,bool dyn=true)
{
	b2BodyDef bodydef;
	bodydef.position.Set(x*P2M,y*P2M);
	if(dyn)
		bodydef.type=b2_dynamicBody;
	b2Body* body=world->CreateBody(&bodydef);

	b2PolygonShape shape;
	shape.SetAsBox(P2M*w/2,P2M*h/2);

	b2FixtureDef fixturedef;
	fixturedef.shape=&shape;
	fixturedef.density=1.0;
	body->CreateFixture(&fixturedef);

}

// Teken vierkant
void drawSquare(b2Vec2* points,b2Vec2 center,float angle)
{
	for(int i=0;i<4;i++)
		drawLine(screen,points[i].x*M2P,points[i].y*M2P,points[(i+1)>3 ? 0 : (i+1)].x*M2P,points[(i+1)>3 ? 0 : (i+1)].y*M2P);
}

// Init wereld met grond
void init()
{
	world=new b2World(b2Vec2(0.0,9.81));
	addRect(WIDTH/2,HEIGHT-50,WIDTH,30,false);
}

// Teken alle rectangles
void display()
{
	SDL_FillRect(screen,NULL,0);
	b2Body* tmp=world->GetBodyList();
	b2Vec2 points[4];
	while(tmp)
	{
		for(int i=0;i<4;i++)
		{
			points[i]=((b2PolygonShape*)tmp->GetFixtureList()->GetShape())->GetVertex(i);
			rotateTranslate(points[i],tmp->GetWorldCenter(),tmp->GetAngle());
		}
		drawSquare(points,tmp->GetWorldCenter(),tmp->GetAngle());
		tmp=tmp->GetNext();
	}
}

int main(int argc,char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow("Box2D Test",
	                          SDL_WINDOWPOS_UNDEFINED,
	                          SDL_WINDOWPOS_UNDEFINED,
	                          640, 480,
	                          0);

	renderer = SDL_CreateRenderer(window, -1, 0);

	screen = SDL_CreateRGBSurface(0, 640, 480, 32, 0x00FF0000, 0x0000FF00 ,0x000000FF, 0xFF000000);
	Uint32 start;
	SDL_Event event;
	bool running=true;
	init();
	while(running)
	{
		start=SDL_GetTicks();
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					running=false;
					break;
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							running=false;
							break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					addRect(event.button.x,event.button.y,20,20,true);
					break;

			}
		}
		display();
		world->Step(1.0/40.0,8,3);	//update
		sdlTexture = SDL_CreateTextureFromSurface(renderer, screen);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
		SDL_RenderPresent(renderer);
		if(1000.0/40>SDL_GetTicks()-start)
			SDL_Delay(1000.0/40-(SDL_GetTicks()-start));
	}
	SDL_Quit();
}
