#include "ConstanteFTP.h"

size_t getTailleFichier(char * filename) {
  struct stat fichierStat;
  stat(filename, &fichierStat);
  return  fichierStat.st_size;
}
