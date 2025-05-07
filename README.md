# isr_dma_bitdoglab

## Código Fonte Básico

```c
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#define LED_PIN 11
#define TAMANHO 16

uint8_t origem[TAMANHO] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t destino[TAMANHO];

int canal_dma;

void dma_isr() {
    dma_hw->ints0 = 1u << canal_dma;
    gpio_put(LED_PIN, 1);
    printf("✅ Transferência DMA finalizada!\n");
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Iniciando exemplo de DMA com interrupção...\n");

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    canal_dma = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(canal_dma);

    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_read_increment(&config, true);
    channel_config_set_write_increment(&config, true);

    dma_channel_set_irq0_enabled(canal_dma, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_isr);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_channel_configure(
        canal_dma,
        &config,
        destino,
        origem,
        TAMANHO,
        true
    );

    while (true) {
        tight_loop_contents();
    }
}
