#include <dlfcn.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>


int ioctl(int fildes, int request, void* data) {
  printf("Running ioctl 0x%X on %i\n",request,fildes);
  if ((request == SIOCSIFADDR) || (request == SIOCSIFFLAGS) || (request == SIOCSIFHWADDR) || (request == SIOCSIFHWBROADCAST) || (request == SIOCDELRT) || (request == SIOCADDRT) || (request == SIOCSIFNETMASK)) {
    //printf("Attempt to change network interface\n");
    //printf("Blocked access!\n");
//TODO: More magic like http://stackoverflow.com/questions/5308090/set-ip-address-using-siocsifaddr-ioctl
//    return 0;

    errno = ENXIO;
    return -1;

  }
  int(*originalIoctl)(int fildes, int request, void* data) = dlsym(RTLD_NEXT,"ioctl");
  return originalIoctl(fildes, request, data);
}
