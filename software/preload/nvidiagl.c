typedef unsigned int uint;

int glXSwapIntervalSGI(int interval) {
  return 0;
}

int glXGetVideoSyncSGI(uint *count) {
  static unsigned int frameCount = 0;
  //TODO: Framecount should depend on current system time
  *count = (frameCount++)/2; // NOTE: Keeps the same frame for 2 calls
  return 0;
}

int glXGetRefreshRateSGI(unsigned int * rate) { //TODO: need an actual prototype
  *rate = 60; //TODO: what does this function return?
  return 0;
}

void glGenFencesNV(int n, uint *fences) {
  static unsigned int curf = 1;
  while(n--) {
    *fences++ = curf++;
  }
  return;
}

void glDeleteFencesNV(int a, const uint *b) {
  return;
}
