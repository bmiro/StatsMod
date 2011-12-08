#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFF_SIZE 256
#define CMD_SIZE 100

#define EXISTENT_FILE "exist"
#define NON_EXISTENT_FILE "smoke"

#define N_SUCCESS 5
#define N_FAIL 7

/* Must containt more than N_SUCCESS characters for write test */
#define TEST_BUFFER "0123456789ABCDEF"

#define INVALID_FD -1

int main() {
  pid_t selfpid;
  FILE *fpipe, *fp;
  int i;
  int f[5];
  int foo;

  char buff[BUFF_SIZE];
  char file_name[80];

  char cmd_insmod[] = "insmod statsmod-wheat.ko pid=%d 2>&1";
  char cmd_rmmod[] = "rmmod statsmod_wheat 2>&1";
  char cmd[CMD_SIZE];

  //TODO check if you are root

  /* File creation for test */
  for (i = 0; i < N_SUCCESS; i++) {
    sprintf(file_name, "file-%d", i);
    f[i] = open(file_name, O_RDWR | O_CREAT);
    if (f[i] < 0) {
      perror("Cannot open existing file.");
      printf("\tSomething happen openning the file, %s exist?\n", EXISTENT_FILE);
    }
    close(f[i]);
  }

  printf("Wheat module test\n\n");

  selfpid = getpid();

  printf("\tThe current pid of this process is %d\n\n", selfpid);
  printf("\tWaiting for user interation (Please press ENTER to continue)\n");
  getchar();

  sprintf(cmd, cmd_insmod, selfpid);
  printf("\tModule insmod test, running command: %s\n", cmd);

  if (!(fpipe = (FILE*)popen(cmd, "r"))) {
    perror("\tProblem calling ismod\n");
    pclose(fpipe);
    exit(1);
  }

  if (fgetc(fpipe) != EOF) {
    printf("\tUnable to load module, maybe already in use?\n");
    pclose(fpipe);
    exit(1);
  }

  pclose(fpipe);

  printf("\tModule successfuly loaded. (Please press ENTER to continue)\n");
  getchar();

  /* Generate successful opens */
  for (i = 0; i < N_SUCCESS; i++) {
    sprintf(file_name, "file-%d", i);
    f[i] = open(file_name, O_RDWR);
    if (f[i] < 0) {
      perror("Cannot open existing file.");
      printf("\tSomething happen openning the file, %s exist?\n", EXISTENT_FILE);
    }
  }

  /* Generate failed opens */
  for (i = 0; i < N_FAIL; i++) {
    foo = open(NON_EXISTENT_FILE, O_RDWR);
    if (foo != -1) {
      printf("\tOpenning an (suposed) inexistent file! %s exist!!\n", NON_EXISTENT_FILE);
    }
  }

  /* Generate successful write */
  for (i = 0; i < N_SUCCESS; i++) {
    if (write(f[i], TEST_BUFFER, i) < 0) {
      perror("Cannot write the existing file");
      printf("\tSomething happen writting the %d bytes to file %d\n", i, i);
    }
  }

  /* Generate failed write */
  for (i = 0; i < N_FAIL; i++) {
    if (write(INVALID_FD, TEST_BUFFER, i) > -1) {
      printf("\tWritten on inexistent file!\n");
    }
  }

  /* Generate succesful lseek */
  for (i = 0; i < N_SUCCESS; i++) {
    if (lseek(f[i], i, SEEK_SET) < 0) {
      perror("Cannot seek the existing file");
      printf("\tSomething happen seeking at %d bytes in file %d\n", i, i);
    }
  }

  /* Generate failed lseek */
  for (i = 0; i < N_FAIL; i++) {
    if (lseek(INVALID_FD, -1, SEEK_SET) > -1) {
      printf("\tSeeked on inexistent file with bad offset!\n");
    }
  }

  /* Generate succesful close */
  for (i = 0; i < N_SUCCESS; i++) {
    if (close(f[i]) < 0) {
      perror("Cannot close existing file");
      printf("\tSomething happen closing the file %d\n", i);
    }
  }

  /* Generate 5 failed close */
  for (i = 0; i < N_FAIL; i++) {
    if (close(INVALID_FD) > -1) {
      printf("\Closed an (suposed) unopened file!!!\n");
    }
  }

  /* Generate succesful clone */

  /* Generate failed clone */



  /* Public interface */

  /* PI: get_stats */

  /* PI: freeze_stats */

  /* PI: microwave_stats */

  /* PI: reset_stats */


   printf("\tOnce you have checked all the values type \"q\" key and ENTER\n\
 \tto remove the module.\n");
  while(getchar() != 'q');

  system(cmd_rmmod);

//   if (!(fp = (FILE*)popen(cmd_rmmod, "r"))) {
//     perror("\tProblem calling ismod\n");
//     pclose(fp);
//     exit(1);
//   }
// 
//   printf("%s\n", fgets(buff, 100, fp));

//   if (fgetc(fp) != EOF) {
//     printf("\tUnable to remove module, maybe not loaded or already in use?\n");
//     pclose(fp);
//     exit(1);
//   }
//   printf("\tModule removed successfully, thanks for your polite interaction, bye!\n");
//   pclose(fp);

  exit(0);
}
