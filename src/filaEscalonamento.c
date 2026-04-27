
#include "filaEscalonamento.h"

void inicializar_fila (FilaEscalonamento *fila, PoliticaEscalonamento politica, ContadorUtilizador contadores[MAX_USERS], int num_users) {
    fila->fila = g_queue_new();
    fila->politica = politica;
    for (int i = 0; i < num_users; i++) {
        fila->contadores[i] = contadores[i];
    }
    fila->num_users = num_users;
}

int obter_e_incrementar_turno (FilaEscalonamento *fila, int user_id) {
    for (int i = 0; i < fila->num_users; i++) {
        if (fila->contadores[i].user_id == user_id) {
            fila->contadores[i].contador++;
            return;
        }
    }
    if (fila->num_users < MAX_USERS) {
        fila->contadores[fila->num_users].user_id = user_id;
        fila->contadores[fila->num_users].contador = 1;
        fila->num_users++;
    }
    return 0;
}

void inserir_comando (FilaEscalonamento *fila, Comando *cmd) {
    if (fila->politica == politica_rr) {
        cmd->turno = obter_e_incrementar_turno(fila, cmd->user_id);
    }
    else {
        cmd->turno = 0;
    }

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
    