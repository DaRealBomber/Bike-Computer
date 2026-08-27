#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft;


int baby_counter;

void setup() {
	Serial.begin(115200);

	tft.init();
	tft.setRotation(1);
	baby_counter = 0;
}



void loop() {
	tft.fillScreen(TFT_BLACK);
	baby_counter += 1;

	tft.setTextSize(2);
	tft.setTextColor(TFT_WHITE);
	tft.setCursor(20, 30);
	tft.println("Chuan Zen Baby Counter: ");
	tft.setCursor(140, 50);
	tft.print(baby_counter);
	
	tft.setCursor(20, 100);
	tft.print("mom finder: error");


	delay(500);


}


