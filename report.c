#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define BUFSIZE sizeof(int)+1

int sig = 0;

void signalHandler(){
  sig = 1;
}

/////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
  char *endptr, buffer, PID[BUFSIZE], tstall[BUFSIZE], execStuff[BUFSIZE], buf[4096];
  int number, i, dFlag = 0, kFlag = 0, waitTime = 0, pid;
  int f2a1[2], a2t1[2], t2r1[2], f2a2[2], a2t2[2], t2r2[2];
  ssize_t size; 

  // Checking arguments
  if (argc < 2 || argc > 5) {
    fprintf(stderr, "Error: Incorrect amount of arguments...need between 2 and 5.\n");
    exit(1);
  }
  number = strtol(argv[1], &endptr, 10);
  if (*endptr != '\0' || number < 1) { 
    fprintf(stderr, "Error: First argument must be a non-zero, positive integer.\n");
    exit(1);
  }
  for (i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      dFlag = 1;
      if (argc < i + 1) {
	fprintf(stderr, "Error: -d must be followed by non-zero, positive integer.\n");
	exit(1);
      }
      waitTime = strtol(argv[i], &endptr, 10);
      if (*endptr != '\0' || waitTime <= 0) {
	fprintf(stderr, "Error: -d must be followed by non-zero, positive integer.\n");
	exit(1);
      }
    }
    else if (strcmp(argv[i], "-k") == 0) {
      kFlag = 1;
    }
    else {
      fprintf(stderr, "Error: Invalid argument(s).\n");
      exit(1);
    }
  }

  // Setting environments
  if (kFlag) {
    setenv("UNITS", "k", 1);
  }
  sprintf(tstall, "%d", waitTime);
  if (dFlag) {
    setenv("TSTALL", tstall, 1);
  }
  pid = getpid();
  sprintf(PID, "%d", pid);
  setenv("TMOM", PID, 1);
  // The infamous signal handler
  signal(SIGUSR1, signalHandler);

  // All the pipes
  pipe(f2a1); pipe(f2a2); pipe(a2t1); pipe(a2t2); pipe(t2r1); pipe(t2r2);

  // writing to stdin adapted from http://stackoverflow.com/questions/15883568/reading-from-stdin 
  while (read(0, &buffer, sizeof(buffer)) > 0) {
    write(f2a1[1], &buffer, sizeof(buffer));
    write(f2a2[1], &buffer, sizeof(buffer));
  }
  close(f2a1[1]); close(f2a2[1]);

  /* The four fork processes adapted from http://www.unix.com/programming/173811-c-execl-pipes.html
     and http://tldp.org/LDP/lpg/node11.html and 
     http://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell*/
  // accessed 1
  if (!(pid = fork())) {
    close(f2a2[0]); close(a2t1[0]); close(a2t2[0]); close(a2t2[1]);
    close(t2r1[0]); close(t2r1[1]); close(t2r2[0]); close(t2r2[1]);
    dup2(f2a1[0], 0); dup2(a2t1[1], 1);
    sprintf(execStuff, "%d", number);
    execl("accessed", "accessed", execStuff, NULL);
  }
  if (pid < 0){
    fprintf(stderr, "Error: Could not create child process \"accessed\".\n");
    exit(1);
  }

  close(f2a1[0]); close(a2t1[1]);
  
  // accessed 2
  if (!(pid = fork())) { 
    close(a2t2[0]); close(t2r1[0]); close(t2r1[1]);
    close(t2r2[0]); close(t2r2[1]);
    dup2(f2a2[0], 0); dup2(a2t2[1], 1);
    sprintf(execStuff, "%d", -number);
    execl("accessed", "accessed", execStuff, NULL);
  }
  if (pid < 0) {
    fprintf(stderr, "Error: Could not create child process: \"accessed\".\n");
    exit(1);
  }
  
  close(f2a2[0]); close(a2t2[1]);

  if ((pid = fork()) == 0) {
    close(f2a2[0]); close(t2r1[0]);
    close(t2r2[0]); close(t2r2[1]);
    dup2(a2t1[0], 0); dup2(t2r1[1], 1);
    execl("totalsize", "totalsize", NULL);
  }
  if (pid < 0) {
    fprintf(stderr, "Error: Could not create child process: \"totalsize\".\n");
    exit(1);
  }

  close(a2t1[0]); close(t2r1[1]);

  if (!(pid = fork())) {
    close(t2r1[0]); close(t2r2[0]);
    dup2(a2t2[0], 0);
    dup2(t2r2[1], 1);
    execl("totalsize", "totalsize", NULL);
  }
  if (pid < 0) {
    fprintf(stderr, "Error: Could not create  child process: \"totalsize\".\n");
    exit(1);
  }

  close(a2t2[0]);  close(t2r2[1]);

  // Waiting for signal
  while(sig == 0) {
    sleep(1);
    printf("*");
  }

  // Adapted from http://stackoverflow.com/questions/6480440/how-to-truncate-c-char
  size = read(t2r1[0], buf, sizeof(buf));
  buf[size - 1] = '\0'; 
  close(t2r1[0]);

  if (kFlag == 1) {
    printf("\nA total of %s are in regular files not accessed for %d days.\n", buf, number);
  }
  else {
    printf("\nA total of %s bytes are in regular files not accessed for %d days.\n", buf, number);
  }

  size = read(t2r2[0], buf, sizeof(buf));
  buf[size - 1] = '\0';
  close(t2r2[0]);

 if (kFlag == 1) {
    printf("----------\nA total of %s are in regular files acessed within %d days.\n", buf, number);
  }
  else {
    printf("----------\nA total of %s bytes are in regular files accessed within %d days.\n", buf, 	 number);
  }

  exit(0);
}
