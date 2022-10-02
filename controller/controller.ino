/*
  Most code in this is modified from Arduino and MPU6050 Accelerometer 
  and Gyroscope Sensor Tutorial by Dejan, https://howtomechatronics.com
*/
#include <Wire.h>
#include <Servo.h>

//Globals for user input
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;
int dataNumber = 0;
int angle = 0;

//Globals for servo
Servo servo1;
int pos = 0;
int servoOffset = 90;

int errorRoll = 6.3;

//Globals for MPU
const int MPU = 0x68;  // MPU6050 I2C address
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch;
float elapsedTime, currentTime, previousTime;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
int c;

void setup() {
  Serial.begin(19200);

  //Reset and wake up MPU
  Wire.begin();                 
  Wire.beginTransmission(MPU);  // MPU=0x68
  Wire.write(0x6B);             
  Wire.write(0x00);             // Set reset bit in register 6B to 0
  Wire.endTransmission(true);   

  //Connect servo
  servo1.attach(4);

  //calculate_IMU_error();
}

void loop() {
  //Use MPU to determine current roll of system
  getRollFromMPU();

  //Accept user input for default angle between -90 and +90
  recvWithEndMarker();
  angle = getUserAngle();

  //Tell servo to point up (6.5 accounts for MPU mounted at angle)
  servo1.write(angle + servoOffset - roll);         
  delay(10);

  //Monitoring
  outputData();
}

void getRollFromMPU() {
  //Accelerometer data stored in 6 contiguous registers in 3 sets of 2 starting with 3B
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  //(ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);  // Read 6 registers total, each axis value is stored in 2 registers

  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;  // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;  // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value

  accAngleY = (-180 * atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2)))/ PI) - 5.6;
  accAngleX = (180 * atan(AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2)))/ PI) + 1.22;

  //Determine pitch and roll
  // === Read gyroscope data === //
  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; // Divide by 1000 to get seconds

  Wire.beginTransmission(MPU);
  Wire.write(0x43); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 4 registers total, each axis value is stored in 2 registers
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;

  // Correct the outputs with the calculated error values
  GyroX = GyroX + 2.6; 
  GyroY = GyroY - 1.9; 
  GyroZ = GyroZ - 1.15;
  
  // Currently the raw values are in degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
  gyroAngleX = gyroAngleX + GyroX * elapsedTime; // deg/s * s = deg
  gyroAngleY = gyroAngleY + GyroY * elapsedTime;

  // Complementary filter - combine acceleromter and gyro angle values
  gyroAngleX = 0.96 * gyroAngleX + 0.04 * accAngleX;
  
  Serial.print(" GyroY: ");
  Serial.print(gyroAngleY);
  Serial.print("   ");

  Serial.print(" GyroAngleY: ");
  Serial.print(gyroAngleY);
  Serial.print("   ");

  Serial.print(" accAngleY: ");
  Serial.print(accAngleY);
  Serial.print("   ");

  gyroAngleY = 0.96 * gyroAngleY + 0.04 * accAngleY;
  
  pitch = gyroAngleX;
  roll = gyroAngleY;
}
//https://forum.arduino.cc/t/serial-input-basics-updated/382007/3
void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
    if (Serial.available() > 0) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

//https://forum.arduino.cc/t/serial-input-basics-updated/382007/3
//If user enters bad input, return current angle
int getUserAngle() {
    if (newData == true) {
        dataNumber = 0;             // new for this version
        dataNumber = atoi(receivedChars);   // new for this version
        newData = false;
        if(dataNumber >= -90 && dataNumber <= 90) {
          return dataNumber;
        }
    }
    return angle;
}

void calculate_IMU_error() {
  // We can call this funtion in the setup section to calculate the accelerometer and gyro data error. From here we will get the error values used in the above equations printed on the Serial Monitor.
  // Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
  // Read accelerometer values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    // Sum all readings
    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
    c++;
  }
  //Divide the sum by 200 to get the error value
  AccErrorX = AccErrorX / 200;
  AccErrorY = AccErrorY / 200;
  c = 0;
  // Read gyro values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();
    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    c++;
  }
  //Divide the sum by 200 to get the error value
  GyroErrorX = GyroErrorX / 200;
  GyroErrorY = GyroErrorY / 200;
  GyroErrorZ = GyroErrorZ / 200;
  // Print the error values on the Serial Monitor
  Serial.print("AccErrorX: ");
  Serial.println(AccErrorX);
  Serial.print("AccErrorY: ");
  Serial.println(AccErrorY);
  Serial.print("GyroErrorX: ");
  Serial.println(GyroErrorX);
  Serial.print("GyroErrorY: ");
  Serial.println(GyroErrorY);
  Serial.print("GyroErrorZ: ");
  Serial.println(GyroErrorZ);
}

void outputData() {
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