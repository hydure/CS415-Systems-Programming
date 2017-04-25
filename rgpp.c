#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define BUF_SIZE 2048

///////////////////////NODE CLASS/////////////////////////////

typedef struct node {
  int LineNumberTag;
  char * fileName;
  char * lineNumber;
  char * extraChars;
  struct node * next;
} NODE;

void push(NODE * head, char * fName, char * lNumber,
	  char * xtraChars) {
  
  NODE * current = head;

  /* go to the end of the list */
  while (current->next != NULL) {
    current = current->next;
  }

  /* now we can add a new node to the list */
  current->next = malloc(sizeof(NODE) + sizeof(int) + sizeof(fName)
			 + sizeof(lNumber) + sizeof(xtraChars));
  current->next->LineNumberTag = 1;
  current->next->fileName = fName;
  current->next->lineNumber = lNumber;
  current->next->extraChars = xtraChars;
  current->next->next = NULL;
}

void printList(NODE * head) {
  NODE * current = head;
  current = current->next;

  while (current != NULL) {
    printf("%s %s %s\n", current->fileName, current->lineNumber, current->extraChars);
    current = current->next;
  }
}

void freeList(NODE * head){
  NODE * tempNode;
  while (head != NULL){
    tempNode = head;
    head = head->next;
    free(tempNode);
  }
}   

///////////////////////////////////////////////////////////////
//idea from http://stackoverflow.com/questions/27303062/strstr-function-like-that-ignores-upper-or-//lower-case
char * strIgnoreCasestr(char* str1, const char* str2){
    char * p1 = str1;
    const char * p2 = str2;
    char * r = *p2 == 0 ? str1 : 0;

    while (*p1 != 0 && *p2 != 0){
        if (tolower(*p1) == tolower(*p2)){
            if (r == 0){
                r = p1 ;
            }
            p2++;
        }
        else{
            p2 = str2;
            if (tolower(*p1) == tolower(*p2)){
                r = p1 ;
                p2++ ;
            }
            else{
                r = 0 ;
            }
        }
        p1++ ;
    }
    return *p2 == 0 ? r : 0 ;
}

///////////////////////////////////////////////////////////////
void bannerLine(int lArg, int wArg, int matchingLines, int matchingWords){
  if (lArg == 1){
    printf("\nTHERE ARE %d LINES THAT MATCH\n\n", matchingLines);
  }
  if (wArg == 1){
    printf("\nTHERE ARE %d MATCHING WORDS\n\n", matchingWords);
  }
}

void printHeader(NODE * current){
  printf("=====================%s\n", current->fileName);
}

///////////////////////////////////////////////////////////////

int main(int argc, char * argv[]){ 

  FILE * stream, * currentFile;
  char line[BUF_SIZE], * fileName, * lineNumber, * words, c, * searchedForWord,
    * sentence, * sentenceToSubtract, * sentenceTaking, * previousFileName;
  NODE * head, * current;
  int lFlag = 0, nFlag = 0, bFlag = 0, wFlag = 0, lengthOfWord, matchingLines = 0,
    i, matchingWords = 0, numberToCompare, count, count2, difference;
  
  // get and check arguments
  while ((c = getopt(argc, argv, "l::n::b::w::")) != -1){
    if (strcmp(argv[1], "-l") != 0 && strcmp(argv[1], "-w") != 0){
      fprintf(stderr, "Invalid first argument.\n");
      exit(1);
    };
    if (c == 'l'){
      if (argc >= 5){
	fprintf(stderr, "Too many arguments.\n");
	exit(1);
      }
      lFlag = 1;
    }else if (c == 'n'){
      nFlag = 1;
    }else if (c == 'b'){
      bFlag = 1;
    }else if (c == 'w'){
      if (argc >= 6){
	fprintf(stderr, "Too many arguments.\n");
	exit(1);
      }
      if (argv[2][0] == '-'){
	fprintf(stderr, "Argument after '-w' must be a word.\n");
        exit(1);
      }
      wFlag = 1;
      searchedForWord = argv[2];
      lengthOfWord = strlen(argv[2]);
    }
    
    else{
      fprintf(stderr, "At Least One Argument is Not Allowed\n");
      exit(1);
    }
  }
  if (wFlag == 1 && lFlag == 1){
    fprintf(stderr, "Can only pick -l or -w.\n");
    exit(1);
  }
  if (wFlag == 0 && lFlag == 0){
    fprintf(stderr, "Need to pick either -l or -w.\n");
    exit(1);
  }

  // open stream
  stream = fdopen(0, "r");

  if (stream == NULL){
    fprintf(stderr, "No input.\n");
    exit(1);
  }  

  // create linked list of file lines
  head = (NODE*)malloc(sizeof(NODE));
  while(fgets(line, sizeof(line), stream) != NULL){
    if(sscanf(line, "%m[^:]:%m[^:]:%m[^\n]", &fileName, &lineNumber, &words) == 3){
      push(head, fileName, lineNumber, words);
      if (lFlag == 1){
	matchingLines++;
      }
      if (wFlag == 1){
	for (i = 0; line[i]; i++){
	  line[i] = tolower(line[i]);
	}
	sentence = malloc(strlen(line)+1);
	strcpy(sentence, line);
	while((sentence = strstr(sentence, searchedForWord)) != NULL){
	  matchingWords++;
	  sentence++;
	}
      }
    }
  }

  fclose(stream);
  free(sentence);

  // possibly print banner
  if (bFlag == 1){
    bannerLine(lFlag, wFlag, matchingLines, matchingWords);
  }

  current = head;
  current = current->next;
  previousFileName = "";
  while (current != NULL){
    printHeader(current);

    // checking to see if still on same file
    if (strcmp(current->fileName, previousFileName) != 0){
      if (strcmp(previousFileName, "") != 0){
	fclose(currentFile);
      }
    }
    currentFile = fopen(current->fileName,"r");	  
    if (currentFile == NULL){
      fprintf(stderr, "File not found.\n");
      exit(1);
    }

    count = 1;
    
    // iterate through each file line
    while(fgets(line, sizeof(line), currentFile) != NULL){
      
      // get line number from linked list
      numberToCompare = (int)strtol(current->lineNumber, (char **)NULL, 10);
      if (numberToCompare == count && current->next != NULL){
	current = current->next;
      }
      
      // case if -l argument is used
      if (lFlag == 1){
	if (numberToCompare == count){
	  printf("%c",'*');
	}
	else{
	  printf("%c",' ');
	}
	if (nFlag == 1){
	  printf("%d:  ", count);
	}
	printf("%s", line);
      }
  
      // case if -w argument is used
      if (wFlag == 1){
	if (nFlag == 1){
	  printf("%d:  ", count);
	}
	if (numberToCompare == count){
	  sentenceToSubtract = malloc(strlen(line)+1);
	  strcpy(sentenceToSubtract, line);
	  sentenceTaking = malloc(strlen(line)+1);
	  strcpy(sentenceTaking, line);
	  while ((sentenceToSubtract = strIgnoreCasestr(sentenceToSubtract, searchedForWord)) != NULL){
	    count2 = 0;
	    difference = strlen(sentenceTaking) - strlen(sentenceToSubtract);
	    while (count2 < difference){
	      printf("%c", sentenceTaking[count2]);
	      count2++;
	    }
            // prints the correct cases for each char in the word
	    for (i=0; i < strlen(searchedForWord); i++){
	      printf("\e[7m%c\e[0m", sentenceTaking[count2+i]);
            }
	    sentenceToSubtract += lengthOfWord;
	    strcpy(sentenceTaking, sentenceToSubtract);
	  }
	  printf("%s", sentenceTaking);
	}
	else{
	  printf("%s", line);
	}
      }
      count++;
    }
    
    // if there is not another line to look at
    if (current->next == NULL){
      fclose(currentFile);
      exit(0);
    }
    previousFileName = current->fileName; 
  }
  freeList(head);
  free(sentenceTaking);
  free(sentenceToSubtract);
  return 0;
}
