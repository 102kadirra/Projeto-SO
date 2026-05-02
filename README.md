# Projeto-SO
## Visão geral
Este projeto implementa um **controller/runner** com comunicação via **FIFO** para escalonamento e execução de comandos. As políticas de escalonamento disponíveis incluem **FCFS** e **Round Robin**, mantendo histórico e registos de execução.

## Estrutura do projeto
```
Projeto-SO/
├─ bin/                  # executáveis (controller, runner)
├─ fifos/                # FIFOs nomeadas
├─ include/              # headers
│  ├─ comando.h
│  ├─ controller.h
│  ├─ filaEscalonamento.h
│  ├─ mensagem.h
│  ├─ runner.h
│  └─ politicas/
│     ├─ fcfs.h
│     └─ roundRobin.h
├─ obj/                  # objetos
├─ src/                  # implementação
│  ├─ comando.c
│  ├─ controller.c
│  ├─ filaEscalonamento.c
│  ├─ mensagem.c
│  ├─ runner.c
│  └─ politicas/
│     ├─ fcfs.c
│     └─ roundRobin.c
├─ contagem.txt
├─ erros.txt
├─ historico.txt
├─ mensagem.txt
├─ out.txt
├─ Makefile
└─ README.md
```

## Compilação
No diretório do projeto:
```bash
make
```

Isto gera os executáveis em `bin/`.

## Execução
Em terminais separados:

**Controller:**
```bash
./bin/controller <parallel-commands> <sched-policy>
```

**Runner:**
```bash
./bin/runner -e <user-id> <command>
./bin/runner -c
./bin/runner -s
```

## Comandos possíveis
### Controller
- `./bin/controller <parallel-commands> <sched-policy>`
	- `<parallel-commands>`: número de comandos em paralelo (>= 1)
	- `<sched-policy>`: `fcfs` ou `RR`

### Runner
- `./bin/runner -e <user-id> <command>`
	- Submete e executa um comando no sistema.
	- O `<command>` aceita pipes (`|`) e redirecionamentos (`>`, `<`, `2>`).
- `./bin/runner -c`
	- Consulta o estado (em execução e agendados).
- `./bin/runner -s`
	- Solicita shutdown gracioso do controller.

### Exemplos
```bash
./bin/controller 2 fcfs
./bin/runner -e 1 "ls -l | wc -l"
./bin/runner -c
./bin/runner -s
```

## Políticas de escalonamento
- **FCFS**: Primeiro a chegar, primeiro a ser executado.
- **Round Robin**: Execução por fatias de tempo (quantum), com preempção.

## Registos
- `historico.txt`: histórico de execuções.
- `erros.txt`: erros registados.
- `contagem.txt`: contagens/estatísticas (se aplicável).
- `out.txt`: saída geral (se aplicável).

## Notas
- As FIFOs são criadas/geridas na pasta `fifos/`.
- O formato das mensagens está definido em `mensagem.h` e `mensagem.c`.

## Limpeza
```bash
make clean
```