#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

typedef struct node {
  long iNumber;
  struct node *next;
} Node;

int main(int argc, char *argv[]){
  char files[8192], *endptr;
  int totalSize, repeatFlag = 0, stall = 0, pid;
  long iNode;
  struct stat info;
  Node *head = NULL, *current;

  if (getenv("TSTALL") != NULL) {
    stall = strtol(getenv("TSTALL"), &endptr, 10);
    if (*endptr != '\0') {
      stall = 0;
    }
    else {
      sleep(stall);
    }
  }
	
  while(scanf("%s", files) != EOF) {	
    repeatFlag = 0;
    if (!stat(files, &info)) {
      if (S_ISREG(info.st_mode)) {
	iNode = (long) info.st_ino;
	Node *newNode = malloc(sizeof(Node));
	newNode->iNumber = iNode;
	newNode->next = NULL;
	if (!head) { 
	  head = newNode;
	}
	current = head;
	while (current->next != NULL) {
	  if (current->iNumber == iNode) {
	    repeatFlag = 1;
            break;
	  }
	  current = current->next;
	}
	if(!repeatFlag) {
	  totalSize += info.st_size;
	  if (head->next){
	    current->next = newNode;
	  }
	}
      }
    }
  }

  if ((getenv("UNITS") != NULL) && strcasecmp(getenv("UNITS"), "k") == 0){
    printf("%dkb\n", totalSize/1024);
  }
  else {
    printf("%d\n", totalSize);
  }
  if (getenv("TMOM") != NULL) {
    pid= strtol(getenv("TMOM"), &endptr, 10);
    if (*endptr != '\0'){
      pid = -1;
    }
    if (pid > 0) {
      kill(pid, SIGUSR1);
    }
  } 
  exit(0);
}
