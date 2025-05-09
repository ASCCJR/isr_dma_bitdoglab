# Exemplo de Transferência DMA Sequencial para UART no Raspberry Pi Pico

Este projeto demonstra o uso do controlador DMA (Direct Memory Access) do Raspberry Pi Pico para transferir dados de diferentes locais da memória sequencialmente para o periférico UART (Universal Asynchronous Receiver-Transmitter), utilizando interrupções para gerenciar o fluxo e sinalização visual com LEDs (indicada no monitor serial).

## ✨ Requisitos Solicitados
Este exemplo de código foi desenvolvido especificamente para atender aos seguintes requisitos relacionados ao uso de DMA com periféricos:

1.  Sempre limpar a interrupção do DMA dentro do handler.
2.  Ajustar corretamente as configurações de incremento e tamanho de dados.
3.  O uso de `tight_loop_contents()` mantém o sistema em estado de espera eficiente.
4.  Fazer múltiplas transferências sequenciais, com LEDs diferentes.
5.  Usar DMA para transferir dados para periféricos como UART.

## ✅ Como os Requisitos Foram Atendidos

O código implementa as seguintes funcionalidades para cumprir os requisitos:

1.  **Sempre limpar a interrupção do DMA dentro do handler:** Na função `dma_isr`, a flag de interrupção para o canal DMA específico é limpa escrevendo `1` no bit correspondente do registrador `dma_hw->ints0`, garantindo que a interrupção seja tratada uma vez por conclusão de transferência.
2.  **Ajustar corretamente as configurações de incremento e tamanho de dados:** A função `iniciar_proxima_transferencia_uart` configura o tamanho da transferência para 8 bits (`DMA_SIZE_8`), o incremento de leitura como verdadeiro (`read_increment=true`) para avançar no buffer de origem, e o incremento de escrita como falso (`write_increment=false`) para sempre escrever no registrador fixo de dados da UART TX.
3.  **O uso de `tight_loop_contents()` mantém o sistema em estado de espera eficiente:** No loop principal (`while(true)`), após verificar a flag de conclusão da transferência, a chamada `tight_loop_contents()` coloca o núcleo do processador em um estado de baixo consumo (aguardando por um evento, que neste caso é a próxima interrupção do DMA), evitando o uso desnecessário de ciclos de CPU.
4.  **Fazer múltiplas transferências sequenciais, com LEDs diferentes:** O código define três buffers de dados distintos (`origem1`, `origem2`, `origem3`). A variável `transferencia_atual` e a flag `transferencia_completa` (setada no ISR) controlam a sequência. O loop principal aguarda a conclusão de uma transferência e, em seguida, chama `iniciar_proxima_transferencia_uart`, que usa `transferencia_atual` para selecionar o próximo buffer a ser enviado. O handler de interrupção (`dma_isr`), por sua vez, muda o estado de um LED (simulado com mensagens no serial) com base em qual transferência (1ª, 2ª, ou 3ª) acabou de ser concluída.
5.  **Usar DMA para transferir dados para periféricos como UART:** A função `iniciar_proxima_transferencia_uart` configura o destino do DMA para o endereço do registrador de dados de transmissão da UART (`&uart_get_hw(UART_ID)->dr`) e configura o DREQ (`DREQ_UART0_TX`), garantindo que o DMA transfira dados para a UART apenas quando ela estiver pronta para recebê-los.

## ⚙️ Funcionalidades Principais

-   **Transferência DMA para UART TX:** Envia dados diretamente da memória para o periférico UART sem intervenção contínua da CPU.
-   **Múltiplas Origens de Dados:** Demonstra a transferência de dados a partir de três buffers distintos.
-   **Sequenciamento via Interrupção:** Utiliza a interrupção de conclusão do DMA para sinalizar o fim de uma transferência e disparar a próxima.
-   **Sinalização Visual (no Serial):** Mensagens associadas a LEDs Vermelho, Verde e Azul são impressas no monitor serial após a conclusão de cada uma das três transferências sequenciais.
-   **Espera Eficiente:** O loop principal aguarda as interrupções do DMA de forma otimizada usando `tight_loop_contents()`.
-   **Configuração Correta do Canal DMA:** Ajustes precisos para o tamanho dos dados e regras de incremento/decremento para a comunicação com a UART.

## 🔌 Diagrama de Conexões (Referência)

Embora a sinalização dos LEDs seja principalmente ilustrada no monitor serial neste código, para uma implementação física os pinos seriam usados como segue:

| Pino Pico | Componente      | Detalhe        |
|-----------|-----------------|----------------|
| GPIO0     | UART0 TX        | Transmissão    |
| GPIO1     | UART0 RX        | Recepção (para stdio) |
| GPIO13    | LED Vermelho    | Sinalização 1  |
| GPIO11    | LED Verde       | Sinalização 2  |
| GPIO12    | LED Azul        | Sinalização 3  |

*(Note: A funcionalidade de "acender" os LEDs físicos é presente no código, mas a demonstração principal da sequência e feedback está no `printf` via serial)*

## 🧠 Estrutura do Código

-   **Buffers de Origem:** Três arrays (`origem1`, `origem2`, `origem3`) contendo os dados a serem enviados.
-   **Variáveis de Controle:** `canal_dma_tx`, `transferencia_atual`, `transferencia_completa` para gerenciar o canal DMA e o estado do sequenciamento.
-   **`apagar_leds()`:** Função utilitária para desligar os LEDs (configurados como ativo alto neste exemplo, `gpio_put` 1 liga, 0 desliga). *Atenção: No código fornecido, `gpio_put(LED_X_PIN, 1)` liga o LED, e `gpio_put(LED_X_PIN, 0)` apaga - a função `apagar_leds` na verdade **liga** os LEDs. O código original talvez usasse LEDs ativo baixo ou houvesse uma inversão na lógica. O `printf` no ISR é a indicação principal da sequência.*
-   **`dma_isr()`:** O manipulador (handler) da interrupção do DMA. É chamado ao final de cada transferência. Ele limpa a flag de interrupção, avança o contador da sequência, setta a flag `transferencia_completa` e altera o LED/imprime a mensagem correspondente à transferência concluída.
-   **`iniciar_proxima_transferencia_uart()`:** Configura e inicia a transferência DMA para a UART. Seleciona o buffer de origem com base em `transferencia_atual`, define as configurações de tamanho, incremento e DREQ, e inicia o canal.
-   **`main()`:** Inicializa stdio, UART, GPIOs dos LEDs, reivindica um canal DMA, configura a interrupção do DMA com `dma_isr`, inicia a primeira transferência e entra no loop principal que espera pela flag `transferencia_completa` para iniciar a próxima transferência, usando `tight_loop_contents()` enquanto espera.

## ▶️ Modo de Uso

1.  Carregue o firmware no Raspberry Pi Pico.
2.  Abra um monitor serial (como minicom, PuTTY, Thonny) conectado ao Pico na taxa de 115200 baud.
3.  Observe a saída no monitor serial. Você verá as mensagens indicando o início de cada transferência ("Iniciando envio UART via DMA X...") seguidas pelas mensagens "LED X aceso (após envio UART via DMA X.)" e, em seguida, os próprios dados enviados por DMA (os caracteres dos buffers 'A'...'P', '1'...'f', 'H'...'qd'). Este ciclo se repetirá continuamente a cada 1 segundo (devido ao `sleep_ms` no loop principal).

## 📌 Notas Adicionais

-   **Reivindicação de Canal DMA:** É uma boa prática usar `dma_claim_unused_channel(true)` para garantir que você obtenha um canal DMA disponível.
-   **Configuração da Interrupção:** O código configura o handler para `DMA_IRQ_0`, que é a interrupção padrão para um conjunto de canais DMA.
-   **Não Bloqueante:** A transferência DMA e o tratamento da interrupção são não bloqueantes. O loop principal fica livre para fazer outras tarefas (neste caso, apenas espera eficientemente) enquanto o DMA move os dados.
-   **Lógica do LED:** A lógica `gpio_put(PIN, 1)` para "acender" no código sugere LEDs conectados em configuração "ativo alto" (GPIO alto = LED liga).
