#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "generated/Embarcatech_Tarefa_27_01.pio.h"
#include <stdio.h>

// Configurações dos pinos + tempo de debounce de 50ms
#define LED_PIN 7
#define BTN_A_PIN 5
#define BTN_B_PIN 6
#define NUM_LEDS 25
#define DEBOUNCE_MS 50  

// Pinos do LED RGB
#define LED_R_PIN 13 // Pino vermelho
#define LED_G_PIN 11 // Pino verde
#define LED_B_PIN 12 // Pino azul

// Mapeamento físico da matriz 5x5 , seguindo a forma sequencial
const uint8_t led_map[NUM_LEDS] = {
    20, 21, 22, 23, 24,    // (Superior)
    15, 16, 17, 18, 19,   
    10, 11, 12, 13, 14,   
     5,  6,  7,  8,  9,   
     0,  1,  2,  3,  4     // (Inferior)
};

// Padrões dos números (0-9)
const bool number_patterns[10][25] = {
    // Número 0
    {
        1,1,1,1,1,   
        1,0,0,0,1,  
        1,0,0,0,1,  
        1,0,0,0,1,  
        1,1,1,1,1   
    },
    // Número 1 
    {
        0,0,1,0,0,  
        0,1,1,0,0,  
        0,0,1,0,0,  
        0,0,1,0,0,  
        0,1,1,1,0   
    },
    // Número 2
    {
        0,1,1,0,0,  
        0,1,0,1,0,  
        0,1,0,0,0,  
        0,0,1,0,0,  
        1,1,1,1,0   
    },
    // Número 3 
    {
        1,1,1,1,1,  
        0,0,0,0,1,  
        1,1,1,1,1,  
        0,0,0,0,1,  
        1,1,1,1,1   
    },
    // Número 4 
    {
        1,0,0,0,1,  
        1,0,0,0,1,  
        1,1,1,1,1,  
        0,0,0,0,1,  
        1,0,0,0,0   
    },
    // Número 5
    {
        0,1,1,1,1,  
        1,0,0,0,0,  
        0,0,1,1,1,  
        0,0,0,1,0,  
        0,1,1,1,1   
    },
    // Número 6 
    {
        1,1,1,1,1,  
        1,0,0,0,0,  
        1,1,1,1,1,  
        1,0,0,0,1,  
        1,1,1,1,1   
    },
    // Número 7 
    {
        1,1,1,1,1,  
        0,0,0,1,0,  
        0,0,1,0,0,   
        0,1,0,0,0,   
        0,0,0,0,1    
    },
    // Número 8 
    {
        1,1,1,1,1,   
        1,0,0,0,1,   
        1,1,1,1,1,   
        1,0,0,0,1,   
        1,1,1,1,1    
    },
    // Número 9 
    {
        1,1,1,1,1,   
        1,0,0,0,1,   
        1,1,1,1,1,   
        0,0,0,0,1,   
        1,1,1,1,1    
    }
};

// Estado global dos botões A e B + tempo
typedef enum {BTN_RELEASED, BTN_PRESSED, BTN_DEBOUNCING} ButtonState;
volatile ButtonState btn_a_state = BTN_RELEASED;
volatile ButtonState btn_b_state = BTN_RELEASED;
volatile uint8_t current_number = 0;
volatile absolute_time_t last_a_time, last_b_time;

// Variáveis para o blink do LED RGB
volatile bool blink_state = false;
volatile bool blink_active = true;

void ws2812_init(PIO pio, uint sm, uint pin);
void put_pixel(uint32_t color);
void update_display();
void debounce_handler(uint gpio, uint32_t events);
void check_buttons();
void blink_led();


uint32_t rgb_to_rbg(uint32_t rgb_color) {
    uint8_t red = (rgb_color >> 16) & 0xFF;   // Extrai o componente vermelho (R)
    uint8_t green = (rgb_color >> 8) & 0xFF;  // Extrai o componente verde (G)
    uint8_t blue = rgb_color & 0xFF;          // Extrai o componente azul (B)

    // Reorganiza os componentes para RBG
    return (red << 16) | (blue << 8) | green;
}

// Inicialização do PIO para a matriz de LED 
void ws2812_init(PIO pio, uint sm, uint pin) {
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, pin, 800000, false);
}

void put_pixel(uint32_t color) {
    // Converte o valor RGB para RBG antes de enviar ao LED
    uint32_t rbg_color = rgb_to_rbg(color);
    pio_sm_put_blocking(pio0, 0, rbg_color << 8u);
}

// Atualização da matriz de LED
void update_display() {
    for(int i = 0; i < NUM_LEDS; i++) {
        uint8_t pos = led_map[i];
        put_pixel(number_patterns[current_number][pos] ? 0x00FF00 : 0x000000);
    }
}

// Handler de interrupção para debounce
void debounce_handler(uint gpio, uint32_t events) {
    if(gpio == BTN_A_PIN) {
        if(btn_a_state == BTN_RELEASED && !gpio_get(BTN_A_PIN)) {
            btn_a_state = BTN_DEBOUNCING;
            last_a_time = get_absolute_time();
        }
    }
    else if(gpio == BTN_B_PIN) {
        if(btn_b_state == BTN_RELEASED && !gpio_get(BTN_B_PIN)) {
            btn_b_state = BTN_DEBOUNCING;
            last_b_time = get_absolute_time();
        }
    }
}

// Verificação periódica do estado dos botões
void check_buttons() {
    absolute_time_t now = get_absolute_time();
    
    // Botão A
    if(btn_a_state == BTN_DEBOUNCING) {
        if(absolute_time_diff_us(last_a_time, now) > DEBOUNCE_MS * 1000) {
            if(!gpio_get(BTN_A_PIN)) {  
                current_number = (current_number + 1) % 10;
                update_display();
                printf("A: %d\n", current_number);
                btn_a_state = BTN_PRESSED;
            } else {
                btn_a_state = BTN_RELEASED;
            }
        }
    }
    
    // Botão B
    if(btn_b_state == BTN_DEBOUNCING) {
        if(absolute_time_diff_us(last_b_time, now) > DEBOUNCE_MS * 1000) {
            if(!gpio_get(BTN_B_PIN)) {  
                current_number = (current_number == 0) ? 9 : current_number - 1;
                update_display();
                printf("B: %d\n", current_number);
                btn_b_state = BTN_PRESSED;
            } else {
                btn_b_state = BTN_RELEASED;
            }
        }
    }
    
    // Reset dos estados quando soltos
    if(btn_a_state == BTN_PRESSED && gpio_get(BTN_A_PIN)) {
        btn_a_state = BTN_RELEASED;
    }
    if(btn_b_state == BTN_PRESSED && gpio_get(BTN_B_PIN)) {
        btn_b_state = BTN_RELEASED;
    }
}

// Função para o blink do LED RGB
void blink_led() {
    static absolute_time_t last_blink_time = {0};
    absolute_time_t now = get_absolute_time();

    if(blink_active && absolute_time_diff_us(last_blink_time, now) > 100000) { // 100ms
        blink_state = !blink_state; // Alterna o estado
        last_blink_time = now;

        if(blink_state) {
            gpio_put(LED_R_PIN, 1); // Acende o LED RGB em vermelho
            gpio_put(LED_G_PIN, 0); // Desliga o verde
            gpio_put(LED_B_PIN, 0); // Desliga o azul
        } else {
            gpio_put(LED_R_PIN, 0); // Apaga o LED RGB
            gpio_put(LED_G_PIN, 0); // Garante que o verde esteja apagado
            gpio_put(LED_B_PIN, 0); // Garante que o azul esteja apagado
        }
    }
}

// Função setup para inicializações e configurações
void setup() {
    stdio_init_all();
    
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);

    // Configuração dos pinos do LED RGB
    gpio_init(LED_R_PIN);
    gpio_init(LED_G_PIN);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // Configuração de interrupções
    gpio_set_irq_enabled(BTN_A_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(BTN_B_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_callback(debounce_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);

    // Inicialização dos LEDs
    ws2812_init(pio0, 0, LED_PIN);
    update_display();
}

// Função main para chamada da função setup, contém um loop que verifica o estado dos botões e uma função check_buttons
int main() {
    setup();

    while(1) {
        check_buttons();
        blink_led();
        sleep_ms(10);
    }
}