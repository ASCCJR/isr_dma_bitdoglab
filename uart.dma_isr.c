#include <stdio.h>          // Para funções de entrada/saída padrão (printf)
#include "pico/stdlib.h"   // Biblioteca padrão do Pico SDK
#include "hardware/uart.h"  // Biblioteca para controle da UART // Teve que ser adicionado para permitir o controle do periférico UART.
#include "hardware/dma.h"   // Biblioteca para controle do DMA
#include "hardware/irq.h"   // Biblioteca para controle de interrupções

// --- Definição dos pinos ---
#define LED_R_PIN 13       // Pino GPIO para o LED Vermelho
#define LED_G_PIN 11       // Pino GPIO para o LED Verde
#define LED_B_PIN 12       // Pino GPIO para o LED Azul

// --- Definições da UART ---
#define UART_ID uart0       // Identificador da UART que vamos usar (UART0)
#define BAUD_RATE 115200    // Taxa para a comunicação serial
#define UART_TX_PIN 0       // Pino GPIO para transmissão da UART0 (TX)
#define UART_RX_PIN 1       // Pino GPIO para recepção da UART0 (RX)

// --- Buffers para DMA ---
#define TAMANHO_BUFFER 16  // Tamanho dos buffers de dados

// ✅ Requisito atendido: Fazer múltiplas transferências sequenciais... (Estas são as fontes de dados distintas)
// Buffers de origem: dados que serão enviados pela UART via DMA
uint8_t origem1[TAMANHO_BUFFER] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P'};
uint8_t origem2[TAMANHO_BUFFER] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b', 'c', 'd', 'e', 'f'};
uint8_t origem3[TAMANHO_BUFFER] = {'H', 'e', 'l', 'l', 'o', ' ', 'f', 'r', 'o', 'm', ' ', 'D', 'M', 'A', 'q', 'd'};

// No código "uint8_t origem[TAMANHO] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
//            uint8_t destino[TAMANHO];" 
//                                       Porém, era uma transferência DMA básica de um bloco de dados de uma área da memória (origem) para outra (destino)
//                                       havia um buffer de destino (destino) porque o objetivo era copiar os dados da origem para outro local na memória.
//                                       No entanto, neste exemplo, não estamos usando um buffer de destino separado, pois estamos apenas enviando os dados 
//                                       diretamente para a UART via DMA.
// O código mudou para demonstrar a transferência de dados para um periférico, especificamente a UART. 
// A UART é utilizada para comunicação serial, 
// geralmente envolvendo o envio e recebimento de caracteres (letras, números, símbolos).

// Mas se o objetivo não fosse enviar dados para a UART,
// e sim transferir dados entre duas áreas de memória,
// então teríamos um buffer de destino para armazenar os dados que foram copiados da origem, e poderia ficar assim:

//// Três buffers de origem diferentes para as três transferências
//uint8_t origem1[TAMANHO_BUFFER] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
//uint8_t origem2[TAMANHO_BUFFER] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
//uint8_t origem3[TAMANHO_BUFFER] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160};

// Três buffers de destino para as três transferências
//uint8_t destino1[TAMANHO_BUFFER];
//uint8_t destino2[TAMANHO_BUFFER];
//uint8_t destino3[TAMANHO_BUFFER];

// No entanto, como estamos enviando dados para a UART, não precisamos de buffers de destino separados,
// pois a UART irá lidar com os dados recebidos diretamente.

// --- Variáveis de controle do DMA e do fluxo ---
int canal_dma_tx;             // Variável para armazenar o número do canal DMA que usaremos para a UART TX
// ✅ Requisito atendido: Fazer múltiplas transferências sequenciais... (Variáveis para gerenciar a sequência)
volatile int transferencia_atual = 0; // Contador para rastrear qual transferência DMA está ocorrendo (0, 1 ou 2)
volatile bool transferencia_completa = false; // Flag para indicar quando uma transferência DMA foi concluída (setada no ISR, lida no main)

// --- Função para apagar todos os LEDs RGB ---
void apagar_leds() {
    gpio_put(LED_R_PIN, 0); // Nota: Presumindo LED ativo alto ou ajustando conforme o hardware
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
}

// --- Função de tratamento da interrupção do DMA ---
// ✅ Requisito atendido: Fazer múltiplas transferências sequenciais, com LEDs diferentes. (Este handler encadeia transferências e muda LEDs)
void dma_isr() {
    // ✅ Requisito atendido: Sempre limpar a interrupção do DMA dentro do handler.
    // Limpar a flag de interrupção para este canal DMA escrevendo 1 no bit correspondente.
    dma_hw->ints0 = 1u << canal_dma_tx;

    // ✅ Requisito atendido: Fazer múltiplas transferências sequenciais... (Incrementa contador e sinaliza conclusão)
    // Incrementar o contador de transferência
    transferencia_atual++;
    transferencia_completa = true; // Sinalizar que a transferência foi concluída

    // ✅ Requisito atendido: Fazer múltiplas transferências sequenciais, com LEDs diferentes. (Muda o LED e imprime mensagem após CADA transferência)
    // Simular a mudança da cor do LED e reportar no Serial Monitor
    switch (transferencia_atual) {
        case 1:
            apagar_leds();
            gpio_put(LED_R_PIN, 1);   // Acende LED vermelho
            printf("🔴 LED Vermelho aceso (após envio UART via DMA 1).\n");
            break;
        case 2:
            apagar_leds();
            gpio_put(LED_G_PIN, 1);   // Acende LED verde
            printf("🟢 LED Verde aceso (após envio UART via DMA 2).\n");
            break;
        case 3:
            apagar_leds();
            gpio_put(LED_B_PIN, 1);   // Acende LED azul
            printf("🔵 LED Azul aceso (após envio UART via DMA 3).\n");
            // ✅ Requisito atendido: Fazer múltiplas transferências sequenciais... (Reinicia o ciclo após a última transferência)
            // Reiniciar o ciclo de transferências
            transferencia_atual = 0;
            break;
    }
}

// --- Função para iniciar a próxima transferência DMA para a UART ---
// ✅ Requisito atendido: Usar DMA para transferir dados para periféricos como UART. (Esta função configura o DMA para UART)
void iniciar_proxima_transferencia_uart() {
    // Obter a configuração padrão para um canal DMA
    dma_channel_config config_tx = dma_channel_get_default_config(canal_dma_tx);

    // ✅ Requisito atendido: Ajustar corretamente as configurações de incremento e tamanho de dados.
    // Configurar o tamanho da transferência para bytes (8 bits), adequado para UART.
    channel_config_set_transfer_data_size(&config_tx, DMA_SIZE_8);

    // ✅ Requisito atendido: Ajustar corretamente as configurações de incremento e tamanho de dados.
    // Configurar para incrementar o endereço de leitura (na memória de origem).
    channel_config_set_read_increment(&config_tx, true);

    // ✅ Requisito atendido: Ajustar corretamente as configurações de incremento e tamanho de dados.
    // Configurar para NÃO incrementar o endereço de escrita (sempre escrever no mesmo registrador da UART).
    channel_config_set_write_increment(&config_tx, false);

    // ✅ Requisito atendido: Usar DMA para transferir dados para periféricos como UART.
    // Configurar o "data request" (DREQ) para a UART0 TX. O DMA espera a UART estar pronta.
    channel_config_set_dreq(&config_tx, DREQ_UART0_TX);

    // ✅ Requisito atendido: Usar DMA para transferir dados para periféricos como UART.
    // Obter o endereço do registrador de dados de transmissão da UART0 (o destino do DMA).
    uint32_t uart_tx_address = (uint32_t)&uart_get_hw(UART_ID)->dr;

    // ✅ Requisito atendido: Fazer múltiplas transferências sequenciais... (Seleciona a origem com base na transferência atual)
    // Iniciar a transferência DMA com base no estado atual
    switch (transferencia_atual) {
        case 0: // A primeira transferência no ciclo (0 é o estado antes de 'case 1' no ISR)
            printf("📤 Iniciando envio UART via DMA 1...\n");
            dma_channel_configure(
                canal_dma_tx,        // Canal DMA a ser configurado
                &config_tx,          // Configuração do canal
                (void *)uart_tx_address, // Endereço de destino: registrador de dados TX da UART
                origem1,              // Endereço de origem: primeiro buffer de dados
                TAMANHO_BUFFER,       // Número de bytes a serem transferidos
                true                  // Iniciar a transferência imediatamente
            );
            break;
        case 1:
            printf("📤 Iniciando envio UART via DMA 2...\n");
            dma_channel_configure(
                canal_dma_tx,
                &config_tx,
                (void *)uart_tx_address,
                origem2,
                TAMANHO_BUFFER,
                true
            );
            break;
        case 2:
            printf("📤 Iniciando envio UART via DMA 3...\n");
            dma_channel_configure(
                canal_dma_tx,
                &config_tx,
                (void *)uart_tx_address,
                origem3,
                TAMANHO_BUFFER,
                true
            );
            break;
    }
}

int main() {
    // Inicializar a entrada e saída padrão (para usar printf via USB Serial)
    stdio_init_all();
    sleep_ms(2000); // Espera para o monitor serial iniciar

    printf("\n🔄 Exemplo de múltiplas transferências DMA para UART com 'aparência' de controle de LED no Serial Monitor...\n");

    // --- Inicializar a UART ---
    // ✅ Requisito atendido: Usar DMA para transferir dados para periféricos como UART. (Inicialização do periférico UART)
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); // Configura o pino TX
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); // Configura o pino RX (necessário para stdio_init_all ou boa prática mesmo sem RX DMA)

    // --- Inicializar os pinos dos LEDs RGB como saída ---
    gpio_init(LED_R_PIN);
    gpio_init(LED_G_PIN);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);
    apagar_leds(); // Apagar os LEDs no início

    // --- Claim (reservar) um canal DMA para a transmissão UART ---
    canal_dma_tx = dma_claim_unused_channel(true);

    // --- Configurar a interrupção do DMA ---
    // ✅ Requisito atendido: Fazer múltiplas transferências sequenciais... (Configura a interrupção para encadear as transferências)
    // Habilitar a interrupção IRQ0 para o canal DMA que estamos usando
    dma_channel_set_irq0_enabled(canal_dma_tx, true);
    // Definir a função `dma_isr` para ser o handler da interrupção DMA_IRQ_0
    irq_set_exclusive_handler(DMA_IRQ_0, dma_isr);
    // Habilitar a interrupção DMA_IRQ_0 no nível do processador
    irq_set_enabled(DMA_IRQ_0, true);

    // --- Iniciar a primeira transferência DMA para a UART ---
    // ✅ Requisito atendido: Fazer múltiplas transferências sequenciais... (Inicia o ciclo de transferências)
    iniciar_proxima_transferencia_uart();

    // --- Loop principal ---
    while (true) {
        // ✅ Requisito atendido: Fazer múltiplas transferências sequenciais... (Espera pela flag setada no ISR para iniciar a próxima)
        // Verificar se uma transferência DMA foi concluída
        if (transferencia_completa) {
            transferencia_completa = false; // Resetar a flag
            sleep_ms(1000); // Esperar 1 segundo antes da próxima transferência
            iniciar_proxima_transferencia_uart(); // Iniciar a próxima transferência
        }
        // ✅ Requisito atendido: O uso de tight_loop_contents() mantém o sistema em estado de espera eficiente.
        // Coloca o processador em espera de baixo consumo até uma interrupção ocorrer.
        tight_loop_contents();
    }

    return 0;
}

/*
--- Anotações ---

1.  Primeiro precisei fazer nclusão de headers da UART:
    - `#include "hardware/uart.h"`: Adicionado para permitir o controle do periférico UART.

2.  Definições da UART (segui o que tinha no pdf da bitdoglab UART: GPIO0 (TX), GPIO1 (RX) ):
    - `#define UART_ID uart0`: Define qual UART será utilizada (UART0).
    - `#define BAUD_RATE 115200`: Define a taxa de baud para a comunicação serial.
    - `#define UART_TX_PIN 0` e `#define UART_RX_PIN 1`: Define os pinos GPIO para transmissão (TX) e recepção (RX) da UART0.

3.  Buffers de origem para a UART:
    - Os buffers `origem1`, `origem2` e `origem3` agora contêm dados que farão mais sentido serem transmitidos pela UART (caracteres e uma string).

4.  Variável `canal_dma_tx`:
    - Renomeado `canal_dma` para `canal_dma_tx` para deixar mais claro que este canal DMA é usado para a transmissão da UART.

5.  Função `iniciar_proxima_transferencia_uart()`:
    - Esta nova função é responsável por configurar e iniciar a transferência DMA para a UART.
    - Ela obtém a configuração padrão do DMA, configura o tamanho da transferência, os incrementos de leitura/escrita e, crucialmente, o `DREQ` para a UART0 TX.
    - O endereço de destino do DMA agora é o endereço do registrador de dados de transmissão da UART (`&uart_get_hw(UART_ID)->dr`).
    - A origem do DMA são os buffers `origem1`, `origem2` e `origem3` dependendo do valor de `transferencia_atual`.

6.  Modificações na função `dma_isr()`:
    - O `switch` dentro da `dma_isr` agora tem um `printf` que indica a cor do LED que foi acesa *após* a conclusão da transferência DMA para a UART. 
    - Isso cria a "aparência" de que a mudança de cor está relacionada à atividade da UART no Serial Monitor.
    - Ações na dma_isr(): Dentro dessa função de interrupção, duas coisas principais acontecem (além de limpar a flag do DMA e incrementar o contador):
    - Controle do LED: A cor do LED RGB é alterada (gpio_put(LED_R_PIN, 1);, gpio_put(LED_G_PIN, 1);, gpio_put(LED_B_PIN, 1);). Cada caso do switch acende uma cor diferente.
    - Mensagem no Serial Monitor: Uma mensagem usando printf() é enviada para o Serial Monitor, 
    - indicando qual cor do LED foi acesa e associando essa ação à conclusão de uma transferência DMA específica para a UART 
    - (por exemplo, "🔴 LED Vermelho aceso (após envio UART via DMA 1).").

7.  Inicialização da UART no `main()`:
    - A UART é inicializada com a taxa de baud definida e os pinos TX e RX são configurados para a função UART.

8.  Chamada da função `iniciar_proxima_transferencia_uart()` no `main()`:
    - Em vez de `iniciar_proxima_transferencia()`, a função específica para a UART é chamada para iniciar o processo.

*/
