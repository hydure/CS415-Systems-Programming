#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>

#define BACKLOG 0
#define MAX_LENGTH 512
#define NAME_LENGTH 64

char hostName[NAME_LENGTH], p1handle[NAME_LENGTH], p2handle[NAME_LENGTH], w1Handle[NAME_LENGTH], 
w2Handle[NAME_LENGTH], w3Handle[NAME_LENGTH], w4Handle[NAME_LENGTH];;
int listener, dgram_fd, conn1, conn2, port, pError,  activeGame = 0, max_fd, w1Connection,
w2Connection, w3Connection, w4Connection, failConnection, w1Flag = 0, w2Flag = 0,
w3Flag = 0, w4Flag = 0,  connections = 0;
socklen_t length;
fd_set sockset;

////////////////////////////////////////////////////////////////////////////////////////////////

void send_msg(char *msg, int sock){
    int left, num, put;
    left = sizeof(msg); put=0;
    
    while (left > 0){
        if((num = write(sock, msg+put, left)) < 0) {
            perror("inet_wstream:write");
            exit(1);
        }
        else{
            left -= num;
        }
        put += num;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////

void createListenerAndWriteToFile(){
    int ecode;
    FILE* file = NULL;
    struct addrinfo hints, *addrlist;
    struct sockaddr_in *localaddr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV | AI_PASSIVE; 
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL; 
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    gethostname(hostName, MAX_LENGTH);
    ecode = getaddrinfo(NULL, "0", &hints, &addrlist);
    if (ecode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ecode));
        exit(1);
    }
    
    localaddr = (struct sockaddr_in *) addrlist->ai_addr;
    
    /*Create socket on which we will accept connections. This is NOT the
     * same as the socket on which we pass data.*/
    if ((listener = socket( addrlist->ai_family, addrlist->ai_socktype, 0 )) < 0 ) {
        perror("inet_rstream:socket");
        exit(1);
    }
    if (bind(listener, (struct sockaddr *)localaddr, sizeof(struct sockaddr_in)) < 0) {
        perror("inet_rstream:bind");
        exit(1);
    }
    length = sizeof(struct sockaddr_in);
    if (getsockname(listener, (struct sockaddr *)localaddr, &length) < 0) {
        perror("inet_rstream:getsockname");
        exit(1);
    }
    port = ntohs(localaddr->sin_port);
    if ((file = fopen("serverInfo.txt", "w+")) == NULL){
        fprintf(stderr, "Bad file\n");
    }
    fprintf(file,"%d\n", port);
    fprintf(file,"%s\n", hostName);
    fclose(file);
    /*Now accept a single connection. Upon connection, data will be
     *     passed through the socket on descriptor conn. */
    if (listen(listener, BACKLOG) < 0)  {
        perror("ttt_server:init_stream:listen");
        exit(1);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////

void createDatagramAndWriteToFile(){
    
    int ecode;
    struct addrinfo hints, *addrlist; 
    struct sockaddr_in *s_in;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICSERV | AI_PASSIVE; hints.ai_protocol = 0;
    hints.ai_canonname = NULL; hints.ai_addr = NULL;
    hints.ai_next = NULL;
    
    if (gethostname(hostName, 10) < 0)  {
        perror("ttt_server:init_dgram:gethostname:");
        exit(1);
    }
    
    strncat(hostName, ".cs.wm.edu", 10);
    
    ecode = getaddrinfo(hostName, "0", &hints, &addrlist);
    if (ecode != 0) {
        fprintf(stderr, "ttt_server:init_dgram:getaddrinfo: %s\n", gai_strerror(ecode));
        exit(1);
    }
    
    s_in = (struct sockaddr_in *) addrlist->ai_addr;
    
    dgram_fd = socket (addrlist->ai_family, addrlist->ai_socktype, 0);
    if (dgram_fd < 0) {
        perror ("ttt_server:init_dgram:socket");
        exit (1);
    }
    
    if (bind(dgram_fd, (struct sockaddr *)s_in, sizeof(struct sockaddr_in)) < 0) {
        perror("ttt_server:init_dgram:bind");
        exit(1);
    }
    
    length = sizeof(struct sockaddr_in);
    if (getsockname(dgram_fd, (struct sockaddr *)s_in, &length) < 0) {
        perror("ttt_server:init_dgram:getsockname");
        exit(1);
    }
    
    FILE* file = fopen("serverInfo.txt", "a");
    fprintf(file, "%d\n", ntohs(s_in->sin_port));
    fclose(file);
}

////////////////////////////////////////////////////////////////////////////////////////////////

void accept_dgram () {
    
    char query_msg [MAX_LENGTH], buffer[128];
    int i;
    struct sockaddr_in from;
    socklen_t fsize = sizeof(from);
    
    if ((recvfrom(dgram_fd, &query_msg, sizeof(query_msg), 0, (struct sockaddr *)&from, &fsize)) < 0)
        perror("ttt_server:accept_dgram:recvfrom");
    printf("Received datagram.\n");
    fflush(stdout);
    strtok(p1handle, "\n");
    if (connections == 1){
        sprintf(buffer, "%s is currently waiting to play Tic Tac Toe.\n", p1handle);
        strcat(query_msg, buffer);
    }
    if (w1Flag){
        sprintf(buffer, "%s is first in line for the next game.\n", strtok(w1Handle, "\n"));
        strcat(query_msg, buffer);
    }
    if (w2Flag){
        sprintf(buffer, "%s is second in line for the next game.\n", strtok(w2Handle, "\n"));
        strcat(query_msg, buffer);
    }
    if (w3Flag){
        sprintf(buffer, "%s is third in line for the next game.\n", strtok(w3Handle, "\n"));
        strcat(query_msg, buffer);
    }
    if (w4Flag){
        sprintf(buffer, "%s is fourth in line for the next game.\n", strtok(w4Handle, "\n"));
        strcat(query_msg, buffer);
    }
    if (w1Flag){
        sprintf(query_msg,"\n");
        strcat(query_msg, buffer);
    }
    if (activeGame == 1){
        sprintf(query_msg,"\nThere is an active game.\nPlayer 1: %s\nPlayer 2: %s", p1handle, p2handle);
    }
    if (strcmp(query_msg, "") == 0)
        sprintf(query_msg, "No current active games or waiting players.");
    if (sendto(dgram_fd, &query_msg, sizeof(query_msg), 0, 
        (struct sockaddr *)&from, sizeof(struct sockaddr_in)) < 0) {
        perror("ttt_server:accept_dgram:sendto");
    exit(1);
        }
        printf("Datagram sent.\n");
        
}

////////////////////////////////////////////////////////////////////////////////////////////////

int accept_stream(){
    int conn;
    struct sockaddr_in peer;
    length = sizeof(peer);
    if ((conn=accept(listener, (struct sockaddr *)&peer, &length)) < 0) {
        perror("inet_rstream:accept");
        exit(1);
    }
    return conn;
}

////////////////////////////////////////////////////////////////////////////////////////////////

void setAndDetermineMaxFDGame(){
    FD_ZERO(&sockset);
    FD_SET(listener, &sockset);
    FD_SET(dgram_fd, &sockset);
    FD_SET(conn1, &sockset);
    FD_SET(conn2, &sockset);
    
    if (conn1 > conn2){
        if (conn1 > dgram_fd)
            max_fd = conn1;
    }
    else if (conn1 > conn2){
        if (conn1 < dgram_fd)
            max_fd = dgram_fd;
    }
    else{
        if (conn2 > dgram_fd)
            max_fd = conn2;
        else
            max_fd = dgram_fd;
    }
    if (listener > max_fd)
        max_fd = listener;
}

////////////////////////////////////////////////////////////////////////////////////////////////

void zeroAndSetAndDetermineMaxFD(){
    FD_ZERO(&sockset);
    FD_SET(listener, &sockset);
    FD_SET(dgram_fd, &sockset);
    
    if(listener > dgram_fd)
        max_fd = listener;
    else
        max_fd = dgram_fd;
}

////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]){
    char p1move[8], p2move[8], msg[2];
    int hits, gamePlayed = 1, p1Flag = 0, p2Flag = 0;
    
    createListenerAndWriteToFile();
    createDatagramAndWriteToFile();
    
    printf("Accepting connections...\n");
    while(1){
        zeroAndSetAndDetermineMaxFD();
        
        // select waits for some input, printing dots while waiting
        if ((hits = select(max_fd + 1, &sockset,NULL, NULL, NULL)) < 0){
            perror("select");
            exit(1);
        }
        else{
            if (FD_ISSET(dgram_fd,&sockset))
                accept_dgram();
            if (FD_ISSET(listener, &sockset)){
                // get player 1
                if (connections == 0){
                    if ((conn1 = accept_stream()) != 0){
                        //printf("Connection 1 accepted!\n");
                        connections++;
                        memset(&p1handle, 0, sizeof(p1handle));
                        // write to waitingPlayer's handle
                        if ((pError = read(conn1, p1handle, sizeof(p1handle))) < 0){
                            perror("recv");
                            exit(1);
                        }
                        if (hits == 0){
                            printf("Game closed.\n");
                            close(conn1);
                            close(conn2);
                            connections = 0;
                            gamePlayed = 1;
                            break;
                        }
                        p1Flag = 1;
                    }
                }
                // get player 2
                else if (connections == 1){
                    //printf("Connection 2 accepted!\n");
                    if ((conn2 = accept_stream()) != 0){
                        memset(&p2handle, 0, sizeof(p2handle));
                        // write to player 2's handle
                        if ((pError = read(conn2, p2handle, sizeof(p2handle))) < 0){
                            perror("recv");
                            exit(1);
                        }
                        if (hits == 0){
                            printf("Game closed.\n");
                            close(conn1);
                            close(conn2);
                            activeGame = 0;
                            connections = 0;
                            gamePlayed = 1;
                            break;
                        }
                        p2Flag = 1;
                    }
                    // start the game
                    send_msg(p2handle, conn1);
                    send_msg(p1handle, conn2);
                    write(conn1, "0", 1);
                    write(conn2, "1", 1);
                    activeGame = 1;
                    connections++;
                    printf("Player 1 handle: %s", p1handle);
                    printf("Player 2 handle: %s", p2handle);
                }
            }
            // when a game is being played
            while (activeGame){
                setAndDetermineMaxFDGame();
                if ((hits = select(max_fd + 1, &sockset,NULL, NULL, NULL)) < 0){
                    perror("select");
                    exit(1);
                }
                if (FD_ISSET(dgram_fd,&sockset)){
                    //printf("Yay a datagram!");
                    accept_dgram();
                }
                if(FD_ISSET(listener, &sockset)){
                    if (!w1Flag){
                        if ((w1Connection = accept_stream()) != 0){
                            memset(&w1Handle, 0, sizeof(w1Handle));
                            // write to waiter's handle
                            if ((pError = read(w1Connection, w1Handle, sizeof(w1Handle))) < 0){
                                perror("recv");
                                exit(1);
                            }
                            if (hits == 0){
                                printf("Waiting player 1 closed.\n");
                                close(w1Connection);
                                break;
                            }
                            w1Flag = 1;
                            send_msg("-w", w1Connection);
                        }
                    }
                    else if (!w2Flag){
                        if ((w2Connection = accept_stream()) != 0){
                            memset(&w2Handle, 0, sizeof(w2Handle));
                            // write to waiter's handle
                            if ((pError = read(w2Connection, w2Handle, sizeof(w2Handle))) < 0){
                                perror("recv");
                                exit(1);
                            }
                            if (hits == 0){
                                printf("Waiting player 2 closed.\n");
                                close(w2Connection);
                                break;
                            }
                            w2Flag = 1;
                            send_msg("-w", w2Connection);
                        }
                    }
                    else if (!w3Flag){
                        if ((w3Connection = accept_stream()) != 0){
                            memset(&w3Handle, 0, sizeof(w3Handle));
                            // write to waiter's handle
                            if ((pError = read(w3Connection, w3Handle, sizeof(w3Handle))) < 0){
                                perror("recv");
                                exit(1);
                            }
                            if (hits == 0){
                                printf("Waiting player 3 closed.\n");
                                close(w3Connection);
                                break;
                            }
                            w3Flag = 1;
                            send_msg("-w", w3Connection);
                        }
                    }
                    else if (!w4Flag){
                        if ((w4Connection = accept_stream()) != 0){
                            memset(&w4Handle, 0, sizeof(w4Handle));
                            // write to waiter's handle
                            if ((pError = read(w4Connection, w4Handle, sizeof(w4Handle))) < 0){
                                perror("recv");
                                exit(1);
                            }
                            if (hits == 0){
                                printf("Waiting player 4 closed.\n");
                                close(w4Connection);
                                break;
                            }
                            w4Flag = 1;
                            send_msg("-w", w4Connection);
                        }
                    }
                    // Send a message to tell them they cannot connect
                    else{
                        if ((failConnection = accept_stream()) != 0){
                            send_msg("-f", failConnection);
                            close(failConnection);
                        }
                        
                    }
                }// end of if(FD_ISSET(listener, &sockset))
                if (FD_ISSET(conn1, &sockset)){
                    if ((hits = read(conn1, p1move, sizeof(p1move))) < 0){
                        perror("recv LOL!");
                        exit(1);
                    }
                    
                    if (hits == 0){
                        printf("Game ended.\n");
                        close(conn1);
                        close(conn2);
                        activeGame = 0;
                        connections = 0;
                        gamePlayed = 1;
                        break;
                    }
                    strtok(p1move, "\n");
                    printf("Player 1 move: %s\n", p1move);
                    
                    send_msg(p1move, conn2);
                    
                }
                
                if (FD_ISSET(conn2, &sockset)){
                    if ((hits = read(conn2, p2move, sizeof(p2move))) < 0 ){
                        perror("recv LOL?");
                        exit(1);
                    }
                    
                    if (hits == 0){
                        printf("Game ended.\n");
                        close(conn1);
                        close(conn2);
                        activeGame = 0;
                        connections = 0;
                        gamePlayed = 1;
                        break;
                    }
                    strtok(p2move, "\n");
                    printf("Player 2 move: %s\n", p2move);
                    send_msg(p2move, conn1);
                } 
            }// end of while (activeGame)
            fflush(stdout);
        }// end of huge else statement (directly after select())
        if (gamePlayed == 1){
            if (w1Flag){
                conn1 = w1Connection; // replace connection
                printf("Connection 1 replaced.\n");
                memset(&p1handle, 0, sizeof(p1handle)); 
                sprintf(p1handle, w1Handle); // replace handle
                if (w3Flag){
                    w1Connection = w3Connection;
                    memset(&w1Handle, 0, sizeof(w1Handle));
                    sprintf(w1Handle, w3Handle);
                    w3Flag = 0;
                }
                else
                    w1Flag = 0;
            }
            else
                p1Flag = 0;
            if (w2Flag){
                conn2 = w2Connection;
                printf("Connection 2 replaced.\n");
                memset(&p2handle, 0, sizeof(p2handle));
                sprintf(p2handle, w2Handle);
                if (w4Flag){
                    w2Connection = w4Connection;
                    memset(&w2Handle, 0, sizeof(w2Handle));
                    sprintf(w2Handle, w4Handle);
                    w4Flag = 0;
                }
                else
                    w2Flag = 0;
            }
            else
                p2Flag = 0;
        }
    }// end of the daemon while loops
    unlink("serverInfo.txt");
    exit(0);
}// end of main
