#define motorWenchDirPin 5
#define motorWenchPwmPin 4
#define proxiUpPin 42      
#define proxiDownPin 40  

#define pwmWench 140

int dirUp = HIGH;
int dirDown = LOW; 
int proxiUp, proxiDown;
int countUp, countDown;
bool stoppedUp, stoppedDown;
int directionWench;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(motorWenchDirPin, OUTPUT);
  pinMode(proxiUpPin, INPUT);
  pinMode(proxiDownPin, INPUT);
  proxiUp = 0;
  stoppedUp = false;
  countUp = 0;
  proxiDown = 0;
  stoppedDown = false;
  countDown = 0;
  directionWench = HIGH;
  digitalWrite(motorWenchDirPin, HIGH);
  analogWrite(motorWenchPwmPin, pwmWench);
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
  updateProxiListener();
  if(stoppedUp){
    stoppedUp = false;
    delay(1000);
    Serial.println("Reversing Direction to LOW");
    setWenchSpeed(pwmWench, LOW);
  }else if (stoppedDown){
    stoppedDown = false;
    delay(1000);
    Serial.println("Reversing Direction to HIGH");
    setWenchSpeed(pwmWench + 85, HIGH);
  }
  delay(50);
}

void updateProxiListener(){
  int up = digitalRead(proxiUpPin);
  int down = digitalRead(proxiDownPin);
  if(up != proxiUp && directionWench == HIGH){
    proxiUp = up;
    Serial.println(up);
    if(proxiUp == 1) {
      countUp++;
      if(countUp == 1){
        setWenchSpeed(70, HIGH);
      }else if(countUp == 2){
        setWenchSpeed(0, HIGH);
        countUp = 0;
        stoppedUp = true;
      }
    }
  }
  if(down != proxiDown && directionWench == LOW){
    proxiDown = down;
    Serial.println(down + "  DOWN");
    if(proxiDown == 1) {
      countDown++;
      if(countDown == 1){
        setWenchSpeed(70, LOW);
      }else if(countDown == 2){
        setWenchSpeed(0, LOW);
        countDown = 0;
        stoppedDown = true;
      }
    }
  }
}

void setWenchSpeed(int pwm, int dir){
  directionWench = dir;
  digitalWrite(motorWenchDirPin, dir);
  analogWrite(motorWenchPwmPin, pwm);
}

