#define WS2818B_PIO_IMPLEMENTATION

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#include "ws2818b.pio.h"

#define LED_COUNT 25
#define LED_PIN 7
#define Botao_A 5
#define Botao_B 6
#define BUZZER_PIN 10
#define GPIO_X 26
#define GPIO_Y 27
#define MAX_LINHAS 4
#define MAX_COLUNAS 4

volatile int mapeamento[5][5] = {{0, 0, 0, 0, 0},
                                 {0, 0, 0, 0, 0},
                                 {0, 0, 0, 0, 0},
                                 {0, 0, 0, 0, 0},
                                 {0, 0, 0, 0, 0}};

volatile int mapa_num[5][5] = {{24, 23, 22, 21, 20},
                               {15, 16, 17, 18, 19},
                               {14, 13, 12, 11, 10},
                               {5, 6, 7, 8, 9},
                               {4, 3, 2, 1, 0}};

volatile int cursorX = 1;
volatile int cursorY = 1;
volatile bool peca_selecionada = false;

struct pixel_t
{
  uint8_t G, R, B;
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;

npLED_t leds[LED_COUNT];

PIO np_pio;
uint sm;

void npInit(uint pin)
{
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0)
  {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true);
  }

  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

  for (uint i = 0; i < LED_COUNT; ++i)
  {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
  leds[index].R = r / 100;
  leds[index].G = g / 100;
  leds[index].B = b / 100;
}

void npClear()
{
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}

void npWrite()
{
  for (uint i = 0; i < LED_COUNT; ++i)
  {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100);
}

void CarregarMapa()
{
  int i;

  npClear();

  for (i = 0; i < 5; i++)
  {
    npSetLED(mapa_num[1][i], 102, 76, 0);
    mapeamento[1][i] = 1;
    npSetLED(mapa_num[2][i], 102, 76, 0);
    mapeamento[2][i] = 1;
    npSetLED(mapa_num[3][i], 102, 76, 0);
    mapeamento[3][i] = 1;
  }

  for (i = 1; i < 4; i++)
  {
    npSetLED(mapa_num[0][i], 102, 76, 0);
    mapeamento[0][i] = 1;
    npSetLED(mapa_num[4][i], 102, 76, 0);
    mapeamento[4][i] = 1;
  }

  npSetLED(mapa_num[2][2], 0, 0, 0);
  mapeamento[2][2] = 0;

  npWrite();
}

void AtualizarMapa()
{
  int i, j;
  npClear();

  for (i = 0; i < 5; i++)
  {
    for (j = 0; j < 5; j++)
    {
      if (mapeamento[i][j] == 0)
        npSetLED(mapa_num[i][j], 0, 0, 0);
      else if (mapeamento[i][j] == 1)
        npSetLED(mapa_num[i][j], 102, 76, 0);
      else if (mapeamento[i][j] == 2)
        npSetLED(mapa_num[i][j], 25, 100, 10);
      else if (mapeamento[i][j] == 3)
        npSetLED(mapa_num[i][j], 8, 0, 79);
    }
  }

  npSetLED(mapa_num[cursorX][cursorY], 150, 150, 150);
  npWrite();
}

// FUNÇÃO PWM DO BUZZER
void buzzer_beep_freq(uint freq, uint duration_ms)
{
  gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
  uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);

  uint32_t clock_freq = clock_get_hz(clk_sys);
  uint32_t top = clock_freq / freq;

  pwm_set_wrap(slice_num, top);
  pwm_set_chan_level(slice_num, pwm_gpio_to_channel(BUZZER_PIN), top / 2);
  pwm_set_enabled(slice_num, true);

  sleep_ms(duration_ms);

  pwm_set_enabled(slice_num, false);
  gpio_set_function(BUZZER_PIN, GPIO_FUNC_SIO);
  gpio_set_dir(BUZZER_PIN, GPIO_OUT);
  gpio_put(BUZZER_PIN, 0);
}

void Joystick_X()
{
  adc_select_input(0);
  int valX = adc_read();

  if (valX > 3000 && cursorX > 0)
    cursorX--;
  else if (valX < 1000 && cursorX < MAX_LINHAS)
    cursorX++;

  while (valX < 1000 || valX > 3000)
  {
    valX = adc_read();
    sleep_ms(50);
  }

  AtualizarMapa();
  sleep_ms(100);
}

void Joystick_Y()
{
  adc_select_input(1);
  int valY = adc_read();

  if (valY > 3000 && cursorY < MAX_COLUNAS)
    cursorY++;
  else if (valY < 1000 && cursorY > 0)
    cursorY--;

  while (valY < 1000 || valY > 3000)
  {
    valY = adc_read();
    sleep_ms(50);
  }

  AtualizarMapa();
  sleep_ms(100);
}

bool Movimento_direita(bool exec)
{
  if (mapeamento[cursorX][cursorY] == 0 &&
      mapeamento[cursorX][cursorY - 1] == 1 &&
      mapeamento[cursorX][cursorY - 2] == 2 && !exec)
  {
    mapeamento[cursorX][cursorY] = 1;
    mapeamento[cursorX][cursorY - 1] = 0;
    mapeamento[cursorX][cursorY - 2] = 0;
    peca_selecionada = false;
    return true;
  }
  return false;
}

bool Movimento_esquerda(bool exec)
{
  if (mapeamento[cursorX][cursorY] == 0 &&
      mapeamento[cursorX][cursorY + 1] == 1 &&
      mapeamento[cursorX][cursorY + 2] == 2 && !exec)
  {
    mapeamento[cursorX][cursorY] = 1;
    mapeamento[cursorX][cursorY + 1] = 0;
    mapeamento[cursorX][cursorY + 2] = 0;
    peca_selecionada = false;
    return true;
  }
  return false;
}

bool Movimento_cima(bool exec)
{
  if (mapeamento[cursorX][cursorY] == 0 &&
      mapeamento[cursorX + 1][cursorY] == 1 &&
      mapeamento[cursorX + 2][cursorY] == 2 && !exec)
  {
    mapeamento[cursorX][cursorY] = 1;
    mapeamento[cursorX + 1][cursorY] = 0;
    mapeamento[cursorX + 2][cursorY] = 0;
    peca_selecionada = false;
    return true;
  }
  return false;
}

bool Movimento_baixo(bool exec)
{
  if (mapeamento[cursorX][cursorY] == 0 &&
      mapeamento[cursorX - 1][cursorY] == 1 &&
      mapeamento[cursorX - 2][cursorY] == 2 && !exec)
  {
    mapeamento[cursorX][cursorY] = 1;
    mapeamento[cursorX - 1][cursorY] = 0;
    mapeamento[cursorX - 2][cursorY] = 0;
    peca_selecionada = false;
    return true;
  }
  return false;
}

void Botao_A_press()
{
  bool exec = false;

  if (!gpio_get(Botao_A))
  {
    buzzer_beep_freq(440, 100); // A4

    while (!gpio_get(Botao_A))
      sleep_ms(50);

    if (peca_selecionada && mapeamento[cursorX][cursorY] == 2 && !exec)
    {
      mapeamento[cursorX][cursorY] = 1;
      peca_selecionada = false;
      exec = true;
    }

    if (!peca_selecionada && mapeamento[cursorX][cursorY] == 1 && !exec)
    {
      mapeamento[cursorX][cursorY] = 2;
      peca_selecionada = true;
      exec = true;
    }

    if (peca_selecionada)
    {
      exec = Movimento_direita(exec);
      exec = Movimento_esquerda(exec);
      exec = Movimento_cima(exec);
      exec = Movimento_baixo(exec);
    }
  }
}

void Botao_B_press()
{
  if (!gpio_get(Botao_B))
  {
    buzzer_beep_freq(660, 100); // E5
    mapeamento[0][4] = 2;
  }
  else
  {
    mapeamento[0][4] = 3;
  }
}

int main()
{
  gpio_init(Botao_A);
  gpio_init(Botao_B);
  gpio_init(BUZZER_PIN);
  gpio_set_dir(Botao_A, GPIO_IN);
  gpio_set_dir(Botao_B, GPIO_IN);
  gpio_pull_up(Botao_A);
  gpio_pull_up(Botao_B);
  gpio_set_dir(BUZZER_PIN, GPIO_OUT);
  gpio_put(BUZZER_PIN, 0);

  stdio_init_all();
  adc_init();
  adc_gpio_init(GPIO_X);
  adc_gpio_init(GPIO_Y);

  npInit(LED_PIN);
  CarregarMapa();
  sleep_ms(100);

  while (true)
  {
    Joystick_X();
    Joystick_Y();
    Botao_A_press();
    Botao_B_press(); // Agora habilitado
    AtualizarMapa();
    sleep_ms(100);
  }
}
