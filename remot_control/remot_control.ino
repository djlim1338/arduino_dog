/*
 * remot_control
 */

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------


//  외부파일 관련 --------------------------------------------------------------------------------------
#include <SoftwareSerial.h>
//----------------------------------------------------------------------------------------------------


//  Pin 관련  -----------------------------------------------------------------------------------------
#define SOFT_SERIAL_RX 2
#define SOFT_SERIAL_TX 3

#define SHIFTREGISTER_OE_PIN 10
#define SHIFTREGISTER_CLOCK 0b00100000
#define SHIFTREGISTER_LATCH 0b01000000
#define SHIFTREGISTER_DATA 0b10000000

#define BUTTON_PIN A0
#define MODE_STATE_LED_PIN 8
#define POWER_STATE_LED_PIN 9
//----------------------------------------------------------------------------------------------------


//  Segment 관련  -------------------------------------------------------------------------------------
#define SEGMENT_ONE_0 0b11111100
#define SEGMENT_ONE_1 0b01100000
#define SEGMENT_ONE_2 0b11011010
#define SEGMENT_ONE_3 0b11110010
#define SEGMENT_ONE_4 0b01100110
#define SEGMENT_ONE_5 0b10110110
#define SEGMENT_ONE_6 0b10111110
#define SEGMENT_ONE_7 0b11100100
#define SEGMENT_ONE_8 0b11111110
#define SEGMENT_ONE_9 0b11100110
#define SEGMENT_ONE_DOT 0b00000001
#define SEGMENT_ONE_RESET 0b00000010
#define SEGMENT_ONE_NOMODE 0b00000011
#define SEGMENT_ONE_RESET_CODE 11
#define SEGMENT_ONE_NOMEDE_CODE 12
#define SEGMENT_PWM 250
//----------------------------------------------------------------------------------------------------


//  Software Serial 관련 ------------------------------------------------------------------------------
SoftwareSerial swSerial(SOFT_SERIAL_RX, SOFT_SERIAL_TX);  //  아두이노 기준으로(RX, TX) 아두이노 RX -> 블루투스 TX, 아두이노 TX -> 블루투스RX 인듯
#define COM_CODE_SIZE 4
#define BLUETOOTH_CODE_SIZE 10
//----------------------------------------------------------------------------------------------------


//  code 관련 ############################################################
#define ERRORCODE -9999
#define WALL01 "----------------------------------------"
#define WALL02 "########################################"
#define ARRAY_MAX_SIZE 100


//  변수 관련 ------------------------------------------------------------------------------------------
unsigned char shiftregisterState = SEGMENT_ONE_RESET;
char allStringData[ARRAY_MAX_SIZE] = "";  //  입력받은 데이터 분류 전 모든 데이터
int allStringDataCount = 0; //  counter
bool stringDataType = 0;  //  0 => Hardware Serial, 1 => Soft
unsigned char numberData[13] = {  //  segment number display
  SEGMENT_ONE_0,
  SEGMENT_ONE_1,
  SEGMENT_ONE_2,
  SEGMENT_ONE_3,
  SEGMENT_ONE_4,
  SEGMENT_ONE_5,
  SEGMENT_ONE_6,
  SEGMENT_ONE_7,
  SEGMENT_ONE_8,
  SEGMENT_ONE_9,
  SEGMENT_ONE_DOT,
  SEGMENT_ONE_RESET,
  SEGMENT_ONE_NOMODE
};
int controlSwitch = 11;
int controlSwitchSecond = 11;
char bluetoothCode[BLUETOOTH_CODE_SIZE] = {
  'B',
  'l',
  'u',
  'e',
  'T',
  'o',
  'o',
  't',
  'h',
  ':'
};
  
//----------------------------------------------------------------------------------------------------

 // Set Up  ------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(9600);
  swSerial.begin(9600);
  DDRD |= 0b11100000;
  pinMode(SHIFTREGISTER_OE_PIN, OUTPUT);
  setShiftregister();
  analogWrite(SHIFTREGISTER_OE_PIN, SEGMENT_PWM);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(MODE_STATE_LED_PIN, OUTPUT);
  pinMode(POWER_STATE_LED_PIN, OUTPUT);
  digitalWrite(POWER_STATE_LED_PIN, HIGH);  //  전원 확인 LED ON

  /*
  Serial.println('⸮');
  Serial.println('⸮', HEX);
  Serial.println('⸮', DEC);
  Serial.println('⸮', BIN);
  */
}
//----------------------------------------------------------------------------------------------------


//  Loop    ------------------------------------------------------------------------------------------
void loop()
{
  buttonScan_main();
    
  if (Serial.available() || swSerial.available()){  //  Serial data read and arranged
    if(Serial.available()) stringDataType = 0;  //  Hard
    else stringDataType = 1;  //  Soft
    while (1){
      delay(1);
      //Serial.println(F(" 읽는중... "));
      if (stringDataType)
        allStringData[allStringDataCount] = swSerial.read();
      else
        allStringData[allStringDataCount] = Serial.read();
        
      if (allStringData[allStringDataCount] == '\n' || allStringData[allStringDataCount] == NULL) break;
      //if (stringDataType == 0 && !Serial.available()) break;
      //if (stringDataType == 1 && !swSerial.available()) break;
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
  delay(10);
}
//----------------------------------------------------------------------------------------------------


//  Data 관련 ------------------------------------------------------------------------------------------
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
  //  Serial.println(controlSwitch);
  //}

    //  데이터 확인 코드
  Serial.println(F(" Data Input Type = "));
  Serial.print(stringDataType);
  if (stringDataType)
    Serial.println(F(" Software Serial Type "));
  else
    Serial.println(F(" Hardware Serial Type "));
    
  Serial.println(F(" charStringData = "));
  Serial.println(charStringData);
  //Serial.println(F(" intStringCharData = "));
  //Serial.println(intStringCharData);
  Serial.println(F(" intStringData = "));
  Serial.println(intStringData);

  if (intStringData < 10 && intStringData >= 0) {
    displaySet(numberData[intStringData]);
    Serial.print(F("디스플레이 수 : "));
    Serial.println(intStringData);
  }
  else if (intStringData == -9999){}
  else {
    Serial.print(F("값을 넘겼습니다. : "));
    Serial.println(intStringData);
  }
  
  for (int count = 0; count < BLUETOOTH_CODE_SIZE; count++) {
    if (charStringData[count] != bluetoothCode[count]) break;
    else if (count < BLUETOOTH_CODE_SIZE - 1) continue;
    else softwareSerialControl_main(intStringData);
  }

  switch (charStringData[0]) {
    case 'R' :
    displayReSet();
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

void softwareSerialControl_main(int value)
{
  Serial.println(F(WALL02)); //  벽02
    switch(value) {
    case 0 :
    Serial.println(F("블루투스 입력 : 0"));
    break;
    
    case 1 :
    Serial.println(F("블루투스 입력 : 1 [지금 강아지 모드]"));
    controlSwitch = controlSwitchSecond = 1;
    break;
    
    case 2 :
    Serial.println(F("블루투스 입력 : 2 [지금 초음파 테스트 모드]"));
    controlSwitch = controlSwitchSecond = 2;
    break;
    
    case 3 :
    Serial.println(F("블루투스 입력 : 3 [지금 라인 트레킹 모드]"));
    controlSwitch = controlSwitchSecond = 3;
    break;
    
    case 4 :
    Serial.println(F("블루투스 입력 : 4"));
    break;
    
    case 5 :
    Serial.println(F("블루투스 입력 : 5"));
    break;
    
    case 6 :
    Serial.println(F("블루투스 입력 : 6"));
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
    Serial.println(F("블루투스 입력 : 11"));
    break;
    
    case 12 :
    Serial.println(F("블루투스 입력 : 12"));
    break;

    default :
    Serial.println(F("블루투스 코드지만 숫자가 명령어에 할당되지 않았습니다."));
    ledBlink(100, 2);
    break;
  }
  Serial.println(F(WALL02)); //  벽02
}

void ledBlink(int delayTime, int count)
{
  for (int i = 0; i < count; i++) {
    digitalWrite(MODE_STATE_LED_PIN, HIGH);
    delay(delayTime);
    digitalWrite(MODE_STATE_LED_PIN, LOW);
    delay(delayTime);
  }
}
//----------------------------------------------------------------------------------------------------


//  display 관련 [shift register]  --------------------------------------------------------------------
void displaySet(unsigned char data)
{
  shiftregisterState = data;
  setShiftregister();
}

void displayReSet()
{
  shiftregisterState = SEGMENT_ONE_RESET;
  setShiftregister();
}

void setShiftregister()
{
  unsigned char mask = 0b00000001;
  unsigned char data = shiftregisterState;

  while (mask) {
    if (data & mask) PORTD |= SHIFTREGISTER_DATA;
    else PORTD &= ~SHIFTREGISTER_DATA;

    PORTD |= SHIFTREGISTER_CLOCK;
    PORTD &= ~SHIFTREGISTER_CLOCK;

    mask <<= 1;
  }

  PORTD |= SHIFTREGISTER_LATCH;
  PORTD &= ~SHIFTREGISTER_LATCH;
}
//----------------------------------------------------------------------------------------------------


//  Buttin 관련 --------------------------------------------------------------------------------------
void buttonScan_main()
{
  int buttonValue = 0;
  static char buttonState = "";
  static char buttonStateSecond = "";
  
  buttonValue = analogRead(BUTTON_PIN);
  //Serial.println(buttonValue);
  if (buttonValue < 100) {
    buttonState = "";  //  초기화
  }
  else if (buttonValue > 620 && buttonValue < 640) {
    buttonState = 'D';  //  감소 버튼
    Serial.println(F("버튼 누름 감지 [D]"));
  }
  else if (buttonValue > 710 && buttonValue < 730) {
    buttonState = 'U';  //  증가 버튼
    Serial.println(F("버튼 누름 감지 [U]"));
  }
  else if (buttonValue > 830 && buttonValue < 850) {
    buttonState = 'C';  //  확인 버튼
    Serial.println(F("버튼 누름 감지 [C]"));
  }
  else if (buttonValue > 1010) {
    buttonState = 'R';  //  리셋 버튼
    Serial.println(F("버튼 누름 감지 [R]"));
  }
  else {};

  if (buttonState != buttonStateSecond){
    switch(buttonState) {
      case 'U' :
      controlSwitch++;
      if (controlSwitch > 9) controlSwitch = 0;
      break;

      case 'D' :
      controlSwitch--;
      if(controlSwitch < 0) controlSwitch = 9;
      if (controlSwitch > 9) controlSwitch = 0;
      break;

      case 'C' :
      switch (controlSwitch) {
        /*case 0: case 1: case 2: case 3: case 4: case 5: case 6:*/ case 7: case 8: case 9:
        Serial.println(F("없는 모드입니다."));
        controlSwitch = controlSwitchSecond;
        ledBlink(100, 2);
      }
      controlSwitchSecond = controlSwitch;
      swSerial.print(F(" BlueTooth:"));
      swSerial.println(controlSwitch);
      Serial.println(controlSwitch);
      break;

      case 'R' :
      controlSwitch = SEGMENT_ONE_RESET_CODE;
      controlSwitchSecond = SEGMENT_ONE_RESET_CODE;
      swSerial.print(F(" BlueTooth:"));
      swSerial.println(controlSwitch);
      Serial.println(controlSwitch);
      break;
    }
    if (controlSwitchSecond == controlSwitch)
      digitalWrite(MODE_STATE_LED_PIN, LOW);
    else
      digitalWrite(MODE_STATE_LED_PIN, HIGH);
    
    displaySet(numberData[controlSwitch]);
  }
  buttonStateSecond = buttonState;
}
//----------------------------------------------------------------------------------------------------
