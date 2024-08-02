#include <Arduino.h>
#include <ESP32Servo.h>
#include <Preferences.h>

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;

Preferences preferences;

int minUs = 500;
int maxUs = 2500;
int servo1Pin = 35;  // leftfront
int servo2Pin = 36;  // leftback
int servo3Pin = 37;  // rightfront
int servo4Pin = 38;  // rightback
int servo5Pin = 39;  // head
float ang[] = { 0, 0, 0, 0, 0, 0 };
float osang[] = { 0, 0, 0, 0, 0, 0 };  //offset angle
float AngS[] = { 0, 0, 0, 0, 0, 0 };   // the current angle pos
int timewalk1 = 100;
int timewalk2 = 50;
int timeLT = 300;  // time for left turn
int timeST = 100;  // time for right turn



float input_ang(int x, float y) {
  float Aang, Bang;  //0-180
  switch (x) {
    case 1:
      Aang = 220 - y - osang[1];
      break;
    case 2:
      Aang = 40 - y - osang[2];
      break;
    case 3:
      Aang = 40 + y + osang[3];
      break;
    case 4:
      Aang = 220 + y + osang[4];
      break;
    case 5:
      Aang = 180 - y;
  }
  Bang = 180 * (Aang) / 260;
  return Bang;
}

void servo(float AngE1, float AngE2, float AngE3, float AngE4, int timewalk) {
  float AngTemp[] = { 0, AngE1 - AngS[1], AngE2 - AngS[2], AngE3 - AngS[3], AngE4 - AngS[4] };
  for (int i = 0; i < timewalk; i++) {
    servo1.write(input_ang(1, AngS[1] + AngTemp[1] * i / timewalk));
    servo2.write(input_ang(2, AngS[2] + AngTemp[2] * i / timewalk));
    servo3.write(input_ang(3, AngS[3] + AngTemp[3] * i / timewalk));
    servo4.write(input_ang(4, AngS[4] + AngTemp[4] * i / timewalk));

    delay(10);
  }
  AngS[1] = AngE1;
  AngS[2] = AngE2;
  AngS[3] = AngE3;
  AngS[4] = AngE4;
}

// void Servo_Offset();
void MoveInit() {
  Serial.println("MoveInit start");
  preferences.begin("memory", false);

  servo1.attach(servo1Pin, minUs, maxUs);
  servo2.attach(servo2Pin, minUs, maxUs);
  servo3.attach(servo3Pin, minUs, maxUs);
  servo4.attach(servo4Pin, minUs, maxUs);
  servo5.attach(servo5Pin, minUs, maxUs);

  Serial.println("MoveInit end");
}

void MoveReset() {
  Serial.println("MoveReset start");
  osang[1] = preferences.getInt("LFAngle", osang[1]);
  osang[2] = preferences.getInt("LBAngle", osang[2]);
  osang[3] = preferences.getInt("RFAngle", osang[3]);
  osang[4] = preferences.getInt("RBAngle", osang[4]);
  for (int i = 0; i < 4; i++) {
    AngS[i] = osang[i];
    Serial.print("MoveReset osang[i]");
    Serial.println(osang[i]);
  }

  Serial.println("MoveReset servo write");
  servo1.write(180 * (220 - osang[1]) / 260);
  servo2.write(180 * (40 - osang[2]) / 260);
  servo3.write(180 * (40 + osang[3]) / 260);
  servo4.write(180 * (220 + osang[4]) / 260);

  Serial.println("MoveReset servo5");
  servo5.write(input_ang(5, 0));
  Serial.println("MoveReset end");
}

void servoLeftFront(float ange, int timewalk) {
  float angtmp = ange - AngS[1];
  for (int i = 0; i < timewalk; i++) {
    servo1.write(input_ang(1, AngS[1] + angtmp * i / timewalk));
    delay(10);
  }
  AngS[1] = ange;
}

void servoLeftBack(float ange, int timewalk) {
  float angtmp = ange - AngS[1];
  for (int i = 0; i < timewalk; i++) {
    servo2.write(input_ang(2, AngS[2] + angtmp * i / timewalk));
    delay(10);
  }
  AngS[2] = ange;
}

void servoRightFront(float ange, int timewalk) {
  float angtmp = ange - AngS[1];
  for (int i = 0; i < timewalk; i++) {
    servo3.write(input_ang(3, AngS[3] + angtmp * i / timewalk));
    delay(1);
  }
  AngS[3] = ange;
}

void servoRightBack(float ange, int timewalk) {
  float angtmp = ange - AngS[1];
  for (int i = 0; i < timewalk; i++) {
    servo4.write(input_ang(4, AngS[4] + angtmp * i / timewalk));
    delay(1);
  }
  AngS[4] = ange;
}

void Servo_Init() {
  Serial.println("Servo_Init start");
  preferences.begin("memory", false);

  servo1.attach(servo1Pin, minUs, maxUs);
  servo2.attach(servo2Pin, minUs, maxUs);
  servo3.attach(servo3Pin, minUs, maxUs);
  servo4.attach(servo4Pin, minUs, maxUs);
  servo5.attach(servo5Pin, minUs, maxUs);

  Serial.println("Servo_Init osang");
  osang[1] = preferences.getInt("LFAngle", osang[1]);
  osang[2] = preferences.getInt("LBAngle", osang[2]);
  osang[3] = preferences.getInt("RFAngle", osang[3]);
  osang[4] = preferences.getInt("RBAngle", osang[4]);
  for (int i = 0; i < 4; i++) {
    AngS[i] = osang[i];
    Serial.print("osang[i]");
    Serial.println(osang[i]);
  }

  Serial.println("Servo_Init servo write");
  servo1.write(180 * (220 - osang[1]) / 260);
  servo2.write(180 * (40 - osang[2]) / 260);
  servo3.write(180 * (40 + osang[3]) / 260);
  servo4.write(180 * (220 + osang[4]) / 260);

  Serial.println("Servo_Init servo5");
  servo5.write(input_ang(5, 0));
  Serial.println("Servo_Init end");
  // delay(5000);
}

void MoveForward() {
  for (int i = 0; i < 10; i++) {
    Serial.print("MoveForward loop count:");
    Serial.println(i);
    servo(15, 0, 0, 0, timewalk1);
    servo(15, 0, 0, 15, timewalk1);
    servo(15, 0, -15, 15, timewalk2);
    servo(15, -15, -15, 15, timewalk2);
    servo(0, -15, -15, 15, timewalk2);
    servo(0, -15, -15, 0, timewalk2);
    servo(0, 0, -15, 0, timewalk1);
    servo(0, 0, 0, 0, timewalk1);
  }
}


void Servo_forward() {
  int walkcount = 1;
  int round = 0;
  while (round < 50) {
    Serial.print("round:");
    Serial.println(round);
    Serial.print("walkcount:");
    Serial.println(walkcount);
    delay(3000);
    switch (walkcount) {
      case 1:
        servo(15, 0, 0, 0, timewalk1);
        servo(15, 0, 0, 15, timewalk1);
        walkcount++;
        break;

      case 2:
        servo(15, 0, -15, 15, timewalk2);
        servo(15, -15, -15, 15, timewalk2);
        walkcount++;
        break;

      case 3:
        servo(0, -15, -15, 15, timewalk2);
        servo(0, -15, -15, 0, timewalk2);
        walkcount++;
        break;

      case 4:
        servo(0, 0, -15, 0, timewalk1);
        servo(0, 0, 0, 0, timewalk1);
        walkcount++;
        break;

        // case 5:
        //   servo(-15, 15, 15, 0, timewalk2);
        //   servo(-15, 15, 15, -15, timewalk2);
        //   walkcount++;
        //   break;

        // case 6:
        //   servo(-15, 15, 0, -15, timewalk2);
        //   servo(-15, 0, 0, -15, timewalk2);
        //   walkcount = 1;
        //   break;
    }
    round++;
  }
  Servo_Init();
}