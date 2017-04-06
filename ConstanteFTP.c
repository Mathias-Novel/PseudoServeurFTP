#include "ConstanteFTP.h"

size_t getTailleFichier(char * filename) {
  struct stat fichierStat;
  if (stat(filename, &fichierStat) == -1) {
    return -1;
  }
  return  fichierStat.st_size;
}
