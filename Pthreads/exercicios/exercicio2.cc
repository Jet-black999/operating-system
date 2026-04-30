/* Exercicio 2: cria 5 processos filhos com fork e cada filho busca
   um numero em 200 posicoes diferentes de um array de 1000 posicoes.
   - se encontrar, imprime a posicao e o numero do processo (1..5)
   - se nao encontrar, imprime 0 */

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int array[1000];
    int numero;
    pid_t pid;
    int i;
    int status;

    for (i = 0; i < 1000; i++) {
        array[i] = i + 1;
    }

    std::cout << "Digite um numero entre 1 e 1000: ";
    if (!(std::cin >> numero)) {
        std::cerr << "Entrada invalida\n";
        return EXIT_FAILURE;
    }

    for (i = 0; i < 5; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            int inicio = i * 200;
            int fim = inicio + 200;
            bool encontrado = false;
            int posicao = -1;

            for (int j = inicio; j < fim; j++) {
                if (array[j] == numero) {
                    encontrado = true;
                    posicao = j;
                    break;
                }
            }

            if (encontrado) {
                std::cout << "Processo " << (i + 1) << ": posicao " << posicao << '\n';
            } else {
                std::cout << "Processo " << (i + 1) << ": 0\n";
            }

            return EXIT_SUCCESS;
        }

        /* pai continua para criar o proximo filho */
    }

    for (i = 0; i < 5; i++) {
        wait(&status);
    }

    return EXIT_SUCCESS;
}
