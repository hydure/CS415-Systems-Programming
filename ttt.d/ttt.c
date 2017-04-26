#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>     
#include <arpa/inet.h>   
#include <sys/time.h>

char hostName[64], streamPortNumber[8],  dgramPortNumber[8], *pError;
int listener, tFlag = 0;
struct timeval timeout;
fd_set sockset;

/////////////////////////////////////////////////////////////////////////////////////////////////

void sendAndRecvDgram() {
    
    char datagram[3] = "";
    int socket_fd, cc, ecode;
    struct sockaddr_in *s_in;
    struct addrinfo hints, *addrlist;  
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICSERV; hints.ai_protocol = 0;
    hints.ai_canonname = NULL; hints.ai_addr = NULL;
    hints.ai_next = NULL;
    
    ecode = getaddrinfo(hostName, dgramPortNumber, &hints, &addrlist);
    if (ecode != 0) {
        fprintf(stderr, "ttt:sendAndRecvDgram:getaddrinfo: %s\n", gai_strerror(ecode));
        exit(1);
    }
    
    s_in = (struct sockaddr_in *) addrlist->ai_addr; 
    socket_fd = socket (addrlist->ai_family, addrlist->ai_socktype, 0);
    if (socket_fd < 0) {
        perror ("ttt:sendAndRecvDgram:socket");
        exit (1);
    }
    
    if (sendto(socket_fd,&datagram,sizeof(datagram),0,(struct sockaddr *) s_in,
        sizeof(struct sockaddr_in)) < 0){
        perror("ttt:send_dgram:sendto");
    exit(1);
        }
        printf("Sent a datagram query.\n");
        char msg [1024];
        
        socklen_t fsize;
        struct sockaddr_in from;
        
        fsize = sizeof(from);
        
        fd_set mask;
        
        FD_ZERO(&mask);
        FD_SET(socket_fd,&mask);
        
        struct timeval t;
        t.tv_sec = 60;
        
        
        cc = recvfrom(socket_fd, &msg, sizeof(msg), 0, (struct sockaddr *)&from, &fsize);
        if (cc < 0) 
            perror("ttt:sendAndRecvDgram:recvfrom");
        printf("Received reply from server.\n\n");
        printf("%s\n",msg);
        fflush(stdout);
        
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int hasWinner(char board[9]) {
    int line;
    
    if ((board[0] != ' ') && (board[0]==board[4] && board[0]==board[8])){
        return 1;
    }
    if((board[2] != ' ') && (board[2]==board[4] && board[2]==board[6])) { 
        return 1; 
    }
    for (line = 0; line <=2; line++) {
        if((board[3 * line] != ' ') && (board[3 * line]==board[3 * line + 1] && 
            board[3 * line]==board[3 * line + 2])){
            return 1;
            }
    }
    for (line = 0; line <=2; line++) {
        if((board[line] != ' ') && (board[line]==board[line + 3] && 
            board[line]==board[line + 6])){
            return 1;
            }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int tie(char board[9]) {
    int i, count = 0;
    for (i = 0; i < 9; i++){
        if(board[i] != ' '){
            count++;
        }
    }
    if (count == 9)
        return 1;
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void send_msg(char *msg){
    int left, num, put;
    left = sizeof(msg); put=0;
    
    while (left > 0){
        if((num = write(listener, msg+put, left)) < 0) {
            perror("inet_wstream:write");
            exit(1);
        }
        else{ 
            left -= num;
        }
        put += num;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void printBoard(char board[9]){
    printf("\n");
    int i;
    for (i = 0; i < 9; i++){
        if (i % 3 == 0 || i % 3 == 1){
            printf(" %c |", board[i]);
        }
        else{
            printf(" %c\n", board[i]);
        }
        if( i==2 || i ==5){
            printf("---|---|---\n");
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void readFromFile(){
    FILE* file;
    if ((file = fopen("serverInfo.txt", "r")) == NULL){
        sleep(60);
        if((file = fopen("serverInfo.txt", "r")) == NULL){
            printf("Client can't access file.\n");
            exit(1);
        }
    }
    
    if ((pError = fgets(streamPortNumber, sizeof(streamPortNumber), file)) != NULL){
        strtok(streamPortNumber, "\n");
    }
    else{
        fprintf(stderr, "Could not get streamPortNumber.\n");
        exit(1);
    }
    //printf("stream port: %s\n", streamPortNumber);
    if ((pError = fgets(hostName, sizeof(hostName), file)) != NULL){
        strtok(hostName, "\n");
    }
    else{
        fprintf(stderr, "Could not get hostName number.\n");
        exit(1);
    }
    //printf("host name: %s\n", hostName);
    if ((pError = fgets(dgramPortNumber, sizeof(dgramPortNumber), file)) != NULL){
        strtok(dgramPortNumber, "\n");
    }
    else{
        fprintf(stderr, "Could not get dgramPortNumber.\n");
        exit(1);
    }
    //printf("datagram port: %s\n", dgramPortNumber);
    fclose(file);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void connectToServer(){
    int ecode;
    struct sockaddr_in *server;
    struct addrinfo hints, *addrlist;
    
    /* Want a sockaddr_in containing the ip address for the system
     *  specified in argv[1] and the streamPortNumber specified in argv[2]. */
    memset( &hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV; hints.ai_protocol = 0;
    hints.ai_canonname = NULL; hints.ai_addr = NULL;
    hints.ai_next = NULL;
    
    ecode = getaddrinfo(hostName, streamPortNumber, &hints, &addrlist);
    if (ecode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ecode));
        exit(1);
    }
    
    server = (struct sockaddr_in *) addrlist->ai_addr;
    
    
    // Create the socket.
    if ((listener = socket( addrlist->ai_family, addrlist->ai_socktype, 0 )) < 0 ) {
        perror("inet_wstream:socket");
        exit(1);
    }
    
    // Connect to data socket on the peer at the specified Internet address.
    if (connect(listener, (struct sockaddr *)server, sizeof(struct sockaddr_in)) < 0) {
        perror("inet_wstream:connect");
        exit(1);
    }
    // if the timeout argument is chosen
    if (tFlag){
        FD_ZERO(&sockset);
        FD_SET(listener, &sockset);
        switch (select(listener+1, &sockset, NULL, NULL, &timeout)){
            case -1:
                err(1, "select");
                close(listener);
                exit(1);
            case 0:
                printf("Connection timed out.\n");
                close(listener);
                exit(0);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv []){
    
    char player1Handle[21] = "", player2Handle[21] = "\n", board[9], choice[4], 
    buffer[1024],  c, *pointer, msg[64];
    int hits, numChoice, validMove = 0, symbol = 0, queryMode = 0, cFlag, i = 0;
    memset(&board, ' ', sizeof(board));
    
    
    //while ((c = getopt(argc, argv, "q::t::")) != -1){
    
    if (argc == 1);	
    else if (argc == 2 && strcmp(argv[1], "-q") == 0)
        queryMode = 1;
    else if (argc == 4 && strcmp(argv[1], "-q") == 0 && strcmp(argv[2], "-t") == 0)  {
        timeout.tv_sec = (int) strtol(argv[3], (char **) NULL, 10);
        queryMode = 1;
    }
    else if (argc == 4 && strcmp(argv[1], "-t") == 0 && strcmp(argv[3], "-q") == 0)  {
        timeout.tv_sec = (int) strtol(argv[2], (char **) NULL, 10);
        tFlag == 1;
        queryMode = 1;
    }
    else if (argc == 3 && strcmp(argv[1], "-t") == 0){
        timeout.tv_sec = (int) strtol(argv[2], (char **) NULL, 10);
        printf("You want to wait for %d seconds.\n", timeout.tv_sec);
        tFlag = 1;
    }
    else  {
        fprintf(stderr, "Argument syntax is invalid.\n");
        exit(1);
    }
    
    readFromFile();
    
    if (queryMode)
        sendAndRecvDgram();
    else{
        connectToServer();
        printf("Connected to server.\n");
        while (!validMove){
            printf("Your Handle: ");
            fgets(player1Handle, sizeof(player1Handle), stdin);
            if (strcmp(player1Handle, player2Handle) != 0)
                validMove = 1;
            else
                printf("Need to input a name less than 20 characters.\n");
        }
        validMove = 0;
        send_msg(player1Handle);
        
        // if the timeout argument is chosen
        if (tFlag){
            FD_ZERO(&sockset);
            FD_SET(listener, &sockset);
            switch (select(listener+1, &sockset, NULL, NULL, &timeout)){
                case -1:
                    err(1, "select");
                    close(listener);
                    exit(1);
                case 0:
                    printf("Connection timed out.\n");
                    close(listener);
                    exit(0);
            }
        }
        //printf("Opponent's Handle: ");
        while((hits = read(listener, &c, 1)) == 1){
            memset(&msg, 0, sizeof(msg));
            sprintf(msg, "%c", c);
            if (c == '\n')
                break;
            if (c == '-')
                cFlag = 1;
            if (cFlag == 1 && c == 'w')
                printf("Game in session. Waiting for next game to start...\n");
            if (cFlag == 1 && c == 'f'){
                printf("Server is full. Try again later.\n");
                close(listener);
                exit(0);
            }
            
        }
        printf("Opponent's Handle: %s\n", msg);
        if (hits == 0){
            printf("Connection closed.\n");
            close(listener);
            exit(1);
        }
        
        while((hits = read(listener, &c, 1)) == 1){
            if (c == '0'){
                printf("You are X\n");
                symbol = 1;
                printBoard(board);
                while(!validMove){
                    printf("Choose a tile [1 - 9]: ");
                    fgets(choice, sizeof(choice), stdin);
                    numChoice = atoi(choice);
                    if (numChoice > 9 || numChoice < 1)
                        printf("\nInvalid move. Need to choose [1,9].\n");
                    else if (board[numChoice-1] != ' ')
                        printf("\nInvalid move. Space already taken.\n");
                    else
                        validMove = 1;
                }
                board[numChoice-1] = 'X';
                strtok(choice, "\n");
                printf("Your move: %s\n", choice);
                printBoard(board);
                send_msg(choice);
                //printf("Sent message to server.\n");
                break;
            }      
            if (c == '1'){
                printf("You are O\n");
                break;
            }
        }// end of while((hits... loop
        if (hits == 0){
            printf("Connection closed.\n");
            close(listener);
            exit(1);
        }
        
        
        while(1){
            
            if((hits = read(listener,buffer, sizeof(buffer))) > 0){ //make <=0 case
                numChoice = (int) strtol(buffer, &pointer, 0);
                printf("You chose: %d", numChoice);
                board[numChoice-1] =  (symbol == 1) ? 'O' : 'X';
                validMove = 0;      
                printBoard(board);
                if (hasWinner(board)){
                    printf("You lose.\n");
                    close(listener);
                    exit(0);
                }
                if (tie(board)){
                    printf("Tie. A strange game. The only winning move is not to play.\n");
                    close(listener);
                    exit(0);
                }
                
                while(!validMove){
                    printf("Choose a tile [1 - 9]: ");
                    fgets(choice, sizeof(choice), stdin);
                    numChoice = (int) strtol(choice, &pointer, 0);
                    printf("You chose: %d\n", numChoice);
                    if(numChoice < 10 && numChoice > 0 && board[numChoice-1] == ' ')
                        validMove = 1;
                    else
                        printf("\nInvalid move.\n");
                }
                board[numChoice-1] =  (symbol == 1) ? 'X' : 'O';
                strtok(choice, "\n");
                printf("Your move: %s\n", choice);
                printBoard(board);
                send_msg(choice);
                if (hasWinner(board)){
                    printf("Yay you win!\n");
                    close(listener);
                    exit(0);
                }
                if (tie(board)){
                    printf("Tie. A strange game.The only winning move is not to play.\n");
                    close(listener);
                    exit(0);
                }
            }// end of if(read(listener... statement
            else if (hits == 0){
                printf("Connection1 closed.\n");
                close(listener);
                exit(1);
            }
        }// end of while(1) loop
    }// end of else statement
    close(listener);
    exit(0);
}// end of main

