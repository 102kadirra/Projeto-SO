#include "filaEscalonamento.h"

gint politica_fcfs (gconstpointer a, gconstpointer b, gpointer _) {

    const Comando *cmd_a = (const Comando *)a;
    const Comando *cmd_b = (const Comando *)b;

    if (cmd_a->tempo_entrada.tv_sec != cmd_b->tempo_entrada.tv_sec) {
        return (gint)(cmd_a->tempo_entrada.tv_sec - cmd_b->tempo_entrada.tv_sec);
    }
    return (gint)(cmd_a->tempo_entrada.tv_usec - cmd_b->tempo_entrada.tv_usec);   

}