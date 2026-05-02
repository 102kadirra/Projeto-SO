#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "filaEscalonamento.h"
#include "comando.h"
#include "mensagem.h"

void enviar_mensagem (const char* fifo_runner, const Mensagem *msg){

    int fd_runner = open (fifo_runner, O_WRONLY); 

    if (fd_runner == -1) {
        perror (" [controller] Erro ao abrir FIFO para escrita");
        return;
    }

    if (write (fd_runner, msg, sizeof(Mensagem)) == -1) {
        perror ("[controller] Erro ao escrever a mensagem");
    }

    close (fd_runner);
}

void despachar_fila (FilaEscalonamento *fila, int *slots_livres) {

    while (*slots_livres > 0 && !fila_vazia(fila)) {
        Comando *cmd = fila_pop(fila);
        
    Mensagem msg;
    memset(&msg, 0, sizeof(Mensagem));
    msg.tipo =OK;
    msg.comando = *cmd;
    strncpy(msg.runner_FIFO, cmd->runner_FIFO, MAX_FIFO_NAME - 1);

    enviar_mensagem(cmd->runner_FIFO, &msg);

    libertar_comando(cmd);
    (*slots_livres)--;
    }
}

void tratar_submit (FilaEscalonamento *fila, Mensagem *msg, int *slots_livres) {
       
        Comando *cmd = criar_comando(msg->comando.user_id, msg->comando.command_id, 0, msg->comando.command, msg->comando.runner_FIFO);

        cmd->tempo_entrada = msg->comando.tempo_entrada;

        inserir_comando(fila, cmd);

        despachar_fila(fila, slots_livres);
}

void tratar_executado (FilaEscalonamento *fila, Mensagem *msg, int *slots_livres) {

    (*slots_livres)++;
    despachar_fila(fila, slots_livres);
}

int main (int argc, char *argv[]) {

    if (argc < 3) {
        write(1, "Uso: ./controller <parallel-commands> <sched-policy>\n", 38);
        return 1;
    }

    int parallel_commands = atoi(argv[1]);
    if (parallel_commands < 1) {
        write(1, "O número de comandos paralelos deve ser >=1\n", 36);
        return 1;
    }

    int slots_livres = parallel_commands;

    PoliticaEscalonamento politica = selecionar_politica(argv[2]);
    if (politica == NULL) {
        write(1, "Política de escalonamento desconhecida. Use 'fcfs' ou 'RR'.\n", 49);
        return 1;
    }

    FilaEscalonamento fila;
    inicializar_fila(&fila, politica, NULL, 0);

    mkdir ("fifos", 0777);

    unlink (FIFO_RUNNER_TO_CONTROLLER);
    if (mkfifo (FIFO_RUNNER_TO_CONTROLLER, 0666) == -1) {
        perror ("Erro ao criar FIFO para comunicação");
        return 1;
    }

    int fd = open (FIFO_RUNNER_TO_CONTROLLER, O_RDWR);
    if (fd == -1) {
        perror ("Erro ao abrir FIFO");
        unlink (FIFO_RUNNER_TO_CONTROLLER);
        return 1;
    }

    Mensagem msg;
    ssize_t bytes_lidos;

    while(1) {
        bytes_lidos = read (fd, &msg, sizeof(Mensagem));
        if (bytes_lidos == -1) {
            perror ("Erro ao ler a mensagem");
            continue;
        }

        switch (msg.tipo) {
            case SUBMIT:
                tratar_submit(&fila, &msg, &slots_livres);
                break;
            case EXECUTADO:
                tratar_executado(&fila, &msg, &slots_livres);
                break;
            default:
                write(1, "Tipo de mensagem desconhecido\n", 30);
        }
    }

    close (fd);
    unlink (FIFO_RUNNER_TO_CONTROLLER);
    return 0;    
}




