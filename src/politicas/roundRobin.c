#include "filaEscalonamento.h"

gint politica_rr (gconstpointer a, gconstpointer b, gpointer _) {

    const Comando *cmd_a = (const Comando *)a;
    const Comando *cmd_b = (const Comando *)b;

    if (cmd_a->turno != cmd_b->turno) {
        return cmd_a->turno - cmd_b->turno;
    }

    if (cmd_a->user_id != cmd_b->user_id) {
        return cmd_a->user_id - cmd_b->user_id;
    }

    if (cmd_a->tempo_entrada.tv_sec != cmd_b->tempo_entrada.tv_sec) {
        return (gint)(cmd_a->tempo_entrada.tv_sec - cmd_b->tempo_entrada.tv_sec);
    }
    return (gint)(cmd_a->tempo_entrada.tv_usec - cmd_b->tempo_entrada.tv_usec);
}

    
