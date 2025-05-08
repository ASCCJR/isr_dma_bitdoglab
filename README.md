# isr_dma_bitdoglab

## Exemplo de DMA com Interrupções e Feedback de LED RGB no Raspberry Pi Pico

O projeto demonstra o uso de DMA (Direct Memory Access) com interrupções no Raspberry Pi Pico, combinado com feedback visual via LEDs RGB. Foi desenvolvido para fins educacionais durante a Residência em Sistemas Embarcados, com o objetivo de explorar transferências de memória assíncronas e gerenciamento de hardware de baixo nível.

## ✨ Funcionalidades
- **Transferências DMA sequenciais** entre buffers de memória.
- **Interrupções configuráveis** para notificar conclusão de operações.
- **Feedback visual com LED RGB**:
  - Vermelho: Transferência 1 concluída.
  - Verde: Transferência 2 concluída.
  - Azul: Transferência 3 concluída.
- Ciclo contínuo de 3 transferências com delay de 1 segundo.

## 🧍 Estrutura do Código
### Principais Componentes
1. **Buffers de Memória**:
   - 3 buffers de origem (`origem1`, `origem2`, `origem3`) com padrões numéricos distintos.
   - 3 buffers de destino (`destino1`, `destino2`, `destino3`) para receber os dados via DMA.

2. **Configuração DMA**:
   - Canal único reutilizado para múltiplas transferências.
   - Modo: incremento automático de endereço (leitura/escrita).
   - Tamanho de transferência: 1 byte (`DMA_SIZE_8`).

3. **Interrupções (IRQ)**:
   - Handler `dma_isr()` gerencia notificações de conclusão.
   - Controle de estado via `transferencia_atual` e `transferencia_completa`.

4. **Controle de LEDs**:
   - Função `apagar_leds()` desativa todos os LEDs antes de atualizar o estado.
   - Lógica de acionamento dentro da ISR para resposta imediata.

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
