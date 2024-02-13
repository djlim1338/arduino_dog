/*
 * remot_slave
 */

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------


//  외부파일 관련 --------------------------------------------------------------------------------------
#include <SoftwareSerial.h>
//----------------------------------------------------------------------------------------------------


//  Pin 관련  -----------------------------------------------------------------------------------------
#define SOFT_SERIAL_RX 2
#define SOFT_SERIAL_TX 3
//----------------------------------------------------------------------------------------------------


//  Software Serial 관련 ------------------------------------------------------------------------------
SoftwareSerial swSerial(SOFT_SERIAL_RX, SOFT_SERIAL_TX);  //  아두이노 기준으로(RX, TX) 아두이노 RX -> 블루투스 TX, 아두이노 TX -> 블루투스 RX 인듯
//----------------------------------------------------------------------------------------------------


//  code 관련 ############################################################
#define ERRORCODE -9999
#define WALL01 "----------------------------------------"
#define WALL02 "########################################"
#define ARRAY_MAX_SIZE 100


//  변수 관련 ------------------------------------------------------------------------------------------
unsigned char shiftregisterState = 0b00000000;
char allStringData[ARRAY_MAX_SIZE] = "";  //  입력받은 데이터 분류 전 모든 데이터
int allStringDataCount = 0; //  counter
bool stringDataType = 0;  //  0 => Hardware Serial, 1 => Soft
//----------------------------------------------------------------------------------------------------

 // Set Up  ------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(9600);
  swSerial.begin(9600);
}
//----------------------------------------------------------------------------------------------------


//  Loop    ------------------------------------------------------------------------------------------
void loop()
{
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
        
      if(allStringData[allStringDataCount] == '\n' || allStringData[allStringDataCount] == NULL) break;
      allStringDataCount++;
      if (allStringDataCount > 200) {
        Serial.println("문장이 끝나지 않아 강제로 빠져나옵니다.");
        break;
      }
    }
    serial_scan();
    if (stringDataType == 0) {
      for (int count = 0; count < allStringDataCount; count++) {
        swSerial.write(allStringData[count]);
        Serial.write(allStringData[count]);
      }
      Serial.println(F(""));
    }
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
    swSerial.write(allStringData[stringCount]);
    stringCount++;
  }

  intStringData = CharIntArrayChangeToInt(intStringCharData);
  //if (intStringData == ERRORCODE) {
  //  Serial.print(F(" 모드 : "));
  //  Serial.println(control_switch);
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
  Serial.println(F(" intStringCharData = "));
  Serial.println(intStringCharData);
  Serial.println(F(" intStringData = "));
  Serial.println(intStringData);
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
//----------------------------------------------------------------------------------------------------
