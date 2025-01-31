#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "generated/Embarcatech_Tarefa_27_01.pio.h"
#include <stdio.h>

// Configurações
#define LED_PIN 7
#define BTN_A_PIN 5
#define BTN_B_PIN 6
#define NUM_LEDS 25
#define DEBOUNCE_MS 50  // Tempo de debounce aumentado

// Mapeamento físico da matriz 5x5 (serpentina)
const uint8_t led_map[NUM_LEDS] = {
     4,  3,  2,  1,  0,
     9,  8,  7,  6,  5,
    14, 13, 12, 11, 10,
    19, 18, 17, 16, 15,
    24, 23, 22, 21, 20
};

// Padrões dos números (0-9)
const bool number_patterns[10][25] = {
    {1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1}, // 0
    {0,0,1,0,0, 0,0,1,1,0, 0,0,1,0,0, 0,0,1,0,0, 0,1,1,1,0}, // 1
    {0,0,1,1,0, 0,1,0,0,1, 0,0,0,1,0, 0,0,1,0,0, 0,1,1,1,1}, // 2
    {1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}, // 3
    {1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,0, 0,0,0,0,1}, // 4
    {1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}, // 5
    {1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}, // 6
    {1,1,1,1,1, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,1,0, 1,0,0,0,0}, // 7
    {1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}, // 8
    {1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}  // 9
};

// Estado global
typedef enum {BTN_RELEASED, BTN_PRESSED, BTN_DEBOUNCING} ButtonState;
volatile ButtonState btn_a_state = BTN_RELEASED;
volatile ButtonState btn_b_state = BTN_RELEASED;
volatile uint8_t current_number = 0;
volatile absolute_time_t last_a_time, last_b_time;

// Protótipos
void ws2812_init(PIO pio, uint sm, uint pin);
void put_pixel(uint32_t color);
void update_display();
void debounce_handler(uint gpio, uint32_t events);
void check_buttons();

// Inicialização do PIO para WS2812
void ws2812_init(PIO pio, uint sm, uint pin) {
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, pin, 800000, false);
}

void put_pixel(uint32_t color) {
    pio_sm_put_blocking(pio0, 0, color << 8u);  // Formato GRB
}

// Atualização da matriz de LEDs
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
            if(!gpio_get(BTN_A_PIN)) {  // Ainda pressionado
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
            if(!gpio_get(BTN_B_PIN)) {  // Ainda pressionado
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

void setup() {
    stdio_init_all();
    
    // Configuração dos botões
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);

    // Configuração de interrupções
    gpio_set_irq_enabled(BTN_A_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(BTN_B_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_callback(debounce_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);

    // Inicialização dos LEDs
    ws2812_init(pio0, 0, LED_PIN);
    update_display();
}

int main() {
    setup();

    while(1) {
        check_buttons();
        sleep_ms(10);
    }
}