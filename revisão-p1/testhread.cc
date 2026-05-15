#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>



/*
essa funçaõ tem esse formato pois 
void* tarefa_da_thread(void* arg)
recebe qualquer tipo de parametro e retorna qualquer tipo de parametr 


*/
void* tarefa_da_thread(void* arg) {
    printf("Thread criada executando.\n");

    printf("ID da thread: %lu\n", pthread_self());

    sleep(2);

    printf("Thread terminou.\n");

    return NULL;
}

int main() {
    pthread_t thread;

    printf("Processo iniciado.\n");
    printf("Thread principal executando.\n");

    /* parametros: 
        &thread            -> onde será armazenado o identificador da thread
        NULL               -> atributos padrão da thread    
        tarefa_da_thread   -> função que a thread vai executar
        NULL               -> argumento enviado para a função da thread

 */

    if (pthread_create(&thread, NULL, tarefa_da_thread, NULL) != 0) {


        perror("Erro ao criar thread");
        return EXIT_FAILURE;
    }

    printf("Thread principal continua executando.\n");

    //faz a thread esperar a outra thread terminar 
    pthread_join(thread, NULL);

    
    printf("Thread principal detectou que a thread terminou.\n");
    printf("Programa finalizando.\n");

    return EXIT_SUCCESS;
}