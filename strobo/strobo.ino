#include <ClickEncoder.h>
#include <TimerOne.h>
#include <LiquidCrystal.h>
#include <TimerThree.h>

#define LCD_RS      12
#define LCD_EN      11
#define LCD_D4       10
#define LCD_D5       9
#define LCD_D6       8
#define LCD_D7       7

#define LCD_CHARS   16
#define LCD_LINES    2

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

#define ENC_A      2
#define ENC_B      3
#define ENC_BUTTON 4

ClickEncoder *encoder;
int16_t last, value;
int32_t freq;

#define LED_PIN 6
static boolean output = HIGH;
static boolean flashlight = HIGH;
static boolean fastmode = LOW;

void timerIsr() {
  encoder->service();
}

void setup() {
  freq = 1;
  
  encoder = new ClickEncoder(ENC_A, ENC_B, ENC_BUTTON, 4);
  
  lcd.begin(LCD_CHARS, LCD_LINES);
  lcd.clear();

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  last = -1;

  pinMode(LED_PIN, OUTPUT);

  Timer3.initialize();
  Timer3.attachInterrupt(flash);
}

void loop() {  
  value += encoder->getValue();
  
  if (value != last) {
    Timer3.stop();
    int delta = value-last;
    last = value;
    
    if (fastmode) {
      freq += delta * 10;
    } else {
      freq += delta;
    }

    freq = max(100,freq);
        
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T: ");
    lcd.print(freq*2);
    lcd.print("uS");
    lcd.setCursor(0, 1);
    lcd.print("F: ");
    lcd.print(1000000.0/float(freq*2));
    lcd.print("Hz");
    
    Timer3.setPeriod(freq);
    Timer3.start();
    lcd.setCursor(15, 0);
    lcd.print(fastmode ? "F" : "S");
    
    lcd.setCursor(15, 1);
    lcd.print((encoder->getAccelerationEnabled()) ? "A" : "L");    
  }

  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    #define VERBOSECASE(label) case label: break;
    switch (b) {
      VERBOSECASE(ClickEncoder::Held);
      VERBOSECASE(ClickEncoder::Released);
      VERBOSECASE(ClickEncoder::Pressed);
      case ClickEncoder::Clicked:
        fastmode = !fastmode;
        lcd.setCursor(15, 0);
        lcd.print(fastmode ? "F" : "S");
        break;
      case ClickEncoder::DoubleClicked:
        encoder->setAccelerationEnabled(!encoder->getAccelerationEnabled());
        lcd.setCursor(15, 1);
        lcd.print((encoder->getAccelerationEnabled()) ? "A" : "L");
        break;
    }
  }
}

void flash() {
  digitalWrite(LED_PIN, output);
  output = !output;
}

