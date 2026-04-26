

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Processo filho
        printf("=== FILHO ===\n");
        printf("PID  do filho : %d\n", getpid());
        printf("PPID do filho : %d\n", getppid());
        sleep(10);
    } else {
        // Processo pai
        printf("=== PAI ===\n");
        printf("PID do pai    : %d\n", getpid());
        printf("PID do filho  : %d\n", pid);
        sleep(10);
    }

    return 0;
}