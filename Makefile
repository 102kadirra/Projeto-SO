CC      = gcc
CFLAGS  = -Wall -g -Iinclude $(shell pkg-config --cflags glib-2.0)
GLIB    = $(shell pkg-config --libs glib-2.0)
 
CONTROLLER_OBJS = obj/controller.o obj/comando.o obj/filaEscalonamento.o \
                  obj/mensagem.o obj/registo.o
RUNNER_OBJS     = obj/runner.o obj/comando.o obj/mensagem.o
 
all: folders controller runner
 
controller: bin/controller
runner:     bin/runner
 
folders:
	@mkdir -p src include obj bin fifos
 
bin/controller: $(CONTROLLER_OBJS)
	$(CC) $^ $(GLIB) -o $@
 
bin/runner: $(RUNNER_OBJS)
	$(CC) $^ $(GLIB) -o $@
 
 
obj/controller.o: src/controller.c \
                  include/controller.h include/mensagem.h \
                  include/filaEscalonamento.h include/comando.h
	$(CC) $(CFLAGS) -c $< -o $@
 
obj/runner.o: src/runner.c \
              include/runner.h include/mensagem.h include/comando.h
	$(CC) $(CFLAGS) -c $< -o $@
 
obj/filaEscalonamento.o: src/filaEscalonamento.c \
                         include/filaEscalonamento.h include/comando.h
	$(CC) $(CFLAGS) -c $< -o $@
 
obj/mensagem.o: src/mensagem.c \
                include/mensagem.h include/comando.h
	$(CC) $(CFLAGS) -c $< -o $@
 
obj/comando.o: src/comando.c \
               include/comando.h
	$(CC) $(CFLAGS) -c $< -o $@
 
obj/registo.o: src/registo.c \
               include/registo.h include/comando.h
	$(CC) $(CFLAGS) -c $< -o $@
 
clean:
	rm -f obj/* bin/*

