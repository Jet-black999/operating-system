/* Exercicio 3: cria cinco processos filhos com fork. Cada filho escreve
   200 numeros em um intervalo diferente, e o pai aguarda todos terminarem. */

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    pid_t pid;
    int status;

    for (int i = 0; i < 5; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            int inicio = i * 200 + 1;
            int fim = inicio + 199;

            for (int num = inicio; num <= fim; num++) {
                std::cout << num;
                if (num < fim) {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;

            return EXIT_SUCCESS;
        }
    }

    for (int i = 0; i < 5; i++) {
        wait(&status);
    }

    return EXIT_SUCCESS;
}
