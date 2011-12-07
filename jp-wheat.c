#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE 256

int main() {
  pid_t selfpid;
  FILE *fpipe;

  //char cmd_insmod[] = "insmod statsmod-wheat.ko pid=";
  char cmd_insmod[] = "echo > b";

  char buff[BUFF_SIZE];
  char c;

  printf("Wheat module test\n\n");

  selfpid = getpid();

  printf("\tThe current pid of this process is %d\n\n", selfpid);
  printf("\tWaiting for user interation (Please press ENTER to continue)\n");
  getchar();

  //sprintf(cmd_insmod, "%s%d", cmd_insmod, selfpid);
  printf("\tModule insmod test, running command: %s\n", cmd_insmod);

  if (!(fpipe = (FILE*)popen(cmd_insmod, "r"))) {
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
