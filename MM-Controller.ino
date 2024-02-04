// подключаем библиотеки
#include <EncButton.h>
#include <microLED.h>
#include <EasyHID.h>
/*
  Скетч к проекту "мультимедиа контроллер"
  Подходит для arduino Nano и MH-ET(Attiny88)
  Для индицации используется адресная лента WS2812B 
*/


/* Пины энкодера */
#define ENC_DT 6
#define ENC_CK 5
#define ENC_SW 3

#define STRIP_PIN 10  // пин ленты
#define NUMLEDS 5     // кол-во светодиодов

#define COLOR_DEBTH 3  //бит на цвет

/* Цвеотвые коды для каждого цвета по HUE-WHEEL */
#define GREEN 500
#define RED 0
#define BLUE 900
#define PURPLE 1200
#define YELLOW 210

/* Насстройки эффектов LED анимации */
#define USE_LED 1  // включить/выключить подсветку (0-выкл / 1вкл)

#define BRIGHT_MAX 200           // яркость LED
#define BRIGHT_MIN 70            // минимальная яркость при переходе на другой цвет
#define FADE_SPEED 5             // скорость затухание цвета в мс
#define BRIGHT_STEP 5            // шаг яркости при затухании
#define ROLLING_SPEED 120        // скорость анимации динамического кольца в мс (выше 130 не ставить)
#define ROLLING_FADE 80          // скорость затухание цвета динамического кольца в мс
#define INDICATE_COLOR_TIME 700  // время в мс на которое горит индикаторный цвет когда не трогают энкодер

#define VOLUME_CYCLE 0  // режим свечение при изменении громкости (0: просто цвет | 1: динамическое кольцо в сторону поворота) (при режиме (1) после первого поворота будет задержка энкодерапока не пройдёт анимация)




//*************** ДЛЯ РАЗРАБОТЧИКОВ ****************
unsigned long timer;  // переменная таймера
unsigned long fadeTimer;
int curentBrght = BRIGHT_MAX;  // переменная яркости для эффектов
bool flag = 0;                 // флаг для правильной работы эффектов от павторных срабатывании
bool LED_Flag = USE_LED;


microLED<NUMLEDS, STRIP_PIN, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_AVER> strip;
EncButton enc(ENC_DT, ENC_CK, ENC_SW);


void setup() {
  HID.begin();
  strip.setBrightness(BRIGHT_MAX);
  if (LED_Flag) {
    strip.fill(mWheel(GREEN));
  } else strip.setBrightness(0);
  strip.show();
}


void loop() {
  encFunction();
}

void encFunction() {
  HID.tick();  // Тикер HID
  enc.tick();  // Тикер энкодера

  /* Отслеживание энкодера и выдача команд */
  if (enc.left()) {
    Keyboard.clickMultimediaKey(KEY_VOL_DOWN);
    if (LED_Flag) {
      switch (VOLUME_CYCLE) {
        case 0:
          fadeIn(2);
          break;
        case 1:
          cycle(0);
          break;
      }
    }
  }
  if (enc.right()) {
    Keyboard.clickMultimediaKey(KEY_VOL_UP);
    if (LED_Flag) {
      switch (VOLUME_CYCLE) {
        case 0:
          fadeIn(2);
          break;
        case 1:
          cycle(1);
          break;
      }
    }
  }

  if (enc.leftH()) {
    Keyboard.clickMultimediaKey(KEY_PREV_TRACK);
    if (LED_Flag) {
      fadeIn(3);
    }
  }
  if (enc.rightH()) {
    Keyboard.clickMultimediaKey(KEY_NEXT_TRACK);
    if (LED_Flag) {
      fadeIn(3);
    }
  }
  if (enc.hasClicks(1)) {
    Keyboard.clickMultimediaKey(KEY_PLAYPAUSE);
    if (LED_Flag) {
      fadeIn(0);
    }
  }
  if (enc.hasClicks(2)) {
    Keyboard.clickMultimediaKey(KEY_MUTE);
    if (LED_Flag) {
      fadeIn(1);
    }
  }
  if (enc.hasClicks(3)) {
    LED_Flag = !LED_Flag;
    if (LED_Flag) {
      strip.fill(mWheel(GREEN));
      strip.setBrightness(BRIGHT_MAX);
      strip.show();
    } else {
      strip.setBrightness(0);
      strip.show();
    }
  }
}


// функция для начала затухание и переключение на индикаторный цвет
void fadeIn(byte color) {
  if (!flag) {
    // уменьшение якости
    while (curentBrght != BRIGHT_MIN) {
      if (millis() - timer >= FADE_SPEED) {
        timer = millis();
        curentBrght -= BRIGHT_STEP;
        strip.setBrightness(curentBrght);
        strip.show();
      }
    }

    // переключение цвета индикации по входной переменной функции
    switch (color) {
      case 0:
        strip.fill(mWheel(BLUE));
        break;
      case 1:
        strip.fill(mWheel(RED));
        break;
      case 2:
        strip.fill(mWheel(PURPLE));
        break;
      case 3:
        strip.fill(mWheel(YELLOW));
        break;
    }

    // увиличение яркости
    while (curentBrght != BRIGHT_MAX) {
      if (millis() - timer >= FADE_SPEED) {
        timer = millis();
        curentBrght += BRIGHT_STEP;
        strip.setBrightness(curentBrght);
        strip.show();
      }
    }
  }
  flag = 1;  // ставим защитный флаг от повторных срабатывании затузания
  deathTime();
}

void cycle(bool mode) {
  if (!flag) {
    // уменьшение якости
    while (curentBrght != 0) {
      if (millis() - timer >= FADE_SPEED) {
        timer = millis();
        curentBrght -= BRIGHT_STEP;
        strip.setBrightness(curentBrght);
        strip.show();
      }
    }

    strip.clear();
    strip.setBrightness(BRIGHT_MAX);
    strip.show();

    byte counter;
    if (!mode) {
      counter = 0;
      while (counter < 5) {
        if (millis() - timer >= ROLLING_SPEED) {
          timer = millis();
          strip.set(counter, mWheel(PURPLE));
          strip.show();
          counter++;
        }
      }
    }

    else {
      counter = 4;
      while (counter != 0) {
        if (millis() - timer >= ROLLING_SPEED) {
          timer = millis();
          strip.set(counter, mWheel(PURPLE));
          strip.show();
          counter--;
        }
      }
    }
  }
  flag = 1;
  deathTime();
}

// функция последнего затухание и переключение на основной цвет
void fadeOut() {
  if (flag) {
    // уменьшаем яркость
    while (curentBrght != BRIGHT_MIN) {
      if (millis() - timer >= FADE_SPEED) {
        timer = millis();
        curentBrght -= BRIGHT_STEP;
        strip.setBrightness(curentBrght);
        strip.show();
      }
    }
    strip.fill(mWheel(GREEN));  // переключаем на основной цвет

    // увиличиваем яркость
    while (curentBrght != BRIGHT_MAX) {
      if (millis() - timer >= FADE_SPEED) {
        timer = millis();
        curentBrght += BRIGHT_STEP;
        strip.setBrightness(curentBrght);
        strip.show();
      }
    }
  }
  flag = 0;  // снимаем защитный флаг
}

// ожидание в индикаторном цвете пока энкодер не будут трогать заданное время
void deathTime() {
  timer = millis();
  // ждём указонное время и до тех пор не переключаем цвет
  while (millis() - timer <= INDICATE_COLOR_TIME) {
    encFunction();
  }
  curentBrght = BRIGHT_MIN;
  fadeOut();
}
