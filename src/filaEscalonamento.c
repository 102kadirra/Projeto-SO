
#include "filaEscalonamento.h"

void inicializar_fila (FilaEscalonamento *fila, PoliticaEscalonamento politica) {
    fila->fila = g_queue_new();
    fila->politica = politica;
}

void inserir_comando (FilaEscalonamento *fila, Comando *cmd) {
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
    