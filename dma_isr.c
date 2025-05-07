#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

// Definição dos pinos do LED RGB (cátodo comum)
#define LED_R_PIN 13  // Vermelho (resistor 220Ω)
#define LED_G_PIN 11  // Verde (resistor 220Ω)
#define LED_B_PIN 12  // Azul (resistor 150Ω) - maior brilho

// Buffers para transferências DMA
#define TAMANHO_BUFFER 16

// Três buffers de origem diferentes para as três transferências
uint8_t origem1[TAMANHO_BUFFER] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t origem2[TAMANHO_BUFFER] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
uint8_t origem3[TAMANHO_BUFFER] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160};

// Três buffers de destino para as três transferências
uint8_t destino1[TAMANHO_BUFFER];
uint8_t destino2[TAMANHO_BUFFER];
uint8_t destino3[TAMANHO_BUFFER];

// Este código define e usa buffers de destino (destino1, destino2, destino3) 
// porque o objetivo da transferência DMA é copiar dados dentro da memória. 
// No código com UART, o destino da transferência DMA era o registrador de dados da UART.

// Variáveis de controle do DMA
int canal_dma;
volatile int transferencia_atual = 0;
volatile bool transferencia_completa = false;

// Função para apagar todos os LEDs
void apagar_leds() {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
}

// Função de interrupção do DMA
void dma_isr() {
    // Limpar flag de interrupção
    dma_hw->ints0 = 1u << canal_dma;
    
    // Indicar qual transferência foi completada
    transferencia_atual++;
    transferencia_completa = true;
    
    // LED correspondente à transferência concluída
    switch (transferencia_atual) {
        case 1:
            apagar_leds();
            gpio_put(LED_R_PIN, 1);  // Acende LED vermelho
            printf("✅ Transferência DMA 1 finalizada!\n");
            break;
        case 2:
            apagar_leds();
            gpio_put(LED_G_PIN, 1);  // Acende LED verde
            printf("✅ Transferência DMA 2 finalizada!\n");
            break;
        case 3:
            apagar_leds();
            gpio_put(LED_B_PIN, 1);  // Acende LED azul
            printf("✅ Transferência DMA 3 finalizada!\n");
            // Reset para começar novamente após um intervalo
            transferencia_atual = 0;
            break;
    }
}

// Função para iniciar a próxima transferência DMA
void iniciar_proxima_transferencia() {
    dma_channel_config config = dma_channel_get_default_config(canal_dma);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_read_increment(&config, true);
    channel_config_set_write_increment(&config, true);
    
    switch (transferencia_atual) {
        case 0:
            // Primeira transferência (origem1 -> destino1)
            printf("Iniciando transferência DMA 1...\n");
            dma_channel_configure(
                canal_dma,
                &config,
                destino1,       // Destino
                origem1,        // Origem
                TAMANHO_BUFFER, // Tamanho
                true            // Iniciar imediatamente
            );
            break;
        case 1:
            // Segunda transferência (origem2 -> destino2)
            printf("Iniciando transferência DMA 2...\n");
            dma_channel_configure(
                canal_dma,
                &config,
                destino2,       // Destino
                origem2,        // Origem
                TAMANHO_BUFFER, // Tamanho
                true            // Iniciar imediatamente
            );
            break;
        case 2:
            // Terceira transferência (origem3 -> destino3)
            printf("Iniciando transferência DMA 3...\n");
            dma_channel_configure(
                canal_dma,
                &config,
                destino3,       // Destino
                origem3,        // Origem
                TAMANHO_BUFFER, // Tamanho
                true            // Iniciar imediatamente
            );
            break;
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Iniciando exemplo de múltiplas transferências DMA com LED RGB...\n");
    
    // Inicializar pinos do LED RGB
    gpio_init(LED_R_PIN);
    gpio_init(LED_G_PIN);
    gpio_init(LED_B_PIN);
    
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);
    
    apagar_leds();

    // Configurar canal DMA
    canal_dma = dma_claim_unused_channel(true);
    
    // Configurar interrupção DMA
    dma_channel_set_irq0_enabled(canal_dma, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_isr);
    irq_set_enabled(DMA_IRQ_0, true);
    
    // Iniciar primeira transferência
    iniciar_proxima_transferencia();
    
    // Loop principal
    while (true) {
        if (transferencia_completa) {
            transferencia_completa = false;
            sleep_ms(1000);  // Esperar 1 segundo entre transferências
            iniciar_proxima_transferencia();
        }
        tight_loop_contents();
    }
}