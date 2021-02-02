#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void){
  char t[100];
  getcwd(t, sizeof(t));
  printf("dir %s\n", t);

  chdir("/bin");

  getcwd(t, sizeof(t));
  printf("dir %s\n", t);

  chdir("/usr/bin");

  getcwd(t, sizeof(t));
  printf("dir %s\n", t);
}
