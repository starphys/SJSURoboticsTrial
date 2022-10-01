/*
  Most code in this is modified from Arduino and MPU6050 Accelerometer 
  and Gyroscope Sensor Tutorial by Dejan, https://howtomechatronics.com
*/
#include <Wire.h>
#include <Servo.h>

Servo servo1;
int pos = 0;

const int MPU = 0x68;  // MPU6050 I2C address
float AccX, AccY, AccZ;

void setup() {
  Serial.begin(19200);

  //reset and wakeup device
  Wire.begin();                 // Initialize comunication
  Wire.beginTransmission(MPU);  // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);             // Talk to the register 6B
  Wire.write(0x00);             // Set reset bit in register 6B to 1
  Wire.endTransmission(true);   //end the transmission

  servo1.attach(4);
}

void loop() {

  //Accelerometer data stored in 6 contiguous registers in 3 sets of 2 starting with 3B
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  //(ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);  // Read 6 registers total, each axis value is stored in 2 registers

  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;  // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;  // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value

  //http://robo.sntiitk.in/2017/12/21/Beginners-Guide-to-IMU.html
  float pitch = (-180 * atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2)))/ PI);
  float roll = (180 * atan(AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2)))/ PI);

  // tell servo to point up (6.5 accounts for MPU mounted at angle)
  servo1.write(90 + 6.5 - roll);         
  delay(10);


  Serial.print("AccX: ");
  Serial.print(AccX);
  Serial.print(", AccY: ");
  Serial.print(AccY);
  Serial.print(", AccZ: ");
  Serial.print(AccZ);
  Serial.print(", roll: ");
  Serial.print(roll);
  Serial.print(", pitch: ");
  Serial.print(pitch);
  Serial.print("\n");
}