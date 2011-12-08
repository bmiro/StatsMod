#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFF_SIZE 256
#define CMD_SIZE 100

#define EXISTENT_FILE "exist"
#define NON_EXISTENT_FILE "smoke"

#define N_SUCCESS 41
#define N_FAIL 23

int main() {
  pid_t selfpid;
  FILE *fpipe;
  int i;
  int f[5];
  int foo;

  char buff[BUFF_SIZE];
  char cmd_insmod[] = "insmod statsmod-wheat.ko pid=%d 2>&1";
  //char cmd_insmod[] = "insmod statsmod-wheat.ko 2>&1";
  char cmd[CMD_SIZE];


  //TODO check if you are root

  printf("Wheat module test\n\n");

  selfpid = getpid();

  printf("\tThe current pid of this process is %d\n\n", selfpid);
  printf("\tWaiting for user interation (Please press ENTER to continue)\n");
  getchar();

  sprintf(cmd, cmd_insmod, selfpid);
  //sprintf(cmd, cmd_insmod);
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

  printf("\tModule successfuly loaded. (Please press ENTER to continue)\n");
  getchar();

  /* Generate successful opens */
  for (i = 0; i < N_SUCCESS; i++) {
    f[i] = open(EXISTENT_FILE, O_RDWR);
    if (f < 0) {
      perror("Cannot open existing file.");
      printf("\tSomething happen openning the file, %s exist?\n", EXISTENT_FILE);
    }
  }

  /* Generate failed opens */
  for (i = 0; i < N_FAIL; i++) {
    foo = open(NON_EXISTENT_FILE, O_RDWR);
    if (foo > -1) {
      printf("\tOpenning an (suposed) inexistent file! %s exist!!\n", NON_EXISTENT_FILE);
    }
  }

  /* Generate 3 succesful close */

  /* Generate 5 failed close */

  /* Generate 7 successful read */

  /* Generate 11 failed read */

  /* Generate 13 successful write */

  /* Generate 17 failed write */

  /* Generate 19 succesful clone */

  /* Generate 23 failed clone */

  /* Generate 29 succesful lseek */

  /* Generate 31 failed lseek */



  /* Public interface */

  /* PI: get_stats */

  /* PI: freeze_stats */

  /* PI: microwave_stats */

  /* PI: reset_stats */


  printf("\tOnce you have checked all the values type \"rmmod\" (without quotes)\n\
\tand press ENTER to remove the module.\n");
  while(strcmp("rmmod", gets(&buff)));

  printf("Thanks for your polite interaction, bye!\n");

  pclose(fpipe);
}
