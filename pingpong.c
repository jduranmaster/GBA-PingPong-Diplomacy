/*****************************************************************************
* PingPong Diplomacy for GBA.
* Author: Ryoga a.k.a. JDURANMASTER
* This module is a sample pong-clone.
*
******************************************************************************
*    - Testing GBA mode 4, GBA mode 3, double buffering, keyboard controlling.
*    - No sound Effects implemented yet.
*
******************************************************************************/  

#include <stdlib.h>
#include <stdio.h>
#include "font.h"

#include "gba.h"         //GBA register definitions.
#include "fade.h"        //background fades.
#include "keypad.h"      //keypad defines
#include "dispcnt.h"     //REG_DISPCNT register defines
#include "dma.h"         //dma defines.
#include "dma.c"         //dma copy function.
#include "util.h"         //utils defines.
#include "util.c"         //utils functions.

//gfx
#include "Rlogo.h"       //8-bit Ryoga Logo
#include "HKlogo.h"      //8-bit HK logo
#include "saludIntro.h"  //8-bit Health logo.
#include "tabchvsusa1.h"  //table tennis. China vs EEUU.
#include "tabchvsusa2.h"  //table tennis. China vs EEUU.
#include "tabfinalch.h"  //final screen. China Wins.
#include "tabfinalusa.h" //final screen. EEUU Wins.
#include "tabmainlogo.h" //main screen of the game.
#include "tabmaonix.h"   //intro screen. For text.
#include "tabusavsch1.h"  //table tennis. EEUU vs China.
#include "tabusavsch2.h"  //table tennis. EEUU vs China.
#include "usacpuwins.h"   // USA-CPU Win Splash
#include "chinacpuwins.h" // China-CPU Win Splash

//some useful colors
#define BLACK 0x0000
#define WHITE 0xFFFF
#define BLUE 0xEE00
#define CYAN 0xFF00
#define GREEN 0x0EE0
#define RED 0x00FF
#define MAGENTA 0xF00F
#define BROWN 0x0D0D

//defines for the video system
//unsigned short* videoBuffer = (unsigned short*)0x6000000;
#define DISPLAY_CONTROLLER *(unsigned long*)0x4000000
#define VIDC_BASE_HALF	((volatile unsigned short int*) 0x04000000)	
#define VCOUNT		(VIDC_BASE_HALF[3])	


unsigned short* ScreenBuffer = (unsigned short*)0x6000000;
unsigned short* ScreenPal = (unsigned short*)0x5000000;

#define VIDEO_MODE_3 0x3
#define BACKGROUND2 0x400
#define SCREEN_W 240
#define SCREEN_H 160

//game constants
#define BALL_SIZE 6
#define PADDLE_WIDTH 8
#define PADDLE_HEIGHT 28
#define PADDLE_SPEED 2
#define MAX_POINTS_TO_WIN 5

//global variables
int paddleX[3];
int paddleY[3];
int ballX, ballY;
int velX = 2, velY = 1;
int score1=0,score2=0;
int state=0;

//clear VideoBuffer with some unused VRAM
void ClearBuffer()
{
	REG_DM3SAD = 0x06010000;//Source Address - Some unused VRAM
	REG_DM3DAD = 0x06000000;//Destination Address - Front buffer
	REG_DM3CNT = DMA_ENABLE | DMA_SOURCE_FIXED | 19200;
}

//wait for the screen to stop drawing
void WaitForVsync()
{
	while((volatile u16)REG_VCOUNT != 160){}
}

//Wait until the start key is pressed
void WaitForStart()
{
	
	u8 t=0;//used for detecting a single press
	
	while (1)
	if( KEY_DOWN(KEYSTART) )
	{
		t++;
		if(t<2){
			return;
		}
	}
	else{
		t = 0;
	}
}

//Wait until the A key is pressed
void WaitForAButton()
{
	
	u8 t=0;//used for detecting a single press
	
	while (1)
	if( KEY_DOWN(KEYA) )
	{
		t++;
		if(t<2){
			return;
		}
	}
	else{
		t = 0;
	}
}

//Wait until the UP direction is pressed
void WaitForUP()
{
	
	u8 t=0;//used for detecting a single press
	
	while (1)
	if( KEY_DOWN(KEYUP) )
	{
		t++;
		if(t<2){
			return;
		}
	}
	else{
		t = 0;
	}
}

//Wait until the DOWN direction is pressed
void WaitForDOWN()
{
	
	u8 t=0;//used for detecting a single press
	
	while (1)
	if( KEY_DOWN(KEYDOWN) )
	{
		t++;
		if(t<2){
			return;
		}
	}
	else{
		t = 0;
	}
}

void setMode(int mode)
{
    DISPLAY_CONTROLLER = mode | BACKGROUND2;
}

inline void drawpixel(int x, int y, unsigned short color)
{
	videoBuffer[y * 240 + x] = color;
}

inline unsigned short getpixel(int x, int y)
{
    return videoBuffer[y * 240 + x];
}

void drawbox(int left, int top, int right, int bottom, unsigned short color)
{
	int x, y;
    for(y = top; y < bottom; y++)
        for(x = left; x < right; x++)
            drawpixel(x, y, color);
}

int buttonPressed(int button)
{
    //pointer to the button interface
    volatile unsigned int *BUTTONS = (volatile unsigned int *)0x04000130;
    
    //see if UP button is pressed
    if (!((*BUTTONS) & button))
        return 1;
    else
        return 0;
}

//draw text using characters contained in font.h
void print(int left, int top, char *str, unsigned short color)
{
    int x, y, draw;
    int pos = 0;
    char letter;
    
    //look at all characters in this string
    while (*str)
    {
        //get current character ASCII code
        letter = (*str++) - 32;
        
        //draw the character
        for(y = 0; y < 8; y++)
            for(x = 0; x < 8; x++)
            {
                //grab a pixel from the font character
                draw = font[letter * 64 + y * 8 + x];
                
                //if pixel = 1, then draw it
                if (draw)
                    drawpixel(left + pos + x, top + y, color);
            }

        //jump over 8 pixels
        pos += 8;
    }
}

void printScores()
{
    char s[5];

    //erase scores
    drawbox(0,0,20,10,BLACK);
    drawbox(220,0,239,10,BLACK);

    //display scores
    sprintf(s,"%i",score1);
    print(5,0,s,WHITE);
    
    sprintf(s,"%i",score2);
    print(220,0,s,WHITE);
}

void eraseBall()
{
	drawbox(ballX, ballY, ballX+BALL_SIZE, ballY+BALL_SIZE, BLACK);
}

void updateBall()
{
    ballX += velX;
    ballY += velY;

    //did ball hit right wall?
    if (ballX > SCREEN_W - BALL_SIZE - 1)
    {
        velX *= -1;
        score1++;
    }

    //did ball hit left wall?
    if (ballX < 1)
    {
        velX *= -1;
        score2++;
    }

    //did ball hit top or bottom walls?
    if (ballY < 12 || ballY > SCREEN_H - BALL_SIZE - 1)
        velY *= -1;

}

void drawBall()
{
	drawbox(ballX, ballY, ballX+BALL_SIZE, ballY+BALL_SIZE, /*MAGENTA*/ WHITE);
}

void erasePaddle1()
{
	drawbox(paddleX[1], paddleY[1], paddleX[1] + PADDLE_WIDTH, paddleY[1] + PADDLE_HEIGHT, BLACK);
}

void updatePaddle2(){
	int py = paddleY[2] + PADDLE_HEIGHT/2;

    //only move computer paddle if ball is moving toward right
    if (/*velX < 0*/velX > 0 && /*ballX < 120*/ ballX > 100) {
        
        if (py > ballY) {
            paddleY[2] -= 1;
            if (paddleY[2] < 12) paddleY[2] = 12;
        }

        if (py < ballY) {
            paddleY[2] += 1;
            if (paddleY[2] > 160-PADDLE_HEIGHT)
                paddleY[2] = 160-PADDLE_HEIGHT;
        }
    }
}

// show the game intro.
void showGameIntro()
{
	int loop;
	
	EraseScreen();
	
	// logo - saludIntro
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = saludIntroPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = saludIntroData[loop];
   	}
	
    WaitForVblank();
	Flip();
	Sleep(2500);
	EraseScreen();
   	
	// logo Hammer Keyboard Studios.
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = HKlogoDataPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = HKlogoDatadata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(2000);
	EraseScreen();
   	
	// logo - Rlogo
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = RlogoPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = Rlogodata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(2000);
	EraseScreen();
	
	// logo - Main Screen
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabmainlogoPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabmainlogodata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(2000);
}

void selectCountryPaddle()
{
	state = 0;
	
	drawbox(0, 0, 239, 159, BLACK);
		
	print(23-16,1,"PING-PONG DIPLOMACY REFERS TO", WHITE);
	print(23-16,11,"THE EXCHANGE OF TABLE TENNIS", WHITE);
	print(23-16,22,"PLAYERS BETWEEN USA AND CHINA", WHITE);
	print(23-16,33,"IN THE EARLYS 1970S. THE", WHITE);
	print(23-16,44,"EVENT MARKED A THAW IN SINO-", WHITE);
	print(23-16,55,"AMERICAN RELATIONS THAT PAVED", WHITE);
	print(23-16,66,"THE VISIT TO BEIJING BY R.N.", WHITE);
	
	print(23-16,77,"SELECT YOUR COUNRTY AND PRESS", MAGENTA);
	print(23-16,87,"THE A-BUTTON OF THE PAD.", MAGENTA);
	
	print(30-16,100,"*", WHITE);
	print(38-16,100,"PEOPLES REPUBLIC OF CHINA", RED);
	print(38-16,110,"UNITED STATES OF AMERICA", CYAN);
	
	print(110-16,130,"2016", WHITE);
	print(25-16,140,"PROGRAMMED BY JDURANMASTER", WHITE);
	
	while((((KEYS) & KEYA))){
		if(!((KEYS) & KEYUP)){
			if(state == 0){
				print(30-16,110,"*", WHITE);
				print(30-16,100,"*", BLACK);
				state = 1;
			}
			else{
				print(30-16,100,"*", WHITE);
				print(30-16,110,"*", BLACK);
				state = 0;
			}	
		}
		if(!((KEYS) & KEYDOWN)){
			if(state == 1){
				print(30-16,100,"*", WHITE);
				print(30-16,110,"*", BLACK);
				state = 0;
			}
			else{
				print(30-16,110,"*", WHITE);
				print(30-16,100,"*", BLACK);
				state = 1;
			}	
		}
	}
}

void drawPaddle1()
{
	if(state ==  0){
		drawbox(paddleX[1], paddleY[1], paddleX[1] + PADDLE_WIDTH, paddleY[1] + PADDLE_HEIGHT, /*CYAN*/ RED);
	}else{
		drawbox(paddleX[1], paddleY[1], paddleX[1] + PADDLE_WIDTH, paddleY[1] + PADDLE_HEIGHT, /*CYAN*/ CYAN);
	}
}

void erasePaddle2()
{
	drawbox(paddleX[2], paddleY[2], paddleX[2] + PADDLE_WIDTH, paddleY[2] + PADDLE_HEIGHT, BLACK);
}

void updatePaddle1(){

    //check for UP button press
    if (buttonPressed(64))
    {
        if (paddleY[1] > 10)
            paddleY[1] -= PADDLE_SPEED;
    }
    
    //check for DOWN button press
    if (buttonPressed(128))
    {
        if (paddleY[1] < SCREEN_H - PADDLE_HEIGHT - 1)
            paddleY[1] += PADDLE_SPEED;
    }
}

void drawPaddle2()
{
	if(state == 0){
		drawbox(paddleX[2], paddleY[2], paddleX[2] + PADDLE_WIDTH, paddleY[2] + PADDLE_HEIGHT, CYAN);
	}else{
		drawbox(paddleX[2], paddleY[2], paddleX[2] + PADDLE_WIDTH, paddleY[2] + PADDLE_HEIGHT, RED);
	}
}

void checkCollisions()
{
    int x,y;
    
    //see if ball hit a paddle
    x = ballX + BALL_SIZE/2;
    y = ballY + BALL_SIZE/2;
    if (getpixel(x,y) != BLACK)
    {
        //we have a hit! 
        velX *= -1;
        ballX += velX;
    }
}

void waitRetrace()
{
	while (VCOUNT != 160);
	while (VCOUNT == 160);
}

void initGameBasicParameters()
{
	//clear buffer
	ClearBuffer();
	
	//Change video mode to MODE_3 with background 2 enabled
	setMode(MODE_3 | BG1_ENABLE);
	
	//select the country paddle.
	selectCountryPaddle();
   	
    //init the ball
    ballX = SCREEN_W / 2 - BALL_SIZE / 2;
    ballY = 40;
    
    //init the left paddle
    paddleX[1] = 10;
    paddleY[1] = SCREEN_H / 2 - PADDLE_HEIGHT / 2;
    
    //init the right paddle
    paddleX[2] = SCREEN_W - 20;
    paddleY[2] = SCREEN_H / 2 - PADDLE_HEIGHT / 2;

    //clear the screen
    drawbox(0, 0, 239, 159, BLACK);
    
    //display title
    print(55-16,1,"PING-PONG DIPLOMACY", MAGENTA);	
}

void showChinaEnding()
{
	int loop;
	
	EraseScreen();
	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabchvsusa1Palette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabchvsusa1data[loop];
   	}
	
    WaitForVblank();
	Flip();
	Sleep(6000);
	EraseScreen();
   	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabchvsusa2Palette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabchvsusa2data[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(6000);
	EraseScreen();
   	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabfinalchPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabfinalchdata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(6000);
	WaitForStart();
	EraseScreen();
	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabmaonixPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabmaonixdata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(6500);
}

void showUSAEnding()
{
	int loop;
	
	EraseScreen();
	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabusavsch1Palette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabusavsch1data[loop];
   	}
	
    WaitForVblank();
	Flip();
	Sleep(6000);
	EraseScreen();
   	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabusavsch2Palette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabusavsch2data[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(6000);
	EraseScreen();
   	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabfinalusaPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabfinalusadata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(6000);
	WaitForStart();
	EraseScreen();
	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabmaonixPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabmaonixdata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(6500);
}

void showPlayerDefeatedByChina()
{
	int loop;
	
	EraseScreen();
	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = chinacpuwinsPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = chinacpuwinsData[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(10000);
	EraseScreen();
	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabmaonixPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabmaonixdata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(6500);
}

showPlayerDefeatedByEEUU()
{
	int loop;
	
	EraseScreen();
	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = usacpuwinsPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = usacpuwinsData[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(10000);
	EraseScreen();
	
	for(loop=0;loop<256;loop++) {
      	ScreenPal[loop] = tabmaonixPalette[loop];     
   	}

   	for(loop=0;loop<19200;loop++) {
      	ScreenBuffer[loop] = tabmaonixdata[loop];
   	}
	
   	WaitForVblank();
	Flip();
	Sleep(6500);
}

int main(void)
{
    //Enable background 2 and set mode to MODE_4
	setMode(MODE_4 | OBJ_MAP_1D | BG2_ENABLE);
	
	showGameIntro();
	WaitForStart();
	EraseScreen();

	initGameBasicParameters();
	
    //game loop
    while(1)
    {
	    if(score1 != MAX_POINTS_TO_WIN && score2 != MAX_POINTS_TO_WIN){
		    waitRetrace();
        
	        eraseBall();
	        erasePaddle1();
	        erasePaddle2();
	
	        updatePaddle1();
	        updatePaddle2();
	        updateBall();
	
	        drawPaddle1();
	        drawPaddle2();
	
	        checkCollisions();
	        drawBall();
	
	        printScores();
	    }else{
		    if(score1 == MAX_POINTS_TO_WIN && state ==  0){ // PLAYER-CHINA
			    score1 = 0;
			    score2 = 0;
			    
			    //Enable background 2 and set mode to MODE_4
				setMode(MODE_4 | OBJ_MAP_1D | BG2_ENABLE);
			    showChinaEnding();
			    EraseScreen();
		    }
		    if(score1 == MAX_POINTS_TO_WIN && state ==  1){ // PLAYER-USA
				score1 = 0;
			    score2 = 0;
			    
			    //Enable background 2 and set mode to MODE_4
				setMode(MODE_4 | OBJ_MAP_1D | BG2_ENABLE);
			    showUSAEnding();
			    EraseScreen();
		    }
		    if(score2 == MAX_POINTS_TO_WIN && state ==  0){ // CPU-CHINA
			    score2 = 0;
			    score1 = 0;
			    
			    //Enable background 2 and set mode to MODE_4
				setMode(MODE_4 | OBJ_MAP_1D | BG2_ENABLE);
			    //showChinaEnding();
				showPlayerDefeatedByEEUU();
			    EraseScreen();
		    }
		    if(score2 == MAX_POINTS_TO_WIN && state ==  1){ // CPU-USA
			    score2 = 0;
			    score1 = 0;
			    
			    //Enable background 2 and set mode to MODE_4
				setMode(MODE_4 | OBJ_MAP_1D | BG2_ENABLE);
			    //showUSAEnding();
				showPlayerDefeatedByChina();
			    EraseScreen();
		    }   
		    
		    initGameBasicParameters();
	    }
    }

    return 0;
}
