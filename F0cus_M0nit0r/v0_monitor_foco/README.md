# V0 - Monitor de Sessão de Estudo (ESP32 + Wokwi + PlatformIO)

## Objetivo
Esta V0 valida a arquitetura da aplicação antes do hardware real chegar.

## O que esta versão faz
- Inicia/encerra uma sessão com botão
- Simula presença por um slide switch
- Mede distância com HC-SR04
- Simula temperatura por potenciômetro
- Simula ruído por potenciômetro
- Calcula presença, ausência, interrupções e score de foco
- Aciona LEDs de estado e buzzer
- Gera relatório final no monitor serial

## Arquivos
- `platformio.ini`: configuração do PlatformIO
- `wokwi.toml`: integração Wokwi + PlatformIO
- `diagram.json`: hardware simulado
- `src/main.cpp`: aplicação principal

## Estados principais
- `IDLE`
- `SESSION_ACTIVE`
- `SESSION_ENDED`

## Estados de atenção
- `FOCUSED`
- `UNSTABLE`
- `TEMPORARY_AWAY`
- `ABSENT`
- `ENV_WARNING`

## Fluxo de uso
1. Abra a pasta no VS Code.
2. Compile com PlatformIO.
3. Execute no Wokwi.
4. Pressione o botão verde para iniciar.
5. Use o slide switch para simular presença.
6. Ajuste o HC-SR04 para variar a distância.
7. Ajuste os dois potenciômetros para simular temperatura e ruído.
8. Pressione o botão novamente para encerrar e imprimir o relatório.

## Observações
- A temperatura e o ruído estão simulados por potenciômetros para simplificar a V0.
- Na V1, os canais de temperatura e ruído podem ser trocados por sensores reais.
- O projeto foi montado sobre ESP32 simulado porque isso aproxima a V0 da V1 final com FreeRTOS nativo.
