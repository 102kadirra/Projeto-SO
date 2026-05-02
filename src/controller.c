#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h> 
#include <glib.h>
#include "filaEscalonamento.h"
#include "politicas/roundRobin.h"
#include "politicas/fcfs.h"
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

// Move o comando da Fila para o Array de 'em execução'
void despachar_fila (FilaEscalonamento *fila, int *slots_livres, Comando *em_execucao, int *num_em_execucao) {

    while (*slots_livres > 0 && !fila_vazia(fila)) {
        Comando *cmd = fila_pop(fila);
        
        // Adiciona à lista de Comandos em Execução para o runner -c 
        em_execucao[*num_em_execucao] = *cmd;
        (*num_em_execucao)++;
        
        Mensagem msg;
        memset(&msg, 0, sizeof(Mensagem));
        msg.tipo = OK;
        msg.comando = *cmd;
        strncpy(msg.runner_FIFO, cmd->runner_FIFO, MAX_FIFO_NAME - 1);

        enviar_mensagem(cmd->runner_FIFO, &msg);

        libertar_comando(cmd);
        (*slots_livres)--;
    }
}

void tratar_submit (FilaEscalonamento *fila, Mensagem *msg, int *slots_livres,
                    Comando *em_execucao, int *num_em_execucao,
                    GHashTable *tabela_turnos) {

    int uid = msg->comando.user_id;

    /* lê o contador actual (0 se chave ausente) e incrementa */
    gpointer valor_atual = g_hash_table_lookup(tabela_turnos,
                                               GINT_TO_POINTER(uid));
    int novo_turno = GPOINTER_TO_INT(valor_atual) + 1;

    /* cria o comando com turno já definido */
    Comando *cmd = criar_comando(uid, msg->comando.command_id,
                                 novo_turno,
                                 msg->comando.command,
                                 msg->comando.runner_FIFO);
    cmd->tempo_entrada = msg->comando.tempo_entrada;

    // persiste o novo contador na hash e insere na fila 
    g_hash_table_insert(tabela_turnos,
                        GINT_TO_POINTER(uid),
                        GINT_TO_POINTER(novo_turno));

    inserir_comando(fila, cmd);

    despachar_fila(fila, slots_livres, em_execucao, num_em_execucao);
}


void tratar_executado (FilaEscalonamento *fila, Mensagem *msg, int *slots_livres,
                       Comando *em_execucao, int *num_em_execucao,
                       GHashTable *tabela_turnos) {

    struct timeval fim;
    gettimeofday(&fim, NULL);

    for (int i = 0; i < *num_em_execucao; i++) {
        if (em_execucao[i].command_id == msg->comando.command_id) {

            /* Calcula duração em milissegundos */
            long duracao = (fim.tv_sec  - em_execucao[i].tempo_entrada.tv_sec)  * 1000L
                         + (fim.tv_usec - em_execucao[i].tempo_entrada.tv_usec) / 1000L;

            /* Registo em ficheiro (open/write/close)*/
            int fd_log = open("historico.txt", O_CREAT | O_APPEND | O_WRONLY, 0644);
            if (fd_log != -1) {
                char buf[128];
                int len = snprintf(buf, sizeof(buf),
                                   "User: %d | CmdID: %d | Duration: %ld ms\n",
                                   em_execucao[i].user_id,
                                   em_execucao[i].command_id,
                                   duracao);
                write(fd_log, buf, len);   /* write() único - atómico para PIPE_BUF */
                close(fd_log);
            }

            /* Decrementa o contador de turnos na tabela_turnos */
            int uid = em_execucao[i].user_id;
            gpointer valor_atual = g_hash_table_lookup(tabela_turnos,
                                                       GINT_TO_POINTER(uid));
            int novo_count = GPOINTER_TO_INT(valor_atual) - 1;

            if (novo_count <= 0) {
                /* Remove a chave para não deixar entradas com count ≤ 0 */
                g_hash_table_remove(tabela_turnos, GINT_TO_POINTER(uid));
            } else {
                g_hash_table_insert(tabela_turnos,
                                    GINT_TO_POINTER(uid),
                                    GINT_TO_POINTER(novo_count));
            }

            /* Remove do array em O(1): copia o último para esta posição */
            em_execucao[i] = em_execucao[--(*num_em_execucao)];
            break;
        }
    }

    (*slots_livres)++;
    despachar_fila(fila, slots_livres, em_execucao, num_em_execucao);
}

static void imprimir_comando_fila(gpointer data, gpointer user_data) {
    Comando *cmd = (Comando *)data;
    int fd = GPOINTER_TO_INT(user_data); // Desempacota o descritor de ficheiro
    char buf[256];
    
    // Formatação exigida pelo enunciado: user-id X - command-id Y
    int len = snprintf(buf, sizeof(buf), "user-id %d - command-id %d\n", 
                       cmd->user_id, cmd->command_id);
    write(fd, buf, len);
}

// Responde ao runner -c
void tratar_consulta (FilaEscalonamento *fila, Mensagem *msg, Comando *em_execucao, int num_em_execucao) {
    int fd = open(msg->runner_FIFO, O_WRONLY);
    if (fd == -1) return;
    
    char buf[512];
    int len;

    // Secção de Execução
    len = snprintf(buf, sizeof(buf), "---\nExecuting\n");
    write(fd, buf, len);
    
    for (int i = 0; i < num_em_execucao; i++) {
        len = snprintf(buf, sizeof(buf), "user-id %d - command-id %d\n", 
                       em_execucao[i].user_id, em_execucao[i].command_id);
        write(fd, buf, len);
    }
    
    // Secção de Fila de Espera (Scheduled) 
    len = snprintf(buf, sizeof(buf), "---\nScheduled\n");
    write(fd, buf, len);
    
    // Percorre a GQueue da GLib e imprime cada comando usando a função auxiliar
    // Pasoou se o descritor de ficheiro (fd) como 'user_data'
    g_queue_foreach(fila->fila, imprimir_comando_fila, GINT_TO_POINTER(fd));
    
    len = snprintf(buf, sizeof(buf), "---\n");
    write(fd, buf, len);
    
    close(fd);
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

    /* Cria a tabela de turnos por utilizador.
     * Chaves e valores são inteiros embalados em ponteiro (sem free necessário). */
    GHashTable *tabela_turnos = g_hash_table_new(g_direct_hash, g_direct_equal);

    // Inicialização do Array de comandos em execução
    Comando *em_execucao = malloc(sizeof(Comando) * parallel_commands);
    int num_em_execucao = 0;

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
    
    int em_encerramento = 0; 

    while(1) {
        bytes_lidos = read (fd, &msg, sizeof(Mensagem));
        if (bytes_lidos <= 0) continue;

        switch (msg.tipo) {
            case SUBMIT:
                if (em_encerramento) {
                    write(1, "[controller] Pedido rejeitado: Sistema em encerramento.\n", 56);
                    
                    // Criar e enviar mensagem de rejeição
                    Mensagem rej;
                    memset(&rej, 0, sizeof(Mensagem));
                    rej.tipo = REJEITADO;
                    enviar_mensagem(msg.runner_FIFO, &rej);
                    
                } else {
                    tratar_submit(&fila, &msg, &slots_livres, em_execucao, &num_em_execucao,
                                  tabela_turnos);
                }
                break;
            case EXECUTADO:
                tratar_executado(&fila, &msg, &slots_livres, em_execucao, &num_em_execucao,
                                 tabela_turnos);
                break;
            case CONSULTA:
                tratar_consulta(&fila, &msg, em_execucao, num_em_execucao);
                break;
            case SHUTDOWN:
                write(1, "[controller] Shutdown iniciado. A aguardar fim dos processos...\n", 64);
                em_encerramento = 1;
                break;
            default:
                write(1, "Tipo de mensagem desconhecido\n", 30);
        }

       
        // Se pediu para encerrar E já não há ninguém a correr E a fila está vazia -> Sai do loop
        if (em_encerramento == 1 && num_em_execucao == 0 && fila_vazia(&fila)) {
            write(1, "[controller] Todos os processos terminaram. A encerrar.\n", 56);
            break; 
        }
    }

   
    g_hash_table_destroy(tabela_turnos);
    free(em_execucao);
    close (fd);
    unlink (FIFO_RUNNER_TO_CONTROLLER);
    return 0;    
}