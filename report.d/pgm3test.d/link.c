#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <sys/time.h>

main(argc,argv)
int argc; char *argv[];
{

  struct stat info;

  if (argc != 2) {
    fprintf(stderr, "fileinfo: usage fileinfo filename\n");
    exit(1);
  }

  if (stat(argv[1], &info) != 0) {
    fprintf(stderr,"fileinfo: cannot stat %s:", argv[1]);
    perror(NULL);
  }
  printf("%s: ", argv[1]);
  printf("(device,i_number)=(%d/%d,%ld)", major(info.st_dev), minor(info.st_dev), (long) info.st_ino);
  printf(" last accessed %d seconds ago\n", time(NULL)-info.st_atime);
  if (S_ISREG(info.st_mode))
    printf("\tdesignated a regular file\n");
  if (S_ISLNK(info.st_mode))
    printf("\tdesignated a symlink\n");
}
