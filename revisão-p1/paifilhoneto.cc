#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t filho;
    pid_t neto;

    printf("PAI iniciado.\n");
    printf("PID do PAI: %d\n\n", getpid());

    filho = fork();

    if (filho < 0) {
        printf("Erro ao criar o filho.\n");
        return 1;
    }

    if (filho == 0) {
        printf("FILHO iniciado.\n");
        printf("PID do FILHO: %d\n", getpid());
        printf("PPID do FILHO: %d\n\n", getppid());

        neto = fork();

        if (neto < 0) {
            printf("Erro ao criar o neto.\n");
            exit(1);
        }

        if (neto == 0) {
            printf("NETO iniciado.\n");
            printf("PID do NETO: %d\n", getpid());
            printf("PPID do NETO: %d\n\n", getppid());

            sleep(2);

            printf("NETO terminou.\n");
            exit(0);
        }

        printf("FILHO criou o NETO.\n");
        printf("PID do NETO: %d\n\n", neto);

        wait(NULL);

        printf("FILHO detectou que o NETO terminou.\n");
        printf("FILHO terminou.\n");

        exit(0);
    }

    printf("PAI criou o FILHO.\n");
    printf("PID do FILHO: %d\n\n", filho);

    wait(NULL);

    printf("PAI detectou que o FILHO terminou.\n");
    printf("PAI terminou.\n");

    return 0;
}

/*
OBS: 

PAI pode esperar o FILHO
FILHO pode esperar o NETO
PAI não espera diretamente o NETO

Quem chama fork() vira o pai do novo processo.

*/