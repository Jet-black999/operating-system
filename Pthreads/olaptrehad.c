#include <stdio.h>
#include <pthread.h>
#define NTHREADS 10


void * printID(void * tid){
printf ("Olá, sou a thread %d\n", (int) tid) ;
pthread_exit (NULL) ;
}

int main()
{
    pthread_t threads [NTHREADS] ;
    int i, status ;
    for (i=0; i<NTHREADS; i++)
    {
        status = pthread_create(&threads[i], NULL, printID,
        (void*)i);
        if (status !=0) 
        {
        printf ("Erro na criação da thread %d\n", i) ;
        exit (-1) ;
        }
    }

    for (i=0; i<NTHREADS; i++)
    pthread_join(threads[i],NULL);
    return (0) ;
    }