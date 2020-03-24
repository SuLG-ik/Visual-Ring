//Изменяемые настройки

#define DEBUG false //TRUE - включить

#define IS_LED_STRIP true   //true - используетс лента, false - лампа



#define MATRIX_WIDTH 3      //Высота
#define MATRIX_HEIGHT 14    //Ширина

#define RADIUS 10           //Рабиус

\

//ищзменяемые настройки

//Не изменять 
#define MATRIX_PIN 13    //Пин ленты
#define ENVOLVED_MIMIMUM 35
#define ZERO 0    // Нулевая константа

#define ms() (long)millis()   //внутреннее время

#define BRIGHTNESS_ZERO ZERO    //нулевая яркость

#define MAX_VALUE 255           //максимальное значение

#define MAP_RADIUS 255/RADIUS   //нужно
#define BUTTON_OUTSIDE A0       //пин звонка
#define BUTTON_INSIDE A2        //пин управленияя

#define SOFTNESS MAX_VALUE/RADIUS                    //Мягкость сглаживания линий

#define MATRIX_NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)    //количество светодиодов

#define MATRIX_COLORS 3      //Кол-во цветов


//Режимы

  #define MODE_LIGHT_OFF 0
  #define MODE_LIGHT_ON 1
  #define MODE_RING 2
  #define MODE_CHANGE_COLOR 3
  #define MODE_CHANGE_COLOR_ANIM 4
  #define MODE_SHACAL_SETTINGS 5
  #define MODE_SHACAL_SETTINGS_ANIM 6

//Режимы
//Не изменять


#include <GyverButton.h>
#include <GyverTimer.h>

#if IS_LED_STRIP
#include <FastLED.h>
#endif
//Переменные


byte maxClickTimes = 3;
byte clickTimes = 0;
long previousShacal = 0;
long shacalFreeTime = 30000;
long shacalTimeout = 10000;
bool forceShacalBlock = false;

uint8_t mode = 0;
bool ledLight = 0;

long tmpInt1 = 0;
long tmpInt2 = 0;
long tmpInt3 = 0;
long tmpInt4 = 0;
long tmpInt5 = 0;
long tmpTimesCounter = 0;

GButton ringButton(BUTTON_OUTSIDE);
GButton controlButton(BUTTON_INSIDE);

#if IS_LED_STRIP

#define RED 0
#define GREEN 1
#define BLUE 2

boolean horizontal_line = true;   //true - горизонтальное расположение ленты, false -вертикальное

long oneTime = 500;
long ringDuration = 5000;

struct CRGB matrix [MATRIX_NUM_LEDS];   //лента
byte brightness = 15;

byte colorState = 0; // 0- red 1- green, 2 - blue

#define MATRIX_LENGTH (horizontal_line) ? MATRIX_HEIGHT : MATRIX_WIDTH

#endif
//Классы

void setup() {

  #if IS_LED_STRIP

    FastLED.addLeds<WS2811, MATRIX_PIN, RGB>(matrix,MATRIX_NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS_ZERO);

  #else
    pinMode(MATRIX_PIN, OUTPUT);
  #endif
  
  #if DEBUG == true
  Serial.begin(9600);

  while(!Serial){};

  toLog("IS_LED_STRIP:");
  toLog(IS_LED_STRIP);
  nextLog();

  
  toLog("FastLed size:");
  #if IS_LED_STRIP
  toLog(FastLED.size());
  #else
  toLog("Unavaible");
  #endif
    
  nextLog();
  nextLog();

  #endif
}

void loop() {

  buttonsTick();
  unshacaler();

  #if IS_LED_STRIP
    stripChecker();
  #else
    rgbChecker();
  #endif
}

void toLog(String message){
  #if DEBUG
  Serial.println(message);
  #endif
}

void toLog(int message){
  #if DEBUG
  Serial.println(message);
  #endif
}

void nextLog(){
  #if DEBUG
  Serial.println("+++++++++++++++++++");
  #endif
}

void buttonsTick(){

  if(!isBlocked()){
    ringButton.tick();
  }
  controlButton.tick();


  if(controlButton.isSingle()){
    if (mode == MODE_RING || mode == MODE_LIGHT_OFF){
      setLightMode(MODE_LIGHT_ON);
    }else{
      setLightMode(MODE_LIGHT_OFF);
    }
  }

  if(controlButton.isDouble()){
    setLightMode(MODE_CHANGE_COLOR);
  }

  if(controlButton.isTriple()){
    setLightMode(MODE_SHACAL_SETTINGS);
  }

  if(ringButton.isPress()){
    setLightMode(MODE_RING);
    shacalCounter();
  }

}

void setLightMode(uint8_t _mode){
  #if DEBUG
    toLog("set mode from:");
    toLog(mode);
    toLog("to:");
    toLog(_mode);
    nextLog();
  #endif
 
  cleanTmp();
  mode = _mode;
}

void cleanTmp(){
  tmpInt1 = 0;
  tmpInt2 = 0;
  tmpInt3 = 0;
  tmpInt4 = 0;
  tmpInt5 = 0;
  tmpTimesCounter = 0;
  toLog("Cleaned");
  nextLog();
}

bool isBlocked(){
  if ((ms() - previousShacal < shacalFreeTime && clickTimes > maxClickTimes) || forceShacalBlock){
    return true;
  }else{
    return false;
  }
}

void shacalCounter(){
  previousShacal = ms();
  clickTimes++;
}

void unshacaler(){
  if(ms() - previousShacal > shacalTimeout){
    clickTimes = 0;
  }
}


void forceShacalChange(){
  clickTimes = 0;
  toLog("changed from");
  toLog(forceShacalBlock);
  forceShacalBlock = !forceShacalBlock;
  toLog("to");
  toLog(forceShacalBlock);
  nextLog();
}

//Если выбрана лента
#if IS_LED_STRIP

uint16_t getAnimTimeout(){
  if (horizontal_line){
    return oneTime/MATRIX_HEIGHT;
  }else{
    return oneTime/MATRIX_WIDTH;
  }
}

void stripChecker(){
  switch(mode){

    case MODE_LIGHT_OFF:
      setAllWhite();
      setLightOff();
    break;

    case MODE_LIGHT_ON:
      setAllWhite();
      setLightOn();
    break;

    case MODE_RING:
      setLightOn();
      ringAnimation(MATRIX_LENGTH);
      autoOffByCounter(6);
    break;

    case MODE_CHANGE_COLOR:
      raiseLightColor();
      setLightMode(MODE_CHANGE_COLOR_ANIM);
    break;

    case MODE_CHANGE_COLOR_ANIM:
      setLightOn();
      ringAnimation(MATRIX_LENGTH);
      autoOffByCounter(2);
    break;

    case MODE_SHACAL_SETTINGS:
      forceShacalChange();
      setLightMode(MODE_SHACAL_SETTINGS_ANIM);
    break;
    case MODE_SHACAL_SETTINGS_ANIM:
      setLightOn();
      fadeAnimation();
      autoOffByCounter(2);
    break;
    default:
      setLightMode(MODE_LIGHT_OFF);
    break;
  };
  
  FastLED.show();
}












void autoOffAnimation(long duration){
  if (ms() - tmpInt2 > duration){
    setLightMode(MODE_LIGHT_ON);
    toLog(ms());
    toLog(tmpInt2);
    toLog(ringDuration);
  }
}

void ringAnimation(long lines){
  if(tmpInt1 == 0){
    tmpInt1 = 1;
    tmpInt2 = ms();
    tmpInt4 = 1;
    toLog(tmpInt2);
  }
  if(ms() - tmpInt3 > getAnimTimeout()){
    raiseAnimLine(lines);
    tmpInt3 = ms();
  
  }

  setEvolvedLine(tmpInt1);
}

void raiseAnimLine(long lines){
  if(tmpInt1 >= lines) {
    tmpInt4 = -1;
    countUp();
  }
  if(tmpInt1 <= 1){
    tmpInt4 = 1;
    countUp();
  }
  if(tmpInt1 < ZERO){
    setLightMode(MODE_LIGHT_ON);
    toLog("Detected bug (tmpInt1 in raiseAnimLine <= 0):");
    toLog(tmpInt1);
    toLog("!Autofixed! (go to main state)");
  }
  tmpInt1+=tmpInt4;
}

CRGB getFadedColor(){
  CRGB color = CRGB(MAX_VALUE,MAX_VALUE,tmpInt1);
  if (!forceShacalBlock){
    color[1] = tmpInt1;
  }else{
    color[0] = tmpInt1;
  }
  return color;
}

void countUp(){
  countUp(1);
}

void countUp(long x){
  toLog("counted");
  toLog(tmpTimesCounter);
  nextLog();
  tmpTimesCounter+=x;
}


void autoOffByCounter(long counts){
  if (tmpTimesCounter > counts){
    setLightMode(MODE_LIGHT_ON);
    toLog("autoOffByCounter");
  }
}

void nextFade(){
  if (tmpInt1 <= ZERO+5) {tmpInt4 = 10; countUp(); toLog(tmpInt1); nextLog();}
  if (tmpInt1 >= MAX_VALUE-5) {tmpInt4 = -10; countUp(); toLog(tmpInt1); nextLog();}
  tmpInt1+=tmpInt4;
}

void fadeAnimation(){
  if(tmpTimesCounter == ZERO){
    tmpInt1 = MAX_VALUE;
    tmpInt4 = -1;
    tmpInt2 = ms();
  }
  if(ms() - tmpInt3 > getAnimTimeout()){
    tmpInt3 = ms();
    nextFade();
  }
  
  CRGB color = getFadedColor();
  setAllOneColor(color);
}

void raiseLightColor(){
  colorState++;
  if(colorState > BLUE){
    colorState = 0;
  }
  toLog(colorState);
}

void setLightOn(){
  FastLED.setBrightness(brightness);
}

void setLightOff(){
  FastLED.setBrightness(BRIGHTNESS_ZERO);
}

void setAllWhite(){
  setAllOneColor(CRGB::Blue);
  setAllOneColor(CRGB::White);
}

//получение номера пикселя по кординатам
uint32_t getByXY(uint32_t x, uint32_t y){
  return horizontal_line ? _getByXY(x, y) : _getByXY(y,x);
}

//вспомогательная функция для получения номера пикселя
uint32_t _getByXY(uint32_t x, uint32_t y){
      if (x > MATRIX_HEIGHT){
          x = MATRIX_HEIGHT;
      }
      if(y > MATRIX_WIDTH){
        y = MATRIX_WIDTH;
      }
      return (y%2 == 0) ? MATRIX_HEIGHT * y - x : ( y -  1 ) * MATRIX_HEIGHT + x - 1;
}

//Установить цвет отдельного пикселя классом CRGB
void setColorToPixel(uint32_t x, uint32_t y, CRGB color){
  matrix[getByXY(x,y)] = color;
}

//Установить цвет отдельного пикселя поцветно
void setColorToPixel(uint32_t x, uint32_t y, byte red, byte green, byte blue){
  setColorToPixel(x,y, CRGB(red,green,blue));
}

//установить цвет на линию классом CRGB
void setColorToLine(uint32_t line, CRGB color){
  uint32_t columns = horizontal_line ? MATRIX_WIDTH : MATRIX_HEIGHT;
  for(uint32_t column = 1; column <= columns; column++){
    setColorToPixel(line, column, color);
  }
}

//установить цвет на линию поцветно
void setColorToLine(uint32_t line, byte red, byte green, byte blue){

  setColorToLine(line, CRGB(red,green,blue));

}

//Установить всё одиним цветом по классу CRGB
void setAllOneColor(CRGB color){
  for (uint32_t pixel; pixel<MATRIX_NUM_LEDS; pixel++){
    matrix[pixel] = color;
  }
}

//установить всё одним цветом поцветно
void setAllOneColor(byte red, byte green, byte blue){
  setAllOneColor(CRGB(red,green,blue));
}

byte filteredColor(int32_t line, int32_t evolved_line){
  int32_t ans = (MAP_RADIUS)*abs(line-evolved_line)+ENVOLVED_MIMIMUM;
  return (ans > MAX_VALUE) ? MAX_VALUE : ans;
}

void setEvolvedLine(uint32_t evolved_line){
  uint32_t lines = horizontal_line ? MATRIX_HEIGHT : MATRIX_WIDTH;

  for (uint32_t line = 1; line  <= lines; line++){


    CRGB color = CRGB(0,0,0);

    if (colorState == GREEN){
      color = CRGB(MAX_VALUE, filteredColor(line, evolved_line),filteredColor(line, evolved_line));
      
    }else if(colorState == BLUE){
      color = CRGB(filteredColor(line, evolved_line), filteredColor(line, evolved_line), MAX_VALUE);
    }else{
      color = CRGB(filteredColor(line, evolved_line), MAX_VALUE, filteredColor(line, evolved_line));
    }
    setColorToLine(line, color);

  }

}

#else

void autoOff(int times){
  if (tmpTimesCounter > times){
    setLightMode(MODE_LIGHT_ON);
  }
}

void blink(int timeout){
  if(ms() - tmpInt1 > timeout){
    ledLight =!ledLight;
    toLog("Blink");
    nextLog();
    tmpTimesCounter++;
    tmpInt1 = ms();
  }
}

void rgbChecker(){

  switch (mode)
  {

  case MODE_LIGHT_ON:
      ledLight = 1;
    break;
  case MODE_LIGHT_OFF:
      ledLight = 0;
  break;

  case MODE_RING:
      blink(500);
      autoOff(6);
  break;
  case  MODE_SHACAL_SETTINGS:
    forceShacalChange();
    setLightMode(MODE_SHACAL_SETTINGS_ANIM);
  break;
  case MODE_SHACAL_SETTINGS_ANIM:
      blink(250);
      if (forceShacalBlock){
        autoOff(4);
      }else{
        autoOff(2);
      }

  break;

  default:
    setLightMode(MODE_LIGHT_OFF);
  break;

  }

  if(ledLight){
    setRGB(1);
  }else{
    setRGB(0);
  }

}

void setRGB(bool ledVal){
  digitalWrite(MATRIX_PIN, ledVal);
}
#endif
