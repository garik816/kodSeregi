#include <microDS3231.h>
#include <GyverTM1637.h>

#define buttonPrevPin 12
#define buttonNextPin 11
#define buttonSetPin 10
#define buttonEnterPin 2

#define zatopleniePin 2
#define budilnikPin 3

#define zone1 13
#define zone2 14
#define zone3 15
#define zone4 16
#define zone5 17

#define klapanOpen 8
#define klapanClose 9

#define CLK A5
#define DIO A4

MicroDS3231 rtc;
GyverTM1637 disp(CLK, DIO);

static unsigned long blinkTimer = millis();
static unsigned long buttonTimer = millis();

bool buttonFlag = false;
bool alarmFlag = false;

bool buttonPrevFlag = false;
bool buttonNextFlag = false;
bool buttonSetFlag = false;
bool buttonEnterFlag = false;

bool menuFlag = false;
bool editingFlag = false;
bool hoursEdit = false;
bool minutesEdit = false;
int menuPosition = 0;

byte hours = 0;
byte minutes = 0;

void rtcInit(void){
    // проверка наличия модуля на линии i2c
  if (!rtc.begin()) {
    Serial.println("DS3231 not found");
  }
}

void blink(int ms){
  if (millis() - blinkTimer > ms) {
    blinkTimer = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}

void clockIninit (void){
    // получаем все данные в структуру
  DateTime now = rtc.getTime();

  // меняем любой параметр
  now.year += 5;
  // now.second;
  // now.minute;
  // now.hour;
  // now.day;
  // now.date;
  // now.month;

  // отправляем в rtc
  rtc.setTime(now);
}

void clockSetupMenu (void){
  if(menuFlag){
    disp.point(menuPosition);
    if(buttonSetFlag){menuPosition++;}
  }
}

void printTime(void) {
  // получаем все данные в структуру и используем их
  // этот способ быстрее и "легче" вызова отдельных get-функций
  DateTime now = rtc.getTime();

  Serial.print(now.hour);
  Serial.print(":");
  hours = now.hour;
  Serial.print(now.minute);
  Serial.print(":");
  minutes = now.minute;
  Serial.print(now.second);
  Serial.print(" ");
  Serial.print(now.day);
  Serial.print(" ");
  Serial.print(now.date);
  Serial.print("/");
  Serial.print(now.month);
  Serial.print("/");
  Serial.println(now.year);
}

void displayInit(void){
  disp.clear();
  disp.brightness(7);  // яркость, 0 - 7 (минимум - максимум)
}

void indicationClock (void){
  disp.displayClock(hours, minutes); // выводим время функцией часов
  disp.point(0);   // выкл точки  
}

void indicationErrors(byte error){
  disp.displayByte(_E, _r, _r, error);
}

void klapanMode (int mode){
  digitalWrite(mode, HIGH);
  delay(50);
  digitalWrite(mode, LOW);
}

void alarm (){
  //int0
  bool btnEnterState = !digitalRead(buttonEnterPin);
  if (btnEnterState && !buttonFlag && millis() - buttonTimer > 50) {
    buttonFlag = true;
    buttonTimer = millis();
    
    if (digitalRead(zone1)==0 || digitalRead(zone2)==0 || digitalRead(zone3)==0 || digitalRead(zone4)==0 || digitalRead(zone5)==0) {
      if (!alarmFlag){
        alarmFlag = true;
        //перекрываем клапан
        klapanMode(klapanClose);
//        delay(1000*60); //ждём
        //индикация
        //спать 
      }
      // индикация номера зоны
      if (digitalRead(zone1)==0) {indicationErrors(1);}
      if (digitalRead(zone2)==0) {indicationErrors(2);}
      if (digitalRead(zone3)==0) {indicationErrors(3);}
      if (digitalRead(zone4)==0) {indicationErrors(4);}
      if (digitalRead(zone5)==0) {indicationErrors(5);}    
    }
    
//    меню
    indicationClock();
  }
}

void budilnik (){
  //int1
  if (!alarmFlag) {
    klapanMode(klapanClose); //закрываем клапан
    delay(1000*60); //ждём
    klapanMode(klapanOpen); //открыли клапан
    //спать
  }
}

void detectButtons(void){

  bool btnPrevState = !digitalRead(buttonPrevPin);
  bool btnNextState = !digitalRead(buttonNextPin);
  bool btnSetState = !digitalRead(buttonSetPin);
  bool btnEnterState = !digitalRead(buttonEnterPin);

  if (btnPrevState && !buttonFlag && millis() - buttonTimer > 50) {
    buttonFlag = true;
    buttonTimer = millis();
    buttonPrevFlag = true;
    if(editingFlag && hoursEdit){
      hours--;
    }
    if(editingFlag && minutesEdit){
      minutes--;
    }
    
  }
  
  if (btnNextState && !buttonFlag && millis() - buttonTimer > 50) {
    buttonFlag = true;
    buttonTimer = millis();
    buttonNextFlag = true;
    if(editingFlag && hoursEdit){
      hours++;
    }
    if(editingFlag && minutesEdit){
      minutes++;
    }
  }
  
  if (btnSetState && !buttonFlag && millis() - buttonTimer > 50) {
    buttonFlag = true;
    buttonTimer = millis();
    buttonSetFlag = true;
    if(editingFlag && hoursEdit){
      hoursEdit = !hoursEdit;
      minutesEdit = !minutesEdit;
    }
    if(editingFlag && minutesEdit){
      minutesEdit = !minutesEdit;
      hoursEdit = !hoursEdit;
    }
  }

  if (btnSetState && buttonFlag && millis() - buttonTimer > 1000) {
    editingFlag = !editingFlag;
    if(!editingFlag){
      hoursEdit = false;
      minutesEdit = false;
    }
    buttonTimer = millis();
  }

  if (btnEnterState && !buttonFlag && millis() - buttonTimer > 50) {
    buttonFlag = true;
    buttonTimer = millis();
    buttonEnterFlag = true;
  }

    
  if ((!btnPrevState || !btnNextState || !btnEnterState  || !btnSetState) && buttonFlag && millis() - buttonTimer > 200) {
    buttonFlag = false;
    buttonTimer = millis();
    Serial.println("released");
  }
}



void setup(){
  Serial.begin(115200);
  delay(1000);
  Serial.println("start");  
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(klapanOpen, OUTPUT);
  pinMode(klapanClose, OUTPUT);
  digitalWrite(klapanOpen, LOW);
  digitalWrite(klapanClose, LOW);
  
  pinMode(buttonPrevPin, INPUT_PULLUP);
  pinMode(buttonNextPin, INPUT_PULLUP);
  pinMode(buttonEnterPin, INPUT_PULLUP);

  pinMode(zatopleniePin, INPUT_PULLUP);
  pinMode(budilnikPin, INPUT_PULLUP);
  
  pinMode(zone1, INPUT_PULLUP);
  pinMode(zone2, INPUT_PULLUP);
  pinMode(zone3, INPUT_PULLUP);
  pinMode(zone4, INPUT_PULLUP);
  pinMode(zone5, INPUT_PULLUP);

  attachInterrupt(0,alarm,FALLING);
  attachInterrupt(1,budilnik,FALLING);

  rtcInit();
}

void loop(){
  blink(1000);
}
