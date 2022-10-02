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

void setup() {
  Serial.begin(19200);

  //Reset and wake up MPU
  Wire.begin();                 
  Wire.beginTransmission(MPU);  // MPU=0x68
  Wire.write(0x6B);             
  Wire.write(0x80);             // Set reset bit in register 6B to 1
  Wire.endTransmission(true);   

  //Connect servo
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
  //Determine pitch and roll
  float pitch = (-180 * atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2)))/ PI);
  float roll = (180 * atan(AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2)))/ PI) + errorRoll;

  //Accept user input for default angle between -90 and +90
  recvWithEndMarker();
  angle = getUserAngle();

  // tell servo to point up (6.5 accounts for MPU mounted at angle)
  servo1.write(angle + servoOffset - roll);         
  delay(10);

  //Monitoring
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