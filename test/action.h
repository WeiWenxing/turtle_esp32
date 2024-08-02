#ifndef ACTION_H
#define ACTION_H
#include <Arduino.h>


void MoveInit();
void MoveReset();
void MoveForward();
void servoLeftFront(float ange, int timewalk);
void servoLeftBack(float ange, int timewalk);
void servoRightFront(float ange, int timewalk);
void servoRightBack(float ange, int timewalk);

#endif  // TTS_H