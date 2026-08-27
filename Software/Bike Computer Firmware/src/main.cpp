#include <Arduino.h>
#include "ICM45686.h"
#include <SPI.h>
#include <SD.h>


#define MOSI    13
#define MISO    25
#define SCK     14

#define IMU_CS  16
#define SD_CS   27

#define ACCEL_FSR_G    16
#define GYRO_FSR_DPS   2000
#define MAX_LSB        32768


ICM456xx IMU(SPI, IMU_CS);

void create_dir(SDFileSystemClass &sd, String path);

File imufile;

unsigned long lastFlush;

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

    //folder creation
    create_dir(SD, "/data");
    create_dir(SD, "/data/imu");
    create_dir(SD, "/data/gpu");
    create_dir(SD, "/data/compass");

    Serial.println("Folder creation complete");

    //file creation
    if (SD.exists("/data/imu/datalog.csv")) {
        SD.remove("/data/imu/datalog.csv");
    }
    imufile = SD.open("/data/imu/datalog.csv", FILE_WRITE);
    imufile.println("Elasped, AccelX, AccelY, AccelZ, GyroX, GyroY, GyroZ, Temperature");

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

    Serial.println("Written");

    //flush check
    if (millis() - lastFlush > 1000) {
        imufile.flush();
        lastFlush = millis();
    }

    // Run @ ODR 100Hz
    delay(10);
}

void create_dir(SDFileSystemClass &sd, String path) {
    if (!sd.exists(path)) {
        sd.mkdir(path);
    }
}