#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ucontext.h>
#include <GL/freeglut.h>
#include <string.h>
#include <X11/Xlib.h>

//#include "print.h"
#include "context.h"
#include "linker.h"
//#include "dongle.h"
//#include "glut.h" //TODO: Remove

#include "io.h"

static void ioWrite(uint16_t port, ioData_t* data, int size) {
  if (port == 0x80) {
    // This is a HW-Test Point
    return;
  }

  printf("Unknown I/O port write %i (Size: %i), data: 0x%08X\n",port,size,data->dword);

  if (1) {
    //usleep(1000*1000);
    return;
  }
  abort();
}

static void ioRead(uint16_t port, ioData_t* data, int size) {

  printf("Unknown I/O port read: %i (Size: %i)\n",port,size);

  if (size == 1) { data->byte  = -1; }
  if (size == 2) { data->word  = -1; }
  if (size == 4) { data->dword  = -1; }

  if (1) {
    //usleep(1000*1000);
    return;
  }
  abort();
}


static bool exceptionHandler(hostContext_t* context) {

  uint8_t* code = (uint8_t*)context->eip;
  uint8_t im = code[1];
  if (*code == 0xE6) {
    ioWrite(im,(ioData_t*)&context->al,1);
    //printf("0x%08X: OUT 	Ib 	(0x%02X) eAX 	AL (0x%08X)\n",context->eip,im,context->al);
    context->eip += 2; 
    return true;
  }
  if (*code == 0xE7) {
    ioWrite(im,(ioData_t*)&context->eax,4);
    //printf("0x%08X: OUT 	Ib 	(0x%02X) 	eAX (0x%08X)\n",context->eip,im,context->eax);
    context->eip += 2; 
    return true;
  }
  if (*code == 0xEE) {
    //printf("0x%08X: OUT 	DX (0x%04X) 	AL (0x%02X)\n",context->eip,context->dx,context->al);
    ioWrite(context->dx,(ioData_t*)&context->al,1);
    context->eip += 1;
    return true;
  }
  if (*code == 0xEF) {
    //printf("0x%08X: OUT 	DX (0x%04X) 	eAX (0x%08X)\n",context->eip,context->dx,context->eax);
    ioWrite(context->dx,(ioData_t*)&context->eax,4);
    context->eip += 1;
    return true;
  }
  if (*code == 0xED) {
    //printf("0x%08X: in      eax, dx (0x%04X) // ecx: %i\n",context->eip,context->dx,context->ecx);
    ioRead(context->dx,(ioData_t*)&context->eax,4);
    context->eip += 1;
    return true;
  }

  printf("Unhandled exception at 0x%08X!\n",context->eip);
  return false;
  
}

static void exceptionHandlerWrapper(uint32_t oldEsp, hostContext_t* context) {
//  printf("Now in handler-wrapper\n");
  hostContext_t tmp;
  getcontext(&tmp.ctx);
  context->fpregs = &context->__fpregs_mem;
  memcpy(context->fpregs,tmp.fpregs,sizeof(struct _libc_fpstate));
  context->esp = oldEsp;
  bool handled = exceptionHandler(context);
  // If handled is false we should generate a new sigsegv for lindy
  setcontext(&context->ctx);
}

static void exceptionTrampoline(int signum, siginfo_t *info, void *ptr) {
  hostContext_t* context = ptr;

bool handled = exceptionHandler(context);
if (!handled) {
  abort();
}
return;


if (context->eip & 0x80000000) {
  printf("SIGSEGV - eip was 0x%08X\n",context->eip);
  printf("Crash in host code?\n");
  abort();
  return;
}

//  printf("SIGSEGV - eip was 0x%08X\n",context->eip);

//  printFrame();

  uint8_t* code = (uint8_t*)context->eip;
  uint32_t data = context->eax;

  uint32_t oldEsp = context->esp;
  context->esp -= sizeof(hostContext_t);
  uintptr_t contextPointer = context->esp;
  memcpy((void*)contextPointer,context,sizeof(hostContext_t));
 
  context->esp -= 4;
  *(uint32_t*)context->esp = contextPointer;
  context->esp -= 4;
  *(uint32_t*)context->esp = oldEsp;
  context->esp -= 4;
   // No return address because we will restore the context

  context->eip = (uintptr_t)exceptionHandlerWrapper;

  
  return;


}	 


// Function preload hooks -------------------------------------------------------------------------


int iopl(int level) {
  printf("Faking iopl to %i\n",level);
  // Install signal handler for I/O emulation now
  static int inst = 0;
  if (inst == 0) {
    struct sigaction act;

    act.sa_sigaction = exceptionTrampoline;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &act, NULL);
    inst = 1;
  }
  return 0;
}

