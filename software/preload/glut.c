#include <math.h>
#include <stdbool.h>
#include <GL/freeglut.h>

//#include "print.h"
#include "linker.h"

/*
bool keys[0x100] = { false };
bool specials[0x10000] = { false };




void(*originalKeyUp)(unsigned char,int,int) = NULL;
void(*originalKeyDown)(unsigned char,int,int) = NULL;
void(*originalSpecialUp)(int,int,int) = NULL;
void(*originalSpecialDown)(int,int,int) = NULL;

void keyUp (unsigned char key, int x, int y) {  
  keys[key] = false;
  printf("Released: %i\n",key);
  if (originalKeyUp) {
    originalKeyUp(key,x,y);
  }
}
void keyDown (unsigned char key, int x, int y) {  
  keys[key] = true;
  printf("Pressed: %i\n",key);
  if (originalKeyDown) {
    originalKeyDown(key,x,y);
  }
  return;
}

void specialUp (int key, int x, int y) {  
  specials[key & 0xFFFF] = false;
  printf("S-Released: %i\n",key);
  if (originalSpecialUp) {
    originalSpecialUp(key,x,y);
  }
}
void specialDown (int key, int x, int y) {  
  specials[key & 0xFFFF] = true;
  printf("S-Pressed: %i\n",key);
  if (originalSpecialDown) {
    originalSpecialDown(key,x,y);
  }
  return;
}


FGAPI void FGAPIENTRY  glutKeyboardFunc(void (*func)(unsigned char key, int x, int y)) {
  originalKeyDown = func;
  return;
}

FGAPI void FGAPIENTRY  glutKeyboardUpFunc(void (*func)(unsigned char key, int x, int y)) {
  originalKeyUp = func;
  return;
}


FGAPI void FGAPIENTRY  glutSpecialFunc(void (*func)(int key, int x, int y)) {
  originalSpecialDown = func;
  return;
}

FGAPI void FGAPIENTRY  glutSpecialUpFunc(void (*func)(int key, int x, int y)) {
  originalSpecialUp = func;
  return;
}
*/

FGAPI void   FGAPIENTRY glutSwapBuffers(void) {
  static float alpha = 0.0;
  alpha += 0.1;
  glEnable(GL_SCISSOR_TEST);
  glScissor(10+50+50*sinf(alpha),10,10,10);
  glClearColor(1.0,0.0,1.0,0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_SCISSOR_TEST);

  FGAPI void FGAPIENTRY  (*originalGlutSwapBuffers)(void) = linkerNext("glutSwapBuffers");
  originalGlutSwapBuffers();
}

// This disables fullscreen 8]
#if 1

FGAPI int    FGAPIENTRY glutEnterGameMode(void) {
  printf("Tried to enter game mode, creating window instead\n");

/*
  FGAPI void FGAPIENTRY  (*originalGlutKeyboardFunc)(void (*func)(unsigned char key, int x, int y)) = linkerNext("glutKeyboardFunc");
  FGAPI void FGAPIENTRY  (*originalGlutKeyboardUpFunc)(void (*func)(unsigned char key, int x, int y)) = linkerNext("glutKeyboardUpFunc");
  FGAPI void FGAPIENTRY  (*originalGlutSpecialFunc)(void (*func)(int key, int x, int y)) = linkerNext("glutSpecialFunc");
  FGAPI void FGAPIENTRY  (*originalGlutSpecialUpFunc)(void (*func)(int key, int x, int y)) = linkerNext("glutSpecialUpFunc");
*/
  glutCreateWindow("Lindbergh Hack by Fox");
//  usleep(1000*1000);
/*
  originalGlutKeyboardFunc(keyDown);
  originalGlutKeyboardUpFunc(keyUp);
  originalGlutSpecialFunc(specialDown);
  originalGlutSpecialUpFunc(specialUp);
*/

/*

unsigned int i;
glutSwapBuffers();
for(i = 0; i < 1000; i++) {
  glutMainLoopEvent();
}

*/



/*
 pthread_t thread;
  int ret = pthread_create( &thread, NULL, glutwhore, NULL);  
*/

  return 1;
}

FGAPI void     FGAPIENTRY glutLeaveGameMode(void) {
  printf("Destroying our window!\n");
  glutDestroyWindow(glutGetWindow());
  return;
}

#endif

//This one somehow used to manage to crash the emu.. not sure why
FGAPI void    FGAPIENTRY  glutSetCursor(int cursor) {
  printf("Nah.. we keep our cursor! Game wanted %i\n",cursor);
  return;
}


