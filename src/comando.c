#include "comando.h"

Comando *criar_comando(int user_id, int command_id, const char *command_str, const char *runner_fifo) {
    Comando *cmd = malloc(sizeof(Comando));
    if (cmd) {
        cmd->user_id = user_id;
        cmd->command_id = command_id;
        strncpy(cmd->command, command_str, 255);
        cmd->command[255] = '\0';
        strncpy(cmd->runner_FIFO, runner_fifo, 255);
        cmd->runner_FIFO[255] = '\0';
        gettimeofday(&cmd->tempo_entrada, NULL);  
    }
    return cmd;
}

void libertar_comando(Comando *cmd) {
    if (cmd) {
        free(cmd);
    }
}

double duracao_execucao (const Comando *c, struct timeval *fim) {
    long segundos = fim->tv_sec - c->tempo_entrada.tv_sec;
    long microssegundos = fim->tv_usec - c->tempo_entrada.tv_usec;
    return (segundos * 1000.0) + (microssegundos / 1000.0);
}




