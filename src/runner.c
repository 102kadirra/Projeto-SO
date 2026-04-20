#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/controller.h"

// Implementação das funções do header
void runner_log(const char *msg) {
    write(STDOUT_FILENO, "[runner] ", 9);
    write(STDOUT_FILENO, msg, strlen(msg));
}

void print_id(int id) {
    char buf[12];
    int len = snprintf(buf, sizeof(buf), "%d", id);
    write(STDOUT_FILENO, buf, len);
}

int main(int argc, char *argv[]) {
    Pedido p;
    p.runner_pid = getpid();
    
    // Lógica de parsing (simplificada para o exemplo)
    if (argc < 2) return 1;
    
    if (strcmp(argv[1], "-e") == 0) p.modo = EXECUTAR;
    else if (strcmp(argv[1], "-c") == 0) p.modo = CONSULTAR;
    else if (strcmp(argv[1], "-s") == 0) p.modo = TERMINAR;

    // Criar FIFO privado
    char meu_fifo[64];
    snprintf(meu_fifo, 64, "/tmp/fifo_%d", p.runner_pid);
    mkfifo(meu_fifo, 0666);

    // Enviar ao Controller
    int fd_servidor = open(SERVER_FIFO, O_WRONLY);
    if (fd_servidor != -1) {
        write(fd_servidor, &p, sizeof(Pedido));
        close(fd_servidor);
    }

    // Se for execução, esperar pelo ID
    if (p.modo == EXECUTAR) {
        int fd_proprio = open(meu_fifo, O_RDONLY);
        int id_recebido;
        read(fd_proprio, &id_recebido, sizeof(int));
        
        runner_log("command ");
        print_id(id_recebido);
        write(STDOUT_FILENO, " submitted\n", 11);
        
        close(fd_proprio);
    }

    unlink(meu_fifo);
    return 0;
}