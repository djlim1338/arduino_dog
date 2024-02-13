
/*
* main_program_last
* 
*/

//  include ############################################################
#include <Servo.h>
#include <SoftwareSerial.h>

Servo servoMotor;


//  pin 관련 ############################################################
#define ENA 5 //  enable A pin  [PWM] 오른쪽
#define ENB 6 //  enable B pin  [PWM] 왼쪽
#define IN1 7 //  IN1 [A] pin [normal]
#define IN2 8 //  IN2 [A] pin [normal]
#define IN3 9 //  IN3 [B] pin [normal]
#define IN4 10 //  IN4 [B] pin  [normal]
#define SERVO_PIN 11 //  servo motor pin  [PWM]
#define ECHO 4 //  초음파 INPUT Pin  [normal]
#define SONAR_TRG_PIN 12 //  초음파 Trigger pin [normal]
#define SOFT_SERIAL_RX 2  //  Software Serial RXD [normal]
#define SOFT_SERIAL_TX 3  //  Software Serial TXD [normal]


//  Software Serial 관련 ############################################################
SoftwareSerial swSerial(SOFT_SERIAL_TX, SOFT_SERIAL_RX);  //  아두이노 기준으로(RX, TX) 아두이노 RX -> 블루투스 TX, 아두이노 TX -> 블루투스 RX 인듯
#define BLUETOOTH_CODE_SIZE 10


//  motor control mode 관련 ############################################################
//  이 차량의 모터 속도는 ENA, ENB의 PWM를 제어하여 속도 조절 [값은 0 ~ 255]
int motorA_speed = 0;  //  모터 A 속도, 차 기준 오른쪽 바퀴
int motorB_speed = 0;  //  모터 B 속도, 차 기준 왼쪽 바퀴
byte motorState = 1; //  1=>이동, 0=>정지


//  sonar control mode 관련 ############################################################
#define LOGIC 34000/1000000/2 //  초음파 센서 거리 구하는 공식
#define SONAR_GO_SPEED 200  //  속도
#define SONAR_GO_DISTANCE 50  //  앞으로 가기전 최소 거리
#define SONAR_BACK_DISTANCE 30  //  뒤로 가기전 최대 거리
#define STOP_DELAY 200  //  뱡향 전환할 때 잠시 멈추는 시간
#define SCAN_DELAY 10 //  초음파 센서 스켄 딜레이 시간
#define SONAR_ANGLE_FRONT 81  //  센서 방향 전방
#define SONAR_RIGHT_SCAN SONAR_ANGLE_FRONT-20  //  센서 방향 우회전 탐색 각도  [Maze 관련]
#define SONAR_LEFT_SCAN SONAR_ANGLE_FRONT+20  //  센서 방향 좌회전 탐색 각도  [Maze 관련]
#define SONAR_ANGLE_HALF_RIGHT SONAR_ANGLE_FRONT-45  //  센서 방향 오른쪽반 탐색 각도  [Dog 관련]
#define SONAR_ANGLE_HALF_LEFT SONAR_ANGLE_FRONT+45  //  센서 방향 왼쪽반 탐색 각도  [Dog 관련]
#define SONAR_ANGLE_RIGHT 0  //  센서 방향 우측
#define SONAR_ANGLE_LEFT 180  //  센서 방향 좌측
int sonarSwingValue = 0;
int sonarSwingPlusValue = 10;
#define SWING_DELAY 100


//  code 관련 ############################################################
#define ERRORCODE -9999
#define WALL01 "----------------------------------------"
#define WALL02 "########################################"
#define ARRAY_MAX_SIZE 100


//  line tracking mode 관련 ############################################################
bool lineTracking_position = true;  //  현재 위치 (true = 오른쪽), (false = 왼쪽)
#define IRPIN_A A1  //  하단 IR핀 우측
#define IRPIN_B A0  //  하단 IR핀 좌측
#define LINE_READ_DELAY 100 //  바닥 IR읽기 지연
#define LINE_TURN_DELAY 500 //  회전 지연
#define LINE_COLOR_WHITE 300 //  하얀색
#define LINE_COLOR_BLACK 600 //  검정색
#define LINE_SPEED_SLOW 150 //  회전시 느린 바퀴 속도
#define LINE_SPEED_FAST 255 //  회전시 빠른 바퀴 속도


//  Maze Mode 관련  ############################################################
#define MAZE_DELAY_COUNT 10  //  우측이 개방되어있는지 확인하는 주기
#define MAZE_WALL01_DISTANCE 15 //  벽으로 인식할 거리  (cm)
#define MAZE_COUNT 1 //  벽 또는 개방을 지났다는 것을 확인할 count
#define MAZE_OPEN_COUNT 1 //  개방을 지났다는 것을 확인할 count
#define MAZE_OPEN_DISTANCE 20 //  개방으로 인식할 거리 (cm)
#define MAZE_MOTOR_GO_SPEED 150  //  미로 찾기 중 차 전진 속도  (PWM)
#define MAZE_MOTOR_TURN_SPEED 200  //  미로 찾기 중 차 회전 속도  (PWM)


//  Dog Mode 관련  ############################################################
#define DOG_MAX 50
#define DOG_MIN 20
#define DOG_ERROR_DOWN 4  //  +- 오차범위
#define NOT_FOUND 'N'
#define FRONT_FOUND 'F'
#define LEFT_FOUND 'L'
#define LEFT_HALF_FOUND 'l'
#define RIGHT_FOUND 'R'
#define RIGHT_HALF_FOUND 'r'
#define DOG_TURN_SPEED 255;
#define DOG_SCAN_DELAY 100
#define DOG_TURN_DELAY 500
#define DOG_GO_SPEED 200
int dogSpeed = 0;
bool dogState = true; //  참=>탐색중, 거짓=>따라가기


//  변수 관련  ############################################################
char allStringData[ARRAY_MAX_SIZE] = "";  //  입력받은 데이터 분류 전 모든 데이터
int allStringDataCount = 0; //  counter
bool stringDataType = 0;  //  0 => Hardware Serial, 1 => Soft
char control_switch = '0';
byte turn_mode = 0;
char bluetoothCode[BLUETOOTH_CODE_SIZE] = { 'B', 'l', 'u', 'e', 'T', 'o', 'o', 't', 'h', ':' };

void setup()
{
  Serial.begin(9600);
  swSerial.begin(9600);

  servoMotor.attach(SERVO_PIN);
  
  pinMode(ENA, OUTPUT); //  ENA 출력
  pinMode(ENB, OUTPUT); //  ENB 출력
  pinMode(IN1, OUTPUT); //  IN1 [A] 출력
  pinMode(IN2, OUTPUT); //  IN2 [A] 출력
  pinMode(IN3, OUTPUT); //  IN3 [B] 출력
  pinMode(IN4, OUTPUT); //  IN4 [B] 출력
  pinMode(SONAR_TRG_PIN, OUTPUT); //  소나 트리거 출력

  pinMode(IRPIN_A, INPUT);  //  IR REMOT 입력
  pinMode(ECHO, INPUT); //  Sonar 입력
  
  serialManualText(); //  설명서 출력
}

//#############################################################################################################################################################



//#############################################################################################################################################################

//############################## void Loop ############################################################

void loop()
{
  if (Serial.available() || swSerial.available()){  //  Serial data read and arranged
    if(Serial.available()) stringDataType = 0;  //  Hard
    else stringDataType = 1;  //  Soft
    delay(1);
    while (1){
      delay(1);
      //Serial.println(F(" 읽는중... "));
      if (stringDataType)
        allStringData[allStringDataCount] = swSerial.read();
      else
        allStringData[allStringDataCount] = Serial.read();

      if(allStringData[0] == ' ') break;
      if(allStringData[allStringDataCount] == '\n' || allStringData[allStringDataCount] == NULL) break;
      allStringDataCount++;
      if (allStringDataCount > 200) {
        Serial.println("문장이 끝나지 않아 강제로 빠져나옵니다.");
        break;
      }
    }
    serial_scan();
    /*
    if (stringDataType == 0) {
      for (int count = 0; count < allStringDataCount; count++) {
        swSerial.write(allStringData[count]);
        Serial.write(allStringData[count]);
      }
      Serial.println(F(""));
    }
    */
    allStringDataCount = 0;
    while(allStringDataCount <= ARRAY_MAX_SIZE)
      allStringData[allStringDataCount++] = ' '; //  배열 정리 (초기화)
      
    allStringDataCount = 0;
    //Serial.println(F(" 완료 "));
  }
  
  /*
  if (swSerial.available()) {
    Serial.println(F(WALL02));
    
    while(swSerial.available()) {
      Serial.write(swSerial.read());
      delay(1);
    }
    
    Serial.println(F("\n"));
    Serial.println(F(WALL02));
  }
  */
  
  switch (control_switch){  //  모드 지속
    case 'N' :
    SonarMode();
    delay(5);
    break;

    case 'L' :
    IrLineMode_2IR();
    break;

    case 'Z' :
    mazeMode_main("");
    break;

    case 'D' :
    dogMode_main();
    break;

    case 'O' :
    reset_all();
    break;

    case 't' :
    servoSwing();
    break;

    //default :;
  }
}


//############################################################################################################################################################

//############################## Set #########################################################################################################################

void reset_all()
{
    motorStop_SP();  //  속도 초기화
    motorReset(); //  motor IN(1~4) 초기화
    ServoControl(90); //  서보모터 중앙정렬
    motorState = 0; //  속도에 0을 곱하여 정지 (스피드 변화 없음)
    lineTracking_position = true; //  line tracking 위치 초기화
    control_switch = "";  //  모드 초기화  잠시 미로찾기 모드로
    serialManualText(); //  설명서 출력
}

//#############################################################################################################################################################


//############################## Serial data control ##########################################################################################

void serial_scan()
{
  Serial.println(F(WALL01)); //  벽
  int stringCount = 0;  //  입력받은 데이터 문자와 정수로 나눌 때 카운트
  
  char intStringCharData[ARRAY_MAX_SIZE] = "";  //  입력받은 데이터 중 int형 저장 (여기에 저장된건 아직 정수형으로 형 변환 전인 문자형)
  int stringIntCount = 0; //  정수형 저장에 따른 배열 인덱스 증가 카운터
  
  char charStringData[ARRAY_MAX_SIZE] = "";  //  입력받은 데이터 중 char형 저장
  int stringCharCount = 0; //  문자형 저장에 따른 배열 인덱스 증가 카운터
  
  int intStringData = 0;  //  정수형으로 변환이 완료된 후의 최종 결과

  while(allStringData[stringCount] != '\n' && allStringData[stringCount] != NULL){  //  Serial data 의 문장이 끝날 때 까지
    if(allStringData[stringCount] >= '0' && allStringData[stringCount] <= '9')  //  문자형 0~9는 int배열에 저장(형 변환 전으로서 저장된 값은 아직 문자형)
      intStringCharData[stringIntCount++] = allStringData[stringCount];
    else
      charStringData[stringCharCount++] = allStringData[stringCount];  //  위 조건이 아닌경우 모두 char배열에 저장
    stringCount++;
  }

  intStringData = CharIntArrayChangeToInt(intStringCharData);
  //if (intStringData == ERRORCODE) {
  //  Serial.print(F(" 모드 : "));
  //  Serial.println(control_switch);
  //}

  /*  //  데이터 확인 코드
  Serial.println(F("Data Input Type : "));
  Serial.print(stringDataType);
  if (stringDataType)
    Serial.println(F(" Software Serial Type "));
  else
    Serial.println(F(" Hardware Serial Type "));
    
  Serial.println(F("charStringData : "));
  Serial.println(charStringData);
  //Serial.println(F("intStringCharData : "));
  //Serial.println(intStringCharData);
  Serial.println(F("intStringData : "));
  Serial.println(intStringData);
  */
  

  change_Mode(charStringData[0]);

  switch(control_switch)  //  데이터가 들어오면 한번 실행, SONAR처럼 계속 실행해야 하는 코드들은 LOOP문 내부 Switch문에 추가로 작성
  {
    case 'M' :
    MotorMode(charStringData[0], intStringData, charStringData[1]);
    break;
    
    case 'S' :
    ServoControl(intStringData);
    control_switch = "";
    break;

    case 'N' :
    SonarMode();
    break;

    case 'L' :
    IrLineMode_2IR();
    break;

    case 'Z' :
    mazeMode_main(charStringData[1]);
    break;

    case 'D' :
    dogMode_main();
    break;

    case 't' :
    servoSwing();
    break;

    case 'O' :
    break;

    default :
    Serial.print(F(" 없는 값을 입력했습니다. \n"));
  }

  for (int count = 0; count < BLUETOOTH_CODE_SIZE; count++) {
    if (charStringData[count] != bluetoothCode[count]) break;
    else if (count < BLUETOOTH_CODE_SIZE - 1) continue;
    else softwareSerialControl_main(intStringData);
  }
  Serial.println(F(WALL01)); //  벽
}

void change_Mode(char change_data)  //  모드 변환
{
  switch(change_data)
  {
    case 'M' :
    control_switch = 'M';
    Serial.println(F(" 모터 제어 명령어"));
    Serial.println(F(" 0~255 => 속도 조절"));
    Serial.println(F(" A, B 0~255 => 각 모터 속도 조절"));
    Serial.println(F(" g => 앞으로"));
    Serial.println(F(" b => 뒤로"));
    Serial.println(F(" m => EN이동 [속도 그대로]"));
    Serial.println(F(" s => EN멈춤 [속도 그대로]"));
    break;
    
    case 'S' :
    control_switch = 'S';
    break;
    
    case 'N' :
    control_switch = 'N';
    motorA_speed = motorB_speed = SONAR_GO_SPEED;
    Serial.println(F(" 초음파 센서 모드 "));
    break;

    case 'L' :
    control_switch = 'L';
    motorState = 1; //  속도에 1을 곱하여 이동  //  EN ON
    go_straight();  //  GO ON
    Serial.println(F(" IR 라인 추적모드 "));
    break;

    case 'Z' :
    control_switch = 'Z';
    Serial.println(F(" 미로찾기 모드"));
    break;

    case 'D' :
    control_switch = 'D';
    motorState = 1;
    Serial.println(F(" 강아지 모드"));
    break;

    case 't' :
    control_switch = 't';
    Serial.println(F(" 서보모터 테스트 모드"));
    break;

    case 'O' :
    control_switch = 'O';
    break;
  }
}

int dataTypeChange (char charData)
{
  return int(charData) - 48;  //  ASCII 코드로 인한 char => int 변환
}

int CharIntArrayChangeToInt(char *arrayData)
{
  int arrayCount = arrayDataCount(arrayData);
  if (arrayCount == 0) return ERRORCODE;
  int returnData = 0;
  int multiplication = 1;
  arrayChange(arrayData);

  for(int i = 0; i < arrayCount; i++){
    returnData += (dataTypeChange(arrayData[i]) * multiplication);
    multiplication = ((multiplication << 3) + (multiplication << 1));
  }

  return returnData;
}

int arrayDataCount(char arrayData[ARRAY_MAX_SIZE])  //  배열 내 데이터 크기 확인
{
  int count;
  for(count = 0; arrayData[count] != NULL; count++);
  return count;
}

void arrayChange(char *arrayData) //  char형 배열을 int형 배열로 전환 및 역으로 정렬
{
  char arrayBuffer[100] = {0};  //  char => int 형으로 바꾸기 위한 임시 버퍼
  int stringCount = 0;  //  원본 카운터
  int arrayCount = 0; //  버퍼 하향계수를 위한 카운터
  
  for(stringCount = 0; arrayData[stringCount] != NULL; stringCount++){
    arrayBuffer[stringCount] = arrayData[stringCount];
  }
  
  arrayCount = arrayDataCount(arrayData) - 1; //  배열은 0부터기 때문에 -1필요
  for(stringCount = 0; arrayBuffer[stringCount] != NULL; stringCount++){
    arrayData[stringCount] = arrayBuffer[arrayCount];
    arrayCount--;
  }
}

void serialManualText() //  설명서 출력
{
  Serial.println(F(WALL01)); //  벽
  Serial.println(F(" 설명서"));
  Serial.println(F(" A => 차 기준 오른쪽"));
  Serial.println(F(" B => 차 기준 왼쪽"));
  Serial.println(F(WALL01)); //  벽
  Serial.println(F(" Serial command"));
  Serial.println(F(" M => Motor control mode"));
  Serial.println(F(" S 0~180 => Servo motor control [ 0~180 각도 조절]"));
  Serial.println(F(" N => Sonar sansor control"));
  Serial.println(F(" L => Line tracking mode"));
  Serial.println(F(" Z => Maze mode [구현 x]"));
  Serial.println(F(" D => Dog mode"));
  Serial.println(F(" t => Servo motor control [ 0~180 각도 왕복]"));
  Serial.println(F(" O => Reset [종료시 필수!]"));
  Serial.println(F(WALL01)); //  벽
  Serial.println(F(" Remote control"));
  Serial.println(F(" 1번 스위치 => 감소"));
  Serial.println(F(" 2번 스위치 => 증가"));
  Serial.println(F(" 3번 스위치 => 모드 설정"));
  Serial.println(F(" 4번 스위치 => 초기화"));
  Serial.println(F(" Mode 0   => 초음파 테스트"));
  Serial.println(F(" Mode 1   => 앞으로 전진"));
  Serial.println(F(" Mode 2   => 뒤로 후진"));
  Serial.println(F(" Mode 3   => A turn"));
  Serial.println(F(" Mode 4   => B turn"));
  Serial.println(F(" Mode 5   =>Dog Mode"));
  Serial.println(F(" Mode 6   =>Line Tracking Mode"));
  Serial.println(F(WALL01)); //  벽01
}

//##############################################################################################################################


//  Software Serial 관련  #######################################################################################################################################
void softwareSerialControl_main(int value)
{
  Serial.println(F(WALL02)); //  벽02
    switch(value) {
    case 0 :
    Serial.println(F("블루투스 입력 : 0 [초음파 테스트]"));
    change_Mode('t');
    break;
    
    case 1 :
    Serial.println(F("블루투스 입력 : 1 [앞으로 전진]"));
    //change_Mode('O');
    motorA_speed = motorB_speed = 200;
    motorState = 1;
    go_straight();
    moveMotorProtocol();
    break;
    
    case 2 :
    Serial.println(F("블루투스 입력 : 2 [뒤로 후진]"));
    //change_Mode('O');
    motorA_speed = motorB_speed = 200;
    motorState = 1;
    go_back();
    moveMotorProtocol();
    break;
    
    case 3 :
    Serial.println(F("블루투스 입력 : 3 [A turn]"));
    //change_Mode('O');
    motorA_speed = motorB_speed = 255;
    motorState = 1;
    point_turn_A();
    moveMotorProtocol();
    break;
    
    case 4 :
    Serial.println(F("블루투스 입력 : 4 [B turn]"));
    //change_Mode('O');
    motorA_speed = motorB_speed = 255;
    motorState = 1;
    point_turn_B();
    moveMotorProtocol();
    break;
    
    case 5 :
    Serial.println(F("블루투스 입력 : 5 [강아지]"));
    change_Mode('D');
    break;
    
    case 6 :
    Serial.println(F("블루투스 입력 : 6 [Line tracking]"));
    change_Mode('L');
    break;
    
    case 7 :
    Serial.println(F("블루투스 입력 : 7"));
    break;
    
    case 8 :
    Serial.println(F("블루투스 입력 : 8"));
    break;
    
    case 9 :
    Serial.println(F("블루투스 입력 : 9"));
    break;
    
    case 10 :
    Serial.println(F("블루투스 입력 : 10"));
    break;
    
    case 11 :
    Serial.println(F("블루투스 입력 : 11 [초기화]"));
    change_Mode('O');
    break;
    
    case 12 :
    Serial.println(F("블루투스 입력 : 12 [모드 알려줘]"));
    swSerial.print(F(" BlueTooth:"));
    switch(control_switch) {
      case 'M' : 
      case 'N' :  
      swSerial.println(ERRORCODE);
      break;
      
      case 'D' : // 지금 강아지 모드
      swSerial.println(1);
      break;
      
      case 't' : // 지금 초음파 테스트 모드
      swSerial.println(2);
      break;
      
      case 'L' : // 지금 라인 트레킹 모드
      swSerial.println(3);
      break;
    }
    break;

    default :
    Serial.println(F("블루투스 코드지만 숫자가 명령어에 할당되지 않았습니다."));
    break;
  }
  Serial.println(F(WALL02)); //  벽02
}


//#############################################################################################################################################################


//############################## motor control #####################################################################################

void moveMotorProtocol()
{
  motorSpeedOver(); //  값 Over 확인
  print_motorSpeed(); //  모터 스피드 출력
  motorAB(motorA_speed, motorB_speed);  //  모터 구동
}

void motorAB(int msA, int msB)  //  motor A, B control
{
  analogWrite(ENA, (msA * motorState));
  analogWrite(ENB, (msB * motorState));
}

void motorStop_SP() //  멈춤  =>  Speed를 0으로 해서 멈춤, 스피드를 올려야 다시감
{
  motorA_speed = 0;
  motorB_speed = 0;
}

void motorReset() { //  모터 초기화
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void motorA_go() {  //  A motor 앞으로
   //  A  앞
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void motorB_go() {  //  B motor 앞으로
   //  B  앞
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void motorA_back() {  //  A motor 뒤로
   //  A  뒤
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void motorB_back() {  //  B motor 뒤로
   //  B  뒤
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void motorA_stop() {  //  A motor 정지
   //  A  정지
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
}

void motorB_stop() {  //  B motor 정지
   //  B  정지
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, HIGH);
}

void go_straight()  //  앞으로
{
  motorA_go();
  motorB_go();
}

void go_back()  //  뒤로
{
  motorA_back();
  motorB_back();
}

void motorStop()  //  정지
{
  motorA_stop();
  motorB_stop();
}

void point_turn_A() //  A방향(오른쪽)으로 point turn
{
  motorA_back();
  motorB_go();
}

void point_turn_B() //  B방향으로(왼쪽) point turn
{
  motorA_go();
  motorB_back();
}

void swing_turn_A() //  A방향으로 swing turn
{
  motorA_stop();
  motorB_go();
}

void swing_turn_B() //  B방향으로 swing turn
{
  motorA_go();
  motorB_stop();
}

void motorSpeedOver() //  PWM의 최소값 0 ~ 최대값 255 를 벗어나면 자동으로 교정
{
  if (motorA_speed > 255){
    motorA_speed = 255;
    Serial.println(F(" 모터A속도 255 초과. 자동으로 교정합니다."));
  }
  
  if (motorB_speed > 255){
    motorB_speed = 255;
    Serial.println(F(" 모터B속도 255 초과. 자동으로 교정합니다."));
  }

  if (motorA_speed & 0x8000) {
    motorA_speed = 0;
    Serial.println(F(" 모터A속도 0 미만. 자동으로 교정합니다."));
  }

  if (motorB_speed & 0x8000) {
    motorB_speed = 0;
    Serial.println(F(" 모터B속도 0 미만. 자동으로 교정합니다."));
  }
}

void print_motorSpeed()
{
  Serial.println(F(" 속도 [A,B] "));
  Serial.println(motorA_speed);
  Serial.println(motorB_speed);
}

//##############################################################################################################################

//############################## Servo Control #####################################################################################

void ServoControl(int serial_Sdata)
{
  int mode_value = serial_Sdata;
  if (mode_value < 0) mode_value = 0;
  if (mode_value > 180) mode_value = 180;
  
  Serial.print(F(" 각도 : "));
  Serial.println(mode_value);

  servoMotor.write(mode_value);
}

void servoSwing()
{
  sonarSwingValue += sonarSwingPlusValue;
  if (sonarSwingValue > 180 || sonarSwingValue < 0) sonarSwingPlusValue *= -1;
  servoMotor.write(sonarSwingValue);
  delay(SWING_DELAY);
}

//##############################################################################################################################


//############################## Motor Mode #####################################################################################

void MotorMode(char change_Mdata, int intData, char change_Mdata02)
{
  switch(change_Mdata)  //  문자형 모드변환
  {
    case 'g' :
    go_straight();
    Serial.println(F(" ##앞으로##"));
    break;

    case 'b' :
    go_back();
    Serial.println(F(" ##뒤로##"));
    break;

    case 'm' :
    motorState = 1; //  속도에 1을 곱하여 이동
    Serial.println(F(" ##EN이동##"));
    break;
    
    case 's' :
    motorState = 0; //  속도에 0을 곱하여 정지;
    Serial.println(F(" ##정지##"));
    break;

    case 'p' :
    switch(change_Mdata02){
      case 'A' :
      Serial.println(F(" ##point turn A##"));
      point_turn_A();
      break;

      case 'B' :
      Serial.println(F(" ##point turn B##"));
      point_turn_B();
      break;

      default :
      Serial.println(F(" A또는 B를 입력해야 합니다."));
    }
    break;

    case 'w' :
    switch(change_Mdata02){
      case 'A' :
      Serial.println(F(" ##swing turn A##"));
      swing_turn_A();
      break;

      case 'B' :
      Serial.println(F(" ##swing turn B##"));
      swing_turn_B();
      break;

      default :
      Serial.println(F(" A또는 B를 입력해야 합니다."));
    }
    break;
    
    case 'A' :
    motorA_speed = intData;
    Serial.println(F(" ##A 모터 속도변환##"));
    break;

    case 'B' :
    motorB_speed = intData;
    Serial.println(F(" ##B 모터 속도변환##"));
    break;
  }

  if (intData == ERRORCODE)
    Serial.println(F(" !Serial int data is ERROR! [숫자 입력 x] "));
  else if (change_Mdata == 'A' || change_Mdata == 'B')
    Serial.println(F(" 두개의 모터중 하나만 속도를 바꿉니다. "));
  else{
    motorA_speed = intData;
    motorB_speed = intData;
  }
  moveMotorProtocol(); //  모터 제어
}
//##############################################################################################################################


//############################## Sonar Mode #####################################################################################

void SonarMode()
{
  float scanSonar_data = scanSonar();
  Serial.println(scanSonar_data);
  /*
  if(scanSonar_data > SONAR_GO_DISTANCE){
    Serial.println(F(" 앞으로 "));
    motorState = 0; //  속도에 0을 곱하여 정지 (스피드 변화 없음)
    delay(STOP_DELAY);
    motorState = 1; //  속도에 1을 곱하여 이동
    go_straight();
  }
  else if(scanSonar_data < SONAR_BACK_DISTANCE){
    Serial.println(F(" 뒤로 "));
    motorState = 0; //  속도에 0을 곱하여 정지 (스피드 변화 없음)
    delay(STOP_DELAY);
    motorState = 1; //  속도에 1을 곱하여 이동
    go_back();
  }
  else {
    Serial.println(F(" 정지 "));
    motorState = 0; //  속도에 0을 곱하여 정지 (스피드 변화 없음)
  }
  //moveMotorProtocol(); //  모터 제어
  */
  delay(SCAN_DELAY);
}

int scanSonar()
{
  int distance = 0;
  
  digitalWrite(SONAR_TRG_PIN, HIGH);
  delay(10);
  digitalWrite(SONAR_TRG_PIN, LOW);
  distance = (pulseIn(ECHO, HIGH) * LOGIC);

  return distance;
}

//############################## Maze Mode #####################################################################################

void mazeMode_main(char testMode)
{
  float front_distance = 0; //  전방 거리
  float right_distance = 0; //  우측 거리
  bool WALL01_ = false;
  bool count_check = false;

  static int WALL01_check_count  = 0;  //  벽 확인 count
  static int open_check_count  = 0;  //  개방 확인 count

  static int delay_count = 0; //  좌측 확인 지연 count

  switch(testMode)
  {
    case 'L' :
    Serial.println(F("미로찾기 좌회전 테스트"));
    motorA_speed = motorB_speed = MAZE_MOTOR_TURN_SPEED;
    mazeTurnLeft();
    control_switch = 'O';
    Serial.println(F("미로찾기 좌회전 완료"));
    break;

    case 'l' :
    Serial.println(F("미로찾기 좌 측정"));
    Serial.println(mazeWallSonar(SONAR_LEFT_SCAN));
    control_switch = 'O';
    break;

    case 'R' :
    Serial.println(F("미로찾기 우회전 테스트"));
    motorA_speed = motorB_speed = MAZE_MOTOR_TURN_SPEED;
    mazeTurnRight();
    control_switch = 'O';
    Serial.println(F("미로찾기 우회전 완료"));
    break;

    case 'r' :
    Serial.println(F("미로찾기 우 측정"));
    Serial.println(mazeWallSonar(SONAR_RIGHT_SCAN));
    control_switch = 'O';
    break;

    case 'S' :
    Serial.println(F("미로찾기 직진 테스트"));
    while (mazeGostraight() != true);
    control_switch = 'O';
    Serial.println(F("미로찾기 직진 완료"));
    break;

    case 's' :
    Serial.println(F("미로찾기 전방 측정"));
    Serial.println(mazeWallSonar(SONAR_ANGLE_FRONT));
    control_switch = 'O';
    break;

    default :
    Serial.println(F("미로찾기를 시작합니다."));
    motorA_speed = motorB_speed = MAZE_MOTOR_GO_SPEED;
    delay_count++;
    /*
    if (mazeGostraight() || delay_count > MAZE_DELAY_COUNT) {
      //todo  구현 해야함
      motorStop();
      moveMotorProtocol();
      delay(10);
      right_distance = mazeWallSonar(SONAR_ANGLE_RIGHT);
      delay(10);
      if (right_distance > MAZE_OPEN_DISTANCE) mazeTurnRight();
      else mazeTurnLeft();
      delay_count = 0;
    }
    */

    if (delay_count > MAZE_DELAY_COUNT) count_check = true;
    
    if (mazeGostraight() || count_check == true) {
      motorStop();
      moveMotorProtocol();
      delay(10);
      right_distance = mazeWallSonar(SONAR_ANGLE_RIGHT);
      delay(10);
      if (right_distance > MAZE_OPEN_DISTANCE) mazeTurnRight();
      else if (count_check == 0) mazeTurnLeft();
      else mazeGostraight();
      delay_count = 0;
    }
  }
  delay(1);
}

float mazeWallSonar(int Angle)
{
  float sansorValue = 0;
  ServoControl(Angle);
  delay(10);
  sansorValue = scanSonar();
  delay(10);
  return sansorValue;
}

bool mazeGostraight()
{
  static float front_distance = 0; //  전방 거리 
  static int count = 0; //  벽 카운트
  
  ServoControl(SONAR_ANGLE_FRONT); //  서보모터 직진 준비
  motorState = 1; //  이동 가능상태
  go_straight(); //  앞으로 전진
  moveMotorProtocol(); //  모터 제어
  delay(15);
  
  front_distance = mazeWallSonar(SONAR_ANGLE_FRONT); //  전방 거리 확인
  Serial.print(F(" 전방 거리 확인 : "));
  Serial.println(front_distance);
  
  if (front_distance < MAZE_WALL01_DISTANCE) count++;
  if (count > MAZE_COUNT) {
    motorState = 0; //  이동 불능상태
    moveMotorProtocol(); //  모터 제어
    count = 0;
    return true;
  }
  return false;
}

void mazeTurnLeft()
{
  int open_check_count = 0;  //  개방 확인 count
  float front_distance = 0; //  전방 거리
  
  motorState = 1; //  이동 가능상태
  point_turn_B(); //  좌측으로 회전
  moveMotorProtocol(); //  모터 제어
  delay(15);

  delay(1000);
  do{
    front_distance = mazeWallSonar(SONAR_RIGHT_SCAN); //  전방 거리 확인
    if (front_distance >= MAZE_OPEN_DISTANCE) open_check_count++;  //  개방 확인
    Serial.print(F(" 좌회전 거리 확인 : "));
    Serial.println(front_distance);
    delay(5);
  } while (open_check_count < MAZE_OPEN_COUNT);
  
  Serial.println(F(" 좌측으로 회전완료"));
  motorState = 0; //  이동 불능상태
  moveMotorProtocol(); //  모터 제어
  ServoControl(SONAR_ANGLE_FRONT); //  서보모터 전방
  delay(100);
}

void mazeTurnRight()
{
  int open_check_count  = 0;  //  개방 확인 count
  float front_distance = 0; //  전방 거리
  
  motorState = 1; //  이동 가능상태
  point_turn_A(); //  우측으로 회전
  moveMotorProtocol(); //  모터 제어
  delay(15);
  
  delay(1000);
  do{
    front_distance = mazeWallSonar(SONAR_LEFT_SCAN); //  전방 거리 확인
    if (front_distance >= MAZE_OPEN_DISTANCE) open_check_count++;  //  개방 확인
    Serial.print(F(" 우회전 거리 확인 : "));
    Serial.println(front_distance);
    delay(5);
  } while (open_check_count < MAZE_OPEN_COUNT);
  
  Serial.println(F(" 우측으로 회전완료"));
  motorState = 0; //  이동 불능상태
  moveMotorProtocol(); //  모터 제어
  ServoControl(SONAR_ANGLE_FRONT); //  서보모터 전방
  delay(100);
}

//##############################################################################################################################


//############################## Line Tracking Mode #####################################################################################

/*
void IrLineMode_1IR()
{
  int irValue = analogRead(IRPIN_A);
  Serial.print(F(" 바닥 값 : "));
  Serial.print(irValue);
  //lineTracking_position;
  motorA_speed = motorB_speed = LINE_SPEED_FAST;
  if(irValue < LINE_COLOR_WHITE){
    Serial.println(F(" ## 하얀색 ## ")); //  좌회전 B쪽으로 turn
    //motorA_speed = LINE_SPEED_FAST;
    //motorB_speed = LINE_SPEED_SLOW;
    swing_turn_B();
  }
  else if(irValue > LINE_COLOR_BLACK){
    Serial.println(F(" ## 검정색 ## ")); //  우회전 A쪽으로 turn
    //motorA_speed = LINE_SPEED_SLOW;
    //motorB_speed = LINE_SPEED_FAST;
    swing_turn_A();
  }
  else Serial.println(F(" ## ??? ## "));

  moveMotorProtocol(); //  모터 제어
  
  delay(LINE_READ_DELAY);
}
*/

void IrLineMode_2IR()
{
  int irValue_A = analogRead(IRPIN_A);
  int irValue_B = analogRead(IRPIN_B);
  //lineTracking_position;
  motorA_speed = motorB_speed = LINE_SPEED_SLOW;
  if(irValue_A > LINE_COLOR_BLACK && irValue_B > LINE_COLOR_BLACK) motorStop_SP(); //  A, B 둘다 검은색이면 정지
  else if(irValue_A > LINE_COLOR_BLACK && irValue_B < LINE_COLOR_WHITE) {
    Serial.println(F(" 우회전"));
    motorB_speed = LINE_SPEED_FAST;
    point_turn_A(); //  A 검은색이면 A쪽으로 회전 (우회전)
  }
  else if(irValue_B > LINE_COLOR_BLACK && irValue_A < LINE_COLOR_WHITE) {
    Serial.println(F(" 좌회전"));
    motorA_speed = LINE_SPEED_FAST;
    point_turn_B(); //  B 검은색이면 B쪽으로 회전 (좌회전)
  }
  else go_straight(); //  둘다 검은색 아니면 직진

  moveMotorProtocol(); //  모터 제어
  
  delay(LINE_READ_DELAY);
}

//##############################################################################################################################


//############################## Dog Mode #####################################################################################

void dogMode_main()
{
  int value = scanSonar();
  
  if (value > DOG_MAX) {
    if (scanSonar() > DOG_MAX){
      dogState = true;
      motorStop_SP(); //  속도 0으로
      moveMotorProtocol();
    }
  }
  else {
    value = scanSonar();
    dogState = false;
  }

  //turnDog();
  ///*
  if (dogState) {
  turnDog();
  }
  else {
    if (value < DOG_MIN - DOG_ERROR_DOWN) {
      //dogSpeed = (150 + value * 2);
      dogSpeed = DOG_GO_SPEED;
      Serial.println(F("뒤로"));
      go_back();
      motorA_speed = dogSpeed;
      motorB_speed = dogSpeed;
      moveMotorProtocol();
    }
    else if (value > DOG_MIN + DOG_ERROR_DOWN) {
      //dogSpeed = (150 + (1 / value) * 100);
      dogSpeed = DOG_GO_SPEED;
      Serial.println(F("앞으로"));
      go_straight();
      motorA_speed = dogSpeed;
      motorB_speed = dogSpeed;
      moveMotorProtocol();
    }
    else {
      motorStop_SP(); //  속도 0으로
      Serial.println(F("정지"));
      moveMotorProtocol();
    }
    Serial.println(value);
  }
  //*/
  delay(100);
}

void turnDog()
{
  int value = scanSonar();
  switch (scanDog()) {
    case FRONT_FOUND :  //  전방에 물체
      Serial.println(F("전방에 물체 발견"));
      ServoControl(SONAR_ANGLE_FRONT);
      dogState = false;
      break;

    case LEFT_FOUND :  //  왼쪽에 물체
    case LEFT_HALF_FOUND :  //  왼쪽반에 물체
      Serial.println(F("왼쪽에 물체 발견"));
      ServoControl(SONAR_ANGLE_FRONT);
      delay(10);
      point_turn_B();
      motorA_speed = DOG_TURN_SPEED;
      motorB_speed = DOG_TURN_SPEED;
      moveMotorProtocol();
      delay(1);
      while (1) {
        value = scanSonar();
        Serial.println(value);
        motorA_speed = DOG_TURN_SPEED;
        motorB_speed = DOG_TURN_SPEED;
        moveMotorProtocol();
        if (value < DOG_MAX) {
          if (scanSonar() < DOG_MAX) {
            dogState = false;
            delay(DOG_TURN_DELAY);
            motorStop_SP(); //  속도 0으로
            moveMotorProtocol();
            Serial.println(F("회전 완료"));
            break;
          }
        }
        delay(5);
      }
      motorStop_SP(); //  속도 0으로
      moveMotorProtocol();
      dogState = false;
      break;

    case RIGHT_FOUND :  //  오른쪽에 물체
    case RIGHT_HALF_FOUND :  //  오른쪽반에 물체
      Serial.println(F("오른쪽에 물체 발견"));
      ServoControl(SONAR_ANGLE_FRONT);
      delay(10);
      point_turn_A();
      motorA_speed = DOG_TURN_SPEED;
      motorB_speed = DOG_TURN_SPEED;
      moveMotorProtocol();
      delay(1);
      while (1) {
        value = scanSonar();
        Serial.println(value);
        motorA_speed = DOG_TURN_SPEED;
        motorB_speed = DOG_TURN_SPEED;
        moveMotorProtocol();
        if (value < DOG_MAX) {
          if (scanSonar() < DOG_MAX) {
            dogState = false;
            delay(DOG_TURN_DELAY);
            motorStop_SP(); //  속도 0으로
            moveMotorProtocol();
            Serial.println(F("회전 완료"));
            break;
          }
        }
        delay(5);
      }
      break;

    case NOT_FOUND :  //  찾지 못함
      Serial.println(F("물체 발견 실패"));
      break;
  }
}

char scanDog()
{
  static int rotate_count = 2;
  static int rotate_state = 1;  //  true=>왼쪽으로 회전중, false=>오른쪽으로 회전중
  int value = 0;

  switch (rotate_count) {
    case 0 :  //  상태 왼쪽 [서쪽]
      ServoControl(SONAR_ANGLE_LEFT);
      delay(DOG_SCAN_DELAY);
      value = scanSonar();
      delay(DOG_SCAN_DELAY);
      value += scanSonar();
      value = value / 2;
      
      Serial.println(F("상태 왼쪽 [서쪽] : "));
      Serial.println(value);
      rotate_state = 1; //  증가  0 => 4
      rotate_count += rotate_state;
      if (value < DOG_MAX) {
        rotate_count = 2;
        return LEFT_FOUND;
      }
      return NOT_FOUND;

    case 1 :  //  상태 왼쪽반 [북서쪽]
      ServoControl(SONAR_ANGLE_HALF_LEFT);
      delay(DOG_SCAN_DELAY);
      value = scanSonar();
      delay(DOG_SCAN_DELAY);
      value += scanSonar();
      value = value / 2;
      
      Serial.print(F("상태 왼쪽반 [북서쪽] : "));
      Serial.println(value);
      rotate_count += rotate_state;
      if (value < DOG_MAX) {
        rotate_count = 2;
        return LEFT_HALF_FOUND;
      }
      return NOT_FOUND;
    
    case 2 :  //  상태 중앙 [북쪽]
      ServoControl(SONAR_ANGLE_FRONT);
      delay(DOG_SCAN_DELAY);
      value = scanSonar();
      delay(DOG_SCAN_DELAY);
      value += scanSonar();
      value = value / 2;
      
      Serial.print(F("상태 중앙 [북쪽] : "));
      Serial.println(value);
      rotate_count += rotate_state;
      if (value < DOG_MAX) {
        rotate_count = 2;
        return FRONT_FOUND;
      }
      return NOT_FOUND;

    case 3 :  //  상태 오른쪽반 [북동쪽]
      ServoControl(SONAR_ANGLE_HALF_RIGHT);
      delay(DOG_SCAN_DELAY);
      value = scanSonar();
      delay(DOG_SCAN_DELAY);
      value += scanSonar();
      value = value / 2;
      
      Serial.print(F("상태 오른쪽반 [북동쪽] : "));
      Serial.println(value);
      rotate_count += rotate_state;
      if (value < DOG_MAX) {
        rotate_count = 2;
        return RIGHT_HALF_FOUND;
      }
      return NOT_FOUND;
    
    case 4 :  //  상태 오른쪽 [동쪽]
      ServoControl(SONAR_ANGLE_RIGHT);
      delay(DOG_SCAN_DELAY);
      value = scanSonar();
      delay(DOG_SCAN_DELAY);
      value += scanSonar();
      value = value / 2;
      
      Serial.print(F("상태 오른쪽 [동쪽]] : "));
      Serial.println(value);
      rotate_state = -1; //  감소 4 => 0
      rotate_count += rotate_state;
      if (value < DOG_MAX) {
        rotate_count = 2;
        return RIGHT_FOUND;
      }
      return NOT_FOUND;
  }
}

//##############################################################################################################################
