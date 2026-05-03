#!/bin/bash
# ==============================================================================
# Script 2: Avaliação de Paralelismo — 1 Slot vs 4 Slots (Round Robin)
# ==============================================================================
# Objetivo: Medir o impacto do número de slots paralelos no desempenho global.
# Com 1 slot, os comandos são serializados; com 4 slots, executam em paralelo.
#
# Cenário:
#   - Política RR em ambos os testes
#   - 10 comandos misturados de 4 utilizadores diferentes
#   - Compara o tempo total e tempos individuais
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

# --- Carga mista de 10 comandos de 4 utilizadores ---
submeter_carga() {
    info "User 1: sleep 2 (demorado)"
    ./bin/runner -e 1 "sleep 2" &

    info "User 2: sleep 1 (médio)"
    ./bin/runner -e 2 "sleep 1" &

    info "User 3: sleep 0.5 (rápido)"
    ./bin/runner -e 3 "sleep 0.5" &

    info "User 4: sleep 2 (demorado)"
    ./bin/runner -e 4 "sleep 2" &

    info "User 1: sleep 1 (médio)"
    ./bin/runner -e 1 "sleep 1" &

    info "User 2: sleep 0.5 (rápido)"
    ./bin/runner -e 2 "sleep 0.5" &

    info "User 3: sleep 2 (demorado)"
    ./bin/runner -e 3 "sleep 2" &

    info "User 4: sleep 1 (médio)"
    ./bin/runner -e 4 "sleep 1" &

    info "User 1: sleep 0.5 (rápido)"
    ./bin/runner -e 1 "sleep 0.5" &

    info "User 2: sleep 2 (demorado)"
    ./bin/runner -e 2 "sleep 2" &
}

# --- Compilação ---
header "Compilação do Projeto"
make clean > /dev/null 2>&1 || true
make
info "Compilação concluída com sucesso."

# --- Limpeza de ficheiros antigos ---
header "Limpeza de ficheiros anteriores"
rm -f historico.txt historico_1slot.txt historico_4slots.txt
rm -f fifos/*
info "Ficheiros de histórico e FIFOs limpos."

# ==============================================================================
# FASE 1: RR com 1 slot paralelo
# ==============================================================================
header "FASE 1: RR com 1 Slot Paralelo"

info "A iniciar o controller com 1 slot e política RR..."
./bin/controller 1 RR &
CONTROLLER_PID=$!

# Espera pela criação dos FIFOs
sleep 1
info "Controller ativo (PID: $CONTROLLER_PID)."

# Regista tempo de início
INICIO_1SLOT=$(date +%s%3N)

# Submeter os 10 comandos
info "A submeter carga mista de 10 comandos (4 utilizadores)..."
submeter_carga

# Aguardar que os runners tenham tempo de enviar os comandos
sleep 1

# Shutdown gracioso
info "A enviar pedido de shutdown..."
./bin/runner -s

# Esperar que o controller termine
wait $CONTROLLER_PID 2>/dev/null || true

# Regista tempo de fim
FIM_1SLOT=$(date +%s%3N)
TOTAL_1SLOT=$((FIM_1SLOT - INICIO_1SLOT))

info "Controller (1 slot) encerrado. Tempo total: ${TOTAL_1SLOT} ms"

# Guardar resultado
mv historico.txt historico_1slot.txt
info "Resultado guardado em: historico_1slot.txt"

# Pausa entre testes
sleep 1

# ==============================================================================
# FASE 2: RR com 4 slots paralelos
# ==============================================================================
header "FASE 2: RR com 4 Slots Paralelos"

info "A iniciar o controller com 4 slots e política RR..."
./bin/controller 4 RR &
CONTROLLER_PID=$!

# Espera pela criação dos FIFOs
sleep 1
info "Controller ativo (PID: $CONTROLLER_PID)."

# Regista tempo de início
INICIO_4SLOTS=$(date +%s%3N)

# Submeter os mesmos 10 comandos
info "A submeter carga mista de 10 comandos (4 utilizadores)..."
submeter_carga

# Aguardar que os runners tenham tempo de enviar os comandos
sleep 1

# Shutdown gracioso
info "A enviar pedido de shutdown..."
./bin/runner -s

# Esperar que o controller termine
wait $CONTROLLER_PID 2>/dev/null || true

# Regista tempo de fim
FIM_4SLOTS=$(date +%s%3N)
TOTAL_4SLOTS=$((FIM_4SLOTS - INICIO_4SLOTS))

info "Controller (4 slots) encerrado. Tempo total: ${TOTAL_4SLOTS} ms"

# Guardar resultado
mv historico.txt historico_4slots.txt
info "Resultado guardado em: historico_4slots.txt"

# ==============================================================================
# APRESENTAÇÃO DOS RESULTADOS
# ==============================================================================
header "Resultados Comparativos — Impacto do Paralelismo"

echo ""
echo -e "${YELLOW}--- 1 Slot Paralelo (historico_1slot.txt) ---${NC}"
cat historico_1slot.txt
echo ""
echo -e "${YELLOW}--- 4 Slots Paralelos (historico_4slots.txt) ---${NC}"
cat historico_4slots.txt
echo ""

# Análise: tempo médio por política
echo -e "${CYAN}--- Análise de Desempenho ---${NC}"
echo ""
echo -e "Tempo total de execução (wall-clock):"
echo -e "  1 Slot:  ${RED}${TOTAL_1SLOT} ms${NC}"
echo -e "  4 Slots: ${GREEN}${TOTAL_4SLOTS} ms${NC}"
echo ""

# Calcular a média dos tempos de resposta individuais
if command -v awk &> /dev/null; then
    MEDIA_1SLOT=$(grep -oP 'Duration: \K[0-9]+' historico_1slot.txt | awk '{s+=$1; n++} END {if(n>0) printf "%.0f", s/n}')
    MEDIA_4SLOTS=$(grep -oP 'Duration: \K[0-9]+' historico_4slots.txt | awk '{s+=$1; n++} END {if(n>0) printf "%.0f", s/n}')

    echo -e "Tempo médio de resposta por comando:"
    echo -e "  1 Slot:  ${RED}${MEDIA_1SLOT} ms${NC}"
    echo -e "  4 Slots: ${GREEN}${MEDIA_4SLOTS} ms${NC}"
    echo ""
fi

if [[ -n "$TOTAL_1SLOT" && -n "$TOTAL_4SLOTS" ]]; then
    if (( TOTAL_4SLOTS < TOTAL_1SLOT )); then
        SPEEDUP=$(awk "BEGIN {printf \"%.1fx\", $TOTAL_1SLOT / $TOTAL_4SLOTS}")
        echo -e "${GREEN}[CONCLUSÃO]${NC} 4 slots é ~${SPEEDUP} mais rápido que 1 slot."
        echo -e "             O paralelismo reduz significativamente o tempo total."
    else
        echo -e "${YELLOW}[CONCLUSÃO]${NC} Resultados similares — a carga pode ser insuficiente para saturar."
    fi
fi

echo ""
info "Teste de paralelismo concluído com sucesso!"
