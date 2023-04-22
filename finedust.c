#include <LiquidCrystal_I2C.h> //LCD 1602 I2C용 라이브러리
#include <Wire.h>
#include "DHT.h"                // 온습도 센서 사용을 위한 라이브러리
       // DHT11의 타입, 핀을 dht로 지정

LiquidCrystal_I2C lcd(0x27, 16, 2); // 접근주소: 0x3F or 0x27 1602 Display

byte humi[8] = {
  // 물컵모양 출력
  0b00000,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
};
byte temp[8] = {
  // 온도계 모양 출력
  0b00100,
  0b01010,
  0b01010,
  0b01010,
  0b01010,
  0b10001,
  0b11111,
  0b01110,
};
byte char_temp[8] = {
  // 온도 단위 출력
  0b10000,
  0b00110,
  0b01001,
  0b01000,
  0b01000,
  0b01000,
  0b01001,
  0b00110,
};

void lcd_setup() {
lcd.init();
lcd.backlight();
lcd.createChar(1, temp);
lcd.createChar(2, humi);
lcd.createChar(3, char_temp);

lcd.setCursor(0, 0);   // 1번째, 1라인
  lcd.write(byte(1));    // 온도계 출력
  lcd.setCursor(5, 0);   // 6번째 1라인
  lcd.write(byte(3));    // 온도 단위 출력

  lcd.setCursor(8, 0);   // 9번째, 1라인
  lcd.write(byte(2));    // 물컵 출력
  lcd.setCursor(13, 0);  // 15번째, 1라인
  lcd.print("%");        // % 출력

  lcd.setCursor(0,1); //1번째,2라인
  lcd.print("F.Dust");
  lcd.setCursor(11,1);
  lcd.print("ug/m^3");

}

void lcd_print(int hymi, int temp, float dust) {
// 출력 버퍼
  char humi_buffer[4] = "";
  char temp_buffer[4] = "";
  char dust_str[6] = "";
  char dust_buffer[6]= "";

  //온습도 출력 --------------------------________________________------------------------________________________

  sprintf(humi_buffer, "%-2d", humi);  //%-2d는 정수로 왼쪽으로(-방향으로) 2칸 정렬   
  sprintf(temp_buffer, "%-2d", temp);


  lcd.setCursor(2, 0);
  lcd.print(temp_buffer);
  lcd.setCursor(10, 0);
  lcd.print(humi_buffer);

  // 미세먼지 값 출력
  // 실수를 정수로 변환( 실수값, 정수부 길이, 소수부 길이, 문자열 버퍼 )
  dtostrf(dust, 3, 1, dust_str);
  sprintf(dust_buffer, "%5s", dust_str);

  lcd.setCursor(6, 1);
  lcd.print(dust_buffer);
}
#define DHT_PIN A1
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

void dht_setup() {
  dht.begin();
}

#define DUST_PIN A0
#define DUST_LED_PIN 12

#define DUST_SAMPLE_TIME 280
#define DUST_WAIT_TIME 40
#define DUST_STOP_TIME 9600

float dust_slot[5] = {0};
float dust_initial = 0;

void dust_setup() {
  pinMode(DUST_LED_PIN, OUTPUT);
//미세먼지 촉값 측정을 시작

float dust_sum = 0;
for (int i = 0; i < 5; i++) {//미세먼지 측정센서 초기 값 구하기
    digitalWrite(DUST_LED_PIN,LOW);//미세먼지 측정 5번하기
    delayMicroseconds(DUST_SAMPLE_TIME);
    dust_sum += analogRead(DUST_PIN);
    delayMicroseconds(DUST_WAIT_TIME);
    digitalWrite(DUST_LED_PIN,HIGH);
    delayMicroseconds(DUST_STOP_TIME);
  }
  dust_initial = ((dust_sum / 5) * 0.5) / 1024.0;
}
  float dust_read() {

    digitalWrite(DUST_LED_PIN,LOW);
    delayMicroseconds(DUST_SAMPLE_TIME);
    float dust_value = analogRead(DUST_PIN);
    delayMicroseconds(DUST_WAIT_TIME);
    digitalWrite(DUST_LED_PIN, HIGH);
    delayMicroseconds(DUST_STOP_TIME);

    dust_slot[4] = ((dust_value * (5.0 / 1024)) - dust_initial) / 0.005;

    float total_dust = 0;
    total_dust = dust_slot[4];
    for (int i = 0; i<4; i++) {
      total_dust += dust_slot[i];
  
    dust_slot[i] = dust_slot[i + 1];
    }

    if (dust_slot[0] != 0) {
      return total_dust / 5;
    }
    else{
      return 0;
    }
  }

#define LED_R_PIN 5
#define LED_G_PIN 6
#define LED_B_PIN 7

void led_setup() {
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
}

void led_set_color(byte red, byte green, byte blue) {
  analogWrite(LED_R_PIN, red);
  analogWrite(LED_G_PIN, green);
  analogWrite(LED_B_PIN, blue);
}

void led_dust_level(float dust) {
  if (dust <= 30.0)
    led_set_color(0, 0, 255);
  else if (dust <= 80.0)
    led_set_color(0, 255, 0);
  else if (dust <= 150.0)
    led_set_color(255, 80, 1);
  else
    led_set_color(255, 0, 0);
}

bool isRun = false;
void serial_wait_start() {
  while(!isRun) {
    if (Serial.available()) {
      int i = 0;
      char buffer[32] = "";

      do{
        buffer[i] = Serial.read();
      } while (buffer[i++] != '\n');
      buffer[i - 1] = 0;

      if (strcmp(buffer, "start") == 0)
        isRun = true;
    }
    delay(1000);
  }
}
 // CSV (C0mma Seperate Variables)형식으로 시리얼 포트에 전송
void serial_send_data(int humi, int temp, float dust) {

  char buffer[64] =  "";
  char dust_str[9] = "";

  dtostrf(dust, 4, 3, dust_str);
  sprintf(buffer, "%d,%d,%s\n", humi, temp, dust_str);
  Serial.print(buffer);

}
  void setup() {
  Serial.begin(9600);

  lcd_setup();
  dht_setup();
  dust_setup();
  led_setup();
}

void loop() {
  serial_wait_start();

  int humi = dht.readHumidity();
  int temp = dht.readTemperature();
  float dust = dust_read();

  lcd_print(humi, temp, dust);
  led_dust_level(dust);
  serial_send_data(humi, temp, dust);

  delay(200);
}
