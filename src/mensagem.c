#include "mensagem.h"

void inicializar_mensagem (Mensagem *msg, TipoMensagem tipo, Comando *comando, const char *runner_FIFO) {
    msg->tipo = tipo;
    msg->comando = *comando;
    strncpy(msg->runner_FIFO, runner_FIFO, MAX_FIFO_NAME - 1);
    msg->runner_FIFO[MAX_FIFO_NAME - 1] = '\0';
}

void set_comando_mensagem (Mensagem *msg, Comando *comando) {
    msg->comando = *comando;
}

Comando* get_comando_mensagem (Mensagem *msg) {
    return &(msg->comando);
}



