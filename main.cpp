/*****************************************************************************
7/1/12 by Brett "Evil_Greven" Jones
Requires GDICanvas.h/cpp
No other libs are required
*****************************************************************************/
#define WIN32_LEAN_AND_MEAN
//////////////////////////////////////////////////////////////////////////////
//INCLUDES
//need this for windows stuff
//////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <mmsystem.h>
//need this for srand/rand
#include <stdlib.h>
//now our bitmapobject class
#include "bitmapobject.h"
//include timer thingy
#include "counter.h"
//////////////////////////////////////////////////////////////////////////////
//DEFINES
//we'll give our window a name
//////////////////////////////////////////////////////////////////////////////
#define WINDOWCLASS "SAGAMEDEV2012"

//now for a title
#define WINDOWTITLE "Bouncery - Team Procrastinator"

//now we need a standard ship size.. how about 45x45?
#define SHIPSIZE 45
//and projectile size.. hmm how about 10x5?
#define BULLETHEIGHT 10
#define BULLETWIDTH 9

//max number of enemies?  hmm how about 20
#define MAXENEMY 20

//max number of bullets? 100's a good one.
#define MAXBULLET 100

//delay before player is allowed to fire? in 1/33sec
#define FIREDELAY 600
//delay before enemy is allowed to fire in 1/33sec
#define ENEMYFIREDELAY 20

//enemy spawn delay in 1/33sec
#define NEWENEMYDELAY 100

//define player agility speed
#define MAXSPEED 8

//define enemy speed
#define ENEMYSPEED 2

//define bullets' speed
#define BULLETSPEED 10

//let's make our window size 640x480
#define WINDOWWIDTH 640
#define WINDOWHEIGHT 480

//////////////////////////////////////////////////////////////////////////////
//MACROS
//we need this to determine if the key is up or down
//////////////////////////////////////////////////////////////////////////////
#define KEYDN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

//////////////////////////////////////////////////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
bool Prog_Init(void); //game data initializer
void Prog_Loop(void); //main game loop
void Prog_Done(void); //game cleanup
void NewLevel(void); //when player dies etc
void DrawMap(void);	  //draw the playfield
void Keyhandler(void);//function to handle specific input
void Animate(void);//function to handle player ship animation
void Move(int x, int y);//move player ship
void FireBullet(int type, int offx, int offy, int ship);//fire bullet
void MoveBullet(void);//move bullets
int CheckHit(int x, int y, int u, int v, int type, int i);//anyone hit?
void Detonate(int b); //throw a wrench in things!

//////////////////////////////////////////////////////////////////////////////
//GLOBALS
//////////////////////////////////////////////////////////////////////////////
HINSTANCE hInstMain=NULL;//main application handle
HWND hWndMain=NULL;      //handle to our main window

//the playfield
BitMapObject bmoMap;
//the player
BitMapObject bmoPlayer;
//the bullets
BitMapObject bmoBullet;
BitMapObject bmoBullet2;
//the explosions
BitMapObject bmoExplosion;
//block
BitMapObject bmoBlock;
BitMapObject bmoBlock1;
BitMapObject bmoBlock2;
BitMapObject bmoBlock3;
BitMapObject bmoBlock4;
//splash
BitMapObject bmoSplash0;
BitMapObject bmoSplash1;
BitMapObject bmoSplash40;
//used for clearing screen
RECT rcTemp;

//first, a coordinate structure.
struct coord
{
	int x;
	int y;
};

//we need this for our enemy structure.
struct enemy
{
	coord Pos; //where is it?
	int FireDelay; //time to next shot?
	BitMapObject bmoEnemy; //the enemys
	int Hp; //how much health does it have?
	int Type; //type of enemy
};

//now for the bullets!
struct bullet
{
	coord Pos;
	coord Off;
	Counter delayY;
	Counter delayX;
	Counter timer;
	int Type;
	int AniFrame;
	//int timer;
};

//our enemy array
enemy Enemy[MAXENEMY];
//our bullet array
bullet Bullets[MAXBULLET];

coord PlayerPos;//where is the player?
coord PlayerVPos;
coord Mouse;
int Map[33][33];
int Save[33][33];
int PlayerHp; //player's health
Counter FireDelay; //player's fire delay
Counter FireDelay2;
Counter DownDelay;
Counter UpDelay;
bool PAUSED; //game paused?
bool ALIVE;
Counter ref;
DWORD start_time; //used for timing
int SpawnTimer; //used for enemy spawn
int CurrentEnemyDelay; //respawn timer counts down per enemy destroyed
int AniFrame = 0;
int Anim=0;
int Speed = 0;
Counter SpeedTimer;
int Slow = BULLETSPEED;
bool DRAWING=false;
bool CHANGING=false;
int BLOCK=1;
int level=-1;
bool splash=false;

//////////////////////////////////////////////////////////////////////////////
//WINDOWPROC
//////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK TheWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	//which message did we get?
	switch(uMsg)
	{
	case WM_KEYDOWN:
		{
			//check for escape key
			if(wParam==VK_ESCAPE)
			{
				PostQuitMessage(0);
				return(0);
			}
			else if(wParam==VK_CONTROL)
			{
				if(!ALIVE)
				{
					NewLevel();
				}
				return(0);
			}
			else if(wParam==VK_PAUSE) //check for pause key
			{
				if(!PAUSED)
				{
					PAUSED=true;
				}
				else if(PAUSED && ALIVE)
				{
					PAUSED=false;
				}
				return(0);//handled message
			}
		}break;
	case WM_DESTROY://the window is being destroyed
		{

			//tell the application we are quitting
			PostQuitMessage(0);

			//handled message, so return 0
			return(0);

		}break;
	case WM_RBUTTONUP:
		{
			BLOCK++;
			if(BLOCK>3)
				BLOCK=1;
		}break;
	case WM_LBUTTONUP:
		{
			CHANGING=false;
		//	for(int i=0; i<33; i++)
		//		for(int j=0; j<33; j++)
		//			Save[i][j]=Map[i][j];
		}break;
	case WM_LBUTTONDOWN:
		{
			if(splash)
			{
				splash=false;
				break;
			}
			CHANGING=true;
			Mouse.x=(LOWORD(lParam)+10)/20;
			Mouse.y=(HIWORD(lParam)+7)/15;
			if(Mouse.x > -1 && Mouse.y > -1 && Map[Mouse.x][Mouse.y]==0)
				DRAWING=true;
			else
				DRAWING=false;
		}break;
	case WM_MOUSEMOVE:
		{
			Mouse.x=(LOWORD(lParam)+10)/20;
			Mouse.y=(HIWORD(lParam)+7)/15;
			if(Mouse.x > -1 && Mouse.y > -1 && CHANGING && Mouse.y<27)
			{
				if(DRAWING)
				{
					if(Map[Mouse.x][Mouse.y]<4)
						Map[Mouse.x][Mouse.y]=BLOCK;
				}
				else
				{
					if(Map[Mouse.x][Mouse.y]<4)
						Map[Mouse.x][Mouse.y]=0;
				}
			}
		}break;
	case WM_PAINT://the window needs repainting
		{
			//a variable needed for painting information
			PAINTSTRUCT ps;
			
			//start painting
			HDC hdc=BeginPaint(hwnd,&ps);

			//redraw the map
			BitBlt(hdc,0,0,WINDOWWIDTH,WINDOWHEIGHT,bmoMap,0,0,SRCCOPY);

			//end painting
			EndPaint(hwnd,&ps);
					
			//handled message, so return 0
			return(0);
		}break;
	}

	//pass along any other message to default message handler
	return(DefWindowProc(hwnd,uMsg,wParam,lParam));
}
//////////////////////////////////////////////////////////////////////////////
//WINMAIN
//////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd)
{
	//assign instance to global variable
	hInstMain=hInstance;

	//create window class
	WNDCLASSEX wcx;

	//set the size of the structure
	wcx.cbSize=sizeof(WNDCLASSEX);

	//class style
	wcx.style=CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

	//window procedure
	wcx.lpfnWndProc=TheWindowProc;

	//class extra
	wcx.cbClsExtra=0;

	//window extra
	wcx.cbWndExtra=0;

	//application handle
	wcx.hInstance=hInstMain;

	//icon
	wcx.hIcon=LoadIcon(NULL,IDI_APPLICATION);

	//cursor
	wcx.hCursor=LoadCursor(NULL,IDC_ARROW);

	//background color
	wcx.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);

	//menu
	wcx.lpszMenuName=NULL;

	//class name
	wcx.lpszClassName=WINDOWCLASS;

	//small icon
	wcx.hIconSm=NULL;

	//register the window class, return 0 if not successful
	if(!RegisterClassEx(&wcx)) return(0);

	//create main window
	hWndMain=CreateWindowEx(0,WINDOWCLASS,WINDOWTITLE, WS_BORDER | WS_SYSMENU | WS_CAPTION| WS_VISIBLE,0,0,WINDOWWIDTH,WINDOWHEIGHT,NULL,NULL,hInstMain,NULL);

	//error check
	if(!hWndMain) return(0);

	//if program initialization failed, then return with 0
	if(!Prog_Init()) return(0);

	//message structure
	MSG msg;
	sndPlaySound("THEME.WAV", SND_ASYNC | SND_LOOP);


	//message pump
	for(;;)	
	{
		//look for a message
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			//there is a message

			//check that we arent quitting
			if(msg.message==WM_QUIT) break;
			
			//translate message
			TranslateMessage(&msg);

			//dispatch message
			DispatchMessage(&msg);
		}

		//run main game loop
		Prog_Loop();
		
	}
	
	//clean up program data
	Prog_Done();

	//return the wparam from the WM_QUIT message
	return(msg.wParam);
}
//////////////////////////////////////////////////////////////////////////////
//INITIALIZATION
//////////////////////////////////////////////////////////////////////////////
bool Prog_Init(void)
{
	//set the client area size
	SetRect(&rcTemp,0,0,WINDOWWIDTH,WINDOWHEIGHT);
	AdjustWindowRect(&rcTemp,WS_BORDER | WS_SYSMENU | WS_CAPTION| WS_VISIBLE,FALSE);//adjust the window size based on desired client area
	SetWindowPos(hWndMain,NULL,0,0,rcTemp.right-rcTemp.left,rcTemp.bottom-rcTemp.top,SWP_NOMOVE);//set the window width and height

	//create map image
	HDC hdc=GetDC(hWndMain);
	bmoMap.Create(hdc,WINDOWWIDTH,WINDOWHEIGHT);
	FillRect(bmoMap,&rcTemp,(HBRUSH)GetStockObject(BLACK_BRUSH));
	ReleaseDC(hWndMain,hdc);

	bmoPlayer.Load(NULL,"ball.bmp");
	bmoBullet.Load(NULL,"smblueball.bmp");
	bmoBullet2.Load(NULL,"smgreenball.bmp");
	bmoExplosion.Load(NULL,"explosion.bmp");
	bmoBlock.Load(NULL,"block.bmp");
	bmoBlock1.Load(NULL,"blockblue.bmp");
	bmoBlock2.Load(NULL,"blockgreen.bmp");
	bmoBlock3.Load(NULL,"blockred.bmp");
	bmoBlock4.Load(NULL,"blockgray.bmp");
	bmoSplash0.Load(NULL,"splash0.bmp");
	bmoSplash1.Load(NULL,"splash1.bmp");
	bmoSplash40.Load(NULL,"splash40.bmp");
	srand(GetTickCount());


	ref.set(33);
	NewLevel();
	return(true);//return success
}
//////////////////////////////////////////////////////////////////////////////
//CLEANUP
//////////////////////////////////////////////////////////////////////////////
void Prog_Done(void)
{
	//////////////////////////
	//clean up code goes here
	//////////////////////////

	bmoPlayer.Destroy();
	bmoBullet.Destroy();
	for(int j=0; j<MAXENEMY; j++)
		Enemy[j].bmoEnemy.Destroy();
}

//////////////////////////////////////////////////////////////////////////////
//MAIN GAME LOOP
//////////////////////////////////////////////////////////////////////////////
void Prog_Loop(void)
{
	DrawMap();
	Keyhandler();
	MoveBullet();
	Animate();
	//shifting to timer system from java game
/*	if(ref.isDone())
	{
		if(!PAUSED)
		{
			/*if(SpawnTimer < 1)
				NewEnemy();
			else
				SpawnTimer--;

			EnemyFire();
		}
		ref.repeat();
	}*/
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void DrawMap()
{
	int calc,calc2;

	FillRect(bmoMap,&rcTemp,(HBRUSH)GetStockObject(BLACK_BRUSH));

	int i;
	for(i=0; i<33; i++)
		for(int j=0; j<33; j++)
			if(Map[i][j]==1)
				BitBlt(bmoMap,i*20-10,j*15-7,20,15,bmoBlock1,0,0,SRCCOPY);
			else if(Map[i][j]==2)
				BitBlt(bmoMap,i*20-10,j*15-7,20,15,bmoBlock2,0,0,SRCCOPY);
			else if(Map[i][j]==3)
				BitBlt(bmoMap,i*20-10,j*15-7,20,15,bmoBlock3,0,0,SRCCOPY);
			else if(Map[i][j]==4)
				BitBlt(bmoMap,i*20-10,j*15-7,20,15,bmoBlock4,0,0,SRCCOPY);
			else if(Map[i][j]==5)
				BitBlt(bmoMap,i*20-10,j*15-7,20,15,bmoBlock,0,0,SRCCOPY);

	for(i=0; i<MAXBULLET; i++)
	{
		if(Bullets[i].Type >= 100)
		{
			calc=Bullets[i].Pos.x-48/2;
			calc2=Bullets[i].Pos.y-48/2;
			BitBlt(bmoMap, calc, calc2, 48, 48, bmoExplosion, 48, Bullets[i].AniFrame*48, SRCAND);
			BitBlt(bmoMap, calc, calc2, 48, 48, bmoExplosion, 0, Bullets[i].AniFrame*48, SRCPAINT);
		}
		else if(Bullets[i].Type >= 10)
		{
			calc=Bullets[i].Pos.x-BULLETWIDTH/2;
			calc2=Bullets[i].Pos.y-BULLETHEIGHT/2;
			BitBlt(bmoMap, calc, calc2, BULLETWIDTH, BULLETHEIGHT, bmoBullet2, BULLETWIDTH, Bullets[i].AniFrame*9-1, SRCAND);
			BitBlt(bmoMap, calc, calc2, BULLETWIDTH, BULLETHEIGHT, bmoBullet2, 0, Bullets[i].AniFrame*9-1, SRCPAINT);
		}
		else if(Bullets[i].Type >= 0)
		{
			calc=Bullets[i].Pos.x-BULLETWIDTH/2;
			calc2=Bullets[i].Pos.y-BULLETHEIGHT/2;
			BitBlt(bmoMap, calc, calc2, BULLETWIDTH, BULLETHEIGHT, bmoBullet, BULLETWIDTH, Bullets[i].AniFrame*9-1, SRCAND);
			BitBlt(bmoMap, calc, calc2, BULLETWIDTH, BULLETHEIGHT, bmoBullet, 0, Bullets[i].AniFrame*9-1, SRCPAINT);
		}
	}

	for(int j=0; j<MAXENEMY; j++)
		if(Enemy[j].Type >= 0)
		{
			calc=Enemy[j].Pos.x-SHIPSIZE/2;
			calc2=Enemy[j].Pos.y-SHIPSIZE/2;
			BitBlt(bmoMap, calc, calc2, SHIPSIZE, SHIPSIZE, Enemy[j].bmoEnemy, SHIPSIZE, 0, SRCAND);
			BitBlt(bmoMap, calc, calc2, SHIPSIZE, SHIPSIZE, Enemy[j].bmoEnemy, 0, 0, SRCPAINT);
		}

	calc=PlayerPos.x-SHIPSIZE/2;
	calc2=PlayerPos.y-SHIPSIZE/2;

	BitBlt(bmoMap, calc, calc2, SHIPSIZE, SHIPSIZE, bmoPlayer, SHIPSIZE, 45*AniFrame, SRCAND);
	BitBlt(bmoMap, calc, calc2, SHIPSIZE, SHIPSIZE, bmoPlayer, 0, 45*AniFrame, SRCPAINT);

	if(splash)
	{
		if(level==0)
			BitBlt(bmoMap, WINDOWWIDTH/4, WINDOWHEIGHT/4, 320, 240, bmoSplash0, 0, 0, SRCCOPY);
		else if(level==1)
			BitBlt(bmoMap, WINDOWWIDTH/4, WINDOWHEIGHT/4, 320, 240, bmoSplash1, 0, 0, SRCCOPY);
		else if(level==40)
			BitBlt(bmoMap, WINDOWWIDTH/4, WINDOWHEIGHT/4, 320, 240, bmoSplash40, 0, 0, SRCCOPY);
	}
	//force window to redraw
	InvalidateRect(hWndMain, NULL, FALSE);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void NewLevel(void)
{
	start_time=GetTickCount();
	PAUSED=false;
	ALIVE=true;
	PlayerPos.x=WINDOWWIDTH/2;
	PlayerVPos.x=PlayerPos.x*100;
	PlayerPos.y=WINDOWHEIGHT-SHIPSIZE;
	PlayerVPos.y=PlayerPos.y*100;
	PlayerHp=100;
	SpawnTimer=NEWENEMYDELAY;
	CurrentEnemyDelay=NEWENEMYDELAY;
	Speed=0;
	Slow=BULLETSPEED;
	Anim=0;
	AniFrame=0;
	int i,j;
	for(i=0; i<33; i++)
		for(j=0; j<33; j++)
			Map[i][j]=0;
	for(j=0; j<MAXBULLET; j++)
		Bullets[j].Type=-1;
	level++;
	//level=35;
	if(level==41)
		level=0;
	if(level<2 || level>39)
		splash=true;
	if(level)
		Map[16][13]=4;
	if(level>1)
	{
		Map[17][13]=4;
		Map[15][13]=4;
	}
	if(level>2)
	{
		Map[19][10]=4;
		Map[13][10]=4;
	}
	if(level>3)
	{
		Map[19][11]=4;
		Map[13][11]=4;
	}
	if(level>4)
	{
		Map[19][9]=4;
		Map[13][9]=4;
	}
	if(level>5)
	{
		Map[19][13]=4;
		Map[13][13]=4;
	}
	if(level>6)
	{
		Map[18][16]=4;
		Map[14][16]=4;
	}
	if(level>7)
	{
		Map[19][16]=4;
		Map[13][16]=4;
	}
	if(level>8)
	{
		Map[17][16]=4;
		Map[15][16]=4;
	}
	if(level>9)
	{
		Map[20][13]=4;
		Map[12][13]=4;
	}
	if(level>10)
	{
		Map[21][13]=4;
		Map[11][13]=4;
	}
	if(level>11)
	{
		Map[21][16]=4;
		Map[11][16]=4;
	}
	if(level>12)
	{
		Map[22][15]=4;
		Map[10][15]=4;
	}
	if(level>13)
	{
		Map[23][12]=4;
		Map[9][12]=4;
	}
	if(level>14)
	{
		Map[24][13]=4;
		Map[8][13]=4;
	}
	if(level>15)
	{
		Map[25][14]=4;
		Map[7][14]=4;
	}
	if(level>16)
	{
		Map[27][14]=4;
		Map[5][14]=4;
	}
	if(level>17)
	{
		Map[28][14]=4;
		Map[4][14]=4;
	}
	if(level>18)
	{
		Map[28][5]=4;
		Map[4][5]=4;
	}
	if(level>19)
	{
		Map[27][6]=4;
		Map[5][6]=4;
	}
	if(level>20)
	{
		Map[27][6]=4;
		Map[5][6]=4;
	}
	if(level>21)
	{
		Map[26][7]=4;
		Map[6][7]=4;
	}
	if(level>22)
	{
		Map[20][6]=4;
		Map[12][6]=4;
	}
	if(level>23)
	{
		Map[19][5]=4;
		Map[13][5]=4;
	}
	if(level>24)
	{
		Map[17][19]=4;
		Map[15][19]=4;
	}
	if(level>25)
	{
		Map[18][19]=4;
		Map[14][19]=4;
	}
	if(level>26)
	{
		Map[19][19]=4;
		Map[13][19]=4;
	}
	if(level>27)
	{
		Map[21][19]=4;
		Map[11][19]=4;
	}
	if(level>28)
	{
		Map[22][19]=4;
		Map[10][19]=4;
	}
	if(level>29)
	{
		Map[23][19]=4;
		Map[9][19]=4;
	}
	if(level>30)
	{
		Map[25][19]=4;
		Map[7][19]=4;
	}
	if(level>31)
	{
		Map[26][19]=4;
		Map[6][19]=4;
	}
	if(level>32)
	{
		Map[27][19]=4;
		Map[5][19]=4;
	}
	if(level>33)
	{
		Map[29][19]=4;
		Map[30][19]=4;
		Map[31][19]=4;
		Map[3][19]=4;
		Map[2][19]=4;
		Map[1][19]=4;

	}
	if(level>34)
	{
		Map[29][10]=4;
		Map[30][10]=4;
		Map[31][10]=4;
		Map[3][10]=4;
		Map[2][10]=4;
		Map[1][10]=4;

	}
	if(level>35)
	{
		Map[25][10]=4;
		Map[24][10]=4;
		Map[23][10]=4;
		Map[9][10]=4;
		Map[8][10]=4;
		Map[7][10]=4;

	}
	if(level>36)
	{
		Map[21][10]=4;
		Map[20][10]=4;
		Map[12][10]=4;
		Map[11][10]=4;

	}
	if(level>37)
	{
		Map[25][22]=4;
		Map[24][23]=4;
		Map[23][24]=4;
		Map[9][24]=4;
		Map[8][23]=4;
		Map[7][22]=4;

	}
	if(level>38)
	{
		Map[20][24]=4;
		Map[19][23]=4;
		Map[18][22]=4;
		Map[14][22]=4;
		Map[13][23]=4;
		Map[12][24]=4;

	}
	if(level>39)
	{
		for(i=0; i<33; i++)
			for(j=0; j<33; j++)
				Map[i][j]=0;

	}
	Map[16][10]=5;
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void Keyhandler(void)
{
	int x,y;
	x=y=0;

	if(PAUSED)
		return;

	if(KEYDN(VK_CONTROL))
	{
		if(FireDelay.isDone())
		{
			FireBullet(0, 0, 0, -1);
			FireDelay.set(FIREDELAY);
		}
	}
	
	if(KEYDN(VK_SHIFT))
	{
		if(FireDelay2.isDone())
		{
			FireBullet(10, 0, 0, -1);
			FireDelay2.set(FIREDELAY);
		}
	}

	if(KEYDN(VK_DOWN) && DownDelay.isDone())
	{	y=1; DownDelay.set(FIREDELAY); }
	else if(KEYDN(VK_UP) && UpDelay.isDone())
	{	y=-1; UpDelay.set(FIREDELAY); }

	if(KEYDN(VK_LEFT))
	{	x=-1; }
	else if(KEYDN(VK_RIGHT))
	{	x=1;}
	
	if(x != 0 || y != 0)
		Move(x,y);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void Animate(void)
{
	PlayerPos.x=PlayerVPos.x/100;
	if(((PlayerPos.x > (SHIPSIZE/2) + Speed) || Speed > 0) && 
		((PlayerPos.x < (WINDOWWIDTH - Speed - (SHIPSIZE/2)) || Speed < 0)))
		PlayerVPos.x+=Speed;
	if(!ref.isDone())
		return;
	//animation direction
	//if(Speed<0) { AniFrame++; }
	//else if(Speed>0) { AniFrame--; }
	if(Anim*Anim/MAXSPEED)
	{
		if(Anim<0) AniFrame++;
		if(Anim>0) AniFrame--;
		Anim=0;
	}
	else { Anim+=Speed; }
	//animation loop
	if(AniFrame>7) { AniFrame=0; }
	else if(AniFrame<0) { AniFrame=7; }
	if(Speed>0)
		ref.set(33/Speed);
	else if(Speed<0)
		ref.set(33/Speed*-1);
	else
		ref.set(33);
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void Move(int x, int y)
{
	//horizontal movement
	if(x>0)
	{
		if(PlayerPos.x < (WINDOWWIDTH - Speed - (SHIPSIZE/2)))
		{
			//PlayerPos.x+=Speed;
			if(Speed<MAXSPEED && SpeedTimer.isDone()) {
				Speed++;
				SpeedTimer.set(abs(Speed*20));
			}
			//AniFrame--;
		}
		else { Speed = 0; }
	}
	else if(x<0)
	{
		if(PlayerPos.x > (SHIPSIZE/2) + Speed)
		{
			//PlayerPos.x-=Speed;
			if(Speed>(-1*MAXSPEED) && SpeedTimer.isDone()) {
				Speed--;
				SpeedTimer.set(abs(Speed*20));
			}

			//AniFrame++;
		}
		else { Speed = 0; }
	}
	//veritcal movement
	if(y>0)
	{
		if(Slow > 2) Slow--;
	}
	else if(y<0)
	{
		if(Slow < BULLETSPEED) Slow++;
	}
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void FireBullet(int type, int xoff, int yoff, int ship)
{
	for(int i=0; i<MAXBULLET; i++)
		if(Bullets[i].Type < 0)
		{
			if(ship < 0) //player bullet
			{
				Bullets[i].Type = type;
				Bullets[i].Pos.x=PlayerPos.x+xoff;
				Bullets[i].Pos.y=PlayerPos.y+yoff;
				Bullets[i].Off.x=Speed/2;
				Bullets[i].Off.y=-1*Slow/2;
				Bullets[i].AniFrame=0;
				if(Slow)
					Bullets[i].delayY.set(33/Slow);
				if(Speed)
					Bullets[i].delayX.set(33/abs(Speed));
				int m;
				if(type==10)
				{
		//		PlaySound("EFX1.WAV", NULL, SND_ASYNC | SND_FILENAME);
					Bullets[i].timer.set(1500);
					//if (m=mciSendString("stop BOMB", NULL, 0, NULL));
					//if (m=mciSendString("play BOMB", NULL, 0, NULL));
					if (m=mciSendString("play BOMB.WAV", NULL, 0, NULL));

				}
				else
				{
				//	PlaySound("EFX2.WAV", NULL, SND_ASYNC | SND_FILENAME);
					//if (m=mciSendString("stop NORMAL", NULL, 0, NULL));
					//if (m=mciSendString("play NORMAL", NULL, 0, NULL));
					if (m=mciSendString("play NORMAL.WAV", NULL, 0, NULL));
				}
				//if (mciSendCommand(pDevice, MCI_STOP, MCI_WAIT, (DWORD)&pp) == 0);
				//if (mciSendCommand(pDevice, MCI_PLAY, MCI_NOTIFY, (DWORD)&pp) == 0);
			   // mciSendString("open EFX1.WAV alias NORMAL", NULL, 0, hWndMain);
				//mciSendString("close NORMAL", NULL, 0, NULL);

				//mciSendString("EFX1.WAV", NULL, 0, NULL);
				return;
			}
		}
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void MoveBullet(void)
{
	int f;
	for(int i=0; i<MAXBULLET; i++)
	{
		if(Bullets[i].Type >= 0)
		{
			if(Bullets[i].delayY.isDone())
			{
				if(Bullets[i].Pos.y > WINDOWHEIGHT || Bullets[i].Pos.y < 0)
				{
					Bullets[i].Type=-1;
				} else {
					if(f=CheckHit(Bullets[i].Pos.x, Bullets[i].Pos.y,
						0, Bullets[i].Off.y, Bullets[i].Type, i) && Bullets[i].Type >= 0)
					{
						Bullets[i].Off.y*=-1;
					}
					Bullets[i].Pos.y+=Bullets[i].Off.y;
					Bullets[i].delayY.repeat();
				}
			}
			if(Bullets[i].delayX.isDone())
			{
				if(Bullets[i].Pos.x > WINDOWWIDTH || Bullets[i].Pos.x < 0)
				{
					Bullets[i].Type=-1;
				} else {
					if(f=CheckHit(Bullets[i].Pos.x, Bullets[i].Pos.y,
						Bullets[i].Off.x, 0, Bullets[i].Type, i) && Bullets[i].Type >= 0)
					{
						//typical behavior
						Bullets[i].Off.x*=-1;
					}
					Bullets[i].Pos.x+=Bullets[i].Off.x;
					Bullets[i].delayX.repeat();
				}
			}
			if(Bullets[i].Type%10 == 0) //player bullet goes up
			{
				if(Bullets[i].timer.isDone() && Bullets[i].Type>0)
				{
					//blow up stuff?
					if(Bullets[i].Type==100)
					{
						if(Bullets[i].AniFrame==5)
						{
							Bullets[i].Type=-1;
						}
						else
						{
							Bullets[i].AniFrame++;
							Bullets[i].timer.set(33/Bullets[i].AniFrame);
							Detonate(i);
						}
					}
					else
					{
						Bullets[i].Type=100;
						Bullets[i].Off.y=0;
						Bullets[i].Off.x=0;
						Bullets[i].AniFrame=0;
						Bullets[i].timer.set(33);
					}
				}
				else
				{
					if(Bullets[i].Off.x<0) { Bullets[i].AniFrame++; }
					else { Bullets[i].AniFrame--; }
						if(Bullets[i].AniFrame>7) { Bullets[i].AniFrame=0; }
					else if(Bullets[i].AniFrame<0) { Bullets[i].AniFrame=7; }
				}

			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////
//BitBlt(bmoMap,i*20-10,j*15-7,20,15,bmoBlock1,0,0,SRCCOPY);
//////////////////////////////////////////////////////////////////////////////
int CheckHit(int x, int y, int u, int v, int type, int i)
{
	if(type == 0 || type == 10)
	{
		
		int mx=(x+10+u)/20;
		int my=(y+7+v)/15;
		int nx,ny;
		bool px,py;
		px=py=false;
		if(Map[mx][my]>0)
		{
			nx=mx*20;
			ny=my*15;
			px=((u>0 && nx>x) || (u<0 && nx<x));
			py=((v>0 && ny>y) || (v<0 && ny<y));
			if (!py && !px)
				return 0;
			if(Map[mx][my] == 5) // fin
			{
				NewLevel();
				return 0;
			}
			else if(Map[mx][my] == 4) // gray
			{
				return 1;
			}
			else if(Map[mx][my] == 3) // red
			{
				if(Bullets[i].Type==10)
					Bullets[i].timer.set(100);
				else
				{ Bullets[i].timer.set(100); Bullets[i].Type=10; }
				return 0;
			}
			else if(Map[mx][my] == 2) // green
			{
				if(Bullets[i].Type==10)
					Bullets[i].timer.set(1500);
				return 0;
			}
			else if(Map[mx][my] == 1) // blue
			{
				return 1;
			}
		}
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void Detonate(int b)
{
	if (int m=mciSendString("play BOOM.WAV", NULL, 0, NULL));
	int calc=48;
	for(int i=0; i<MAXBULLET; i++)
	{
		if(i!=b && Bullets[i].Type>=0)
		{
			if(!!(0x8000000 & (Bullets[i].Pos.x-Bullets[b].Pos.x-calc 
				& Bullets[i].Pos.y-Bullets[b].Pos.y-calc
				& -Bullets[i].Pos.x+Bullets[b].Pos.x-calc
				& -Bullets[i].Pos.y+Bullets[b].Pos.y-calc)))
			{
				//yep collision!
				if(Bullets[i].Pos.y<Bullets[b].Pos.y)
					Bullets[i].Off.y=-1*BULLETSPEED;
				else if(Bullets[i].Pos.y>Bullets[b].Pos.y)
					Bullets[i].Off.y=BULLETSPEED;
				if(Bullets[i].Pos.x<Bullets[b].Pos.x)
					Bullets[i].Off.x=-1*BULLETSPEED;
				else if(Bullets[i].Pos.x>Bullets[b].Pos.x)
					Bullets[i].Off.x=BULLETSPEED;
			}
		}
	}
}
