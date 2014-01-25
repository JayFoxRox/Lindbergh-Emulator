#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>

static void* linkerResolve(const char* libN, const char* sym) {
  void* lib = dlopen(libN,RTLD_LAZY);
  if (lib == NULL) {
    printf("%i (%s) - ",errno,dlerror());
  }
  return dlsym(lib,sym);
}

static void* linkerNext(const char* sym) {
  return dlsym(RTLD_NEXT,sym);
}
