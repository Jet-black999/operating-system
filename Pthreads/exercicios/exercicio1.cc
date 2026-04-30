/* Exercicio 1: cria dois processos com fork no Linux.
   - pai imprime "Eu sou o pai"
   - primeiro filho imprime "Eu sou o 1º filho"
   - segundo filho imprime "Eu sou o 2º filho" */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Para fork
#include <sys/types.h> // Para pid_t
#include <sys/wait.h> // Para wait

int main(void)
{
    pid_t pid1, pid2;
    int status;

    pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid1 == 0) {
        /* Primeiro filho */
        printf("Eu sou o 1º filho\n");
        return EXIT_SUCCESS;
    }

    /* Executado apenas pelo pai */
    pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid2 == 0) {
        /* Segundo filho */
        printf("Eu sou o 2º filho\n");
        return EXIT_SUCCESS;
    }

    /* Pai */
    printf("Eu sou o pai\n");

    /* Espera pelos dois filhos terminarem */
    while (wait(&status) > 0)
        ;

    return EXIT_SUCCESS;
}
