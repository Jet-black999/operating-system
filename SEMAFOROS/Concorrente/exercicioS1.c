#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define QUANTIDADE_ITENS 10

int buffer;

sem_t vazio;
sem_t cheio;
sem_t mutex;



void *produtor(void *arg) {
    (void)arg;
    //gera itens de 1 a 10 
    for (int item = 1; item <= QUANTIDADE_ITENS; item++) {
        //se vazop for igual a 0,ele bloqueia 
        sem_wait(&vazio);
        sem_wait(&mutex);

        buffer = item;
        printf("Produtor produziu: %d\n", buffer);

        sem_post(&mutex);
        sem_post(&cheio);

        sleep(1);
    }

    pthread_exit(NULL);
}

void *consumidor(void *arg) {
    (void)arg;

    for (int i = 0; i < QUANTIDADE_ITENS; i++) {
        sem_wait(&cheio);
        sem_wait(&mutex);

        int item = buffer;
        printf("Consumidor consumiu: %d\n", item);

        sem_post(&mutex);
        sem_post(&vazio);

        sleep(1);
    }

    pthread_exit(NULL);
}

int main(void) {
    pthread_t thread_produtor;
    pthread_t thread_consumidor;

    if (sem_init(&vazio, 0, 1) != 0) {
        perror("Erro ao inicializar semaforo vazio");
        return EXIT_FAILURE;
    }

    if (sem_init(&cheio, 0, 0) != 0) {
        perror("Erro ao inicializar semaforo cheio");
        sem_destroy(&vazio);
        return EXIT_FAILURE;
    }

    if (sem_init(&mutex, 0, 1) != 0) {
        perror("Erro ao inicializar semaforo mutex");
        sem_destroy(&vazio);
        sem_destroy(&cheio);
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread_produtor, NULL, produtor, NULL) != 0) {
        perror("Erro ao criar thread produtora");
        sem_destroy(&vazio);
        sem_destroy(&cheio);
        sem_destroy(&mutex);
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread_consumidor, NULL, consumidor, NULL) != 0) {
        perror("Erro ao criar thread consumidora");
        sem_destroy(&vazio);
        sem_destroy(&cheio);
        sem_destroy(&mutex);
        return EXIT_FAILURE;
    }

    pthread_join(thread_produtor, NULL);
    pthread_join(thread_consumidor, NULL);

    sem_destroy(&vazio);
    sem_destroy(&cheio);
    sem_destroy(&mutex);

    return EXIT_SUCCESS;
}
