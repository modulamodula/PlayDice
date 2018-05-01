#pragma once
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "Settings.h"
#include "WString.h"

extern const boolean DEBUGFRAME;

extern uint16_t bpm;
extern int8_t seqStep;
extern int8_t editStep;
extern editType editMode;
extern float cvRandVal;
extern boolean gateRandVal;
extern uint8_t cvSeqNo;
extern uint8_t gateSeqNo;
extern uint8_t numSeqA;
extern seqMode cvLoopMode;
extern uint8_t numSeqB;
extern seqMode gateLoopMode;
extern seqType activeSeq;
extern CvPatterns cv;
extern GatePatterns gate;

extern float getRandLimit(CvStep s, rndType getUpper);
extern boolean checkEditing();
extern ClockHandler clock;

class DisplayHandler {
public:
	DisplayHandler();
	void updateDisplay();
	void init();
	int cvVertPos(float voltage);
	void drawDottedVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
	//void drawParam(const char s[], float v, int16_t x, int16_t y, uint16_t w, boolean selected);
	void drawParam(const char s[], String v, int16_t x, int16_t y, uint16_t w, boolean selected);
	Adafruit_SSD1306 display;
private:
	long clockSignal;
	uint32_t frameStart;
};

//	Putting the constructor here with display class initialised after colon ensures that correct constructor gets called and does not blank settings
DisplayHandler::DisplayHandler():
	display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS) {
}

// carry out the screen refresh building the various UI elements
void DisplayHandler::updateDisplay() {
	if (DEBUGFRAME) {
		frameStart = micros();
	}

	boolean editing = checkEditing();		// set to true if currently editing to show detailed parameters

	display.clearDisplay();

	//	Write the sequence number for CV and gate sequence
	display.setTextSize(1);

	if (!editing || activeSeq == SEQCV) {
		display.setCursor(0, 0);
		display.print("cv");
	}
	if (!editing || activeSeq == SEQGATE) {
		display.setCursor(0, 39);
		display.print("Gt");
	}

	display.setTextSize(2);
	if (!editing || activeSeq == SEQCV) {
		display.setCursor(1, 11);
		display.print(cvSeqNo + 1);
	}
	if (!editing || activeSeq == SEQGATE) {
		display.setCursor(1, 50);
		display.print(gateSeqNo + 1);
	}
	display.setTextSize(1);

	// Draw a dot if we have a clock high signal
	if (clock.hasSignal()) {
		display.drawFastVLine(127, 0, 1, WHITE);
	}

	//	Draw arrow beneath/above sequence number if selected for editing
	if (editStep == -1) {
		display.drawLine(4, activeSeq == SEQCV ? 34 : 32, 6, activeSeq == SEQCV ? 32 : 34, WHITE);
		display.drawLine(6, activeSeq == SEQCV ? 32 : 34, 8, activeSeq == SEQCV ? 34 : 32, WHITE);
	}

	// Draw the sequence steps for CV and gate sequence
	for (int i = 0; i < 8; i++) {
		int voltHPos = 17 + (i * 14);
		//int voltVPos = 27 - round(cv.seq[cvSeqNo].Steps[i].volts * 5);
		int voltVPos = cvVertPos(cv.seq[cvSeqNo].Steps[i].volts);

		// Draw CV pattern
		if (!editing || activeSeq == SEQCV) {
			// Draw voltage line 

			if (cv.seq[cvSeqNo].Steps[i].stutter > 0) {
				float w = (float)12 / cv.seq[cvSeqNo].Steps[i].stutter;

				for (int sd = 0; sd < cv.seq[cvSeqNo].Steps[i].stutter; sd++) {
					// draw jagged stripes showing stutter pattern
					display.fillRect(voltHPos + round(sd * w), voltVPos + (sd % 2 ? 0 : 1), round(w), 2, WHITE);
				}
			}
			else {
				display.fillRect(voltHPos + 2, voltVPos, 8, 2, WHITE);
			}

			//	show randomisation by using a vertical dotted line with height proportional to amount of randomisation
			if (cv.seq[cvSeqNo].Steps[i].rand_amt > 0) {
				float randLower = constrain(getRandLimit(cv.seq[cvSeqNo].Steps[i], LOWER), 0, 5);
				float randUpper = constrain(getRandLimit(cv.seq[cvSeqNo].Steps[i], UPPER), 0, 5);
				drawDottedVLine(voltHPos, 2 + cvVertPos(randUpper), 1 + cvVertPos(randLower) - cvVertPos(randUpper), WHITE);
			}
			// draw amount of voltage selected after randomisation applied
			if (seqStep == i) {
				display.fillRect(voltHPos, round(26 - (cvRandVal * 5)), 13, 4, WHITE);
			}		
		}

		// Draw gate pattern 
		if (!editing || activeSeq == SEQGATE) {
			if (gate.seq[gateSeqNo].Steps[i].on || gate.seq[gateSeqNo].Steps[i].stutter > 0) {
				if (seqStep == i && !gateRandVal) {
					display.drawRect(voltHPos + 4, 50, 6, 14, WHITE);		// draw gate as empty rectange for current step if set 'on' but randomised 'off'
				}
				else {
					if (gate.seq[gateSeqNo].Steps[i].stutter > 0) {
						
						// draw base line
						display.drawFastHLine(voltHPos + 3, 63, 8, WHITE);
						float w = (float)8 / gate.seq[gateSeqNo].Steps[i].stutter;
						for (int sd = 0; sd < round((float)gate.seq[gateSeqNo].Steps[i].stutter / 2); sd++) {
							// draw vertical stripes showing stutter layout - if gate is off then stutter starts later														
							display.fillRect(voltHPos + 3 + (gate.seq[gateSeqNo].Steps[i].on ? 0 : round(w)) + (sd * round(w * 2)), 50, round(w), 14, WHITE);
						}
					}
					else {
						display.fillRect(voltHPos + 4, 50, 6, 14, WHITE);
					}
				}
			}
			else {
				display.fillRect(voltHPos + 4, 63, 6, 1, WHITE);
			}
			// draw current step - larger block if 'on' larger base if 'off'
			if (seqStep == i) {
				if (gateRandVal) {
					display.fillRect(voltHPos + 3, 45, 8, 29, WHITE);
				}
				else {
					display.fillRect(voltHPos + 3, 62, 8, 2, WHITE);
				}
			}

			// draw line showing random amount
			uint8_t rndTop = round(gate.seq[gateSeqNo].Steps[i].rand_amt * (float)(24 / 10));
			drawDottedVLine(voltHPos, 64 - rndTop, rndTop, WHITE);
		}

		//	Draw arrow beneath step selected for editing
		if (editStep == i) {
			display.drawLine(voltHPos + 4, activeSeq == SEQCV ? 34 : 32, voltHPos + 6, activeSeq == SEQCV ? 32 : 34, WHITE);
			display.drawLine(voltHPos + 6, activeSeq == SEQCV ? 32 : 34, voltHPos + 8, activeSeq == SEQCV ? 34 : 32, WHITE);
		}

	}

	//	if currently or recently editing show values in bottom area of screen
	if (editing) {
		if (activeSeq == SEQGATE) {
			if (editMode == STEPR || editMode == STEPV || editMode == STUTTER) {
				drawParam("Gate", String(gate.seq[gateSeqNo].Steps[editStep].on ? "ON" : "OFF"), 0, 0, 36, editMode == STEPV);
				drawParam("Random", String(gate.seq[gateSeqNo].Steps[editStep].rand_amt), 38, 0, 44, editMode == STEPR);
				drawParam("Stutter", String(gate.seq[gateSeqNo].Steps[editStep].stutter), 81, 0, 47, editMode == STUTTER);
			}

			if (editMode == PATTERN || editMode == SEQS || editMode == SEQMODE) {
				drawParam("Pattern", String(gateSeqNo + 1), 0, 0, 49, editMode == PATTERN);
				drawParam("Loop", String(gateLoopMode == LOOPCURRENT ? "ONE" : "ALL"), 52, 0, 34, editMode == SEQMODE);
				drawParam("Loops", String(numSeqB), 90, 0, 34, editMode == SEQS);
			}
		}

		if (activeSeq == SEQCV) {
			if (editMode == STEPR || editMode == STEPV || editMode == STUTTER) {
				drawParam("Volts", String(cv.seq[cvSeqNo].Steps[editStep].volts), 0, 40, 36, editMode == STEPV);
				drawParam("Random", String(cv.seq[cvSeqNo].Steps[editStep].rand_amt), 38, 40, 44, editMode == STEPR);
				drawParam("Stutter", String(cv.seq[cvSeqNo].Steps[editStep].stutter), 81, 40, 47, editMode == STUTTER);
			}

			if (editMode == PATTERN || editMode == SEQS || editMode == SEQMODE) {
				drawParam("Pattern", String(cvSeqNo + 1), 0, 40, 49, editMode == PATTERN);
				drawParam("Loop", String(cvLoopMode == LOOPCURRENT ? "ONE" : "ALL"), 52, 40, 34, editMode == SEQMODE);
				drawParam("Loops", String(numSeqA), 90, 40, 34, editMode == SEQS);
			}
		}
	}

	if (display.display() && DEBUGFRAME) {
		Serial.print("Frame start: "); Serial.print(frameStart); Serial.print(" end: "); Serial.print(micros()); Serial.print(" time: "); Serial.println(micros() - frameStart);
	}
}


//	returns the vertical position of a voltage line on the cv channel display
int DisplayHandler::cvVertPos(float voltage) {
	return 27 - round(voltage * 5);
}

void DisplayHandler::drawDottedVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	for (int d = y; d < y + h; d += 3) {
		display.drawPixel(x, d, color);
	}
}

void DisplayHandler::drawParam(const char s[], String v, int16_t x, int16_t y, uint16_t w, boolean selected) {
	display.setCursor(x + 4, y + 3);
	display.println(s);
	display.setCursor(x + 4, y + 13);
	display.println(v);
	if (selected) {
		display.drawRect(x, y, w, 24, INVERSE);
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