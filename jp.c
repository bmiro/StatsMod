#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "statsmod-common.h"

#define BUFF_SIZE 256
#define CMD_SIZE 100

#define N_SUCCESS 5
#define N_FAIL 7

#define SUCCESS 0
#define FAIL 1

#define NON_EXISTENT_FILE "dijkstraisalive"

/* Must containt more than N_SUCCESS characters for write test */
#define TEST_BUFFER "0123456789ABCDEF"

#define INVALID_FD -1

int main() {
  pid_t selfpid, childpid;

  int i;
  int foo;

  FILE *fpipe, *fp;
  int f[5];
  int smr;

  struct t_info stats;

  char buff[BUFF_SIZE];
  char file_name[80];

  int results[NUM_INTERCEPTED_CALLS][2];

  char cmd_insmod_wheat[] = "insmod statsmod-wheat.ko pid=%d 2>&1";
  char cmd_rmmod_wheat[] = "rmmod statsmod_wheat 2>&1";
  char cmd_insmod_reaper[] = "insmod statsmod-reaper.ko";
  char cmd_rmmod_reaper[] = "rmmod statsmod_reaper 2>&1";
  char cmd_mknod[] = "mknod /dev/smr c 169 0";
  char cmd[CMD_SIZE];

  /* File creation for test */
  for (i = 0; i < N_SUCCESS; i++) {
    sprintf(file_name, "file-%d", i);
    f[i] = open(file_name, O_RDWR | O_CREAT);
    if (f[i] < 0) {
      perror("Cannot open existing file.");
      printf("\tSomething happen openning the file, %s exist?\n", file_name);
    }
    close(f[i]);
  }

  printf("Wheat module test\n\n");

  selfpid = getpid();

  printf("\tThe current pid of this process is %d\n\n", selfpid);
  printf("\tWaiting for user interation (Please press ENTER to continue)\n");
  getchar();

  sprintf(cmd, cmd_insmod_wheat, selfpid);
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

  sprintf(cmd, cmd_insmod_reaper);
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
      printf("\tSomething happen openning the file, %s exist?\n", file_name);
    } else {
      results[OPEN][SUCCESS]++;
    }
  }

  /* Generate failed opens */
  for (i = 0; i < N_FAIL; i++) {
    foo = open(NON_EXISTENT_FILE, O_RDWR);
    if (foo != -1) {
      printf("\tOpenning an (suposed) inexistent file! %s exist!!\n", NON_EXISTENT_FILE);
    } else {
      results[OPEN][FAIL]++;
    }
  }

  /* Generate successful write */
  for (i = 0; i < N_SUCCESS; i++) {
    if (write(f[i], TEST_BUFFER, i) < 0) {
      perror("Cannot write the existing file");
      printf("\tSomething happen writting the %d bytes to file %d\n", i, i);
    } else {
      results[WRITE][SUCCESS]++;
    }
  }

  /* Generate failed write */
  for (i = 0; i < N_FAIL; i++) {
    if (write(INVALID_FD, TEST_BUFFER, i) > -1) {
      printf("\tWritten on inexistent file!\n");
    } else {
      results[WRITE][FAIL]++;
    }
  }

  /* Generate succesful lseek */
  for (i = 0; i < N_SUCCESS; i++) {
    if (lseek(f[i], i, SEEK_SET) < 0) {
      perror("Cannot seek the existing file");
      printf("\tSomething happen seeking at %d bytes in file %d\n", i, i);
    } else {
      results[LSEEK][SUCCESS]++;
    }
  }

  /* Generate failed lseek */
  for (i = 0; i < N_FAIL; i++) {
    if (lseek(INVALID_FD, -1, SEEK_SET) > -1) {
      printf("\tSeeked on inexistent file with bad offset!\n");
    } else {
      results[LSEEK][FAIL]++;
    }
  }

  /* Generate succesful close */
  for (i = 0; i < N_SUCCESS; i++) {
    if (close(f[i]) < 0) {
      perror("Cannot close existing file");
      printf("\tSomething happen closing the file %d\n", i);
    } else {
      results[CLOSE][SUCCESS]++;
    }
  }

  /* Generate 5 failed close */
  for (i = 0; i < N_FAIL; i++) {
    if (close(INVALID_FD) > -1) {
      printf("Closed an (suposed) unopened file!!!\n");
    } else {
      results[CLOSE][FAIL]++;
    }
  }


  /* Testing device in dev */
  smr = open("/dev/smr", O_RDWR);
  if (smr < 0) {
    printf("\t Failed opening smr device\n");
    exit(1);
  }

  foo = read(smr, &stats, sizeof(struct t_info));
  if (foo != sizeof(struct t_info)) {
    printf("\t Something went wrong reading stats using smr device.\n");
    exit(1);
  }

  printf("\tTesting read on device smr, reading default stats (OPEN) [13 / 5+1 / 7]: \n");
  printf("\tTotal: %d\n \tSuccess: %d\n \tFail: %d\n \tTime: %u\n\n", \
         stats.total, stats.success, stats.fail, stats.time);

  printf("\t\tPassed succesfully, press ENTER to continue\n");
  getchar();
  /**************************** Testing read smr ****************************/

  printf("\tTesting ioctl on device smr, change to CLONE stats [ 1 / 1 / 0 ]: \n");
  printf("\tShould show %d success and 0 fail (2nd popen in jp.c code). \n", 1);
  foo = ioctl(smr, CHANGE_SYSCALL, CLONE);
  if (foo < 0) {
    printf("\tError in ioctl setting CLONE syscall.\n");
  }
  foo = read(smr, &stats, sizeof(struct t_info));
  if (foo != sizeof(struct t_info)) {
    printf("\tSomething went wrong reading stats using smr device.\n");
    exit(1);
  }
  printf("\tTotal: %d\n \tSuccess: %d\n \tFail: %d\n \tTime: %u\n\n", \
         stats.total, stats.success, stats.fail, stats.time);

  printf("\t\tPassed succesfully, press ENTER to continue\n");
  getchar();
  /*********************** Testing cur_process & reset ***********************/
  childpid = fork();

  if (childpid == 0) { /* Child */
    printf("\t[CHILD] I'm the child and I have stats copied from my father but he will reset them.\n");
    while(1);
  }
  printf("\tTesting ioctl on device smr, changing to pid %d and resseting them. [0 / 0 / 0 / 0 ]\n", childpid);
  foo = ioctl(smr, CHANGE_PROCESS, &childpid);
  if (foo < 0) {
    printf("\tError in ioctl changing process to child.\n");
  }

  foo = ioctl(smr, RESET_CUR_PROCESS, NULL);
  if (foo < 0) {
    printf("\tError in ioctl reseting child stats.\n");
  }

  foo = read(smr, &stats, sizeof(struct t_info));
  if (foo != sizeof(struct t_info)) {
    printf("\tSomething went wrong reading stats using smr device.\n");
    exit(1);
  }
  printf("\tTotal: %d\n \tSuccess: %d\n \tFail: %d\n \tTime: %u\n\n", \
         stats.total, stats.success, stats.fail, stats.time);

  printf("\t\tPassed succesfully, press ENTER to continue\n");
  getchar();
  /************************** ioctl change process **************************/
  printf("\tShowing again self stats (with another clone of the fork). [ 2 / 2 / 0 ]\n");
  ioctl(smr, CHANGE_PROCESS, &selfpid);
  foo = read(smr, &stats, sizeof(struct t_info));
  if (foo != sizeof(struct t_info)) {
    printf("\tSomething went wrong reading stats using smr device.\n");
    exit(1);
  }
  printf("\tTotal: %d\n \tSuccess: %d\n \tFail: %d\n \tTime: %u\n\n", \
         stats.total, stats.success, stats.fail, stats.time);

  printf("\t\tPassed succesfully, press ENTER to continue\n");
  getchar();
  /************************** ioctl disable/enable **************************/
  printf("\tTesting desfuse of the module, of this %d OPEN only 2 will be acounted.\n", N_SUCCESS);

  foo = ioctl(smr, CHANGE_SYSCALL, CLOSE);
  if (foo < 0) {
    printf("\tError in ioctl setting CLOSE syscall.\n");
  }
  foo = ioctl(smr, DISABLE_SYSCALL, CLOSE);
  if (foo < 0) {
    printf("\tError in ioctl stoping CLOSE syscall.\n");
  }

  foo = ioctl(smr, CHANGE_SYSCALL, OPEN);
  if (foo < 0) {
    printf("\tError in ioctl setting OPEN syscall.\n");
  }
  printf("\tPrevious OPEN stats:\n");
  foo = read(smr, &stats, sizeof(struct t_info));
  if (foo != sizeof(struct t_info)) {
    printf("\tSomething went wrong reading stats using smr device.\n");
    exit(1);
  }
  printf("\tTotal: %d\n \tSuccess: %d\n \tFail: %d\n \tTime: %u\n\n", \
         stats.total, stats.success, stats.fail, stats.time);

  foo = ioctl(smr, DISABLE_SYSCALL, OPEN);
  if (foo < 0) {
    printf("\tError in ioctl stoping OPEN syscall.\n");
  }
  for (i = 0; i < N_SUCCESS; i++) {
    if (i == (N_SUCCESS-2)) {
      foo = ioctl(smr, ENABLE_SYSCALL, OPEN);
      if (foo < 0) {
        printf("\tError in ioctl starting OPEN syscall.\n");
      }
    }
    sprintf(file_name, "file-%d", i);
    f[i] = open(file_name, O_RDWR);
    if (f[i] < 0) {
      perror("Cannot open existing file.");
      printf("\tSomething happen openning the file, %s exist?\n", file_name);
    } else {
      results[OPEN][SUCCESS]++;
    }
    close(f[i]);
  }

  foo = ioctl(smr, ENABLE_SYSCALL, CLOSE);
  if (foo < 0) {
    printf("\tError in ioctl starting CLOSE syscall.\n");
  }

  printf("\tActual OPEN stats (+2 success):\n");
  foo = read(smr, &stats, sizeof(struct t_info));
  if (foo != sizeof(struct t_info)) {
    printf("\tSomething went wrong reading stats using smr device.\n");
    exit(1);
  }
  printf("\tTotal: %d\n \tSuccess: %d\n \tFail: %d\n \tTime: %u\n\n", \
         stats.total, stats.success, stats.fail, stats.time);

  printf("\t\tPassed succesfully, press ENTER to continue\n");
  getchar();
  /************************** summary **************************/

  printf("\tIn dmesg the stats must be:\n");

  printf("\t8 Succesful Open ((5 test + 2 of popen + 1 smr)\n");
  printf("\t7 Failed Writes of the test \n\n");

  printf("\t~50 Succesful Writes (5 for test and all the console printing)\n");
  printf("\t7 Failed Writes of the test \n\n");

  printf("\t5 Successful lseek of the test \n");
  printf("\t7 Failed lseek of the test \n\n");

  printf("\t8 Successful close (5 test + 2 of popen + 1 smr)\n");
  printf("\t7 Failed close of the test \n\n");

  printf("\t2 Succesful clone 1 from 2nd popen an 1 from fork \n");
  printf("\t0 Failed clone of the test \n\n");

  printf("\t Note that opens & closes matches.\n")

  printf("\tTo check the dmesg type \"make rmmod\" and \"dmesg\" in other shell.\n");
  printf("\tThen you can kill this process.\n");

  close(smr);
  kill(childpid, SIGTERM);

  while(1);

}
