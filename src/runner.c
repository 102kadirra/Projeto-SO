#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "comando.h"
#include "mensagem.h"


void enviar_mensagem (const char* fifo_ControllerToRunner, const Mensagem *msg) {

    int fd_controller = open (FIFO_RUNNER_TO_CONTROLLER, O_WRONLY);
    if (fd_controller == -1) {
        perror ("Erro ao abrir FIFO para escrita");
        unlink (fifo_ControllerToRunner);
        exit(1);
    }

    ssize_t bytes_escritos = write (fd_controller, msg, sizeof(Mensagem));
    if (bytes_escritos == -1) {
        perror ("[runner] Erro ao escrever a mensagem");
        close (fd_controller);
        unlink (fifo_ControllerToRunner);
        exit(1);
    }

    close (fd_controller);
}

void receber_mensagem (const char* fifo_ControllerToRunner, Mensagem *msg) {

    int fd_runner = open (fifo_ControllerToRunner, O_RDONLY);
    if (fd_runner == -1) {
        perror ("Erro ao abrir FIFO para leitura");
        unlink (fifo_ControllerToRunner);
        exit(1);
    }

    ssize_t bytes_lidos = read (fd_runner, &msg, sizeof(Mensagem));
    if (bytes_lidos == -1) {
        perror ("Erro ao ler a mensagem");
    }

    close (fd_runner);
    unlink (fifo_ControllerToRunner);
}

void executar_comando (Comando comando) {
    pid_t pid = fork();
    if (pid == -1) {
        perror ("Erro ao criar processo filho");
        exit(1);
    }

    if (pid == 0) {
   
        char *args[64];
        int argc = 0;
        char buffer[256];
        strncpy (buffer, comando.command, 255);
        buffer[255] = '\0';

        char *token = strtok (buffer, " ");
        while (token != NULL && argc < 63) {
            args[argc++] = token;
            token = strtok (NULL, " ");
        }
        args[argc] = NULL;

        execvp (args[0], args);
        perror ("Erro ao executar o comando");
        exit(1);
    }
    else {
        int status;
        waitpid (pid, &status, 0);
    }
}

void modo_executar (int user_id, const char *command) {

    char fifo_ControllerToRunner[256];
    snprintf (fifo_ControllerToRunner, sizeof(fifo_ControllerToRunner), "fifos/controllerToRunner_%d", getpid());

    unlink (fifo_ControllerToRunner);

    if (mkfifo (fifo_ControllerToRunner, 0666) == -1) {
        perror ("Erro ao criar FIFO");
        exit(1);
    }

    Comando comando;

    comando.user_id = user_id;
    comando.command_id = getpid();
    strncpy (comando.command, command, sizeof(comando.command) - 1);
    comando.command[sizeof(comando.command) - 1] = '\0';
    strncpy (comando.runner_FIFO, fifo_ControllerToRunner, sizeof(comando.runner_FIFO) - 1);
    comando.runner_FIFO[sizeof(comando.runner_FIFO) - 1] = '\0';
    gettimeofday (&comando.tempo_entrada, NULL);

    Mensagem msg;

    msg.tipo = SUBMIT;
    msg.comando = comando;
    strncpy (msg.runner_FIFO, fifo_ControllerToRunner, sizeof(msg.runner_FIFO) - 1);
    msg.runner_FIFO[sizeof(msg.runner_FIFO) - 1] = '\0';

    enviar_mensagem (fifo_ControllerToRunner, &msg);

    char buffer[256];
    int tamanho = snprintf (buffer, sizeof(buffer), "[runner] command %d submitted\n", comando.command_id);
    write(1, buffer, tamanho);

    Mensagem resposta;
    receber_mensagem (fifo_ControllerToRunner, &resposta);

    tamanho = snprintf (buffer, sizeof(buffer), "[runner] executing commando %d\n", comando.command_id);
    write(1, buffer, tamanho);

    executar_comando (comando);

    if (mkfifo (fifo_ControllerToRunner, 0666) == -1) {
        perror ("Erro ao criar FIFO para resposta");
        exit(1);
    }

    msg.tipo = EXECUTADO;
    enviar_mensagem (fifo_ControllerToRunner, &msg);
    unlink (fifo_ControllerToRunner);

    tamanho = snprintf (buffer, sizeof(buffer), "[runner] command %d finished\n", comando.command_id);
    write (1, buffer, tamanho);
}

int main (int argc, char *argv[]) {

    if (argc < 2) {
        write (2, "Uso: ./runner <user-id> <command> | ./runner -c | ./runner -s \n", 68);
        return 1;
    }

    if (strcmp (argv[1], "-e") == 0) {
        if (argc < 4) {
            write (2, "Uso: ./runner -e <user-id> <command>\n", 40);
            return 1;
        }
        int user_id = atoi (argv[2]);
        modo_executar (user_id, argv[3]);
    }
    else {
        write (2, "modo de utilização inválido", 30);
        return 1;
    }

    return 0;
}