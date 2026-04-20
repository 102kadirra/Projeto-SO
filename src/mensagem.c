#include "mensagem.h"

void inicializar_mensagem (Mensagem *msg, TipoMensagem tipo, Comando *comando) {
    msg->tipo = tipo;
    msg->comando = *comando;
    memset(msg->runner_FIFO, 0, MAX_FIFO_NAME);
}

void set_comando_mensagem (Mensagem *msg, Comando *comando) {
    msg->comando = *comando;
}

Comando* get_comando_mensagem (Mensagem *msg) {
    return &(msg->comando);
}



