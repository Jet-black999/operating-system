#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_cond_t condicao;

int vez = 1;

int proxima_vez(int id) {
    if (id == 1) {
        return 2;
    }

    if (id == 2) {
        return 0;
    }

    return 1;
}

void *processo(void *arg) {
    int id = *(int *)arg;

    while (1) {
        pthread_mutex_lock(&mutex);

        while (vez != id) {
            pthread_cond_wait(&condicao, &mutex);
        }

        printf("p%d\n", id);
        fflush(stdout);

        vez = proxima_vez(id);

        pthread_cond_broadcast(&condicao);
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }

    pthread_exit(NULL);
}

int main(void) {
    pthread_t p0;
    pthread_t p1;
    pthread_t p2;

    int id0 = 0;
    int id1 = 1;
    int id2 = 2;

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Erro ao inicializar mutex");
        return EXIT_FAILURE;
    }

    if (pthread_cond_init(&condicao, NULL) != 0) {
        perror("Erro ao inicializar variavel de condicao");
        pthread_mutex_destroy(&mutex);
        return EXIT_FAILURE;
    }

    if (pthread_create(&p0, NULL, processo, &id0) != 0) {
        perror("Erro ao criar p0");
        pthread_cond_destroy(&condicao);
        pthread_mutex_destroy(&mutex);
        return EXIT_FAILURE;
    }

    if (pthread_create(&p1, NULL, processo, &id1) != 0) {
        perror("Erro ao criar p1");
        pthread_cond_destroy(&condicao);
        pthread_mutex_destroy(&mutex);
        return EXIT_FAILURE;
    }

    if (pthread_create(&p2, NULL, processo, &id2) != 0) {
        perror("Erro ao criar p2");
        pthread_cond_destroy(&condicao);
        pthread_mutex_destroy(&mutex);
        return EXIT_FAILURE;
    }

    pthread_join(p0, NULL);
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    pthread_cond_destroy(&condicao);
    pthread_mutex_destroy(&mutex);

    return EXIT_SUCCESS;
}
