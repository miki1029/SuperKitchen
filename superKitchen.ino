// motor
#define PinA 7
#define PinB 6
#define PinC 5
#define PinD 4

// switch, led
const int SWITCHES[8] = { 23, 25, 27, 29, 31, 33, 35, 37};
const int LEDS[8] = { 22, 24, 26, 28, 30, 32, 34, 36};

// buzzer
#define BUZZER 40

// smoke sensor
#define SMOKE_SENSOR A0

/******************** setup ********************/
void setup() {
  int i;
  Serial.begin(115200);

  // switch, led
  for(i=0; i<8; i++) {
    pinMode(SWITCHES[i], INPUT);
    pinMode(LEDS[i], OUTPUT);
  }

  // buzzer
  pinMode(BUZZER, OUTPUT);

  // motor
  pinMode(PinA, OUTPUT);
  pinMode(PinB, OUTPUT);
  pinMode(PinC, OUTPUT);
  pinMode(PinD, OUTPUT);
}

/******************** loop ********************/
void loop() {
  int i;
  for(i=0; i<8; i++) {
    // LED 제어 점등 및 소등 제어
    digitalWrite(LEDS[i], digitalRead(SWITCHES[i]));

    // switch 확인
    if(digitalRead(SWITCHES[i]) == 1) {
      // cook process
      cook(i);
    }
  }
  delay(200);
}

/******************** cook process ********************/
enum { ALARM = -1, STOP, LEV1, LEV2, LEV3 };

int menus[][20] = {
  { LEV3, 4, LEV3, 5, ALARM, 3, LEV2, 2, STOP, 5 }, // 짜파게티
  { LEV1, 1, LEV3, 3, ALARM, 3, LEV1, 3, STOP, 5 }, // 볶음밥
  { LEV2, 1, LEV1, 1, LEV2, 1, LEV3, 1, STOP, 5 } // simple test
};

void cook(int i) {
  int* menu = menus[i];
  int menuStep, menuTime;
  int lastStep = STOP;
  int stepi = 0;
  int smk;
  int smkCnt = 0;
  bool runState = true;

  // cook start
  Serial.print("Cook Start ");
  Serial.println(i);
  // 강불로 점화
  fire(LEV3, lastStep);
  delay(4000);
  lastStep = LEV3;

  while(runState) {
    // 레시피 데이터를 차례로 가져옴
    menuStep = menu[stepi];
    menuTime = menu[stepi+1];

    // 불의 세기 조정단계
    if(menuStep > 0) {
      Serial.print("  Fire: "); Serial.println(menuStep);
      // 불의 세기 조정
      fire(menuStep, lastStep);
      // 변경 후 1회 Buzzer 울림
      buzzer(1);
      lastStep = menuStep;
      Serial.print("  delay(m): "); Serial.println(menuTime);

      // menuTime(분)만큼 대기하며 센서 값 측정
      Serial.print("  smoke: ");
      int countLoop = menuTime * 60;
      for(int j=0; j<countLoop; j++) {
        // 1초씩 delay
        delay(1000);
        // 측정값 500 이상 5회 초과할 시: 가스 누출 또는 화재 발생
        if(smokeDanger(smkCnt)) {
          Serial.println("\nGas!!");
          // 불을 소화하고
          fire(STOP, menuStep);
          // 5초간 Buzzer 연속 울림
          digitalWrite(BUZZER, HIGH);
          delay(5000);
          digitalWrite(BUZZER, LOW);
          // 레시피 종료
          runState = false;
          break;
        }

      } // 레시피 시간 경과
      Serial.println("\n");
    }
    // 요리의 중간 단계에 도달함을 알림
    else if(menuStep == ALARM) {
      Serial.print("  buzzer: "); Serial.print(menuTime);
      // 정해진 횟수만큼 buzzer 울림
      buzzer(menuTime);
      Serial.println("...\n");
    }
    // 요리의 완성을 알림. 불을 소화하고 5회 울림
    else if(menuStep == STOP) {
      Serial.print("  Fire: "); Serial.println(menuStep);
      fire(menuStep, lastStep);
      buzzer(menuTime);
      Serial.println("Cook End\n");
      break;
    }
    stepi += 2;
  }
}

/******************** buzzer process ********************/
void buzzer(int num) {
  for(int i=0; i<num; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);
    delay(100);
  }
}

/******************** smoke process ********************/
bool smokeDanger(int& smkCnt) {
  // 센서 값 측정
  int smk = analogRead(SMOKE_SENSOR);
  Serial.print(smk);
  Serial.print(" ");
  if(smk > 500) {
    smkCnt++;
    // 측정값 500 이상 5회 초과할 시: 가스 누출 또는 화재 발생
    if(smkCnt > 5) {
      // 요리 중단
      return true;
    }
  }
  // 요리 진행
  return false;
}

/******************** motor process ********************/
void fire(int level, int lastLevel) {
  if(level > lastLevel) {
    counterclockwise(level, lastLevel);
  }
  else if(level < lastLevel) {
    clockwise(level, lastLevel);
  }
}
void clockwise(int level, int lastLevel) {
  int repeat = (lastLevel - level) * 60;
  for(int i=0;i<repeat;i++) {
    for(int i=0;i<4;i++) {
      stepMove(3-i);
      delay(5);
    }
  }
}
void counterclockwise(int level, int lastLevel) {
  int repeat = (level - lastLevel) * 60;
  for(int i=0;i<repeat;i++){
    for(int i=0;i<4;i++) {
      stepMove(i);
      delay(5);
    }
  }
}
void stepMove(int i) {
  switch(i) {
  case 0:
    digitalWrite(PinA, LOW);
    digitalWrite(PinB, LOW);
    digitalWrite(PinC, LOW);
    digitalWrite(PinD, HIGH);
    break;
  case 1:
    digitalWrite(PinA, LOW);
    digitalWrite(PinB, LOW);
    digitalWrite(PinC, HIGH);
    digitalWrite(PinD, LOW);
    break;
  case 2:
    digitalWrite(PinA, LOW);
    digitalWrite(PinB, HIGH);
    digitalWrite(PinC, LOW);
    digitalWrite(PinD, LOW);
    break;
  case 3:
    digitalWrite(PinA, HIGH);
    digitalWrite(PinB, LOW);
    digitalWrite(PinC, LOW);
    digitalWrite(PinD, LOW);
    break;
  }
}
