#!/bin/bash

echo "A compilar o projeto..."
make clean
make

echo "----------------------------------------"
echo "A iniciar o Controller (2 slots paralelos)"
echo "----------------------------------------"
# Arranca o controller em background (o & no fim faz isso)
./bin/controller 2 fcfs &
CONTROLLER_PID=$!

# Dá 1 segundo para o controller arrancar e criar os FIFOs
sleep 1

echo "A enviar uma enxurrada de comandos..."
# Enviamos vários comandos ao mesmo tempo (o & envia-os para background para não esperar)
./bin/runner -e 1 "sleep 4" &
./bin/runner -e 2 "sleep 4" &
./bin/runner -e 3 "ls -l | wc -l > contagem_script.txt" &
./bin/runner -e 4 "sleep 2" &
./bin/runner -e 5 "echo 'Teste Final' > final.txt" &

# Esperamos meio segundo para dar tempo de os comandos chegarem à fila
sleep 0.5

echo "----------------------------------------"
echo "A consultar o estado (Deve mostrar 2 a executar e 3 na fila)"
echo "----------------------------------------"
./bin/runner -c

echo "----------------------------------------"
echo "A enviar comando de Shutdown..."
echo "----------------------------------------"
./bin/runner -s

# Espera que o controller desligue de forma limpa
wait $CONTROLLER_PID

echo "----------------------------------------"
echo "Teste concluído com sucesso!"
echo "Verifica o historico.txt para veres os tempos."