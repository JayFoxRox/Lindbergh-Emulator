#include <dlfcn.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <unistd.h>


#include <GL/glx.h>
#include <X11/extensions/xf86vmode.h>


Bool XF86VidModeSwitchToMode(
           Display *display,
           int screen,
           XF86VidModeModeInfo *modeline) {
printf("Scumbag X11\n");
//exit(5);
return 1;
}

/*
  Bool XF86VidModeGetAllModeLines(
           Display *display,
           int screen,
           int *modecount_return,
           XF86VidModeModeInfo ***modesinfo) {
printf("Scumbag X11\n");
exit(6);
return 1;
}
*/


/*
__GLXextFuncPtr glXGetProcAddressARB (const GLubyte* procName) {
  __GLXextFuncPtr result;
  printf("* hook-glx.c: glXGetProcAddressARB(\"%s\")\n", procName);
//exit(4);
usleep(1000*1000);
  return NULL;
}

void glXSwapBuffers(Display* dpy, GLXDrawable drawable) {
//real_glXSwapBuffers(dpy, drawable);
//hookfunc();
exit(2);
  return;
}
*/
