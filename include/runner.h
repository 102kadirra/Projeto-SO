#ifndef RUNNER_H
#define RUNNER_H

#include "comando.h"
#include "controller.h" 


void runner_log(const char *msg);

void print_id(int id);

int parse_args(int argc, char *argv[], Pedido *p);

#endif