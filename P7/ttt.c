/************************************************************************
 * 			         ttt.c
 * Simple ttt client. No queries, no timeouts.
 * Uses deprecated address translation functions.
 * 
 * Phil Kearns
 * April 12, 1998
 * Modified April 2017 By Colin Lightfoot
 ************************************************************************/

#include "common.h"
#include "child.h"

void dump_board();
void drawLine(char * board);

FILE *read_from, *write_to;

int main(int argc, char **argv)

{
    char hostid[128], handle[32], opphandle[32], junk;
    char my_symbol; /* X or O ... specified by server in MATCH message */
    char board[9];
    unsigned short xrport;
    int sock, sfile, noBeep = 0;
    struct sockaddr_in remote;
    struct hostent *h;
    int num, i, move, valid, finished;
    struct tttmsg inmsg, outmsg;
    
    
    if (argc != 1) {
        fprintf(stderr,"ttt:usage is ttt\n");
        exit(1);
    }
    
    /* Get host,port of server from file. */
    
    if ( (sfile = open(SFILE, O_RDONLY)) < 0) {
        perror("TTT:sfile");
        exit(1);
    }
    i=0;
    while (1) {
        num = read(sfile, &hostid[i], 1);
        if (num == 1) {
            if (hostid[i] == '\0') break;
            else i++;
        }
        else {
            fprintf(stderr, "ttt:error reading hostname\n");
            exit(1);
        }
    }
    if (read(sfile, &xrport, sizeof(int)) != sizeof(unsigned short)) {
        fprintf(stderr, "ttt:error reading port\n");
        exit(1);
    }
    close(sfile);
    
    
    /* Got the info. Connect. */
    
    if ( (sock = socket( AF_INET, SOCK_STREAM, 0 )) < 0 ) {
        perror("ttt:socket");
        exit(1);
    }
    
    bzero((char *) &remote, sizeof(remote));
    remote.sin_family = AF_INET;
    if ((h = gethostbyname(hostid)) == NULL) {
        perror("ttt:gethostbyname");
        exit(1);
    }
    bcopy((char *)h->h_addr, (char *)&remote.sin_addr, h->h_length);
    remote.sin_port = xrport;
    if ( connect(sock, (struct sockaddr *)&remote, sizeof(remote)) < 0) {
        perror("ttt:connect");
        exit(1);
    }
    
    /* We're connected to the server. Engage in the prescribed dialog */
    
    /* Await WHO */
    
    bzero((char *)&inmsg, sizeof(inmsg));  
    getmsg(sock, &inmsg);
    if (inmsg.type != WHO) protocol_error(WHO, &inmsg);
    
    /* Send HANDLE */
    
    printf("Enter handle (31 char max):");
    fgets(handle, 31, stdin);
    bzero((char *)&outmsg, sizeof(outmsg));
    outmsg.type = HANDLE;
    strncpy(outmsg.data, handle, 31); outmsg.data[31] = '\0';
    putmsg(sock, &outmsg);
    start_child("wish", &read_from, &write_to);
    fprintf(write_to, "source ttt.tcl\n");
    fprintf(write_to, ".c itemconfigure you -text \"You: %s\"\n", handle);
    fprintf(write_to, ".c itemconfigure status -text \"Awaiting match...\"\n");
    
    /* Await MATCH */
    
    bzero((char *)&inmsg, sizeof(inmsg));  
    getmsg(sock, &inmsg);
    if (inmsg.type != MATCH) protocol_error(MATCH, &inmsg);
    my_symbol = inmsg.board[0];
    strncpy(opphandle, inmsg.data, 31); opphandle[31] = '\0';
    printf("You are playing %c\t your opponent is %s\n\n", my_symbol, opphandle);
    if (my_symbol == 'X'){
        fprintf(write_to, ".c itemconfigure you -text \"You: %s (X)\"\n", handle);
        fprintf(write_to, ".c itemconfigure opp -text \"Opponent: %s (O)\"\n", opphandle);
        fprintf(write_to, "set mySymbol \"X\"\n");
        fprintf(write_to, "set oppSymbol \"O\"\n");
    }
    else{
        fprintf(write_to, ".c itemconfigure you -text \"You: %s (O)\"\n", handle);
        fprintf(write_to, ".c itemconfigure opp -text \"Opponent: %s (X)\"\n", opphandle);
        fprintf(write_to, "set mySymbol \"O\"\n");
        fprintf(write_to, "set oppSymbol \"X\"\n");
    } 
    fprintf(write_to, ".c itemconfigure status -text \"Awaiting Opponent Move...\"\n");
    fprintf(write_to, "tk busy .c\n");
    fprintf(write_to, "tk busy .bf.sresign\n");
    /* In the match */
    
    for(i=0; i<9; i++) board[i]=' ';
    finished = 0;
    while(!finished){
        
        /* Await WHATMOVE/RESULT from server */
        
        bzero((char *)&inmsg, sizeof(inmsg));  
        getmsg(sock, &inmsg);
        switch (inmsg.type) {
            
            case WHATMOVE:
                fprintf(write_to, ".c itemconfigure status -text \"Your move...\"\n");
                fprintf(write_to, "tk busy forget .c\n");
                fprintf(write_to, "tk busy forget .bf.sresign\n");
                for(i=0; i<9; i++){
                    board[i]=inmsg.board[i];
                    fprintf(write_to, ".c itemconfigure text%d -text %c\n" , i+1, board[i]);
                }
                dump_board(stdout,board);
                do {
                    if (noBeep == 0){
                        fprintf(write_to, "ring\n");
                    }
                    num = fscanf(read_from, "%d", &move);
                    
                    valid = 0;
                    if (num == EOF) {
                        fprintf(stderr,"ttt:unexpected EOF on standard input\n");
                        exit(1);
                    }
                    if (num == 0) {
                        if (fread(&junk, 1, 1, stdin)==EOF) {
                            fprintf(stderr,"ttt:unexpected EOF on standard input\n");
                            exit(1);
                        }
                        continue;
                    }
                    if (move == 11){
                        bzero((char *)&outmsg, sizeof(outmsg));
                        outmsg.type = RESULT;
                        sprintf(&outmsg.res, "R");
                        putmsg(sock, &outmsg);
                        fprintf(write_to, ".c itemconfigure status -text \"You resigned.\"\n");
                        fprintf(write_to, "tk busy forget .bf.exit\n");
                        finished = 1;
                    }
                    if (move == 10){
                        fprintf(write_to, ".c itemconfigure status -text \"Opponent resigned.\"\n");
                        finished = 1;
                    }

                    if ((num == 1) && (move >= 1) && (move <= 9)) valid=1;
                    if ((valid) && (board[move-1] != ' '))valid=0;
                } while (!valid);
                
                /* Send MOVE to server */
                
                bzero((char *)&outmsg, sizeof(outmsg));
                outmsg.type = MOVE;
                sprintf(&outmsg.res, "%c", move-1);
                putmsg(sock, &outmsg);
                fprintf(write_to, ".c itemconfigure text%d -text \"$mySymbol\"\n", move);
                fprintf(write_to, ".c itemconfigure status -text \"Awaiting Opponent Move...\"\n");
                fprintf(write_to, "tk busy .c\n");
                fprintf(write_to, "tk busy .bf.sresign\n");
                break;
                
                case RESULT:
                    //printf("%c", inmsg.res);
                    switch (inmsg.res) {
                        case 'W':
                            for(i=0; i<9; i++){
                                board[i]=inmsg.board[i];
                                fprintf(write_to, ".c itemconfigure text%d -text %c\n" , i+1, board[i]);
                            }
                            dump_board(stdout,board);
                            printf("You win\n");
                            fprintf(write_to, ".c itemconfigure status -text \"You win!\"\n");
                            drawLine(board);
                            fprintf(write_to, "tk busy forget .bf.exit\n");
                            break;
                        case 'L':
                            for(i=0; i<9; i++){
                                board[i]=inmsg.board[i];
                                fprintf(write_to, ".c itemconfigure text%d -text %c\n" , i+1, board[i]);
                            }
                            dump_board(stdout,board);
                            printf("You lose\n");
                            fprintf(write_to, ".c itemconfigure status -text \"You lose.\"\n");
                            drawLine(board);
                            fprintf(write_to, "tk busy forget .bf.exit\n");
                            break;
                        case 'D':
                            for(i=0; i<9; i++){
                                board[i]=inmsg.board[i];
                                fprintf(write_to, ".c itemconfigure text%d -text %c\n" , i+1, board[i]);
                            }
                            dump_board(stdout,board);
                            printf("Draw\n");
                            fprintf(write_to, ".c itemconfigure status -text \"Draw.\"\n");
                            fprintf(write_to, "tk busy forget .bf.exit\n");
                            break;
                        case 'R':
                            fprintf(write_to, ".c itemconfigure status -text \"Opponent resigned.\"\n");
                            fprintf(write_to, "tk busy forget .bf.exit\n");
                        default:
                            printf("Invalid result code\n");
                            exit(1);
                    }
                    finished = 1;
                    break;
                    
                        default:
                            protocol_error(MOVE, &inmsg);
        }
    }
    return(0);
}


void
dump_board(FILE *s, char *board)
{
    fprintf(s,"%c | %c | %c\n", board[0], board[1], board[2]);
    fprintf(s,"----------\n");
    fprintf(s,"%c | %c | %c\n", board[3], board[4], board[5]);
    fprintf(s,"----------\n");
    fprintf(s,"%c | %c | %c\n", board[6], board[7], board[8]);
}

void drawLine(char *board){
    
    if ((board[0] != ' ') && (board[0]==board[4] && board[0]==board[8])){
        fprintf(write_to, ".c create line 50 50 250 250 -tag winbar -width 9 -fill {red} \n");
    }
    if((board[2] != ' ') && (board[2]==board[4] && board[2]==board[6])) { 
        fprintf(write_to, ".c create line 50 250 250 50 -tag winbar -width 9 -fill {red} \n");
    }
    if((board[0] != ' ') && (board[0]==board[1] && board[0]==board[2])) { 
        fprintf(write_to, ".c create line 50 50 250 50 -tag winbar -width 9 -fill {red} \n");
    }
    if((board[3] != ' ') && (board[3]==board[4] && board[3]==board[5])) { 
        fprintf(write_to, ".c create line 50 150 250 150 -tag winbar -width 9 -fill {red} \n");
    }
    if((board[6] != ' ') && (board[6]==board[7] && board[6]==board[8])) { 
        fprintf(write_to, ".c create line 50 250 250 250 -tag winbar -width 9 -fill {red} \n");
    }
    if((board[0] != ' ') && (board[0]==board[3] && board[0]==board[6])) { 
        fprintf(write_to, ".c create line 50 50 50 250 -tag winbar -width 9 -fill {red} \n");
    }
    if((board[1] != ' ') && (board[1]==board[4] && board[1]==board[7])) { 
        fprintf(write_to, ".c create line 150 50 150 250 -tag winbar -width 9 -fill {red} \n");
    }
    if((board[2] != ' ') && (board[2]==board[5] && board[2]==board[8])) { 
        fprintf(write_to, ".c create line 250 50 250 250 -tag winbar -width 9 -fill {red} \n");
    }
}

