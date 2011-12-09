#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "statsmod-common.h"

#define BUFF_SIZE 256
#define CMD_SIZE 100

#define EXISTENT_FILE "exist"
#define NON_EXISTENT_FILE "smoke"

#define N_SUCCESS 5
#define N_FAIL 7

#define SUCCESS 0
#define FAIL 1

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
      printf("\tSomething happen openning the file, %s exist?\n", EXISTENT_FILE);
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

  /* Generate succesful clone */

  /* Generate failed clone */



  /* Public interface */

  /* PI: get_stats */

  /* PI: freeze_stats */

  /* PI: microwave_stats */

  /* PI: reset_stats */


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

  printf("\tTesting read on device smr, reading default stats (OPEN): \n");
  printf("\tTotal: %d\n \tSuccess: %d\n \tFail: %d\n \tTime: %u\n\n", \
         stats.total, stats.success, stats.fail, stats.time);

  printf("\tTesting ioctl on device smr, change to WRITE stats: \n");
  printf("\tShould show values greater than the previous one +8 in success. \n");
  ioctl(smr, CHANGE_SYSCALL, WRITE);
  foo = read(smr, &stats, sizeof(struct t_info));
  if (foo != sizeof(struct t_info)) {
    printf("\tSomething went wrong reading stats using smr device.\n");
    exit(1);
  }
  printf("\tTotal: %d\n \tSuccess: %d\n \tFail: %d\n \tTime: %u\n\n", \
         stats.total, stats.success, stats.fail, stats.time);

  childpid = fork();

  if (childpid == 0) { //Child
    foo = open("/dev/null", O_WRONLY);
    if (foo < 0) {
      printf("Child failed opening /dev/null in write mode.\n");
      exit(1);
    }
    for (i = 0; i < N_SUCCESS; i++) {
      
    }
    close(foo);
    while(1);
  }
  printf("\tTesting ioctl on device smr, changint to pid %d that will do %d successful writes and 0 fails. \n", childpid, N_SUCCESS);
  


  printf("\tIn dmesg all stats of this process must have %d success and %d fails.\n");

  printf("\tOnce you have checked all the values type \"make rmmod\" in other shell and kill this process.\n");

  close(smr);

  while(1);

}
