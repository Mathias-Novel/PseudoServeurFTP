#include <stdio.h>
#include <stdlib.h>


int main() {
  FILE * f = fopen("testCharFin","w+");

  if (f == NULL) {
    printf("Fichier mal ouvert\n");
    exit(0);
  }

  char test[15] = "je suis un test";
  test[7] = '\0';


  fwrite(test, 15, 1, f);

  fclose(f);

  return 0;
}
