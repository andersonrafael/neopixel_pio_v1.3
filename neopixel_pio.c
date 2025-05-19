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

#define NOTE_B0 31
#define NOTE_C1 33 
#define NOTE_CS1 35 
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS146
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978

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
volatile int movimentos_errados = 0;
volatile bool mostrar_dica = false;

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

void play_victory_melody()
{
  int melody[] = {
      NOTE_E5, NOTE_E5, NOTE_F5, NOTE_G5,
      NOTE_G5, NOTE_F5, NOTE_E5, NOTE_D5,
      NOTE_C5, NOTE_C5, NOTE_D5, NOTE_E5,
      NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5, NOTE_E5, NOTE_F5, NOTE_G5,
      NOTE_G5, NOTE_F5, NOTE_E5, NOTE_D5,
      NOTE_C5, NOTE_C5, NOTE_D5, NOTE_E5,
      NOTE_E5, NOTE_D5, NOTE_D5};

  int noteDurations[] = {
      4, 4, 4, 4,
      4, 4, 4, 4,
      4, 4, 4, 4,
      4, 4, 2,
      4, 4, 2, 4, 4, 4, 4,
      4, 4, 4, 4,
      4, 4, 4, 4,
      4, 4, 2,
      4, 4, 2};

  for (int i = 0; i < sizeof(melody) / sizeof(int); i++)
  {
    if (melody[i] == 0)
    {
      sleep_ms(1000 / noteDurations[i]);
    }
    else
    {
      buzzer_beep_freq(melody[i], 1000 / noteDurations[i]);
      sleep_ms(50); // Pequena pausa entre notas
    }
  }
}

void play_startup_melody()
{
  // Notas da melodia característica
  int bond_melody[] = {
      NOTE_E4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_E4,
      NOTE_E4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_E4, NOTE_E4, NOTE_E4,
      NOTE_E4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_E4,
      NOTE_E4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_G4, NOTE_E4, NOTE_E4, NOTE_E4,
      NOTE_A4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5,
      NOTE_E5, NOTE_E5, 0, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_G4, NOTE_G4,
      NOTE_E4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4, NOTE_F4};

  // Durações das notas (8 = colcheia, 4 = semínima)
  int bond_durations[] = {
      8, 16, 16, 8, 16, 16, 8, 16,
      16, 8, 16, 16, 8, 16, 16, 8,
      8, 16, 16, 8, 16, 16, 8, 16,
      16, 8, 16, 16, 8, 16, 16, 8,
      8, 8, 8, 8, 8, 8, 8, 8,
      16, 8, 8, 16, 8, 8, 8, 16, 16, 16, 8, 16, 16, 8, 16, 16, 8, 16};

  // Toca a melodia principal
  for (int i = 0; i < sizeof(bond_melody) / sizeof(int); i++)
  {
    if (bond_melody[i] == 0)
    {
      sleep_ms(1000 / bond_durations[i]);
    }
    else
    {
      buzzer_beep_freq(bond_melody[i], 1000 / bond_durations[i]);
      sleep_ms(30); // Pausa curta entre notas
    }
  }

  buzzer_beep_freq(NOTE_E5, 150);
  sleep_ms(50);
  buzzer_beep_freq(NOTE_D5, 150);
  sleep_ms(50);
  buzzer_beep_freq(NOTE_C5, 150);
  sleep_ms(50);
  buzzer_beep_freq(NOTE_B4, 150);
  sleep_ms(150);
  buzzer_beep_freq(NOTE_F4, 300);
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

  // Se estiver mostrando dica, pisca a peça que pode ser movida
  if (mostrar_dica)
  {
    static bool pisca = false;
    pisca = !pisca;

    // Procura uma peça que pode ser movida (valor 2)
    for (i = 0; i < 5; i++)
    {
      for (j = 0; j < 5; j++)
      {
        if (mapeamento[i][j] == 2)
        {
          // Verifica se pode mover para alguma direção
          if ((j > 1 && mapeamento[i][j - 1] == 1 && mapeamento[i][j - 2] == 0) || // esquerda
              (j < 3 && mapeamento[i][j + 1] == 1 && mapeamento[i][j + 2] == 0) || // direita
              (i > 1 && mapeamento[i - 1][j] == 1 && mapeamento[i - 2][j] == 0) || // cima
              (i < 3 && mapeamento[i + 1][j] == 1 && mapeamento[i + 2][j] == 0))   // baixo
          {
            if (pisca)
            {
              npSetLED(mapa_num[i][j], 255, 255, 0); // Amarelo brilhante
            }
            else
            {
              npSetLED(mapa_num[i][j], 25, 100, 10); // Verde normal
            }
          }
        }
      }
    }
  }

  npSetLED(mapa_num[cursorX][cursorY], 150, 150, 150);
  npWrite();
}

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

bool Movimento_valido()
{
  // Verifica se o movimento atual é válido
  if (peca_selecionada && mapeamento[cursorX][cursorY] == 0)
  {
    // Verifica movimento para a esquerda
    if (cursorY > 1 && mapeamento[cursorX][cursorY - 1] == 1 && mapeamento[cursorX][cursorY - 2] == 2)
      return true;

    // Verifica movimento para a direita
    if (cursorY < 3 && mapeamento[cursorX][cursorY + 1] == 1 && mapeamento[cursorX][cursorY + 2] == 2)
      return true;

    // Verifica movimento para cima
    if (cursorX > 1 && mapeamento[cursorX - 1][cursorY] == 1 && mapeamento[cursorX - 2][cursorY] == 2)
      return true;

    // Verifica movimento para baixo
    if (cursorX < 3 && mapeamento[cursorX + 1][cursorY] == 1 && mapeamento[cursorX + 2][cursorY] == 2)
      return true;
  }
  return false;
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

    if (Movimento_valido())
    {
      movimentos_errados = 0; 
      mostrar_dica = false;   
    }
    else
    {
      movimentos_errados++; 
    }

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

    if (Movimento_valido())
    {
      movimentos_errados = 0;
      mostrar_dica = false;
    }
    else
    {
      movimentos_errados++;
    }

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

    if (Movimento_valido())
    {
      movimentos_errados = 0;
      mostrar_dica = false;
    }
    else
    {
      movimentos_errados++;
    }

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

    if (Movimento_valido())
    {
      movimentos_errados = 0;
      mostrar_dica = false;
    }
    else
    {
      movimentos_errados++;
    }

    return true;
  }
  return false;
}

void Botao_A_press()
{
  bool exec = false;

  if (!gpio_get(Botao_A))
  {
    buzzer_beep_freq(440, 100); 

    while (!gpio_get(Botao_A))
      sleep_ms(50);

    if (peca_selecionada && mapeamento[cursorX][cursorY] == 2 && !exec)
    {
      mapeamento[cursorX][cursorY] = 1;
      peca_selecionada = false;
      exec = true;
      movimentos_errados++;
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

    if (movimentos_errados >= 2)
    {
      mostrar_dica = true;
    }
  }
}

void Restart(){

  int linha,coluna,i;
  cursorX = 2;
  cursorY = 2;
  peca_selecionada = false;
  movimentos_errados = 0;

  int mapeamento_vazio[5][5] = {{0, 0, 0, 0, 0},
                              {0, 0, 0, 0, 0},
                              {0, 0, 0, 0, 0},
                              {0, 0, 0, 0, 0},
                              {0, 0, 0, 0, 0}};  

  int mapeamento_pisca[5][5] = {{0, 1, 1, 1, 0},
                                 {1, 1, 1, 1, 1},
                                 {1, 1, 0, 1, 1},
                                 {1, 1, 1, 1, 1},
                                 {0, 1, 1, 1, 0}};

  play_victory_melody();

  for ( i = 0; i < 3; i++){
    for ( linha = 0; linha <= MAX_LINHAS; linha++){
      for ( coluna = 0; coluna <= MAX_COLUNAS; coluna++){
        mapeamento[linha][coluna] = mapeamento_vazio[linha][coluna];
      }
    }

    AtualizarMapa();
    sleep_ms(500);

    for ( linha = 0; linha <= MAX_LINHAS; linha++){
      for ( coluna = 0; coluna <= MAX_COLUNAS; coluna++){
        mapeamento[linha][coluna] = mapeamento_pisca[linha][coluna];
      }
    }

    AtualizarMapa();
    sleep_ms(500);
  }
}

void Botao_B_press()
{
  if (!gpio_get(Botao_B))
  {
    Restart();
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

  play_startup_melody();
  CarregarMapa();
  sleep_ms(100);

  while (true)
  {
    Joystick_X();
    Joystick_Y();
    Botao_A_press();
    Botao_B_press();
    AtualizarMapa();
    sleep_ms(50);
  }
}
