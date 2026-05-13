/* Exercicio 3: cria cinco threads POSIX. Cada thread escreve 200 numeros
   em um intervalo diferente e o main aguarda todas terminarem. */

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <pthread.h>

struct ThreadArgs {
    int inicio;
    int fim;
};
// Mutex para sincronizar a impressão
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void* imprimir_intervalo(void* arg)
{
    // Converte o argumento para o tipo correto
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    std::ostringstream buffer;

    for (int num = args->inicio; num <= args->fim; num++) {
        buffer << num;
        //verifica se é o último número para evitar espaço extra
        if (num < args->fim) {
            buffer << ' ';
        }
    }
    buffer << '\n';

    pthread_mutex_lock(&print_mutex);
    std::cout << buffer.str();
    pthread_mutex_unlock(&print_mutex);

    return nullptr;
}

int main()
{ //cria as threads e espera por elas
    pthread_t threads[5];
    ThreadArgs args[5];

    for (int i = 0; i < 5; i++) {
        args[i].inicio = i * 200 + 1;
        args[i].fim = args[i].inicio + 199;
        // Cria a thread para imprimir o intervalo
        if (pthread_create(&threads[i], nullptr, imprimir_intervalo, &args[i]) != 0) {
            std::perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], nullptr);
    }

    pthread_mutex_destroy(&print_mutex);
    return EXIT_SUCCESS;
}
