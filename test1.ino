//Code kết hợp phần cứng đầu tiên. Test bằng arduino uno. Dùng 2 interrupt để tối ưu hóa việc mcu đọc mpu6050 (chưa update mpu6050 và servo mode)


#include <Arduino.h>

#define BUZZER_PASSIVE true    // true nếu là buzzer passive 

#define NOTE_G4  392
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784

const int hallDigitalPin = 2;
const int buttonPin = 3;
const int ledPin = 13;
const int piezoPin = 7;   // low level trigger: LOW = kêu, HIGH = im
const int motorPin = 10;

volatile unsigned int countFrequency = 0;
volatile uint16_t countTotal = 0;
volatile unsigned long lastHallInterruptTime = 0;
const unsigned long hallDebounceDelay = 500;

volatile bool playPeppaMelodyFlag = false;
volatile unsigned long lastButtonInterruptTime = 0;
const unsigned long buttonDebounceDelay = 200;

// --- Peppa Pig theme (theo bản nhạc, ♩ = 90 trên nửa nốt) ---
const int QUARTER = 333;   // nốt đen (ms)  -> đổi số này để nhanh/chậm
const int EIGHTH  = 167;   // móc đơn (ms)

const int peppaMelody[] = {
  NOTE_G5, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_G4,                 // ô 1
  NOTE_G4, NOTE_B4, NOTE_D5, NOTE_F5, NOTE_E5, NOTE_C5         // ô 2
};

const int peppaDurations[] = {
  QUARTER, EIGHTH, EIGHTH, QUARTER, QUARTER,
  EIGHTH, EIGHTH, EIGHTH, EIGHTH, QUARTER, QUARTER
};

const int totalNotes = sizeof(peppaMelody) / sizeof(peppaMelody[0]);

void buzzerOff() {
  noTone(piezoPin);
  digitalWrite(piezoPin, HIGH);   // HIGH = im
}

void buzzerOn(int freq, int duration) {
#if BUZZER_PASSIVE
  tone(piezoPin, freq);
#else
  digitalWrite(piezoPin, LOW);
#endif
  delay(duration);
  buzzerOff();
}

void countRotation() {
  unsigned long currentTime = millis();
  if (currentTime - lastHallInterruptTime > hallDebounceDelay) {
    ++countFrequency;
    if (countFrequency >= 2) {
      countFrequency = 0;
      ++countTotal;
    }
    lastHallInterruptTime = currentTime;
  }
}

void buttonISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastButtonInterruptTime > buttonDebounceDelay) {
    playPeppaMelodyFlag = true;
    lastButtonInterruptTime = currentTime;
  }
}

void playPeppaPigTheme() {
  const int gap = 25;             // khoảng tách giữa các nốt (không làm lệch nhịp)
  for (int i = 0; i < totalNotes; i++) {
    buzzerOn(peppaMelody[i], peppaDurations[i] - gap);
    delay(gap);
  }
}

void setup() {
  digitalWrite(piezoPin, HIGH);
  pinMode(piezoPin, OUTPUT);
  buzzerOff();

  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  pinMode(hallDigitalPin, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(9600);
  delay(1000);

  playPeppaMelodyFlag = false;
  attachInterrupt(digitalPinToInterrupt(hallDigitalPin), countRotation, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
}

void loop() {
  if (playPeppaMelodyFlag) {
    delay(20);
    if (digitalRead(buttonPin) == LOW) {
      detachInterrupt(digitalPinToInterrupt(buttonPin));
      playPeppaPigTheme();
      attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
    }
    playPeppaMelodyFlag = false;
  }

  Serial.print("Count Total: ");
  Serial.print(countTotal);
  Serial.print(" | Frequency: ");
  Serial.println(countFrequency);
  delay(300);
}
