#!/bin/bash
# ==============================================================================
# Script 1: Avaliação de Justiça — FCFS vs Round Robin (Starvation Test)
# ==============================================================================
# Objetivo: Demonstrar que FCFS pode causar starvation (o User 2 espera por todos
# os comandos demorados do User 1) enquanto RR distribui de forma mais justa.
#
# Cenário:
#   - Controller com 1 slot paralelo
#   - User 1 submete 4 comandos demorados (sleep 3)
#   - User 2 submete 1 comando rápido (sleep 0.1) logo a seguir
#   - Mede-se o tempo de resposta de cada comando
# ==============================================================================

set -e

# --- Cores para output legível ---
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# --- Funções auxiliares ---
header() {
    echo ""
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}========================================${NC}"
}

info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

warn() {
    echo -e "${YELLOW}[AVISO]${NC} $1"
}

# --- Compilação ---
header "Compilação do Projeto"
make clean > /dev/null 2>&1 || true
make
info "Compilação concluída com sucesso."

# --- Limpeza de ficheiros antigos ---
header "Limpeza de ficheiros anteriores"
rm -f historico.txt historico_fcfs.txt historico_rr.txt
rm -f fifos/*
info "Ficheiros de histórico e FIFOs limpos."

# ==============================================================================
# FASE 1: Teste com política FCFS
# ==============================================================================
header "FASE 1: Política FCFS (1 slot)"

info "A iniciar o controller com 1 slot e política FCFS..."
./bin/controller 1 fcfs &
CONTROLLER_PID=$!

# Espera pela criação dos FIFOs
sleep 1
info "Controller ativo (PID: $CONTROLLER_PID)."

# User 1 envia rajada de 4 comandos demorados
info "User 1: A submeter 4 comandos demorados (sleep 3)..."
./bin/runner -e 1 "sleep 3" &
./bin/runner -e 1 "sleep 3" &
./bin/runner -e 1 "sleep 3" &
./bin/runner -e 1 "sleep 3" &

# Pequena pausa para garantir que os comandos do User 1 chegam primeiro
sleep 0.2

# User 2 envia 1 comando rápido
info "User 2: A submeter 1 comando rápido (sleep 0.1)..."
./bin/runner -e 2 "sleep 0.1" &

# Aguardar que os runners tenham tempo de enviar os comandos
sleep 0.5

# Shutdown gracioso — espera que todos terminem
info "A enviar pedido de shutdown (aguarda fim dos processos)..."
./bin/runner -s

# Esperar que o controller termine
wait $CONTROLLER_PID 2>/dev/null || true
info "Controller FCFS encerrado."

# Guardar resultado
mv historico.txt historico_fcfs.txt
info "Resultado guardado em: historico_fcfs.txt"

# Pausa entre testes
sleep 1

# ==============================================================================
# FASE 2: Teste com política Round Robin
# ==============================================================================
header "FASE 2: Política RR (1 slot)"

info "A iniciar o controller com 1 slot e política RR..."
./bin/controller 1 RR &
CONTROLLER_PID=$!

# Espera pela criação dos FIFOs
sleep 1
info "Controller ativo (PID: $CONTROLLER_PID)."

# User 1 envia rajada de 4 comandos demorados (cenário idêntico)
info "User 1: A submeter 4 comandos demorados (sleep 3)..."
./bin/runner -e 1 "sleep 3" &
./bin/runner -e 1 "sleep 3" &
./bin/runner -e 1 "sleep 3" &
./bin/runner -e 1 "sleep 3" &

# Pequena pausa para garantir que os comandos do User 1 chegam primeiro
sleep 0.2

# User 2 envia 1 comando rápido
info "User 2: A submeter 1 comando rápido (sleep 0.1)..."
./bin/runner -e 2 "sleep 0.1" &

# Aguardar que os runners tenham tempo de enviar os comandos
sleep 0.5

# Shutdown gracioso — espera que todos terminem
info "A enviar pedido de shutdown (aguarda fim dos processos)..."
./bin/runner -s

# Esperar que o controller termine
wait $CONTROLLER_PID 2>/dev/null || true
info "Controller RR encerrado."

# Guardar resultado
mv historico.txt historico_rr.txt
info "Resultado guardado em: historico_rr.txt"

# ==============================================================================
# APRESENTAÇÃO DOS RESULTADOS
# ==============================================================================
header "Resultados Comparativos"

echo ""
echo -e "${YELLOW}--- FCFS (historico_fcfs.txt) ---${NC}"
cat historico_fcfs.txt
echo ""
echo -e "${YELLOW}--- Round Robin (historico_rr.txt) ---${NC}"
cat historico_rr.txt
echo ""

# Análise simples: tempo do User 2
echo -e "${CYAN}--- Análise de Starvation ---${NC}"
TEMPO_USER2_FCFS=$(grep "User: 2" historico_fcfs.txt | head -1 | grep -oP 'Duration: \K[0-9]+')
TEMPO_USER2_RR=$(grep "User: 2" historico_rr.txt | head -1 | grep -oP 'Duration: \K[0-9]+')

echo -e "Tempo de resposta do User 2 (cmd rápido):"
echo -e "  FCFS: ${RED}${TEMPO_USER2_FCFS} ms${NC} (esperou por TODOS os do User 1)"
echo -e "  RR:   ${GREEN}${TEMPO_USER2_RR} ms${NC} (intercalado com justiça)"
echo ""

if [[ -n "$TEMPO_USER2_FCFS" && -n "$TEMPO_USER2_RR" ]]; then
    if (( TEMPO_USER2_FCFS > TEMPO_USER2_RR )); then
        echo -e "${GREEN}[CONCLUSÃO]${NC} RR previne starvation — User 2 foi atendido mais cedo."
    else
        echo -e "${YELLOW}[CONCLUSÃO]${NC} Ambas as políticas tiveram desempenho similar neste cenário."
    fi
fi

echo ""
info "Teste de comparação de políticas concluído com sucesso!"
