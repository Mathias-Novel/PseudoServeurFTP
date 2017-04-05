/*
 * ClientFTP.c
 */
 #include "ConstanteFTP.h"

int clientGlob;

void handlerSigInt(int sig) {
    Rio_writen(clientGlob, COMMANDE_BYE, BUFFER_SIZE);
    exit(0);
}

void lire(char * dest, int taille, FILE * stream) {
  fgets(dest,taille,stream);
  char * retourChariot = malloc(1);
  if ( (retourChariot = strchr(dest,'\n')) != NULL) {
    *retourChariot = '\0';
  }

  //free(retourChariot);
}

int authentification(rio_t rio, int clientfd) {
  char buf[MAX_NAME_LEN];

  //Attente de la demande d'authentification
  Rio_readnb(&rio, buf, BUFFER_SIZE);
  if (strcmp(buf,DEMANDE_AUTHENTIFICATION) != 0) {
    return 0;
  }

  //Envoie de l'user
  printf("UserName : \n");
  //scanf("%s",buf);
  lire(buf,MAX_NAME_LEN,stdin);
  Rio_writen(clientfd, buf, BUFFER_SIZE);

  //Verification de l'existence
  Rio_readnb(&rio, buf, BUFFER_SIZE);
  if (strcmp(buf,ACK) != 0) {
    printf("%s\n",buf);
    return 0;
  }
  printf(" - OK\n");

  //Envoie du mot de passe
  printf("PassWord : \n");
  //scanf("%s",buf);
  lire(buf,MAX_NAME_LEN,stdin);

  Rio_writen(clientfd, buf, BUFFER_SIZE);

  //Verification du mot de passe
  Rio_readnb(&rio, buf, BUFFER_SIZE);
  if (strcmp(buf,ACK) != 0) {
    return 0;
  }
  printf(" - OK\n");

  return 1;
}

void commandeGet(int clientfd, rio_t rio, char * filename) {
  if (filename == NULL) {
      printf("usage get <filename>\n");
  } else {
    char buf[MAXLINE];
    int n, fichierTaille, tailleRestante;
    clock_t debut, fin, tempsTotal;

    //On envoie la commande
    Rio_writen(clientfd, COMMANDE_GET, MAX_NAME_LEN);
    //Et son argument
    Rio_writen(clientfd, filename, MAX_NAME_LEN);

    //On recupere la taille du fichier
    Rio_readnb(&rio, buf, BUFFER_SIZE);


    //Si le fichier existe
    if ((fichierTaille = atoi(buf)) >= 0) {

      //Ouverture du fichier de sorite
      FILE * sortie = fopen("sortieClient","w+");

      tailleRestante = fichierTaille;
      debut = clock();
      //Lecture de la reponse
      while((n = Rio_readnb(&rio, buf, BUFFER_SIZE)) > 0 && tailleRestante - n > 0) {
        Fputs(buf, sortie);
        tailleRestante-= n;
      }
      //Ecriture d'une partie du dernier paquet recupere
      for (int i = 0; i < tailleRestante; i++) {
        fputc(buf[i],sortie);
      }
      fin = clock();

      tempsTotal = fin - debut;
      if (tempsTotal/1000 == 0) {
        printf("le fichier %s de %d octet(s) a ete telecharge en %d seconde(s) (%d octet(s)/seconde)\n\n",filename, fichierTaille, 1, fichierTaille);
      } else {
        printf("le fichier %s de %d octet(s) a ete telecharge en %ld seconde(s) (%ld ko/seconde)\n\n",filename, fichierTaille, tempsTotal / 1000000, (fichierTaille/(tempsTotal/1000)));
      }

      //Fermeture du fichier
      fclose(sortie);
    } else {
      printf("Ce fichier n'existe pas\n");
    }
  }
}

void commandeLs(int clientfd, rio_t rio) {
  char buf[MAXLINE];

  //Envoie de la commande au serveur
  Rio_writen(clientfd, COMMANDE_LS, MAX_NAME_LEN);

  //Le serveur peut il repondre a cette requette ?
  Rio_readnb(&rio, buf, BUFFER_SIZE);
  if (strcmp(buf,ACK) == 0) {

    Rio_readnb(&rio, buf, BUFFER_SIZE); //lecture de la taille
    while(atoi(buf) > 0) { //tant que la taille de la prochaine ligne est superieur a 0
      Rio_readnb(&rio, buf, BUFFER_SIZE); //une ligne du ls
      Fputs(buf, stdout);
      fflush(stdout);
      Rio_readnb(&rio, buf, BUFFER_SIZE); //tailee de la prochaine ligne
    }


  } else {
    printf("Impossible d'effectuer cette commande\n");
  }
}

void commandePwd(int clientfd, rio_t rio) {
  char buf[BUFFER_SIZE];

  //Envoie de la commande au serveur
  Rio_writen(clientfd, COMMANDE_PWD, BUFFER_SIZE);


  Rio_readnb(&rio, buf, BUFFER_SIZE);
  if (strcmp(buf,NACK) != 0) {
    Fputs(buf, stdout);
    fflush(stdout);
  } else {
    printf("Erreur cote serveur");
  }

}

void commandeCd(int clientfd, rio_t rio, char * destination) {
  char buf[BUFFER_SIZE];

  //Envoie de la commande au serveur
  Rio_writen(clientfd, COMMANDE_CD, BUFFER_SIZE);
  //Envoie de la destination au serveur
  Rio_writen(clientfd, destination, BUFFER_SIZE);

  Rio_readnb(&rio, buf, BUFFER_SIZE);
  if (strcmp(buf,ACK) != 0) {
    printf("Operation non accomplie\n");
  } else {
    printf("Operation reussie\n");
  }
}

void commandeRmR(int clientfd, rio_t rio, char * dirName) {
  char buf[BUFFER_SIZE];

  //Verification du nom
  if (dirName == NULL) {
    printf("usage rm -r <nom du repertoire>\n");
    return;
  }

  //Envoie de la commande au serveur
  Rio_writen(clientfd, COMMANDE_RM_R, BUFFER_SIZE);

  //Envoie du nom du repertoire
  Rio_writen(clientfd, dirName, BUFFER_SIZE);

  //Reception de la reponse
  Rio_readnb(&rio, buf, BUFFER_SIZE);
  if (strcmp(buf,ACK) != 0) {
    printf("Operation non accomplie\n");
  } else {
    printf("Operation reussie\n");
  }

}

void commandeRm(int clientfd, rio_t rio, char * filename) {
  char buf[BUFFER_SIZE];

  //Verification du nom
  if (filename == NULL) {
    printf("usage rm <nom du fichier>\n");
    return;
  }

  //Envoie de la commande au serveur
  Rio_writen(clientfd, COMMANDE_RM, BUFFER_SIZE);

  //Envoie du nom du fichier
  Rio_writen(clientfd, filename, BUFFER_SIZE);

  //Reception de la reponse
  Rio_readnb(&rio, buf, BUFFER_SIZE);
  if (strcmp(buf,ACK) != 0) {
    printf("Operation non accomplie\n");
  } else {
    printf("Operation reussie\n");
  }
}

void commandeMkdir(int clientfd, rio_t rio, char * dirName) {
  char buf[BUFFER_SIZE];

  //Verification du nom
  if (dirName == NULL) {
    printf("usage mkdir <nom du repertoire>\n");
    return;
  }

  //Envoie de la commande au serveur
  Rio_writen(clientfd, COMMANDE_MKDIR, BUFFER_SIZE);

  //Envoie du nom du fichier
  Rio_writen(clientfd, dirName, BUFFER_SIZE);

  //Reception de la reponse
  Rio_readnb(&rio, buf, BUFFER_SIZE);
  if (strcmp(buf,ACK) != 0) {
    printf("Operation non accomplie\n");
  } else {
    printf("Operation reussie\n");
  }
}

void commandePut(int clientfd, rio_t rio, char * filename) {
  char buf[BUFFER_SIZE];
  int n;

  //Verification du nom
  if (filename == NULL) {
    printf("usage rm <nom du fichier>\n");
    return;
  }

  //Envoie de la commande au serveur
  Rio_writen(clientfd, COMMANDE_PUT, BUFFER_SIZE);

  //Envoie du nom du fichier
  Rio_writen(clientfd, filename, BUFFER_SIZE);

  //Ouverture du fichier
  FILE * fichier = fopen(filename, "r");

  printf("Ouverture du fichier\n");
  //Envoie de la taille du fichier, -1 si le fichier n'existe pas
  if (fichier == NULL) {
    printf("Fichier inexistant\n");
    strcpy(buf,"-1");
    Rio_writen(clientfd, buf, BUFFER_SIZE);
    return 1;
  } else {
    sprintf(buf, "%d",(int) getTailleFichier(filename));
    Rio_writen(clientfd, buf, BUFFER_SIZE);
  }

  printf("Debut de l'envoie du fichier\n");
  //Ecriture du fichier
  int octets = 0;
  while ( (n = fread(buf, 1, BUFFER_SIZE, fichier)) > 0) {
      octets+=n;
      Rio_writen(clientfd, buf, BUFFER_SIZE);
  }

  printf("Fichier %s correctement envoye (%d octets envoyes)\n", filename, octets);

  fclose(fichier);
}

int main(int argc, char **argv)
{
    int clientfd;
    char *host;
    rio_t rio;
    int boucle = 1;
    struct cmdline *l;

    Signal(SIGINT, handlerSigInt);

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, PORT);
    clientGlob = clientfd;

    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    printf("client connected to server OS\n");



    //Initialisation de rio
    Rio_readinitb(&rio, clientfd);


    if (!authentification(rio, clientfd)) {
      exit(0);
    }

    while (boucle) {
        //Lecture de la commande
        printf("FTP > ");
        l = readcmd();


        if ( l->seq[0][0] != NULL) {
          //Commande get
          if (strcmp("get", l->seq[0][0]) == 0) {
            commandeGet(clientfd, rio, l->seq[0][1]);

          //Commande ls
        }else if (strcmp("ls", l->seq[0][0]) == 0) {
            commandeLs(clientfd, rio);

          //Commande ls
        } else if (strcmp("pwd", l->seq[0][0]) == 0) {
          commandePwd(clientfd, rio);

          //Commande cd
        } else if (strcmp("cd", l->seq[0][0]) == 0) {
          commandeCd(clientfd, rio, l->seq[0][1]);

          //Commande rm
        } else if (strcmp("rm", l->seq[0][0]) == 0) {
          if (l->seq[0][1] != NULL && strcmp("-r", l->seq[0][1]) == 0) {
            commandeRmR(clientfd, rio, l->seq[0][2]);
          } else {
            commandeRm(clientfd, rio, l->seq[0][1]);
          }

          //Commande mkdir
        } else if (strcmp("mkdir", l->seq[0][0]) == 0) {
          commandeMkdir(clientfd, rio, l->seq[0][1]);

          //Commande put
        } else if (strcmp("put", l->seq[0][0]) == 0) {
          commandePut(clientfd, rio, l->seq[0][1]);

          //Commande bye
        } else if (strcmp("bye" , l->seq[0][0]) == 0) {
            //On previent le serveur
            Rio_writen(clientfd, COMMANDE_BYE, MAX_NAME_LEN);
            //Et on prepare la sortie de la boucle
            boucle = 0;


        //Autre
        } else {
          printf("Commande inconue\n");
        }


        }
    }

    printf("JE SORT DE LA BOUCLE\n");

    //Fermeture du client
    Close(clientfd);
    exit(0);
}
