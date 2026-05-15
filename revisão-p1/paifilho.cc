#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t filho1;
    pid_t filho2;

    printf("Processo pai iniciado.\n");
    printf("PID do pai: %d\n\n", getpid());

    filho1 = fork();

    if (filho1 < 0) {
        printf("Erro ao criar o primeiro filho.\n");
        return 1;
    }

    if (filho1 == 0) {
        printf("FILHO 1 executando.\n");
        printf("PID do filho 1: %d\n", getpid());
        printf("PPID do filho 1: %d\n", getppid());

        sleep(2);

        printf("FILHO 1 terminou.\n\n");
        exit(0);
    }

    filho2 = fork();

    if (filho2 < 0) {
        printf("Erro ao criar o segundo filho.\n");
        return 1;
    }

    if (filho2 == 0) {
        printf("FILHO 2 executando.\n");
        printf("PID do filho 2: %d\n", getpid());
        printf("PPID do filho 2: %d\n", getppid());

        sleep(4);

        printf("FILHO 2 terminou.\n\n");
        exit(0);
    }

    printf("PAI criou dois filhos.\n");
    printf("PID do filho 1: %d\n", filho1);
    printf("PID do filho 2: %d\n\n", filho2);

    printf("PAI esperando os filhos terminarem...\n\n");

    wait(NULL);
    wait(NULL);

    printf("PAI detectou que os dois filhos terminaram.\n");
    printf("PAI finalizando...\n");

    return 0;
}