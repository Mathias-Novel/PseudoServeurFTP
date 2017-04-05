#ifndef __CONSTANTEFTP_H
#define __CONSTANTEFTP_H

#include "csapp.h"
#include "readcmd.h"
#include "ConstanteFTP.h"

#include <time.h>
#include <dirent.h>


#define NBPROC 3

#define MAX_NAME_LEN 128
#define BUFFER_SIZE 128
#define PORT 2121

#define ACK "1"
#define NACK "0"

#define COMMANDE_GET "20"

#define COMMANDE_BYE "50"

#define COMMANDE_LS "60"
#define COMMANDE_PWD "61"
#define COMMANDE_CD "62"

#define COMMANDE_RM "70"
#define COMMANDE_RM_R "71"
#define COMMANDE_MKDIR "72"
#define COMMANDE_PUT "73"

#define DEMANDE_AUTHENTIFICATION "80"

size_t getTailleFichier(char * filename);

#endif
