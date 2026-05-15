#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>


//estrutura necessaria para passar dados para a thread 
struct ThreadArgs {
    int numero_thread;
    int tempo_execucao;
};

void* tarefa_da_thread(void* arg) {
    ThreadArgs* dados = static_cast<ThreadArgs*>(arg);

    std::cout << "Thread " << dados->numero_thread << " iniciou.\n";
    std::cout << "PID do processo: " << getpid() << "\n";
    std::cout << "ID da thread: " << (unsigned long)pthread_self() << "\n\n";

    sleep(dados->tempo_execucao);

    std::cout << "Thread " << dados->numero_thread
              << " terminou apos "
              << dados->tempo_execucao
              << " segundos.\n\n";

    return nullptr;
}

int main() {
    const int QUANTIDADE_THREADS = 3;
    //array de threads 
    pthread_t threads[QUANTIDADE_THREADS];
    //argumentos das threads 
    ThreadArgs argumentos[QUANTIDADE_THREADS];

    std::cout << "Processo principal iniciado.\n";
    std::cout << "PID do processo principal: " << getpid() << "\n\n";

    for (int i = 0; i < QUANTIDADE_THREADS; i++) {
        argumentos[i].numero_thread = i + 1;
        argumentos[i].tempo_execucao = i + 1;
        /* 
        Crie uma thread.
    Guarde o identificador dela em threads[i].
    Use configuração padrão.
    Execute a função tarefa_da_thread.  
    Passe &argumentos[i] como dado de entrada.
        
        
        */
        int resultado = pthread_create(
            &threads[i],
            nullptr,
            tarefa_da_thread,
            &argumentos[i]
        );
        //mesma ideia de criar um fork,se o falor foir !=0 deu erro 
        if (resultado != 0) {
            std::cerr << "Erro ao criar a thread " << i + 1 << "\n";
            return EXIT_FAILURE;
        }
    }

    std::cout << "Thread principal criou todas as threads.\n\n";


     /*threads[i]      -> thread que a main deve esperar
    nullptr            -> não queremos capturar valor de retorno da thread */
    
    for (int i = 0; i < QUANTIDADE_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }

    std::cout << "Todas as threads terminaram.\n";
    std::cout << "Programa finalizado.\n";

    return EXIT_SUCCESS;
}