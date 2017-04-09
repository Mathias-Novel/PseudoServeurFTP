#include "ConstanteFTP.h"
#include "ServeurFTP.h"

pid_t pids[NBPROC];
pid_t pidPere;

void handlerSigChild(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0);
}

void handlerSigInt(int sig) {
    if (pidPere == getpid()) {
        for (int i =0; i < NBPROC; i++) {
            kill(pids[i],SIGINT);
        }
    }
    exit(0);
}

int commandeGet(int connfd, rio_t rio) {
  /*char * filename = malloc(strlen(arg));
  strcpy(filename,arg);*/
  char buf[MAX_NAME_LEN];
  size_t n;
  char * filename;
  long int offset;

  //Recuperation de(s) argument(s) (nottament le nom du fichier)
  Rio_readnb(&rio, buf, MAX_NAME_LEN);
  printf("with arg(s) : _%s_\n", buf);
  filename = malloc(sizeof(buf));
  strcpy(filename, buf);

  //Ouverture du fichier
  FILE * fichier = fopen(filename, "r");


  printf("Ouverture du fichier\n");
  //Envoie de la taille du fichier, -1 si le fichier n'existe pas
  if (fichier == NULL) {
    printf("Fichier inexistant\n");
    strcpy(buf,"-1");
    Rio_writen(connfd, buf, BUFFER_SIZE);
    return 1;
  } else {
    sprintf(buf, "%d",(int) getTailleFichier(filename));
    Rio_writen(connfd, buf, BUFFER_SIZE);
  }

  //On recoit la taille du fichier deja present chez le client
  //Si il n'existe pas alors o recoit -1 et on commence a envoyer des le debut du fichier
  //Sinon on envoie que la partie manquante
  Rio_readnb(&rio, buf, MAX_NAME_LEN);
  if ((offset = atoi(buf)) == -1) {
    offset = 0;
  } else {
    printf("Reprise après panne avec l'offset %d\n",offset);
  }
  fseek(fichier, offset, SEEK_SET);

  printf("Debut de l'envoie du fichier\n");
  //Ecriture du fichier
  int octets = 0;
  while ( (n = fread(buf, 1, BUFFER_SIZE, fichier)) > 0) {
      octets+=n;
      Rio_writen(connfd, buf, BUFFER_SIZE);
  }

  printf("Fichier %s correctement envoye (%d octets envoyes)\n", filename, octets);

  fclose(fichier);


  return 1;
}

void affichePermissions(struct stat stat,int connfd, rio_t rio, char * buf) {
  int i = 0;
  if (stat.st_mode & S_IRUSR) buf[i] = 'r'; else buf[i] = '-'; i++;
  if (stat.st_mode & S_IWUSR) buf[i] = 'w'; else buf[i] = '-'; i++;
  if (stat.st_mode & S_IXUSR) buf[i] = 'x'; else buf[i] = '-'; i++;

  if (stat.st_mode & S_IRGRP) buf[i] = 'r'; else buf[i] = '-'; i++;
  if (stat.st_mode & S_IWGRP) buf[i] = 'w'; else buf[i] = '-'; i++;
  if (stat.st_mode & S_IXGRP) buf[i] = 'x'; else buf[i] = '-'; i++;

  if (stat.st_mode & S_IROTH) buf[i] = 'r'; else buf[i] = '-'; i++;
  if (stat.st_mode & S_IWOTH) buf[i] = 'w'; else buf[i] = '-'; i++;
  if (stat.st_mode & S_IXOTH) buf[i] = 'x'; else buf[i] = '-'; i++;


}

int commandeLs(int connfd, rio_t rio) {
  DIR * repertoireCourant;
  struct dirent * rep;
  struct stat fichierStat;
  char buf[BUFFER_SIZE];
  char tailleLigne[BUFFER_SIZE];
  int i;


  repertoireCourant = opendir(".");

  //Si il n'y a pas eu de probleme durant l'ouverture
  if (repertoireCourant)  {
    Rio_writen(connfd, ACK, BUFFER_SIZE);

    //printf("================ Commande ls =============\n");
    while ((rep = readdir(repertoireCourant)) != NULL)    {
      memset(buf, '\0', BUFFER_SIZE);
      //On ouvre l'element avec stat afin d'avoir le plus d'informations possible dessus
      stat(rep->d_name, &fichierStat);

      //affichage des permissions
      affichePermissions(fichierStat, connfd, rio, buf);
      i = 9;

      //Affichage de repertoire ou fichier
      if(S_ISDIR(fichierStat.st_mode)) {
        strcat(buf," repertoire\t");
        i+= 12;
      } else if (S_ISREG(fichierStat.st_mode)) {
        strcat(buf," fichier\t");
        i+= 9;
      }

      //Affichage du nom
      //strcat(buf,rep->d_name);
      for (int j = 0; j < strlen(rep->d_name); j++) {
        buf[i] = rep->d_name[j];
        i++;
      }
      buf[i] = '\n';
      buf[i+1] = '\0';

      printf("%s",buf);
      fflush(stdout);

      //Envoie de la taille de la ligne
      sprintf(tailleLigne, "%d",i);
      //printf("Taille de la ligne %s\n",tailleLigne);
      Rio_writen(connfd, tailleLigne, BUFFER_SIZE);
      //Puis de la ligne
      Rio_writen(connfd, buf, BUFFER_SIZE);

    }
    closedir(repertoireCourant);
    //printf("==========================================\n");

  } else {
    Rio_writen(connfd, NACK, BUFFER_SIZE);
  }

  sprintf(tailleLigne, "%d",0);
  printf("Taille de la ligne %s\n",tailleLigne);
  Rio_writen(connfd, tailleLigne, BUFFER_SIZE);

  return 1;
}

int commandePwd(int connfd, rio_t rio) {
  char buf[MAX_NAME_LEN];
  if (getcwd(buf, MAX_NAME_LEN) != NULL) {
    strcat(buf,"\n\0");
    Rio_writen(connfd, buf, BUFFER_SIZE);
  } else {
    Rio_writen(connfd, NACK, BUFFER_SIZE);
  }

  return 1;
}

int commandeCd(int connfd, rio_t rio) {
  char destination[BUFFER_SIZE];
  struct stat destinationStat;
  int retour;

  //Lecture de la destination
  Rio_readnb(&rio, destination, BUFFER_SIZE);

  //Si la destination n'existe pas
  retour = stat(destination, &destinationStat);
  if (retour == -1) {
    Rio_writen(connfd, NACK, BUFFER_SIZE);
    printf("La destination n'existe pas\n");
    return 1;
  }

  //Si la destination n'est pas un repertoire
  if (!S_ISDIR(destinationStat.st_mode)) {
    printf("La destination n'est pas un repertoire\n");
    Rio_writen(connfd, NACK, BUFFER_SIZE);
    return 1;
  }

  //On tente de changer de repertoire
  retour = chdir(destination);
  if (retour == -1) {
    printf("Impossible de changer de repertoire\n");
    Rio_writen(connfd, NACK, BUFFER_SIZE);
  } else {
    printf("Changement de repertoire vers %s realise\n",destination);
    Rio_writen(connfd, ACK, BUFFER_SIZE);
  }

  return 1;
}

int suprimeRec(char * dossier, int cran) {
  DIR * repertoireCourant;
  struct dirent * rep;
  char path[1024];

  //Ameliore l'affichage
  for(int i = 0; i < cran; i++) {
    printf("\t");
  }
  printf("Tentative ouverture |%s|\n",dossier);
  repertoireCourant = opendir(dossier);


  //Si il n'y a pas eu de probleme durant l'ouverture
  if (repertoireCourant)  {
    while ((rep = readdir(repertoireCourant)) != NULL)    {
      snprintf(path, 1024, "%s/%s",dossier,rep->d_name);
      //Ameliore l'affichage
      for(int i = 0; i < cran + 1; i++) {
        printf("\t");
      }
      printf("objet courant |%s|",path);

      //On supprime le repertoire ou fichier
      if(rep->d_type == DT_DIR) {
          //Si ce n'est pas le repertoire . ou ..
          if (strcmp(rep->d_name,"..") != 0 && strcmp(rep->d_name,".") != 0 ) {
            printf(" - Parcours\n");
            if(!suprimeRec(path,cran+1)) {
              return 0;
            }
          } else {
            printf(" - Ignore\n");
          }

      } else {
        printf(" - Supression\n");
        //A decommenter si certain du fonctionnement
        /*if (remove(path) == -1) {
          return 0;
        }*/
      }

    }
    closedir(repertoireCourant);
    //A decommenter si certain du fonctionnement
    /*if (remove(dossier) == -1) {
      return 0;
    }*/

    //Ameliore l'affichage
    for(int i = 0; i < cran; i++) {
      printf("\t");
    }
    printf("%s - Supression\n",dossier);
    return 1;

  } else {
    //Ameliore l'affichage
    for(int i = 0; i < cran; i++) {
      printf("\t");
    }
    printf("Pas repertoire courant %s\n",dossier);
    return 0;
  }

}

int commandeRm(int connfd, rio_t rio, int dossier) {
  char destination[BUFFER_SIZE];
  char buf[BUFFER_SIZE];

  //Recuperation de l' argument
  Rio_readnb(&rio, destination, MAX_NAME_LEN);
  printf("with arg(s) : _%s_\n", destination);

  //Si c'est rm -r
  //Pas implanter
  if (dossier) {
    if (suprimeRec(destination,0) != 1) {
      printf("dossier non suprimer\n");
      Rio_writen(connfd, NACK, BUFFER_SIZE);
    } else {
      Rio_writen(connfd, ACK, BUFFER_SIZE);
    }
    return 1;
  }

  //Si c'est rm
  if (remove(destination) == -1) {
    printf("Fichier non suprimer\n");
    Rio_writen(connfd, NACK, BUFFER_SIZE);
  } else {
    Rio_writen(connfd, ACK, BUFFER_SIZE);
  }


  return 1;
}

int commandeMkdir(int connfd, rio_t rio) {
  char dirName[MAX_NAME_LEN];
  size_t n;
  char * filename;

  //Recuperation du nom de dossier
  Rio_readnb(&rio, dirName, MAX_NAME_LEN);
  printf("with arg(s) : _%s_\n", dirName);

  //Creation du dossier
  if (mkdir(dirName,0777) == -1) {
    Rio_writen(connfd, NACK, BUFFER_SIZE);
  } else {
    Rio_writen(connfd, ACK, BUFFER_SIZE);
  }

  return 1;
}

int commandePut(int connfd, rio_t rio) {
  char buf[MAXLINE];
  char filename[MAXLINE];
  int n, fichierTaille, tailleRestante;
  clock_t debut, fin, tempsTotal;

  //On recupere le nom du fichier
  Rio_readnb(&rio, filename, BUFFER_SIZE);
  printf("with arg(s) : _%s_\n", buf);

  //On recupere la taille du fichier
  Rio_readnb(&rio, buf, BUFFER_SIZE);


  //Si le fichier existe
  if ((fichierTaille = atoi(buf)) >= 0) {

    //Ouverture du fichier de sorite
    FILE * sortie = fopen("sortieServeur","w+");
    printf("Fichier cree\n");

    tailleRestante = fichierTaille;
    debut = clock();
    //Lecture de la reponse
    while((n = Rio_readnb(&rio, buf, BUFFER_SIZE)) > 0 && tailleRestante - n > 0) {
      fwrite(buf, n, 1, sortie);
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

  return 1;
}

int traiteConnection(int connfd, rio_t rio) {
    char commande[MAX_NAME_LEN];
    char arg[MAX_NAME_LEN];

    //Recupereration de la commande
    Rio_readnb(&rio, commande, MAX_NAME_LEN);
    printf("Server received command : _%s_\n", commande);


    //Commande get
    if (strcmp(commande,COMMANDE_GET) == 0) {
      return commandeGet(connfd, rio);

    //Commande ls
    } else if (strcmp(commande,COMMANDE_LS) == 0) {
      return commandeLs(connfd, rio);

    //Commande pwd
    } else if (strcmp(commande,COMMANDE_PWD) == 0) {
      return commandePwd(connfd, rio);

    //Commande cd
    } else if (strcmp(commande,COMMANDE_CD) == 0) {
      printf("Commande cd\n");
      return commandeCd(connfd, rio);

    //Commande rm -r
  } else if (strcmp(commande,COMMANDE_RM_R) == 0) {
    return commandeRm(connfd, rio, 1);

    //Commande rm
    } else if (strcmp(commande,COMMANDE_RM) == 0) {
    return commandeRm(connfd, rio, 0);

    //Commande mkdir
  } else if (strcmp(commande,COMMANDE_MKDIR) == 0) {
    return commandeMkdir(connfd, rio);

    //Commande put
  } else if (strcmp(commande,COMMANDE_PUT) == 0) {
    return commandePut(connfd, rio);

    //Commande bye
    } else if(strcmp(commande,COMMANDE_BYE) == 0) {
      return 0;


    //Autre
  /*  } else {
        strncpy(buf, "Commande inconue par le serveur", MAX_NAME_LEN);
        Rio_writen(connfd, buf, MAX_NAME_LEN);
        printf("%s\n",buf);*/
    }

    printf("Commande traitee\n\n");
    return 1;
}

int userExiste(char * username) {
  return strcmp(username,"user") == 0;
}

int motDePasseCorrect(char * username, char * mdp) {
  return strcmp(mdp,"admin") == 0;
}

int authentification(int connfd, rio_t rio) {
  char buf[MAX_NAME_LEN];
  char username[MAX_NAME_LEN];

  //Demande d'authentification
  Rio_writen(connfd, DEMANDE_AUTHENTIFICATION, BUFFER_SIZE);

  //Reception de l'user
  Rio_readnb(&rio, buf, BUFFER_SIZE);
  printf("User : %s  receptionne\n",buf);

  //Verification de l'existence
  if (!userExiste(buf)) {
    Rio_writen(connfd, NACK, BUFFER_SIZE);
    printf("Cet user n'existe pas\n");
    return 0;
  }
  strcpy(username,buf);
  printf("Cet user existe\n");
  Rio_writen(connfd, ACK, BUFFER_SIZE);


  //Reception du mot de passe
  Rio_readnb(&rio, buf, BUFFER_SIZE);
  printf("MDP : %s  receptionne\n",buf);

  //Verification du mot de passe
  if (!motDePasseCorrect(username,buf)) {
    Rio_writen(connfd, NACK, BUFFER_SIZE);
    printf("Mauvais mot de passe\n");
    return 0;
  }
  Rio_writen(connfd, ACK, BUFFER_SIZE);

  return 1;
}

void connectionTraitant(int listenfd, struct sockaddr_in clientaddr, socklen_t clientlen, char * client_hostname, char * client_ip_string) {
    rio_t rio;
    int connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

    /* determine the name of the client */
    Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);

    /* determine the textual representation of the client's IP address */
    Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);

    printf("server connected to %s (%s) with child %d\n", client_hostname,client_ip_string,getpid());

    Rio_readinitb(&rio, connfd);

    if(authentification(connfd,rio)) {
      while(traiteConnection(connfd,rio));
    }

    printf("Server deconnected from %s (%s) with child %d\n\n", client_hostname,client_ip_string,getpid());
    Close(connfd);
}

/*
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main() {
    /* Gestion des signaux */
    Signal(SIGCHLD, handlerSigChild);
    Signal(SIGINT, handlerSigInt);
    pidPere = getpid();


    int listenfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    pid_t pid;


    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(PORT);

    for (int i = 0; i < NBPROC; i++) {  //Creation de la pool de processus
        if ((pid = Fork()) == 0) {      //Ce seront les fils qui vont trater les connections
            while(1) {
                connectionTraitant(listenfd, clientaddr, clientlen, client_hostname, client_ip_string);
            }
        } else {
            pids[i] = pid;
        }
    }


    for (int i = 0; i < NBPROC; i++) {
        wait(NULL);
    }

    exit(0);
}
