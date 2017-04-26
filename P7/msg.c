/************************************************************************

 Functions for reading and writing "messages" over virtual circuits for
 the tic-tac-toe system.

 Phil Kearns							         
 April 12, 1998						         
 								         
*************************************************************************/

#include "common.h"

void
putmsg(int s, struct tttmsg *m)
// int s               socket on which sending
// struct tttmsg *m    pointer to the message to be sent

{
  int bytes_sofar, to_go, num;
  char *addr;

  bytes_sofar = 0;
  to_go = sizeof(struct tttmsg);

  addr = (char *) m;

  while (to_go > 0) {
    num = write(s, &(addr[bytes_sofar]), to_go);
    if (num < 0) {
      perror("putmsg");
      exit(1);
    }
  to_go -= num; bytes_sofar += num;  
  }
}

void
getmsg(int s, struct tttmsg *m)
// int s              socket on which receiving
// struct tttmsg *m   container for the message

{
  int bytes_sofar, to_go, num;
  char *addr;

  bytes_sofar = 0;
  to_go = sizeof(struct tttmsg);

  addr = (char *) m;

  while (to_go > 0) {
    num = read(s, &(addr[bytes_sofar]), to_go);
    if (num < 0) {
      perror("putmsg");
          exit(1);
    }
    if (num == 0) {
      fprintf(stderr, "Unexpected EOF\n");
      exit(1);
    }
    to_go -= num; bytes_sofar += num;
  }
}

void
protocol_error(char expected, struct tttmsg *offender)

{
  char *stype;

  fprintf(stderr, "Protocol error: expected ");
  switch (expected) {
  case 'A':
    stype = "WHO"; break;
  case 'B':
    stype = "HANDLE"; break;
  case 'C':
    stype = "MATCH"; break;
  case 'D':
    stype = "WHATMOVE"; break;
  case 'E':
    stype = "MOVE"; break;
  case 'F':
    stype = "RESULT"; break;
  default:
    stype = "UNKNOWN"; break;
  }
  fprintf(stderr, "%s message; got following message:\n", stype);
  dumpmsg(offender);
  exit(1);
}

void
dumpmsg(struct tttmsg *m)

{
  char *stype, datastring[32];
  int i;
  
  switch (m->type) {
  case 'A':
    stype = "WHO"; break;
  case 'B':
    stype = "HANDLE"; break;
  case 'C':
    stype = "MATCH"; break;
  case 'D':
    stype = "WHATMOVE"; break;
  case 'E':
    stype = "MOVE"; break;
  case 'F':
    stype = "RESULT"; break;
  default:
    stype = "UNKNOWN"; break;
  }
  fprintf(stderr,"\tTYPE:     %s\n",stype);
  fprintf(stderr,"\tBOARD:    [");
  for (i=0; i<8; i++)
    fprintf(stderr,"%c,",m->board[i]);
  fprintf(stderr,"%c]\n",m->board[8]);
  bcopy(datastring,&(m->data),31);
  datastring[31]='\0';
  fprintf(stderr,"\tDATA:     %s\n", datastring);
  fprintf(stderr,"\tRES:      %c\n", m->res);
}
