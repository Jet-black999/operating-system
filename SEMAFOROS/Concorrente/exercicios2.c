#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t sem_p0;
sem_t sem_p1;
sem_t sem_p2;

void *p0(void *arg) {
    (void)arg;

    while (1) {
        sem_wait(&sem_p0);

        printf("p0\n");
        fflush(stdout);

        sem_post(&sem_p1);
        sleep(1);
    }

    pthread_exit(NULL);
}

void *p1(void *arg) {
    (void)arg;

    while (1) {
        sem_wait(&sem_p1);

        printf("p1\n");
        fflush(stdout);

        sem_post(&sem_p2);
        sleep(1);
    }

    pthread_exit(NULL);
}

void *p2(void *arg) {
    (void)arg;

    while (1) {
        sem_wait(&sem_p2);

        printf("p2\n");
        fflush(stdout);

        sem_post(&sem_p0);
        sleep(1);
    }

    pthread_exit(NULL);
}

int main(void) {
    pthread_t thread_p0;
    pthread_t thread_p1;
    pthread_t thread_p2;

    if (sem_init(&sem_p0, 0, 0) != 0) {
        perror("Erro ao inicializar semaforo p0");
        return EXIT_FAILURE;
    }

    if (sem_init(&sem_p1, 0, 1) != 0) {
        perror("Erro ao inicializar semaforo p1");
        sem_destroy(&sem_p0);
        return EXIT_FAILURE;
    }

    if (sem_init(&sem_p2, 0, 0) != 0) {
        perror("Erro ao inicializar semaforo p2");
        sem_destroy(&sem_p0);
        sem_destroy(&sem_p1);
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread_p0, NULL, p0, NULL) != 0) {
        perror("Erro ao criar thread p0");
        sem_destroy(&sem_p0);
        sem_destroy(&sem_p1);
        sem_destroy(&sem_p2);
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread_p1, NULL, p1, NULL) != 0) {
        perror("Erro ao criar thread p1");
        sem_destroy(&sem_p0);
        sem_destroy(&sem_p1);
        sem_destroy(&sem_p2);
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread_p2, NULL, p2, NULL) != 0) {
        perror("Erro ao criar thread p2");
        sem_destroy(&sem_p0);
        sem_destroy(&sem_p1);
        sem_destroy(&sem_p2);
        return EXIT_FAILURE;
    }

    pthread_join(thread_p0, NULL);
    pthread_join(thread_p1, NULL);
    pthread_join(thread_p2, NULL);

    sem_destroy(&sem_p0);
    sem_destroy(&sem_p1);
    sem_destroy(&sem_p2);

    return EXIT_SUCCESS;
}
