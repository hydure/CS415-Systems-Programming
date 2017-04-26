#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>

typedef struct node {
  long iNumber;
  struct node *next;
} Node;

int main(int argc, char *argv[]) {
  char files[8192], *endptr;
  int number, daysAgo, repeatFlag = 0;
  time_t theTime;
  long iNode;
  struct stat info;
  Node *head = NULL, *current;

  if (argc != 2) {
    fprintf(stderr, "Need exactly 2 arguments: ./accessed [number]\n");
    exit(1);
  }

  number = strtol(argv[1], &endptr, 10);
  if (*endptr != '\0') {
    fprintf(stderr, "Error with inputted number.\n");
    exit(1);
  }

  if (!number) {
    fprintf(stderr, "Cannot input the number 0.\n");
    exit(1);
  }

  while(scanf("%s", files) != EOF) {
    repeatFlag = 0;
    if (!stat(files, &info)) {
      if (S_ISREG(info.st_mode)) {
        iNode = (long) info.st_ino;

        Node *newNode = malloc(sizeof(Node));
        newNode->iNumber = iNode;
        newNode->next = NULL;

        if (head == NULL) {
	  head = newNode;
	  time(&theTime);
	  daysAgo = (theTime - info.st_atime)/86400;
        }
        else {
	  current = head;
	  while (current->next != NULL) {
	    if (current->iNumber == iNode) {
	      repeatFlag = 1;
	    }
	    current = current->next;
	  }
	  if(!repeatFlag) {
	    current->next = newNode;
	    time(&theTime);
	    daysAgo = (theTime - info.st_atime)/86400;
	  }
	}
      }
      else{
	continue;
      }
      if (number > 0){
	if (daysAgo >= number){
	  printf("%s\n", files);
	}
      }
      else {
	if (daysAgo < abs(number)){
	  printf("%s\n", files);
	}
      }
    }
  }
  exit(0);
}
