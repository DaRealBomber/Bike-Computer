#include <Arduino.h>
#include "ICM45686.h"
#include <SPI.h>
#include <SD.h>
#include <SparkFun_u-blox_GNSS_v3.h>
#include <SparkFun_MMC5983MA_Arduino_Library.h>
#include <TFT_eSPI.h>
#include <Wire.h>

#define I2C_SDA 32
#define I2C_SCL 33

#define MOSI    13
#define MISO    25
#define SCK     14

#define IMU_CS  16
#define SD_CS   27

#define ACCEL_FSR_G    16
#define GYRO_FSR_DPS   2000
#define MAX_LSB        32768

#define GNSS_RX 18
#define GNSS_TX 17

HardwareSerial GNSSSerial(1);
SFE_UBLOX_GNSS_SERIAL myGNSS;

ICM456xx IMU(SPI, IMU_CS);
void create_dir(SDFileSystemClass &sd, String path);
File imufile;
File gpsfile;
unsigned long lastFlush;
SFE_MMC5983MA myMag;
TFT_eSPI tft;

void setup() {
    int ret;
    Serial.begin(115200);

    SPI.begin(SCK, MISO, MOSI);

    // pinMode(SD_CS, OUTPUT);
    pinMode(IMU_CS, OUTPUT);

    // digitalWrite(SD_CS, HIGH);
    digitalWrite(IMU_CS, HIGH);

    
    // SD Card setup
    if (!SD.begin(SD_CS, SPI)) {
        Serial.println("SD Intialization failed");
        while(1);
    }
    
    Serial.println("SD setup successfully");

    // IMU Setup
    
    //while(!Serial) {}

    ret = IMU.begin();
    if (ret != 0) {
        Serial.print("IMU Intialization failed");
        Serial.println(ret);
        while(1);
    }

    IMU.startAccel(100, ACCEL_FSR_G);
    IMU.startGyro(100,GYRO_FSR_DPS);
    delay(100);
    Serial.println("IMU setup successfully");

    //gps startup
    GNSSSerial.begin(
        38400,
        SERIAL_8N1,
        GNSS_RX,
        GNSS_TX
    );

    while (!myGNSS.begin(GNSSSerial)) {
        Serial.println("GNSS not detected");
        delay(1000);
    }

    myGNSS.setUART1Output(COM_TYPE_UBX);
    myGNSS.setAutoPVT(true);
    Serial.println("GNSS setup complete");

    // Serial.print("Module: ");
    // Serial.println(myGNSS.getModuleName());

    // Serial.print("Firmware: ");
    // Serial.println(myGNSS.getFirmwareType());

    // Serial.print("Protocol version: ");
    // Serial.print(myGNSS.getProtocolVersionHigh());
    // Serial.print(".");
    // Serial.println(myGNSS.getProtocolVersionLow());

    // Serial.print("Unique ID: ");
    // Serial.println(myGNSS.getUniqueChipIdStr());

    //Display setup
	tft.init();
	tft.setRotation(2);
	tft.fillScreen(TFT_BLACK);
	tft.setTextSize(2);
	tft.setTextColor(TFT_WHITE);
	tft.setCursor(20, 30);
	tft.println("Hello World");

    //folder creation
    create_dir(SD, "/data");
    create_dir(SD, "/data/imu");
    create_dir(SD, "/data/gps");
    create_dir(SD, "/data/compass");

    Serial.println("Folder creation complete");

    //file creation
    if (SD.exists("/data/imu/datalog.csv")) {
        SD.remove("/data/imu/datalog.csv");
    }
    imufile = SD.open("/data/imu/datalog.csv", FILE_WRITE);
    imufile.println("Elasped, AccelX, AccelY, AccelZ, GyroX, GyroY, GyroZ, Temperature");

    if (SD.exists("/data/gps/test1.csv")) {
        SD.remove("/data/gps/test1.csv");
    }

    gpsfile = SD.open("/data/gps/test1.csv", FILE_WRITE);
    gpsfile.println("Elapsed, TYPE, SATS, Lattitude, Longitude, Altitude");

    // compass intializiation
    // Wire.begin(I2C_SDA, I2C_SCL);

    // if (myMag.begin() == false)
    // {
    //     Serial.println("MMC5983MA did not respond - check your wiring. Freezing.");
    //     while (true)
    //         ;
    // }

    // myMag.softReset();

    // Serial.println("MMC5983MA connected");
}




void loop() {

    inv_imu_sensor_data_t imu_data;
    float accel_g[3] = { 0 };
    float gyro_dps[3] = { 0 };
    float temp_degc;

    // Read registers
    IMU.getDataFromRegisters(imu_data);

    // Format data for Serial Plotter
    accel_g[0]  = (float)(imu_data.accel_data[0] * ACCEL_FSR_G) / MAX_LSB;
    accel_g[1]  = (float)(imu_data.accel_data[1] * ACCEL_FSR_G) / MAX_LSB;
    accel_g[2]  = (float)(imu_data.accel_data[2] * ACCEL_FSR_G) / MAX_LSB;

    gyro_dps[0]  = (float)(imu_data.gyro_data[0] * GYRO_FSR_DPS) / MAX_LSB;
    gyro_dps[1]  = (float)(imu_data.gyro_data[1] * GYRO_FSR_DPS) / MAX_LSB;
    gyro_dps[2]  = (float)(imu_data.gyro_data[2] * GYRO_FSR_DPS) / MAX_LSB;
    
    temp_degc = 25 + ((float)imu_data.temp_data/128);
    // print to csv
    imufile.print(millis());
    imufile.print(",");
    imufile.print(accel_g[0]);
    imufile.print(",");
    imufile.print(accel_g[1]);
    imufile.print(",");
    imufile.print(accel_g[2]);
    imufile.print(",");
    imufile.print(gyro_dps[0]);
    imufile.print(",");
    imufile.print(gyro_dps[1]);
    imufile.print(",");
    imufile.print(gyro_dps[2]);
    imufile.print(",");
    imufile.println(temp_degc);

    if (myGNSS.getPVT() == true){
        
        gpsfile.print(millis());
        gpsfile.print(",");
        gpsfile.print(myGNSS.getFixType());
        gpsfile.print(",");
        gpsfile.print(myGNSS.getSIV());
        gpsfile.print(",");
        gpsfile.print(myGNSS.getLatitude());
        gpsfile.print(",");
        gpsfile.print(myGNSS.getLongitude());
        gpsfile.print(",");
        gpsfile.println(myGNSS.getAltitudeMSL());
    }


    //flush check
    if (millis() - lastFlush > 1000) {
        imufile.flush();
        gpsfile.flush();
        lastFlush = millis();
    }

    // // compass
    // uint32_t currentX = 0;
    // uint32_t currentY = 0;
    // uint32_t currentZ = 0;
    // double scaledX = 0;
    // double scaledY = 0;
    // double scaledZ = 0;

    // // This reads the X, Y and Z channels consecutively
    // // (Useful if you have one or more channels disabled)
    // currentX = myMag.getMeasurementX();
    // currentY = myMag.getMeasurementY();
    // currentZ = myMag.getMeasurementZ();

    // // Or, we could read all three simultaneously
    // //myMag.getMeasurementXYZ(&currentX, &currentY, &currentZ);

    // Serial.print("X axis raw value: ");
    // Serial.print(currentX);
    // Serial.print("\tY axis raw value: ");
    // Serial.print(currentY);
    // Serial.print("\tZ axis raw value: ");
    // Serial.println(currentZ);

    // // The magnetic field values are 18-bit unsigned. The _approximate_ zero (mid) point is 2^17 (131072).
    // // Here we scale each field to +/- 1.0 to make it easier to convert to Gauss.
    // //
    // // Please note: to properly correct and calibrate the X, Y and Z channels, you need to determine true
    // // offsets (zero points) and scale factors (gains) for all three channels. Futher details can be found at:
    // // https://thecavepearlproject.org/2015/05/22/calibrating-any-compass-or-accelerometer-for-arduino/
    // scaledX = (double)currentX - 131072.0;
    // scaledX /= 131072.0;
    // scaledY = (double)currentY - 131072.0;
    // scaledY /= 131072.0;
    // scaledZ = (double)currentZ - 131072.0;
    // scaledZ /= 131072.0;

    // // The magnetometer full scale is +/- 8 Gauss
    // // Multiply the scaled values by 8 to convert to Gauss
    // Serial.print("X axis field (Gauss): ");
    // Serial.print(scaledX * 8, 5); // Print with 5 decimal places

    // Serial.print("\tY axis field (Gauss): ");
    // Serial.print(scaledY * 8, 5);

    // Serial.print("\tZ axis field (Gauss): ");
    // Serial.println(scaledZ * 8, 5);

    // Serial.println();
    // delay(100);
}

void create_dir(SDFileSystemClass &sd, String path) {
    if (!sd.exists(path)) {
        sd.mkdir(path);
    }
}