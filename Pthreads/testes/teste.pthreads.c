/*   Mesmo processo
    A thread principal e a thread secundária terão o mesmo PID.
    Isso prova que ambas pertencem ao mesmo processo.
Threads diferentes
    Elas terão TIDs diferentes.
    Isso mostra que são fluxos de execução distintos.
Memória compartilhada
    O endereço da variável global será o mesmo para ambas.
Stack separada
    Os endereços das variáveis locais serão diferentes, 
    porque cada thread tem sua própria pilha. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>

//
long obter_tid(void) 
{
    return syscall(SYS_gettid);
}

//cria uma variavel global
int variavel_global = 100;

//cria a thread
void *funcao_thread(void *arg) {
    int variavel_local_thread = 200;

    printf("=== THREAD SECUNDARIA ===\n");
    printf("PID  : %d\n", getpid());
    printf("PPID : %d\n", getppid());
    printf("TID  : %ld\n", obter_tid());
    printf("Endereco da global: %p -> valor=%d\n", (void *)&variavel_global, variavel_global);
    printf("Endereco da local da thread: %p -> valor=%d\n",
           (void *)&variavel_local_thread, variavel_local_thread);

    sleep(10); // tempo para inspecionar com ps/pstree
    return NULL;
}

int main(void) {
    pthread_t th;
    int variavel_local_main = 50;

    printf("=== THREAD PRINCIPAL ===\n");
    //Pid do processo atual 
    printf("PID  : %d\n", getpid());
    //oq é ppdid?? Pid do processo pai do processo atual 
    printf("PPID : %d\n", getppid());
    //id para cada thread
    printf("TID  : %ld\n", obter_tid());
    printf("Endereco da global: %p -> valor=%d\n", (void *)&variavel_global, variavel_global);
    printf("Endereco da local da main: %p -> valor=%d\n",
           (void *)&variavel_local_main, variavel_local_main);

    if (pthread_create(&th, NULL, funcao_thread, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }

    pthread_join(th, NULL);
    return 0;
}



