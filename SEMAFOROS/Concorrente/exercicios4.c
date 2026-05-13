#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

int buffer1;
int buffer2;

sem_t buffer1_vazio;
sem_t buffer1_cheio;
sem_t buffer2_vazio;
sem_t buffer2_cheio;

void *th0(void *arg) {
    (void)arg;

    while (1) {
        int valor;

        printf("Digite um valor inteiro: ");
        fflush(stdout);

        if (scanf("%d", &valor) != 1) {
            printf("\nEntrada invalida. Encerrando th0.\n");
            exit(EXIT_FAILURE);
        }

        int calculo_parcial = valor * 2;

        sem_wait(&buffer1_vazio);
        buffer1 = calculo_parcial;
        sem_post(&buffer1_cheio);
    }

    pthread_exit(NULL);
}

void *th1(void *arg) {
    (void)arg;

    while (1) {
        sem_wait(&buffer1_cheio);
        int calculo_parcial = buffer1;
        sem_post(&buffer1_vazio);

        int resultado = calculo_parcial + 10;

        sem_wait(&buffer2_vazio);
        buffer2 = resultado;
        sem_post(&buffer2_cheio);
    }

    pthread_exit(NULL);
}

void *th2(void *arg) {
    (void)arg;

    while (1) {
        sem_wait(&buffer2_cheio);
        int resultado = buffer2;
        sem_post(&buffer2_vazio);

        printf("Resultado final: %d\n", resultado);
        fflush(stdout);
    }

    pthread_exit(NULL);
}

int main(void) {
    pthread_t thread0;
    pthread_t thread1;
    pthread_t thread2;

    if (sem_init(&buffer1_vazio, 0, 1) != 0) {
        perror("Erro ao inicializar buffer1_vazio");
        return EXIT_FAILURE;
    }

    if (sem_init(&buffer1_cheio, 0, 0) != 0) {
        perror("Erro ao inicializar buffer1_cheio");
        sem_destroy(&buffer1_vazio);
        return EXIT_FAILURE;
    }

    if (sem_init(&buffer2_vazio, 0, 1) != 0) {
        perror("Erro ao inicializar buffer2_vazio");
        sem_destroy(&buffer1_vazio);
        sem_destroy(&buffer1_cheio);
        return EXIT_FAILURE;
    }

    if (sem_init(&buffer2_cheio, 0, 0) != 0) {
        perror("Erro ao inicializar buffer2_cheio");
        sem_destroy(&buffer1_vazio);
        sem_destroy(&buffer1_cheio);
        sem_destroy(&buffer2_vazio);
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread0, NULL, th0, NULL) != 0) {
        perror("Erro ao criar th0");
        sem_destroy(&buffer1_vazio);
        sem_destroy(&buffer1_cheio);
        sem_destroy(&buffer2_vazio);
        sem_destroy(&buffer2_cheio);
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread1, NULL, th1, NULL) != 0) {
        perror("Erro ao criar th1");
        sem_destroy(&buffer1_vazio);
        sem_destroy(&buffer1_cheio);
        sem_destroy(&buffer2_vazio);
        sem_destroy(&buffer2_cheio);
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread2, NULL, th2, NULL) != 0) {
        perror("Erro ao criar th2");
        sem_destroy(&buffer1_vazio);
        sem_destroy(&buffer1_cheio);
        sem_destroy(&buffer2_vazio);
        sem_destroy(&buffer2_cheio);
        return EXIT_FAILURE;
    }

    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    sem_destroy(&buffer1_vazio);
    sem_destroy(&buffer1_cheio);
    sem_destroy(&buffer2_vazio);
    sem_destroy(&buffer2_cheio);

    return EXIT_SUCCESS;
}
