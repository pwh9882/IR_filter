//===================================================
// 사용법: 아래의 모든 코드를 복사, 붙여넣기 한 후 filtered_ir_distance()함수를 사용하면 됨.
// _INTERVAL_DIST가 거리 측정 주기. 가능하면 이 값을 사용해서 거리를 측정하길 바람.
// 스파이크 제거 후 EMA 필터를 0.35의 ALPHA 값으로 적용함. EMA 필터가 필요없거나 수정하고 싶으면 filtered_ir_distance()함수의 아래부분을 참조.
// 필터를 적용해도 큰 노이즈가 있으면 센서의 위치를 먼저 제대로 설정했는지 확인 바람.
// noise_filter_test.ino 라는 테스트 코드가 있으니 그것을 사용해서 제대로 작동하는지 확인하는 것을 추천
// # 거리 보정 코드는 들어가있지 않음.
//===================================================
#define _INTERVAL_DIST 30  // DELAY_MICROS * samples_num^2 의 값이 최종 거리측정 인터벌임. 넉넉하게 30ms 잡음.
#define DELAY_MICROS  1500 // 필터에 넣을 샘플값을 측정하는 딜레이(고정값!)
#define EMA_ALPHA 0.35     // EMA 필터 값을 결정하는 ALPHA 값. 작성자가 생각하는 최적값임.
float ema_dist=0;            // EMA 필터에 사용할 변수
float filtered_dist;       // 최종 측정된 거리값을 넣을 변수. loop()안에 filtered_dist = filtered_ir_distance(); 형태로 사용하면 됨.
float samples_num = 3;     // 스파이크 제거를 위한 부분필터에 샘플을 몇개 측정할 것인지. 3개로 충분함! 가능하면 수정하지 말 것.

// 기존에 제공된 ir_distance 함수. 변경사항 없음.
float ir_distance(void){ // return value unit: mm
  float val;
  float volt = float(analogRead(PIN_IR));
  val = ((6762.0/(volt-9.0))-4.0) * 10.0;
  return val;
}
// ================
float under_noise_filter(void){ // 아래로 떨어지는 형태의 스파이크를 제거해주는 필터
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

float filtered_ir_distance(void){ // 아래로 떨어지는 형태의 스파이크를 제거 후, 위로 치솟는 스파이크를 제거하고 EMA필터를 적용함.
  // under_noise_filter를 통과한 값을 upper_nosie_filter에 넣어 최종 값이 나옴.
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
