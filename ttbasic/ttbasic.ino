/*
  TOYOSHIKI Tiny BASIC for Arduino
 (C)2012 Tetsuya Suzuki
 */

//
// Arduino Uno互換機+「アクティブマトリクス蛍光表示管（CL-VFD）MW25616L 実験用表示モジュール」対応
// 2018/03/01 by たま吉さん 
//

void basic(void);

void setup(void){
  // put your setup code here, to run once:
  Serial.begin(115200);
  randomSeed(analogRead(0));
}

void loop(void){
  // put your main code here, to run repeatedly:
  basic();
}
