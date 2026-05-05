
#include "filaEscalonamento.h"
#include "politicas/fcfs.h"
#include "politicas/roundRobin.h"

void inicializar_fila (FilaEscalonamento *fila, PoliticaEscalonamento politica) {
    fila->fila = g_queue_new();
    fila->politica = politica;
}



void inserir_comando (FilaEscalonamento *fila, Comando *cmd) {
    /*if (fila->politica == politica_rr) {
        cmd->turno = obter_e_incrementar_turno(fila, cmd->user_id);
    }
    else {
        cmd->turno = 0;
    }*/

    g_queue_insert_sorted(fila->fila, cmd, fila->politica, NULL);
}
   

Comando *fila_pop (FilaEscalonamento *fila) {
    if (g_queue_is_empty(fila->fila)) {
        return NULL;
    }
    return g_queue_pop_head(fila->fila);
}

int tamanho_fila (FilaEscalonamento *fila) {
    return g_queue_get_length(fila->fila);
}

gboolean fila_vazia (FilaEscalonamento *fila) {
    return g_queue_is_empty(fila->fila);
}
    
PoliticaEscalonamento selecionar_politica (const char *nome_politica) {
    if (strcmp(nome_politica, "fcfs") == 0) {
        return politica_fcfs;
    } else if (strcmp(nome_politica, "RR") == 0) {
        return politica_rr;
    } else {
        return NULL;
    }
}