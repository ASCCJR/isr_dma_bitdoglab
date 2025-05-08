#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h" // Inclui a biblioteca para usar o hardware DMA
#include "hardware/irq.h" // Inclui a biblioteca para configurar e gerenciar interrupções

// Definição dos pinos do LED RGB (cátodo comum)
#define LED_R_PIN 13  // Vermelho (resistor 220Ω)
#define LED_G_PIN 11  // Verde (resistor 220Ω)
#define LED_B_PIN 12  // Azul (resistor 150Ω) - maior brilho

// Buffers para transferências DMA
#define TAMANHO_BUFFER 16

// Três buffers de origem diferentes para as três transferências DMA (dados a serem copiados)
uint8_t origem1[TAMANHO_BUFFER] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
uint8_t origem2[TAMANHO_BUFFER] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
uint8_t origem3[TAMANHO_BUFFER] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160};

// Três buffers de destino para as três transferências DMA (onde os dados serão copiados)
uint8_t destino1[TAMANHO_BUFFER];
uint8_t destino2[TAMANHO_BUFFER];
uint8_t destino3[TAMANHO_BUFFER];

// Este código define e usa buffers de destino (destino1, destino2, destino3) 
// porque o objetivo da transferência DMA é copiar dados *dentro da memória* (RAM para RAM). 

// Variáveis de controle do DMA e Interrupção
int canal_dma; // Número do canal DMA utilizado
volatile int transferencia_atual = 0; // Conta qual transferência DMA terminou (controlado pela ISR)
volatile bool transferencia_completa = false; // Flag sinalizado pela ISR para o loop principal

// Função para apagar todos os LEDs 
void apagar_leds() {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
}

// Função de interrupção do DMA (ISR - Interrupt Service Routine)
// Esta função é chamada *automaticamente* pelo hardware quando uma transferência DMA completa.
void dma_isr() {
    // Limpar flag de interrupção para este canal DMA no grupo IRQ 0.
    // É crucial limpar o flag para evitar que a interrupção seja disparada novamente imediatamente.
    dma_hw->ints0 = 1u << canal_dma;
    
    // Indicar qual transferência foi completada
    transferencia_atual++;
    // Sinaliza para o loop principal que uma transferência terminou e os dados estão prontos
    transferencia_completa = true;
    
    // LED correspondente à transferência concluída (ação realizada dentro da ISR)
    switch (transferencia_atual) {
        case 1:
            apagar_leds();
            gpio_put(LED_R_PIN, 1);  // Acende LED vermelho
            printf("✅ Transferência DMA 1 finalizada!\n");
            break;
        case 2:
            apagar_leds();
            gpio_put(LED_G_PIN, 1);  // Acende LED verde
            printf("✅ Transferência DMA 2 finalizada!\n");
            break;
        case 3:
            apagar_leds();
            gpio_put(LED_B_PIN, 1);  // Acende LED azul
            printf("✅ Transferência DMA 3 finalizada!\n");
            // Reset para começar novamente após um intervalo
            transferencia_atual = 0;
            break;
    }
}

// Função para configurar e iniciar a próxima transferência DMA
void iniciar_proxima_transferencia() {
    // Obtém uma configuração padrão para o canal DMA
    dma_channel_config config = dma_channel_get_default_config(canal_dma);
    // Define que cada transferência moverá 1 byte (uint8_t)
    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    // Habilita o incremento do endereço de leitura (move para o próximo byte na origem)
    channel_config_set_read_increment(&config, true);
    // Habilita o incremento do endereço de escrita (move para o próximo byte no destino)
    channel_config_set_write_increment(&config, true);
    
    // Configura e inicia a transferência DMA com base na 'transferencia_atual'
    switch (transferencia_atual) {
        case 0: // Primeira transferência (origem1 -> destino1)
            printf("Iniciando transferência DMA 1...\n");
            // Configura e inicia o canal DMA
            dma_channel_configure(
                canal_dma,            // Canal a configurar
                &config,              // Configurações
                destino1,            // Endereço de destino na memória
                origem1,             // Endereço de origem na memória
                TAMANHO_BUFFER,       // Número total de transferências (bytes)
                true                 // Iniciar a transferência imediatamente após configurar
            );
            break;
        case 1: // Segunda transferência (origem2 -> destino2)
            printf("Iniciando transferência DMA 2...\n");
            // Configura e inicia o canal DMA para a segunda transferência
            dma_channel_configure(
                canal_dma,
                &config,
                destino2,       // Destino
                origem2,        // Origem
                TAMANHO_BUFFER, // Tamanho
                true            // Iniciar imediatamente
            );
            break;
        case 2: // Terceira transferência (origem3 -> destino3)
            printf("Iniciando transferência DMA 3...\n");
            // Configura e inicia o canal DMA para a terceira transferência
            dma_channel_configure(
                canal_dma,
                &config,
                destino3,       // Destino
                origem3,        // Origem
                TAMANHO_BUFFER, // Tamanho
                true            // Iniciar imediatamente
            );
            break;
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Iniciando exemplo de múltiplas transferências DMA com LED RGB...\n");
    
    // Inicializar pinos do LED RGB (não diretamente relacionado a DMA/IRQ)
    gpio_init(LED_R_PIN);
    gpio_init(LED_G_PIN);
    gpio_init(LED_B_PIN);
    
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);
    
    apagar_leds();

    // --- CONFIGURAÇÃO DA INTERRUPÇÃO DMA ---
    // Configurar canal DMA:
    // Solicita um canal DMA não utilizado. O 'true' marca o canal como usado.
    canal_dma = dma_claim_unused_channel(true);
    
    // Habilita a interrupção para o canal DMA escolhido no grupo IRQ 0 do controlador DMA.
    dma_channel_set_irq0_enabled(canal_dma, true);
    // Associa a função 'dma_isr' ao manipulador exclusivo para a interrupção DMA IRQ 0.
    irq_set_exclusive_handler(DMA_IRQ_0, dma_isr);
    // Habilita a interrupção DMA IRQ 0 no controlador de interrupção geral do chip.
    irq_set_enabled(DMA_IRQ_0, true);
    // --- FIM DA CONFIGURAÇÃO DA INTERRUPÇÃO DMA ---

    // Iniciar a primeira transferência DMA para dar início ao ciclo
    iniciar_proxima_transferencia();
    
    // Loop principal do programa
    while (true) {
        // A CPU fica aqui, esperando o sinal da interrupção.
        // Verifica se a flag 'transferencia_completa' foi definida pela ISR.
        if (transferencia_completa) {
            // Reseta a flag para esperar pela próxima interrupção
            transferencia_completa = false;
            
            // Espera um pouco (ação no loop principal após a interrupção)
            sleep_ms(1000);
            
            // Inicia a próxima transferência DMA (ação no loop principal, disparada pela conclusão da anterior via IRQ)
            iniciar_proxima_transferencia();
        }
        // tight_loop_contents() é uma otimização para loops vazios ou quase vazios.
        tight_loop_contents();
    }
}
