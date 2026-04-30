/* Exercicio 1: cria duas threads POSIX. A thread principal imprime
   "Eu sou o pai" e cada thread imprime sua mensagem de filho. */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* primeiro_filho(void* arg)
{
    (void)arg;
    printf("Eu sou o 1º filho\n");
    return NULL;
}

void* segundo_filho(void* arg)
{
    (void)arg;
    printf("Eu sou o 2º filho\n");
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;

    if (pthread_create(&t1, NULL, primeiro_filho, NULL) != 0) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    if (pthread_create(&t2, NULL, segundo_filho, NULL) != 0) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    printf("Eu sou o pai\n");

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return EXIT_SUCCESS;
}
