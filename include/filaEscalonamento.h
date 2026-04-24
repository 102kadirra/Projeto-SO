
#include <glib.h>

#include "comando.h"

typedef GCompareDataFunc PoliticaEscalonamento;

typedef struct {
    GQueue* fila;
    PoliticaEscalonamento politica;
} FilaEscalonamento;

void inicializar_fila (FilaEscalonamento *fila, PoliticaEscalonamento politica);
void inserir_comando (FilaEscalonamento *fila, Comando *cmd);
Comando *fila_pop (FilaEscalonamento *fila);
int tamanho_fila (FilaEscalonamento *fila);
gboolean fila_vazia (FilaEscalonamento *fila);





