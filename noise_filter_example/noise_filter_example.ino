#include <Servo.h>
Servo myservo;
// Arduino pin assignment
#define PIN_IR A0
#define PIN_LED 9
#define PIN_SERVO 7

unsigned long last_sampling_time_dist, last_sampling_time_serial; // unit: ms
bool event_dist, event_serial;
float raw_dist;
//===================================================
// ⚠⚠❗❗⚠⚠ 코드를 작동시키기 전에 _DUTY_NEU의 값을 각자의 중립위치각도로 수정 후 사용!!!
#define _DUTY_NEU 1550 // neutral position
//===================================================
#define _INTERVAL_DIST 30   // USS interval (unit: ms)
#define _INTERVAL_SERIAL 30 // serial interval (unit: ms)


#define DELAY_MICROS  1500
#define EMA_ALPHA 0.35
float filtered_dist;
float ema_dist = 0;
float samples_num = 3;

float ir_distance(void){ // return value unit: mm
  float val;
  float volt = float(analogRead(PIN_IR));
  val = ((6762.0/(volt-9.0))-4.0) * 10.0;
  return val;
}

// ================
float under_noise_filter(void){
  int currReading;
  int largestReading = 0;
  for (int i = 0; i < samples_num; i++) {
    currReading = ir_distance();
    if (currReading > largestReading) { largestReading = currReading; }
    // Delay a short time before taking another reading
    delayMicroseconds(DELAY_MICROS);
  }
  return largestReading;
}
float filtered_ir_distance(void){
  int currReading;
  int lowestReading = 1024;
  for (int i = 0; i < samples_num; i++) {
    currReading = under_noise_filter();
    if (currReading < lowestReading) { lowestReading = currReading; }
  }
  // eam 필터 추가
  ema_dist = EMA_ALPHA*lowestReading + (1-EMA_ALPHA)*ema_dist;
  return ema_dist;
}
//===================================================

void setup() {
  Serial.begin(57600);
  myservo.attach(PIN_SERVO); 
  myservo.writeMicroseconds(_DUTY_NEU);
  delay(1000);
  // initialize last sampling time
  last_sampling_time_dist = last_sampling_time_serial = 0;
  event_dist = event_serial = false;

}

void loop() {
  unsigned long time_curr = millis();
  if(time_curr >= last_sampling_time_dist + _INTERVAL_DIST) {
        last_sampling_time_dist += _INTERVAL_DIST;
        event_dist = true;
  }
  if(time_curr >= last_sampling_time_serial + _INTERVAL_SERIAL) {
        last_sampling_time_serial += _INTERVAL_SERIAL;
        event_serial = true;
  }
  if(event_dist) {
    event_dist = false;
    raw_dist = ir_distance();
    //==============================================
    filtered_dist = filtered_ir_distance();
    //===============================================
  }
  if(event_serial) {
    event_serial = false;    
    // output the read value to the serial port
    Serial.print("min:0,max:500,filtered_dist:");
    Serial.print(filtered_dist);
    Serial.print(", raw_dist:");
    Serial.println(raw_dist);
  }
}
