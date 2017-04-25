// Colin Lightfoot CS 415 Drunken Beetle Project

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h> // for errno
#include <ctype.h>

int main(int argc, char * argv[]){

  // ensures there is only three arguments
  if (argc != 3){
    fprintf(stderr, "Usage: ./beetle X Y, where X is the dimensions of board and Y is number of beetles\n");
    exit(1);
  }
 
  int i = 0;
  int j = 0;

  for (i = 1; i < argc; i++){
    j = 0;      
    while(argv[i][j] != '\0'){
      if (!isdigit(argv[i][j])){
	fprintf(stderr, "Can ONLY input nonzero positive integers as large as 9223372036854775807 for the second and third arguments.\n");
	exit(1);
      }
      j++;
    }
  }

  char * pointer;
  errno = 0;

  long dimensions = strtol(argv[1], &pointer, 10);
  if (errno != 0){
    fprintf(stderr, "Error: Overflow for dimensions of board input.\n");
    exit(1);
  }
  
  long numberOfBeetles = strtol(argv[2], &pointer, 10);
  if (errno != 0){
    fprintf(stderr, "Error: Overflow for number of beetles input.\n");
    exit(1);
  }

  long k;
  long survivalInSeconds;
  long sumOfSeconds = 0;
  float averageLifetime;
  double x;
  double y;
  long degree;

  // create the number of beetles
  for(k = 0; k < numberOfBeetles; k++){
    x = dimensions/2;
    y = dimensions/2;
    survivalInSeconds = 0;
    
    // moves beetle while it has not fallen off
    while ( x < dimensions && x > 0 && y < dimensions && y > 0){
      degree = random();
      x = x + cos(degree);
      y = y + sin(degree);
      survivalInSeconds++;
      
      // if beetle has not fallen off it gets to pass out for one second
      if (x < dimensions && x > 0 && y < dimensions && y > 0){
	survivalInSeconds++;
      }

      // if beetle has fallen off it does not get to sleep and its survival is added to the sum
      else{
	sumOfSeconds += survivalInSeconds;
      }
    }
  }
  
  // the avergae lifetime of all the beetles is calculated
  averageLifetime = sumOfSeconds/numberOfBeetles;
  
  printf("%ld by %ld square, %ld beetles, mean beetle lifetime is %.1f\n", dimensions, dimensions,
	 numberOfBeetles, averageLifetime);

  return 0;

}
