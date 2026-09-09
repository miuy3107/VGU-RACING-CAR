#include <ESP32Servo.h>

const int PIN_SERVO_STEERING = 21; 
const int PIN_SERVO_GEAR_1   = 22; 
const int PIN_SERVO_GEAR_2   = 23; 
const int PIN_ESC            = 4;  
const int PIN_HALL_SENSOR    = 25; // Gắn ở trục truyền động 2 bánh sau

Servo steeringServo;
Servo gearServo1;
Servo gearServo2;
Servo escMotor;

volatile unsigned long pulseCount = 0;
unsigned long lastTime = 0;
float wheelRpm = 0;
float previousRpm = 0;

// Số nam châm gắn trên trục truyền động
const int MAGNETS_PER_REVOLUTION = 1; 

void IRAM_ATTR countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_HALL_SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_HALL_SENSOR), countPulse, FALLING);

  steeringServo.attach(PIN_SERVO_STEERING);
  gearServo1.attach(PIN_SERVO_GEAR_1);
  gearServo2.attach(PIN_SERVO_GEAR_2);
  escMotor.attach(PIN_ESC, 1000, 2000); 

  steeringServo.write(90);
  gearServo1.write(90);
  gearServo2.write(90);
  escMotor.writeMicroseconds(1000); 
  
  delay(2000); 
  Serial.println("System Initialized & Ready.");
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= 1000) {
    noInterrupts();
    unsigned long counts = pulseCount;
    pulseCount = 0;
    interrupts();

    // Tính RPM chính xác theo số nam châm trên trục sau
    wheelRpm = ((float)counts / MAGNETS_PER_REVOLUTION) * 60.0; 
    
    handleTransmissionLogic(wheelRpm, previousRpm);

    previousRpm = wheelRpm;
    lastTime = currentTime;
  }
}

int currentGear = 1; // Khởi động ở số 1

void handleTransmissionLogic(float currentRpm, float lastRpm) {
  float acceleration = currentRpm - lastRpm;

  // Điều kiện tăng số (Upshift) và kiểm tra xem đã chạm số cao nhất chưa
  if (currentRpm > 400 && abs(acceleration) <= 20) {
    if (currentGear < 3) { // Giả sử hộp số tối đa 3 cấp
      currentGear++;       // Tăng số lên 1 bậc
      Serial.print("Đường bằng -> Tăng lên số: "); Serial.println(currentGear);
      
      // Điều khiển góc servo tương ứng với cấp số mới
      if (currentGear == 2) {
        gearServo1.write(110); 
        gearServo2.write(110);
      } else if (currentGear == 3) {
        gearServo1.write(135); 
        gearServo2.write(135);
      }
    } else {
      // Đã ở số cao nhất rồi thì giữ nguyên, không gọi lệnh upshift nữa
      // Tránh làm cháy/kẹt servo
    }
  } 
  else if (acceleration < -50 && currentRpm > 200) {
    if (currentGear > 1) { // Kiểm tra nếu chưa phải số thấp nhất thì mới trả số
      currentGear--;
      Serial.print("Lên dốc -> Trả về số: "); Serial.println(currentGear);
      
      if (currentGear == 2) {
        gearServo1.write(110); 
        gearServo2.write(110);
      } else if (currentGear == 1) {
        gearServo1.write(45); 
        gearServo2.write(45);
      }
    }
  }
}