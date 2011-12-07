#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE 256
#define CMD_SIZE 100

int main() {
  pid_t selfpid;
  FILE *fpipe;

  char buff[BUFF_SIZE];
  //char cmd_insmod[] = "insmod statsmod-wheat.ko pid=%d 2>&1";
  char cmd_insmod[] = "insmod statsmod-wheat.ko 2>&1";
  char cmd[CMD_SIZE];
  //char cmd_insmod[] = "echo > b";

  printf("Wheat module test\n\n");

  selfpid = getpid();

  printf("\tThe current pid of this process is %d\n\n", selfpid);
  printf("\tWaiting for user interation (Please press ENTER to continue)\n");
  getchar();

  //sprintf(cmd, cmd_insmod, selfpid);
  sprintf(cmd, cmd_insmod);
  printf("\tModule insmod test, running command: %s\n", cmd);

  if (!(fpipe = (FILE*)popen(cmd, "r"))) {
    perror("\tProblem calling ismod\n");
    pclose(fpipe);
    exit(1);
  }

  //printf(fgetc(fpipe));
//   fgets(buff, 10, fpipe);
//   printf(buff);



  if (fgetc(fpipe) != EOF) {
    printf("\tUnable to load module, maybe already in use?\n");
    pclose(fpipe);
    exit(1);
  }

  printf("\tModule successfuly loaded. (Please press ENTER to continue)\n");
  getchar();


  printf("\tOnce you have checked all the values type \"rmmod\" (without quotes)\n\
\tand press ENTER to remove the module.\n");
  while(strcmp("rmmod", gets(buff)));



  /* Extra */
  /* Module insertion */

  /* Module deletion */

  /* Public interface */

  /* PI: get_stats */

  /* PI: freeze_stats */

  /* PI: microwave_stats */

  /* PI: reset_stats */

  printf("Thanks for your polite interaction, bye!");

  pclose(fpipe);
}
