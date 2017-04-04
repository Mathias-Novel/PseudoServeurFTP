/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"

#define MAX_NAME_LEN 256

void echo(int connfd);

void handlerSigChild(int sig)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) != -1)
    printf("Handler reaped child %d\n", (int)pid);
    return;
}

/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv)
{
    Signal(SIGCHLD, handlerSigChild);
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    pid_t pid;
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);
    
    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);
    while (1) {
        
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        if ((pid = Fork()) == 0) {
            close(listenfd);

            /* determine the name of the client */
            Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);
            
            /* determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);
            
            printf("server connected to %s (%s)\n", client_hostname,client_ip_string);

            echo(connfd);
            Close(connfd);
            exit(0);
        } else {
            Close(connfd);
        }

        
    }
    exit(0);
}

