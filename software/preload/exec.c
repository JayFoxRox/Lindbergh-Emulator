#include <stdio.h>
#include <stdlib.h>

// Make sure that all processes we start also stay in our environment

int execl(const char *path, const char *arg, ...){ printf("EXEC FUCK\n"); exit(42); }
int execlp(const char *file, const char *arg, ...){ printf("EXEC FUCK\n"); exit(42); }
int execle(const char *path, const char *arg, ...){ printf("EXEC FUCK\n"); exit(42); }
int execv(const char *path, char *const argv[]){ printf("EXEC FUCK\n"); exit(42); }
int execvp(const char *file, char *const argv[]){ printf("EXEC FUCK\n"); exit(42); }
int execvpe(const char *file, char *const argv[], char *const envp[]){ printf("EXEC FUCK\n"); exit(42); }
