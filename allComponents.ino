#include <EnableInterrupt.h>
#include <TinyGPS.h>


#define encoderUpPinA 53                    //++++++++++++++++++++++++++++++++++++//
#define encoderUpPinB 51                    //------------------------------------//
#define encoderDownPinA 50                  //------------------------------------//
#define encoderDownPinB 52                  //---------------PINS-----------------//
#define tmpProcPin A9                       //------------------------------------//
#define tmpPowPin A10                       //------------------------------------//
#define airDustSensorPin A11				//------------------------------------//
#define airDustSensorPulsePin 37			//------------------------------------//
#define surfaceDustSensorPin A12			//------------------------------------//
#define motorUpDirPin 3						//------------------------------------//
#define motorUpPwmPin 2					//------------------------------------//
#define motorDownDirPin 5					//------------------------------------//
#define motorDownPwmPin 4				//------------------------------------//		
#define motorCleanDirPin 6					//------------------------------------//
#define motorCleanPwmPin 7					//------------------------------------//
#define emgStopPin 10						//------------------------------------//
#define proxiUpPin 42						//----------------PINS----------------//
#define proxiDownPin 40						//------------------------------------//
#define proxiLeftPin 43						//------------------------------------//
#define proxiRightPin 41                    //++++++++++++++++++++++++++++++++++++//

#define ENCODER_SAMPLING_TIME 50.0         //milliseconds
#define TOTAL_CODE_DELAY_REQ 300

double kp = 0.04;
double ki = 0.04;
double kd = 0.04;

volatile double encoderUpPos = 0;           //++++++++++++++++++++++++++++++++++++//
volatile double encoderDownPos = 0;         //-------------ENCODERS---------------//
double lastReportedUpPos = 0;
double lastReportedDownPos = 0;
long int lastTimeUp = 0;
long int lastTimeDown = 0;
volatile boolean A1_set = false;			//------------------------------------//
volatile boolean B1_set = false;			//------------------------------------//
volatile boolean A2_set = false;			//------------------------------------//
volatile boolean B2_set = false;			//------------------------------------//
double spdUp = 0;                           //------------------------------------//
double spdDown = 0;                         //++++++++++++++++++++++++++++++++++++//

double errSpdUp, reqSpd, errSpdSumUp, errSpdPreUp;
double errPos, errPosSum, errPosPre;
double errSpdDown, reqSpdDown, errSpdSumDown, errSpdPreDown;




TinyGPS gps;                                                  //++++++++++++++++++++++++++++++++++++// 
float flat, flon;                                             //------------------------------------//                                                                                         
unsigned long date, time;                                     //------------------------------------//
int year;                                                     //----------------GPS-----------------//
byte month, day, hour, minute, second, hundredths;            //++++++++++++++++++++++++++++++++++++//

int proxiUp, proxiDown, proxiLeft, proxiRight;                     	//----------------Proximity-----------------//
double tmpProc, tmpPow;                     						//---------------Temperature-----------------//
double surfaceDustSensor, airDustSensor;                //---------------Dust-----------------//

void gpsOut(TinyGPS &gps);
double tempOut(double temp);
void printAll();

void setup()  
{
  pinMode(encoderUpPinA, INPUT); 
  pinMode(encoderUpPinB, INPUT); 
  digitalWrite(encoderUpPinA, HIGH);  // turn on pull-up resistor
  digitalWrite(encoderUpPinB, HIGH);  // turn on pull-up resistor

  pinMode(encoderDownPinA, INPUT); 
  pinMode(encoderDownPinB, INPUT); 
  digitalWrite(encoderDownPinA, HIGH);  // turn on pull-up resistor
  digitalWrite(encoderDownPinB, HIGH);  // turn on pull-up resistor

  pinMode(emgStopPin, INPUT);
  pinMode(airDustSensorPulsePin, OUTPUT); 

  enableInterrupt(encoderUpPinA, doEncoderUpA, CHANGE);                //++++++++++++++++++++++++++++++++++++//    
  enableInterrupt(encoderUpPinB, doEncoderUpB, CHANGE); //-------------ENCODER----------------//

  enableInterrupt(encoderDownPinA, doEncoderDownA, CHANGE);              //------------------------------------//
  enableInterrupt(encoderDownPinB, doEncoderDownB, CHANGE); 
  

  digitalWrite(motorUpDirPin, HIGH);
  digitalWrite(motorDownDirPin, LOW);

  
  reqSpd = 60;
  errPosSum = 0;
  errSpdSumUp = 0;
  errSpdSumDown = 0;
  
  Serial.begin(115200);         //--------------Computer-----------------//
  Serial2.begin(9600);        //----------------GPS-----------------//
  Serial3.begin(115200);          //----------------XBee-----------------//
  flat = 0.00f;
  flon = 0.00f;
  proxiUp = 0;
  proxiDown = 0;
  tmpProc = 0;
  tmpPow = 0;
  delay(500);
}

void loop() // run over and over
{
  bool newdata = false;
  unsigned long start = millis();
  proxiUp = digitalRead(proxiUpPin);
  proxiDown = digitalRead(proxiDownPin);
  proxiLeft = digitalRead(proxiLeftPin);
  proxiRight = digitalRead(proxiRightPin);
  
  //surfaceDustSensor = analogRead(surfaceDustSensorPin);
  //delay(5);
  //airDustSensor = getAirDustLevel();
  //delay(5);
  
  while (millis() - start < 40) 									//++++++++++++++++++++++++++++++++++++//	
  {																	//------------------------------------//
    if (Serial2.available()){										//------------------------------------//
      char c = Serial2.read();										//------------------------------------//
      if (gps.encode(c)) 											//------------------------------------//
      {	                                      //-----------------GPS----------------//
        newdata = true;												//------------------------------------//
        break;														//------------------------------------//
      }																//------------------------------------//
    }																//------------------------------------//
  }															//++++++++++++++++++++++++++++++++++++//

///////////// motor PID control  ///////////
   kp = 7.4;
  ki = 1.5;
  kd = 3.2;
  spdUp = (encoderUpPos-lastReportedUpPos) * 1000 * 60/(millis() - lastTimeUp);
  lastReportedUpPos = encoderUpPos;
  lastTimeUp = millis();
  spdDown = (encoderDownPos - lastReportedDownPos) * 1000 * 60/(millis() - lastTimeDown); 
  lastReportedDownPos = encoderDownPos;
  lastTimeDown = millis();

  errPos = encoderUpPos - encoderDownPos;
  
  /*if(abs(errPos) <= 0.0001){
    errPosSum = 0;
  }*/
  int pwmErrPos = errPos*kp + errPosSum*ki + (errPos - errPosPre)*kd;
  Serial.print("PwmPosError:: ");
  Serial.print(pwmErrPos);
  Serial.print("   errorPos::");
  Serial.println(errPos);
  errPosPre = errPos;
  errPosSum += errPos;
  if(errPosSum > 1000){
    //Serial.println("errorPosSum above 1000 !!!!");
    errPosSum = 1000;
  }else if(errPosSum < -1000){
    //Serial.println("errorPosSum below -1000 !!!!");
    errPosSum = -1000;
  }
  
  if(abs(errPos) > 1.0){
    kp = 0;
    ki = 0;
    kd = 0;
  }else{
    kp = 0.05;
    ki = 0.03;
    kd = 0.01;
  }
  errSpdUp = reqSpd - spdUp;
  int pwmPulseUp = errSpdUp*kp + errSpdSumUp*ki + (errSpdUp - errSpdPreUp)*kd - pwmErrPos;
  if(pwmPulseUp > 70){
    pwmPulseUp = 70;
  }
  if(pwmPulseUp < 0){
    pwmPulseUp = 0;
  }
  errSpdPreUp = errSpdUp;
  errSpdSumUp += errSpdUp;
  if(errSpdSumUp > 2000){
    //Serial.println("errorSumUp above 4000 !!!!");
    errSpdSumUp = 2000;
  }else if(errSpdSumUp < -2000){
    //Serial.println("errorSumUp below -4000 !!!!");
    errSpdSumUp = -2000;
  }
  
  errSpdDown = reqSpd - spdDown;
  int pwmPulseDown = errSpdDown*kp + errSpdSumDown*ki + (errSpdDown - errSpdPreDown)*kd + pwmErrPos;
  if(pwmPulseDown > 70){
    pwmPulseDown = 70;
  }
  if(pwmPulseDown < 0){
    pwmPulseDown = 0;
  }
  errSpdPreDown = errSpdDown;
  errSpdSumDown += errSpdDown;
  if(errSpdSumDown > 2000){
    //Serial.println("errorSumDown above 4000 !!!!");
    errSpdSumDown = 2000;
  }else if(errSpdSumDown < -2000){
    //Serial.println("errorSumDown below -4000 !!!!");
    errSpdSumDown = -2000;
  }
  
  if(spdUp-spdDown > 10){
    //Serial.println("STOP!!!!!!");
  }
  
  analogWrite(motorUpPwmPin, pwmPulseUp);
  analogWrite(motorDownPwmPin, pwmPulseDown);
 
  
  //// GPS DATA UPDATE  //////
  if (newdata) 
  {
    gpsOut(gps);			
  }

  tmpOut();
  
  int delayReq = TOTAL_CODE_DELAY_REQ - millis() + start;
  delay((delayReq < 0) ? 0 : delayReq);					//Code delay compensation//
  
  printAll();														//send everything to XBee//
}

//print everything as one string to the XBee RX pin
void printAll(){
  int d = static_cast<int>(day);
  int m = static_cast<int>(month);
  int hh = static_cast<int>(hour);
  int mm = static_cast<int>(minute);
  String date, time;

  if(d < 10){
    date = "0" + String(d);
  }else date = String(d);

  if(m < 10){
    date.concat("/0" + String(m));
  }else date.concat("/" + String(m));

  if(hh < 10){
    time = "0" + String(hh);
  }else time = String(hh);

  if(mm < 10){
    time.concat(".0" + String(mm));
  }else time.concat("." + String(mm));
  
  String finalOutXBee = String(flat,8) + "/" + String(flon, 8) + "/" + tmpProc + "/" + tmpPow + "/" + time + "/" + String(spdUp, 2) + "/" + String(spdDown, 2);
  
  Serial.println("bot=1\tlt=" + String(flat,6) + "\tln=" + String(flon,6) + "\ttime=" + time + "\tpUP=" + proxiUp + "\tpDown=" + proxiDown + "\tpLeft=" + proxiLeft + "\tpRight=" + proxiRight + "\ttProc=" + tmpProc + "\ttPow=" + tmpPow + "\tencU=" + String(spdUp, 2) + "\tencD=" + String(spdDown, 2) + "\tairDust=" + String(airDustSensor, 2)  + "mg/m^3" + "\tsurfaceDust=" + String(surfaceDustSensor, 2));
  Serial3.print(finalOutXBee);
  
 
}  

void tmpOut(){                                    
  tmpProc = analogRead(tmpProcPin);
  tmpProc = tmpProc * 5.0/10.24;
  delay(15);                                         //to let the arduino ADC cooldown
  tmpPow = analogRead(tmpPowPin);
  tmpPow = tmpPow * 5.0/10.24;
  delay(5);                                         //to let the arduino ADC cooldown
}  

double getAirDustLevel(){
  digitalWrite(airDustSensorPulsePin, LOW); // power on the LED
  delayMicroseconds(280);
  double voMeasured = analogRead(airDustSensorPin); // read the dust value
  delayMicroseconds(40);
  digitalWrite(airDustSensorPulsePin, HIGH); // turn the LED off
  delayMicroseconds(9680);
  double calcVoltage = voMeasured * (5.0 / 1024);
  double dustDensity = 0.172 * calcVoltage - 0.0999;
  return dustDensity;
}

void gpsOut(TinyGPS &gps)
{
  unsigned long age;

  gps.f_get_position(&flat, &flon, &age);
  
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  hour += 5;
  minute += 30;
  if(minute >= 60){
    hour += 1;
    minute -= 60;
  }
  if(hour >= 24){
    day += 1;
    hour -= 24;
  }
}

// Interrupt on A changing state
void doEncoderUpA(){
  A1_set = digitalRead(encoderUpPinA) == HIGH;
  // and adjust counter + if A leads B
  encoderUpPos += (A1_set != B1_set) ? (double) +1/2400 : (double) -1/2400;
}

// Interrupt on B changing state
void doEncoderUpB(){
  B1_set = digitalRead(encoderUpPinB) == HIGH;
  // and adjust counter + if B follows A
  encoderUpPos += (A1_set == B1_set) ? (double) +1/2400 : (double) -1/2400;
}

void doEncoderDownA(){
  A2_set = digitalRead(encoderDownPinA) == HIGH;
  // and adjust counter + if A leads B
  encoderDownPos += (A2_set != B2_set) ? (double) +1/2400 : (double) -1/2400;
}

// Interrupt on B changing state
void doEncoderDownB(){
  B2_set = digitalRead(encoderDownPinB) == HIGH;
  // and adjust counter + if B follows A
  encoderDownPos += (A2_set == B2_set) ? (double) +1/2400 : (double) -1/2400;
}
