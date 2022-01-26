String inputString = "";
bool stringComplete = false;

const int pinBuzz = 3;
const int pinTrig = 4;
const int pinEcho = 5;

bool security = false;

void setup() {
  pinMode(pinTrig, OUTPUT      );
  pinMode(pinEcho, INPUT       );
  
  Serial.begin(9600);
  Serial.println("mesure distance with HC-SR04");

  inputString.reserve(10);
}

void loop() {
  int dist = get_dist();
    
  if(dist != 0) {
    Serial.print("dist");
    Serial.println(dist/10);

  }
  if(dist < 750){
    if(security){
      //Serial.println("warning");  
      tone(pinBuzz, 500);
      delay(50);
      noTone(pinBuzz);
      delay(5);
    }
    else;
    //delay(500);
  }
  else;

  if (stringComplete) {
    if(inputString.substring(0,3)=="sec"){
      if     (inputString.substring(3,4)=="1")
        security = true;
      else if(inputString.substring(3,4)=="0")
        security = false;
      else;
    }
    inputString = "";
    stringComplete = false;
  }
  
  delay(500);
  
}

void trig(void) {
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW );
}

unsigned long get_echoTime(void) {
  unsigned long echoTime = 0;
  while(digitalRead(pinEcho) == LOW );
  unsigned long startTime = micros();
  while(digitalRead(pinEcho) == HIGH);
  echoTime = micros() - startTime;
  /*
  17 * 240 / 100 = 40.8(mm), 17 * 23000 / 100 = 3910(mm)
  */
  if(echoTime >= 240 && echoTime <= 23000)
    return echoTime;
  else
    return 0;
}

int get_dist(void) {
  trig(); /* transmit trigger pulese */
  unsigned long echo_time = get_echoTime();
  if(echo_time != 0){
    /*
    340(m) / 1(sec) = 2 * distance(mm) / echo_time(us)
    340,000(mm) / 1,000,000(us) = 2 * distance(mm) / echo_time(us)
    2,000,000 * distance = 340,000 * echo_time
    distance = 340,000 * echo_time / 2,000,000
    = 17 * echo_time / 100
    */
    unsigned long distance = 17 * echo_time / 100;
    return distance;
  }
  else 
    return 0;
}


void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
