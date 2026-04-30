/* Exercicio 4: cria um processo filho com fork. Pai e filho calculam
   resultado[i] = dados[i] * 4 + 20 para 500 posicoes cada um.
   O array resultado e impresso na ordem de 0 a 999. */

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int dados[1000];
    int resultado[1000];
    pid_t pid;
    int status;

    for (int i = 0; i < 1000; i++) {
        dados[i] = i;
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        for (int i = 500; i < 1000; i++) {
            resultado[i] = dados[i] * 4 + 20;
        }
        return EXIT_SUCCESS;
    }

    for (int i = 0; i < 500; i++) {
        resultado[i] = dados[i] * 4 + 20;
    }

    wait(&status);

    for (int i = 0; i < 1000; i++) {
        std::cout << resultado[i] << std::endl;
    }

    return EXIT_SUCCESS;
}
