#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <sys/types.h>
#include "comando.h"


#define SERVER_FIFO "/tmp/controller_fifo"

void responder_ao_runner(pid_t pid_cliente, int id_atribuido);

void log_msg(const char *msg);

#endif