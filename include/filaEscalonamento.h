
#include <glib.h>

typedef GCompareFunc PoliticaEscalonamento;

typedef struct FilaEscalonamento {
    GQueue* fila;
    PoliticaEscalonamento politica;
} FilaEscalonamento;





