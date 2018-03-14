#pragma once
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "Settings.h"

extern const boolean DEBUGFRAME;

extern int bpm;
extern Sequence seq;
extern int seqStep;
extern int editStep;
extern int editMode;
extern float randAmt;
extern int sequenceA;
extern int numSeqA; 
extern int modeSeqA;
extern long lastEncoder;


class DisplayHandler {
public:
	DisplayHandler() {
		Adafruit_SSD1306 display(OLED_RESET);
	};
	int displayRefresh;
	void setDisplayRefresh(int requestedRefresh);
	void updateDisplay();
	void init();
	Adafruit_SSD1306 display;
};


//	request a display refresh - set the area of the screen that needs refreshing next time a refresh is available
void DisplayHandler::setDisplayRefresh(int requestedRefresh) {

	//	if previously requested update is top and now requesting bottom refresh all (and viece versa) - otherwise set refresh type to passed value
	if ((displayRefresh == REFRESHTOP && requestedRefresh == REFRESHBOTTOM) || (displayRefresh == REFRESHBOTTOM && requestedRefresh == REFRESHTOP)) {
		displayRefresh = REFRESHFULL;
	}
	else {
		displayRefresh = requestedRefresh;
	}
}


// carry out the screen refresh building the various UI elements
void DisplayHandler::updateDisplay() {
	if (DEBUGFRAME) {
		Serial.print("Frame start: "); Serial.print(millis());
	}

	display.clearDisplay();

	//	Write the sequence number for sequence A
	display.setCursor(0, 0); 
	display.print("cv");
	display.setCursor(1, 11);
	display.setTextSize(2);
	display.print(sequenceA);
	display.setTextSize(1);

	//	Draw arrow beneath sequence number if selected for editing
	if (editStep == -1) {
		display.drawLine(4, 34, 6, 32, WHITE);
		display.drawLine(6, 32, 8, 34, WHITE);
	}

	// Draw the sequence steps for pattern A (sample and hold)
	for (int i = 0; i < 8; i++) {
		int voltHPos = 17 + (i * 14);
		int voltVPos = 27 - (seq.Steps[i].volts * 5);

		// Draw voltage line showing randomisation by using a dotted line with 3 or 4 dots if randomisation is greater or less than 5
		if (seq.Steps[i].rand_amt > 0) {
			for (int j = 0; j < 12; j++) {
				if (j % (seq.Steps[i].rand_amt > 5 ? 4 : 3) == 0) {
					display.drawPixel(voltHPos + (seq.Steps[i].rand_amt > 5 ? 2 : 1) + j, voltVPos, WHITE);
					display.drawPixel(voltHPos + (seq.Steps[i].rand_amt > 5 ? 2 : 1) + j, voltVPos + 1, WHITE);
				}
			}
		}
		else {
			display.drawFastHLine(voltHPos, voltVPos, 13, WHITE);
			display.drawFastHLine(voltHPos, voltVPos + 1, 13, WHITE);
		}

		// draw amount of voltage selected after randomisation applied
		if (seqStep == i) {
			// Draw voltage line
			display.fillRect(voltHPos, round(26 - (randAmt * 5)), 13, 4, WHITE);
		}

		//	Draw arrow beneath step selected for editing
		if (editStep == i) {
			display.drawLine(voltHPos + 4, 34, voltHPos + 6, 32, WHITE);
			display.drawLine(voltHPos + 6, 32, voltHPos + 8, 34, WHITE);

			// draw box around current step whilst editing
			if (millis() - lastEncoder < 500) {
				
				//display.drawFastVLine(voltHPos - 1, 0, 30, WHITE);
				//display.drawRect(voltHPos - 1, 0, 15, 30, INVERSE);
			}
		}

	}

	//	if currently or recently editing show values in bottom area of screen
	if (lastEncoder > 0 && millis() - lastEncoder < 5000) {
		const int Panel1X = 4;

		if (editMode == STEPR || editMode == STEPV || editMode == STUTTER) {
			display.setCursor(Panel1X, 43);
			display.println("Volts");
			display.setCursor(Panel1X, 53);
			display.println(seq.Steps[editStep].volts);
			if (editMode == STEPV) {
				display.drawRect(0, 40, 36, 24, INVERSE);
			}

			display.setCursor(42, 43);
			display.println("Random");
			display.setCursor(42, 53);
			display.println(seq.Steps[editStep].rand_amt);
			if (editMode == STEPR) {
				display.drawRect(38, 40, 44, 24, INVERSE);
			}

			display.setCursor(85, 43);
			display.println("Stutter");
			display.setCursor(85, 53);
			display.println(modeSeqA);
			if (editMode == STUTTER) {
				display.drawRect(81, 40, 47, 24, INVERSE);
			}

		}

		if (editMode == PATTERN || editMode == SEQS || editMode == SEQMODE) {
			display.setCursor(4, 43);
			display.println("Pattern");
			display.setCursor(4, 53);
			display.println(sequenceA);
			if (editMode == PATTERN) {
				display.drawRect(0, 40, 49, 24, INVERSE);
			}

			display.setCursor(56, 43);
			display.println("Loop");
			display.setCursor(56, 53);
			display.println(modeSeqA == LOOPCURRENT ? "ONE" : "ALL");
			if (editMode == SEQMODE) {
				display.drawRect(52, 40, 34, 24, INVERSE);
			}

			display.setCursor(94, 43);
			display.println("Loops");
			display.setCursor(94, 53);
			display.println(numSeqA);
			if (editMode == SEQS) {
				display.drawRect(90, 40, 38, 24, INVERSE);
			}
		}
	} else {
		display.setCursor(91, 43);
		display.print("e: "); display.print(editMode);
		display.setCursor(91, 53);
		display.print("b: "); display.println(bpm);
	}
	display.display();
	displayRefresh = REFRESHOFF;

	if (DEBUGFRAME) {
		Serial.print("  Frame end: "); Serial.println(millis());
	}
}

void DisplayHandler::init() {
	const unsigned char diceBitmap[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xCF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x3C, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x7E, 0x03, 0xF0, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x7E, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x1C, 0x7C, 0x07, 0xC0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xC0, 0x00, 0xFE, 0x00, 0xF0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x07, 0x1F, 0x80, 0x7C, 0x7C, 0x18, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xEE, 0x0E, 0x1F, 0x80, 0x00, 0xFC, 0x08, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x03, 0x9C, 0x0F, 0x8F, 0x00, 0xFC, 0x28, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x01, 0xF0, 0xF9, 0xF1, 0x80, 0x1F, 0x80, 0x10, 0xC8, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x0F, 0x81, 0xFC, 0x70, 0xF0, 0x1F, 0x8C, 0x01, 0x88, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x7C, 0x01, 0xFC, 0x38, 0x1E, 0x0F, 0x1F, 0x03, 0x18, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x03, 0xE0, 0x01, 0xF0, 0x1F, 0x83, 0xC0, 0x1F, 0x86, 0x18, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x07, 0xC0, 0x7C, 0x1F, 0x0C, 0xD0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x38, 0x7E, 0x00, 0x03, 0xE3, 0xE0, 0x0F, 0x00, 0x19, 0xD0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x60, 0xFE, 0x00, 0x07, 0xF1, 0xE0, 0x03, 0xF0, 0x39, 0xD0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x68, 0xFE, 0x00, 0x07, 0xE0, 0x70, 0x00, 0x7C, 0x31, 0xB0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x2C, 0x30, 0x00, 0x03, 0xC3, 0x30, 0x00, 0x0E, 0x03, 0xA0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x26, 0x00, 0xF0, 0x00, 0x1E, 0x10, 0x00, 0x00, 0x09, 0x20, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x23, 0x03, 0xF8, 0x00, 0xF0, 0x10, 0x00, 0x00, 0xD8, 0x20, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x31, 0x83, 0xF8, 0x07, 0x80, 0x10, 0x00, 0x00, 0xD8, 0x60, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x30, 0xC3, 0xF0, 0x3C, 0x07, 0x90, 0x00, 0x00, 0xB8, 0x60, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x10, 0x60, 0x01, 0xE0, 0x0F, 0x90, 0x00, 0x00, 0xBB, 0x60, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x10, 0x30, 0x0F, 0x00, 0x0F, 0x90, 0x00, 0x01, 0xB3, 0x40, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x18, 0x18, 0x7C, 0x00, 0x1F, 0x90, 0x00, 0x01, 0x87, 0x40, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x18, 0x08, 0xE0, 0x00, 0x1F, 0x10, 0x00, 0x01, 0x87, 0xC0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x18, 0x00, 0x0C, 0x00, 0x0E, 0x10, 0x00, 0x01, 0x06, 0x80, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x08, 0x02, 0x3C, 0x00, 0x00, 0x10, 0x00, 0x01, 0x01, 0x80, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x08, 0x42, 0x3E, 0x00, 0x00, 0x10, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x0C, 0xE3, 0x7E, 0x00, 0x00, 0x10, 0x00, 0x03, 0x43, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x0C, 0xE3, 0x7E, 0x00, 0x00, 0x10, 0x00, 0x02, 0x66, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x04, 0xF3, 0x7C, 0x00, 0x00, 0x10, 0x00, 0x02, 0xE4, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x04, 0x73, 0x78, 0x07, 0x00, 0x10, 0x00, 0x02, 0xEC, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x06, 0x71, 0x00, 0x0F, 0x80, 0x10, 0x00, 0x02, 0xD8, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x06, 0x71, 0x00, 0x0F, 0x80, 0x18, 0x00, 0x06, 0xD0, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x02, 0x01, 0x80, 0x1F, 0x80, 0x18, 0x03, 0x84, 0xB0, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x03, 0x01, 0x80, 0x1F, 0x01, 0x98, 0x07, 0xC4, 0x20, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x01, 0x81, 0x80, 0x1F, 0x03, 0xD8, 0x07, 0xE4, 0x60, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x0E, 0x07, 0xDC, 0x03, 0xE4, 0xC0, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x40, 0x80, 0x00, 0x07, 0xDF, 0x03, 0xE4, 0x80, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x60, 0x80, 0x00, 0x0F, 0xDB, 0xE1, 0xE9, 0x80, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x20, 0x80, 0x00, 0x0F, 0x98, 0x7C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x30, 0xC0, 0x00, 0x07, 0x18, 0x0F, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x18, 0x43, 0x80, 0x00, 0x70, 0x03, 0xE6, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x08, 0x47, 0xC0, 0x01, 0xE0, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x0C, 0x4F, 0xC0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x06, 0x4F, 0xC0, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x06, 0x4F, 0x81, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x03, 0x2F, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x01, 0x06, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xC7, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
	display.dim(true);
	display.clearDisplay();
	display.setTextColor(WHITE);
	display.drawBitmap(0, 0, diceBitmap, 128, 64, WHITE);
	display.display();
}