#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <ctype.h>

//////////////////////////Structs////////////////////////

typedef struct threadBuffer{
  char** buffer; // dynamic buffer
  pthread_mutex_t lock;
  int readpos, writepos;
  pthread_cond_t notempty;
  pthread_cond_t notfull;
} ThreadBuffer;

typedef struct node {
  char* data;
  int freq;
  struct node *next;
  struct node* prev;
} Node;

typedef struct list {
  Node *start;
  Node *end;
} List;

//////////////////////Function Prototypes//////////////////

void initializeBuffer (ThreadBuffer *buf, int numberOfLines);
static void reader (ThreadBuffer *buf);
static void counter (ThreadBuffer *buf, char* name);
static void * readCall(void* data);
static void * countCall (void* data);
int initializeList(List *sList, char* data);
int printList(List *sList);
void insertWord(List *sList, char * oneWord);
void createNewThread(char* name);

/////////////////////////Global Variables////////////////////////

char *finalLine, **fileNames, *threadName = "a", newName = 'b';
int numberOfLines, maxCounters, listCount = 0, EOFFlag = 0, stopCounter = 0,
  threadCount = 0, fileCount = 0, oneFile = 1, bufferFull = 0,
  numLinesSet = 0, maxCounterSet = 0, fileDelaySet = 0, threadDelaySet = 0;
long fileDelay, threadDelay;
FILE* file;
ThreadBuffer buffy;
List oddWordsList, evenWordsList;
struct timespec fileWait, threadWait;
pthread_t thread1, thread2, newThread, *threadNames;
void* returnValue;

///////////////////////////Main/////////////////////////////////

int main(int argc, char *argv[]){

  int i, numberOfFiles;

  // needs 10 commandline arguments
  if (argc < 10){
    printf("Error. Please enter correct arguments\n");
    exit(1);
  }

  for (i = 1; i < argc; i++){
    if (strcmp(argv[i], "-b") == 0){
      numberOfLines = atoi(argv[i+1]);
      if (numberOfLines <= 0){
	fprintf(stderr, "Enter a positive integer for numlines.\n");
	exit(1);
      }
      numLinesSet = 1;
    }
    if (strcmp(argv[i], "-t") == 0){
      maxCounters = atoi(argv[i+1]);
      if (maxCounters > 26 || maxCounters < 1){
	fprintf(stderr, "Can only have up to 26 maxcounters.\n");
	exit(1);
      }
      maxCounterSet = 1;
    }
    if (strcmp(argv[i], "-d") == 0){
      fileDelay = atoi(argv[i+1]);
      if (fileDelay < 0){
	fprintf(stderr, "The filedelay must be zero or greater.\n");
	exit(1);
      }
      fileDelaySet = 1;
    }
    if (strcmp(argv[i], "-D") == 0){
      threadDelay = atoi(argv[i+1]);
      if (threadDelay < 0){
	fprintf(stderr, "The threaddelay must be zero or greater.\n");
	exit(1);
      }
      threadDelaySet = 1;
    }
  }

  threadNames = malloc(sizeof(pthread_t) * maxCounters+1);
  fileNames = malloc(sizeof(char) * argc + 1);
  finalLine = malloc((LINE_MAX * sizeof(char))+1);
  finalLine = "\n";

  // get correct time values
  if (fileDelay <= 999)
    fileWait.tv_nsec = fileDelay*1000000;
  else
    fileWait.tv_sec = fileDelay/1000;
  if (threadDelay <= 999)
    threadWait.tv_nsec = threadDelay*1000000;
  else
    threadWait.tv_sec = threadDelay/1000;

  numberOfFiles = argc - 9;

  //stick file names in file array
  while(numberOfFiles > 0){
    char* fileToOpen = argv[8+numberOfFiles];
    fileNames[fileCount] = fileToOpen;
    fileCount++;
    numberOfFiles--;
  }

  initializeBuffer(&buffy, numberOfLines);
  initializeList(&oddWordsList, "0");
  initializeList(&evenWordsList, "0");

  //if the buffersize is 1, set numberOfLines to 2 to fix empty checks
  if (numberOfLines == 1){
    numberOfLines++;
  }

  // create all the threads
  pthread_create (&thread1, NULL, readCall, 0);
  pthread_create (&thread2, NULL, countCall, (void*) threadName);

  // wait until all threads finish
  pthread_join (thread1, &returnValue);
  pthread_join (thread2, &returnValue);

  // get rid of the zeros at the front of the lists
  oddWordsList.start = oddWordsList.start->next;
  evenWordsList.start = evenWordsList.start->next;

  printf("\nODD WORDS\n");
  printf("---------\n");
  if (oddWordsList.start != NULL){
    printList(&oddWordsList);
  }
  else{
    printf("No Odd Words Found\n\n");
  }
        
  printf("\nEVEN WORDS\n");
  printf("----------\n");
  if (evenWordsList.start != NULL){
    printList(&evenWordsList);
  }
  else{
    printf("No Even Words Found\n");
  }

  return 0;
}

/////////////////////////Methods////////////////////////////////

void createNewThread(char* name){
  threadName = name;
  pthread_t newThread;
  threadNames[threadCount] = newThread;
  threadCount++;

  maxCounters--;
  pthread_create (&newThread, NULL, countCall, (void*) threadName);
  int j;
  for (j = 0; j < threadCount-25; j++){
    pthread_join (threadNames[j], &returnValue);
  }
}

////////////////////////////////////////////////////////////////

void* readCall (void* data){

  while (EOFFlag == 0){
    reader (&buffy);
    nanosleep(&fileWait, NULL);

  }
  return NULL;
}

////////////////////////////////////////////////////////////////

static void reader (ThreadBuffer *buf){

  char *oneLine = malloc((LINE_MAX * sizeof(char))+1);;
  pthread_mutex_lock(&buf->lock);

  //wait until buffer is not full
  while ((buf->writepos+1) % numberOfLines == buf->readpos){
    pthread_cond_wait (&buf->notfull, &buf->lock);
  }

  memset(oneLine, 0, sizeof(LINE_MAX * sizeof(char))+1);

  //if its the first time entering or other file has been closed
  if (oneFile == 1){

    while(1){
      file = fopen(fileNames[fileCount-1], "r");
      if (file == NULL){
	fprintf(stderr, "File %s could not be opened. Continuing with rest of files...\n", fileNames[fileCount-1]);
	fileCount--;
	if (fileCount <= 0){
	  break;
	}
      }
      else
	break;
    }
  }
  if (fileCount != 0){
    oneFile = 0;
    fgets(oneLine, LINE_MAX, file);

    //if end of one file
    if (feof(file)){
      oneFile = 1;
      fclose(file);
      fileCount--;
    }

    //if end of all files
    if (fileCount == 0){
      EOFFlag = 1;
    }

    // if it is the last line, set finalLine so counter knows when to stop
    if (EOFFlag == 1){
      finalLine = oneLine;
    }

    // write line from file to buffer and continue
    buf->buffer[buf->writepos] = oneLine;
    buf->writepos++;
    if (buf->writepos >= numberOfLines){
      buf->writepos = 0;
      bufferFull = 1;
      if (maxCounters > 1){
	char* str = malloc(sizeof(char));
	str[0] = newName;
	createNewThread(str);
	newName++;
      }
    }

    // signal that the buffer now isn't empty
    pthread_cond_signal (&buf->notempty);
    pthread_mutex_unlock (&buf->lock);
  }

  //final file could not be opened
  else{
    EOFFlag = 1;
    pthread_cond_signal (&buf->notempty);
    stopCounter = 1;
    pthread_mutex_unlock (&buf->lock);
  }
}

////////////////////////////////////////////////////////////////

void* countCall (void* data){
  char* name = (char*) data;
  while (stopCounter == 0){
    counter (&buffy, name);
  }
  return NULL;
}

////////////////////////////////////////////////////////////////

static void counter (ThreadBuffer *buf, char* name){
  //local val so we know which thread stopped it
  int thisStop = 0, j = 0, k = 0;
  char c = '\0', *data, *oneWord;
  data = malloc((LINE_MAX * sizeof(char)) + 1);
  memset(data, 0, sizeof(LINE_MAX * sizeof(char)) + 1);
  pthread_mutex_lock (&buf->lock);

  while (buf->writepos == buf->readpos){
    if (stopCounter == 1){
      break;
    }
    pthread_cond_wait (&buf->notempty, &buf->lock);

  }
  // if counts are done, other threads should skip this
  if (stopCounter == 0){

    /* Read the data from buffer and advance read pointer */
    data = buf->buffer[buf->readpos];
    nanosleep(&threadWait, NULL);
    if (strcmp(data, finalLine) == 0 && EOFFlag == 1){
      stopCounter = 1;
      thisStop = 1;
    }

    // for EOF checks
    strcat(data,"\n");

    //loop through data to split words
    while (c != '\n'){
      oneWord = malloc((LINE_MAX* sizeof(char))+1);
      memset(oneWord, 0, sizeof(LINE_MAX * sizeof(char)) + 1);
      c = data[j];
      while (!isspace(c)){
	oneWord[k] = c;
	j++;
	k++;
	c = data[j];
      }
      j++;


      //if oneWord isn't empty:
      if (strcmp(oneWord, "") != 0){

	//check if odd or even and insert into list alphabetically
	if (strlen(oneWord) % 2 == 0){
	  insertWord(&evenWordsList, oneWord);
	}
	else{
	  insertWord(&oddWordsList, oneWord);
	}
	printf("%s", name);
	fflush(stdout);
      }
    }
  }
  buf->readpos++;
  if (buf->readpos >= numberOfLines)
    buf->readpos = 0;
  /* Signal that the buffer is now not full */
  pthread_cond_signal (&buf->notfull);

  pthread_mutex_unlock (&buf->lock);

  //cancel all remaning threads
  if (thisStop == 1){
    int y = 0;
    for (y = 0; y < threadCount-25; y++){
      pthread_cancel(threadNames[y]);
    }
    pthread_cancel(thread2);
    pthread_cancel(thread1);
  }
}

////////////////////////////////////////////////////////////////

void initializeBuffer (ThreadBuffer *buf, int numberOfLines){
  buf->buffer = malloc((sizeof(char*)+1)*numberOfLines);
  memset(buf->buffer, 0, sizeof(buf->buffer));
  pthread_mutex_init (&buf->lock, NULL);
  pthread_cond_init (&buf->notempty, NULL);
  pthread_cond_init (&buf->notfull, NULL);
  buf->readpos = 0;
  buf->writepos = 0;
}

////////////////////////////////////////////////////////////////

int initializeList(List *sList, char* data){
  Node *newNode;
  newNode = malloc(sizeof(Node));
  newNode->data = data;
  newNode->freq = 1;
  newNode->next = NULL;
  newNode->prev = NULL;
  sList->start = newNode;
  sList->end = newNode;
  return 0;
}

////////////////////////////////////////////////////////////////

int printList(List *sList){
  Node *current = sList->start;
  while(current != sList->end) {
    fprintf(stderr, "%s  %d\n", current->data, current->freq);
    current = current->next;
  }
  fprintf(stderr, "%s  %d\n", current->data, current->freq);
  return 0;
}

////////////////////////////////////////////////////////////////

void insertWord(List *sList, char* oneWord){
  Node *current = sList->start;

  if (current != sList->end)
    current = current->next;

  while (current != sList->end && (strcmp(oneWord, current->data) > 0)){
    current = current->next;
  }
  if (strcmp(oneWord, current->data) == 0){
    current->freq = current->freq + 1;
  }

  else if (current == sList->end){
                
    Node *new = malloc(sizeof(Node));
    new->data = oneWord;
    new->freq = 1;
                
    if (strcmp(oneWord, current->data) > 0 || current == sList->start){
      new->next = NULL;
      new->prev = current;
      current->next = new;
      sList->end = new;
      listCount++;
    }
    else{
      new->next = current;
      new->prev = current->prev;
      current->prev->next = new;
      current->prev = new;
      listCount++;
    }
  }
  else{
    Node *new;
    new = malloc(sizeof(Node));
    new->data = oneWord;
    new->freq = 1;
    new->next = current;
    new->prev = current->prev;
    current->prev->next = new;
    current->prev = new;
    listCount++;
  }
}
