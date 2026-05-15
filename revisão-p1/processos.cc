#include <stdio.h> //printf e scanf

#include<unistd.h>//me permite pegar o pid do processo 
/*

fork()
getpid()
getppid()
pid_T
*/

int main() {
    //pid_t é o tipo especifico para pegar pid de processos 
    pid_t pid;

    //cria um processo filho 
    pid = fork();

    //verifica se deu erro na criaçaõ do processo.Se for menor que 0 é porque deu errado 
    if (pid < 0) {
        printf("Erro ao criar processo.\n");
    }
    else if (pid == 0) {
        printf("Sou o processo filho.\n");
        //pega o pid do processo atual 
        printf("Meu PID: %d\n", getpid());
        printf("PID do meu pai: %d\n", getppid());
    }
    else {
        printf("Sou o processo pai.\n");
        printf("Meu PID: %d\n", getpid());
        printf("PID do meu filho: %d\n", pid);
    }

    return 0;
}
