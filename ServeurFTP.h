#ifndef __SERVEURFTP_H
#define __SERVEURFTP_H

#include "csapp.h"
#include "readcmd.h"

void echoF(int connfd);

void connectionTraitant(int listenfd, struct sockaddr_in clientaddr, socklen_t clientlen, char * client_hostname, char * client_ip_string);

#endif
