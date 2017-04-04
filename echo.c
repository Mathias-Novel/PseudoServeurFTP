/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"

/* Renvoi la taille du message reçu de la part du client au client */
void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %u bytes\n", (unsigned int)n);
        Rio_writen(connfd, buf, n);
    }
}

/* Renvoi du fichier, designe par le client, présent sur le serveur vers le client */
void echoF(int connfd) {
    char buf[MAXLINE];
    size_t n;
    rio_t rio;

    Rio_readinitb(&rio, connfd);

    //Recuperer le nom du fichier
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("Server received filename : %s\n", buf);

    char * filename = malloc(strlen(buf));
    strcpy(filename,buf);
    filename[strlen(filename) - 1] = '\0';

    //Ouverture du fichier    
    int fdin = Open(filename, O_RDONLY, 0);
    
    //Ecriture du fichier
    while ( (n = Read(fdin, buf, strlen(buf))) > 0) {
        Rio_writen(connfd, buf, n);
    }

    printf("Fichier %s correctement envoye\n", filename);

    Close(fdin);
    free(buf);
    free(filename);

    printf("COMM\n");
}