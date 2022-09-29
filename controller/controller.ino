/*
  Most code in this is modified from Arduino and MPU6050 Accelerometer 
  and Gyroscope Sensor Tutorial by Dejan, https://howtomechatronics.com
*/
#include <Wire.h>

const int MPU = 0x68; // MPU6050 I2C address

float AccX, AccY, AccZ;

void setup() {


  Serial.begin(19200);
  
  //reset and wakeup device
  Wire.begin();                      // Initialize comunication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x80);                  // Set reset bit in register 6B to 1
  Wire.endTransmission(true);        //end the transmission

}

void loop() {

  //Accelerometer data stored in 6 contiguous registers in 3 sets of 2 starting with 3B
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); //(ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  
  AccX = (Wire.read() << 8 | Wire.read()); // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()); // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()); // Z-axis value

  Serial.print("AccX: ");
  Serial.print(AccX);
  Serial.print(", AccY: ");
  Serial.print(AccY);
  Serial.print(", AccZ: ");
  Serial.print(AccZ);
  Serial.print("\n");
  }