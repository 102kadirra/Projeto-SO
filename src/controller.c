#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "../include/comando.h"

#define SERVER_FIFO "/tmp/controller_fifo"

void responder_ao_runner(pid_t pid_cliente, int id_atribuido) {
    char path[64];
    snprintf(path, 64, "/tmp/fifo_%d", pid_cliente);
    
    int fd_res = open(path, O_WRONLY);
    if (fd_res != -1) {
        // Envia o ID para o runner que está à espera
        write(fd_res, &id_atribuido, sizeof(int));
        close(fd_res);
    }
}

// Função auxiliar para escrever no terminal sem printf
void log_msg(const char *msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main(int argc, char *argv[]) {
    // 1. Validar argumentos (ex: ./controller 5 policy1)
    if (argc < 3) {
        log_msg("Uso: ./controller <parallel-commands> <sched-policy>\n");
        return 1;
    }

    // 2. Criar o Pipe Central
    unlink(SERVER_FIFO); // Remove se já existir
    if (mkfifo(SERVER_FIFO, 0666) == -1) {
        log_msg("Erro ao criar FIFO central\n");
        return 1;
    }

    // 3. Abrir o FIFO
    // Usamos O_RDWR para o controller não receber EOF quando um runner fecha
    int fd_central = open(SERVER_FIFO, O_RDWR);
    if (fd_central == -1) {
        log_msg("Erro ao abrir FIFO central\n");
        return 1;
    }

    log_msg("[controller] Pronto para receber pedidos...\n");

    Pedido pedido_recebido;
    int contador_id = 1;

    // 4. Ciclo de Eventos Principal
    while (read(fd_central, &pedido_recebido, sizeof(Pedido)) > 0) {
        
        switch (pedido_recebido.modo) {
            case EXECUTAR:
                // Atribuir ID ao comando
                pedido_recebido.dados.command_id = contador_id++;
                
                log_msg("[controller] Novo comando para escalonar.\n");
                
                // TODO: inserir na tua FilaEscalonamento (Glib)
                
                // NOTA: Para o runner não ficar bloqueado para sempre,
                // deves responder-lhe confirmando a receção (ver abaixo).
                break;

            case CONSULTAR:
                log_msg("[controller] Pedido de consulta recebido.\n");
                // TODO: Iterar pela fila e enviar estado ao runner
                break;

            case TERMINAR:
                log_msg("[controller] A encerrar servidor...\n");
                close(fd_central);
                unlink(SERVER_FIFO);
                return 0;

            default:
                log_msg("[controller] Pedido inválido recebido.\n");
                break;
        }
    }

    return 0;
}