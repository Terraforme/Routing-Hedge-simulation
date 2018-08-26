/* Prend en entrée : un entier, un caractère et une liste de noms de fichier
 * Lis stdin et écrit dans les fichiers successivement en prenant
 * le caractère comme délimiteur */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int read_until(char *buffer, char marker, int n)
/* Lit n caractères, ou moins si on rencontre le marker. Si on le rencontre,
 * on arrête de lire stdin. Renvoie le nombre de caractères lus. */
{
  for (int i=0; i<n; i++)
  {
    buffer[i] = getchar();
    if (buffer[i] == EOF || buffer[i] == marker) return i;
  }
  return n;
}

int main(int argc, char *argv[])
{
  if (argc < 3) return -1;
  
  int file_count = atoi(argv[1]);
  //printf("File count : %d\n", file_count);
  char marker = argv[2][0];
  
  if (argc < file_count + 2) return -1;
  
  for (int i=0; i<file_count; i++)
  {
    //printf("First file : %s\n", argv[3+i]);
    int fd = open(argv[3+i], O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    char buffer[1024]; int n_read;
    while (1)
    {
      n_read = read_until(buffer, marker, 1024);
      //printf("Writing %d in %s\n", n_read, argv[3+i]);
      write(fd, buffer, (size_t) n_read);
      if (n_read != 1024) break;
    }
     
    close(fd);
  }
  
  return 0;
}
