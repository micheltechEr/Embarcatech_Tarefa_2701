# Tarefa de Interrupções com RP2040 - BitDogLab

## Descrição do Projeto

Este projeto foi desenvolvido para praticar o uso de **interrupções** conforme ensinado na aula do dia **27/01/2025**, utilizando também técnicas de **debounce** para evitar falsos ou múltiplos acionamentos de botões em um único pressionamento.

A tarefa consistiu na criação de um **contador** com funcionalidades de incremento e decremento. Para isso, foram utilizados os seguintes componentes:
- Uma matriz de LEDs **5x5 WS2812** para exibir os números.
- Dois botões (**A** e **B**) conectados aos pinos **5** e **6**, respectivamente, para incrementar e decrementar o contador.
- Um LED RGB no pino **13** para indicar o funcionamento do sistema.

Como o uso de botões pode causar o problema conhecido como **bouncing**, foi implementada uma função de **debouncing via software** para mitigar esse comportamento. Além disso, foi adicionado um efeito de **piscagem (blink)** no LED RGB, que pisca **5 vezes por segundo na cor vermelha**.

---

## Componentes Utilizados

- **Matriz de LEDs 5x5 (WS2812):** Exibe os números do contador.
- **Botão A (Pino 5):** Incrementa o valor do contador.
- **Botão B (Pino 6):** Decrementa o valor do contador.
- **LED RGB (Pino 13):** Pisca na cor vermelha para indicar o funcionamento.

---

## Características do Projeto

- Implementação de **interrupções** para os botões.
- Tratamento de **debouncing** via software.
- Controle da matriz de LEDs para exibir números.
- Controle do LED RGB com efeito de piscagem.
- Código funcional, bem estruturado e comentado para facilitar a compreensão.

---

## Guia de Funcionamento na Sua Máquina

Para executar este projeto localmente, siga as instruções abaixo:

1. **Clone o repositório:**
   - Abra o **VS Code** e clone este repositório para sua máquina.

2. **Importe o projeto:**
   - Certifique-se de ter as extensões do **Raspberry Pi Pico** instaladas no VS Code.
   - Importe o projeto para poder compilá-lo e executá-lo na placa RP2040.

3. **Conecte a placa:**
   - Conecte a placa ao computador via USB e coloque-a no modo **BOOTSEL**.

4. **Compile o código:**
   - Compile o código diretamente no VS Code.

5. **Simulação no Wokwi:**
   - Para simular o projeto, abra o arquivo `diagram.json` disponível nos arquivos do projeto e execute-o no [Wokwi](https://wokwi.com).

6. **Execute na placa:**
   - Após a compilação e com a placa no modo **BOOTSEL**, clique em **Executar** ou **Run** para carregar o programa na placa.

---

## Funcionalidades

O funcionamento do projeto é simples e intuitivo:
- O **LED RGB** pisca **5 vezes por segundo na cor vermelha** para indicar que o sistema está ativo.
- Pressione o **Botão A** para **incrementar** o valor exibido na matriz de LEDs.
- Pressione o **Botão B** para **decrementar** o valor exibido na matriz de LEDs.

---

## Vídeo da Solução

Confira o vídeo demonstrativo da solução no link abaixo:

[🔗 Clique aqui para acessar o vídeo](https://drive.google.com/file/d/1yrtnHHN4gYKSWzX-coU8zV7vBeXCnQ_Q/view?usp=sharing)

---

### Observações Finais

Este projeto foi desenvolvido com foco em boas práticas de programação, organização e documentação. Caso tenha dúvidas ou sugestões, sinta-se à vontade para abrir uma **issue** ou entrar em contato.

