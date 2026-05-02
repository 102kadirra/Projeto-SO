#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

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

    ssize_t bytes_lidos = read (fd_runner, msg, sizeof(Mensagem));
    if (bytes_lidos == -1) {
        perror ("Erro ao ler a mensagem");
    }

    close (fd_runner);
    unlink (fifo_ControllerToRunner);
}

// Suporte a Pipes (|) e Redirecionamentos (>, <, 2>)
void executar_comando (Comando comando) {
    char *comandos[20];
    int num_cmds = 0;
    char *saveptr1;
    
    char buffer[256];
    strncpy (buffer, comando.command, 255);
    buffer[255] = '\0';
    
    // Divide a frase inteira pelos pipes '|' primeiro
    char *token = strtok_r(buffer, "|", &saveptr1);
    while (token != NULL && num_cmds < 20) {
        comandos[num_cmds++] = token;
        token = strtok_r(NULL, "|", &saveptr1);
    }
    
    // Cria os canais de comunicação
    int pipes[20][2];
    for (int i = 0; i < num_cmds - 1; i++) {
        pipe(pipes[i]); 
    }
    
    for (int i = 0; i < num_cmds; i++) {
        pid_t pid = fork(); 
        if (pid == -1) {
            perror ("Erro ao criar processo filho"); 
            exit(1);
        }

        //FILHO
        if (pid == 0) { 
        
            // Liga as saídas e entradas aos tubos (se houver pipes)
            if (i > 0) dup2(pipes[i-1][0], 0);
            if (i < num_cmds - 1) dup2(pipes[i][1], 1);
            
            // FILHO FECHA TODOS OS LADOS DOS PIPES 
            for (int j = 0; j < num_cmds - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Lê palavra a palavra e deteta operadores
            char *args[64];
            int argc = 0;
            char *saveptr2;
            
            char *arg = strtok_r(comandos[i], " ", &saveptr2);
            while (arg != NULL && argc < 63) {
                
                // Redirecionamento de Saída Normal
                if (strcmp(arg, ">") == 0) {
                    char *file = strtok_r(NULL, " ", &saveptr2); // Apanha o nome do ficheiro
                    if (file) {
                        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd != -1) { dup2(fd, 1); close(fd); }
                    }
                } 
                // Redirecionamento de Erros
                else if (strcmp(arg, "2>") == 0) {
                    char *file = strtok_r(NULL, " ", &saveptr2);
                    if (file) {
                        int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd != -1) { dup2(fd, 2); close(fd); }
                    }
                } 
                // Redirecionamento de Entrada
                else if (strcmp(arg, "<") == 0) {
                    char *file = strtok_r(NULL, " ", &saveptr2);
                    if (file) {
                        int fd = open(file, O_RDONLY);
                        if (fd != -1) { dup2(fd, 0); close(fd); }
                    }
                } 
                // Se não for nenhum símbolo, é uma palavra normal do comando
                else {
                    args[argc++] = arg;
                }
                
                // Passa para a próxima palavra
                arg = strtok_r(NULL, " ", &saveptr2);
            }
            args[argc] = NULL;
            
            execvp(args[0], args);
            perror ("Erro ao executar o comando"); 
            _exit(1); 
        }
    }
    
    // PAI: Fecha e espera que todos terminem
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    for (int i = 0; i < num_cmds; i++) {
        int status;
        waitpid(-1, &status, 0); 
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

    // Verificar se o comando foi aceite
    if (resposta.tipo != OK) {
        write(2, "[runner] Erro: O comando foi rejeitado pelo servidor (Sistema em encerramento).\n", 80);
        // O unlink já é feito dentro do receber_mensagem, por isso basta sair
        exit(1);
    }

    // Se chegou aqui, é porque recebeu OK
    tamanho = snprintf (buffer, sizeof(buffer), "[runner] executing command %d\n", comando.command_id);
    write(1, buffer, tamanho);

    executar_comando (comando);

    msg.tipo = EXECUTADO;
    enviar_mensagem (fifo_ControllerToRunner, &msg);
    unlink (fifo_ControllerToRunner);

    tamanho = snprintf (buffer, sizeof(buffer), "[runner] command %d finished\n", comando.command_id);
    write (1, buffer, tamanho);
}

void modo_consulta() {
    char fifo_nome[256];
    snprintf(fifo_nome, sizeof(fifo_nome), "fifos/runner_consulta_%d", getpid());
    unlink(fifo_nome);
    mkfifo(fifo_nome, 0666);

    Mensagem msg;
    msg.tipo = CONSULTA; 
    strncpy(msg.runner_FIFO, fifo_nome, sizeof(msg.runner_FIFO) - 1);
    
    enviar_mensagem(fifo_nome, &msg); 

    int fd = open(fifo_nome, O_RDONLY);
    char buf[512];
    int n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        write(1, buf, n);
    }
    close(fd);
    unlink(fifo_nome);
}

// -s
void modo_shutdown() {
    Mensagem msg;
    memset(&msg, 0, sizeof(Mensagem));
    msg.tipo = SHUTDOWN;
    
    // O shutdown não precisa de resposta, só de avisar o controller
    int fd_controller = open(FIFO_RUNNER_TO_CONTROLLER, O_WRONLY);
    if (fd_controller == -1) {
        perror("Erro ao abrir FIFO do controller para shutdown");
        exit(1);
    }
    
    write(fd_controller, &msg, sizeof(Mensagem));
    close(fd_controller);
    
    write(1, "Pedido de shutdown enviado. O servidor vai encerrar de forma limpa.\n", 68);
}

int main (int argc, char *argv[]) {
    if (argc < 2) {
        write (2, "Uso: ./runner -e <user-id> <command> | ./runner -c | ./runner -s \n", 66);
        return 1;
    }
    
    if (strcmp (argv[1], "-e") == 0) {
        if (argc < 4) {
            write (2, "Uso: ./runner -e <user-id> <command>\n", 37);
            return 1;
        }
        int user_id = atoi (argv[2]);
        modo_executar (user_id, argv[3]);
    }
    else if (strcmp (argv[1], "-c") == 0) {
        modo_consulta(); 
    }
    else if (strcmp (argv[1], "-s") == 0) {
        modo_shutdown();
    }
    else {
        write (2, "modo de utilização inválido\n", 29);
        return 1;
    }
    return 0;
}