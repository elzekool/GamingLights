
//
// GamingLights
// 
// An Arduino program to control a number of lights (LEDs). Should be used with
// the GamingLights Python program to send new light programs to the Arduino
//
// Configuration:
// --------------------------------------------------
// LED_OUT_OFF: Output for OFF state 
// LED_OUT_ON: Output for ON state
// SETTINGS_SIZE: Max program size, note that there are two buffers (1)
// ledPins[] = { ... }: Output pins, a max of 16 outputs can be controlled
//
// Protocol:
// --------------------------------------------------
// The protocol is a simple byte-by-byte protocol send over Serial,
//
// [ 0xFE ]                                                           : Start marker
//
// [ LIGHTS MSB ] [ LIGHTS LSB ] [ DELAY MSB ] [ DELAY LSB ] [ 0x00 ] : Light configuration
// [ LIGHTS MSB ] [ LIGHTS LSB ] [ DELAY MSB ] [ DELAY LSB ] [ 0x01 ] :
// [ LIGHTS MSB ] [ LIGHTS LSB ] [ DELAY MSB ] [ DELAY LSB ] [ 0x02 ] : 
// [ LIGHTS MSB ] [ LIGHTS LSB ] [ DELAY MSB ] [ DELAY LSB ] [ .... ] : 
//
// [ 0xFF ] [ 0xFF ] [ 0xFF ] [ 0xFF ] [ 0xFF ]                       : End marker
//
// Licence: Attribution-NonCommercial-ShareAlike 4.0 International
// Author: Elze Kool <info@kooldevelopment.nl>
//


//
// User definable constants
//
#define LED_OUT_OFF LOW
#define LED_OUT_ON HIGH
#define SETTINGS_SIZE 128


#define WRITE_IDX_INVALID -1
#define CNT(a) (sizeof(a) / sizeof((a)[0]))

struct ConfigItem {
  uint16_t leds;
  uint16_t delayInMs;
};

// Storage for settings
struct ConfigItem settings_1[SETTINGS_SIZE];
struct ConfigItem settings_2[SETTINGS_SIZE];

// Reference and index to current read position
struct ConfigItem *readSettingBase;
uint8_t readSettingIdx;

// Reference and index to current write position
struct ConfigItem *writeSettingBase;
uint8_t writeSettingIdx;
int8_t writeByteIndex = WRITE_IDX_INVALID;
uint8_t writeByteBuffer[5];

// Led pins
const uint8_t ledPins[] = {
  8, 9, 10,
  11, 12, 13
};

// Storage for wait delay
unsigned long waitMillis;

//
// Clear Settings Object
//
void clearSettings(struct ConfigItem *settings)
{
  memset(settings, 0, sizeof(struct ConfigItem) * SETTINGS_SIZE);
}

//
// Swap read/write settings and restart indexes
//
void swapSettings()
{
  struct ConfigItem *tmpBase;
  tmpBase = readSettingBase;
  readSettingBase = writeSettingBase;
  writeSettingBase = tmpBase;
  readSettingIdx = writeSettingIdx;  
}

//
// Handle input from serial port
//
void handleSerialInput()
{
  // Check if serial data is available, if not stop
  if (Serial.available() == 0) {
    return;
  }

  uint8_t c = (uint8_t) Serial.read();

  // If we just starting with the read wait for an 0xFE marker
  if (writeByteIndex == WRITE_IDX_INVALID) {
    if (c == 0xFE) {
      writeByteIndex = 0;
      writeSettingIdx = 0;
      clearSettings(writeSettingBase);
    }
    return;
  }

  // For byte 0..3 just store the data recieved
  if (writeByteIndex >= 0 && writeByteIndex <= 3) {
    writeByteBuffer[writeByteIndex] = c;
    writeByteIndex++;
    return;
  }

  // We are at byte 4 now, two valid things can happen:
  //
  // 1. A byte corresponding with writeSettingIdx is recieved and lower than SETTINGS_SIZE
  // 2. All 0xFF, we are at the end of the input, store new settings  
  //
  // If not, stop reading and set writeByteIndex to WRITE_IDX_INVALID to restart reading
  
  if (c == writeSettingIdx && writeSettingIdx < SETTINGS_SIZE) {
    writeSettingBase[writeSettingIdx].leds = ((uint16_t) writeByteBuffer[0] << 8) | (uint16_t) writeByteBuffer[1];
    writeSettingBase[writeSettingIdx].delayInMs = ((uint16_t) writeByteBuffer[2] << 8) | (uint16_t) writeByteBuffer[3];       

    // Make sure delayMs is > 0, if not error
    if (writeSettingBase[writeSettingIdx].delayInMs == 0) {
      writeByteIndex = WRITE_IDX_INVALID;
      return;
    }  
      
    writeSettingIdx++;
    writeByteIndex = 0;
    return;
  }

  if (writeByteBuffer[0] == 0xFF && writeByteBuffer[1] == 0xFF && writeByteBuffer[2] == 0xFF && writeByteBuffer[3] == 0xFF && c == 0xFF) {
    // All 0xFF recieved, we can swap buffers and start over
    swapSettings();
    waitMillis = 0;
    writeByteIndex = WRITE_IDX_INVALID;
    return;
  }
  
  // Invalid data recieved, stop parsing current data and wait for marker
  writeByteIndex = WRITE_IDX_INVALID;
}

//
// Handle LED output state
//
void handleLedState()
{  
  unsigned long curMillis = millis();
  
  // Handle initial value
  if (waitMillis == 0) {
    waitMillis = curMillis;
  }

  // Check if wait has passed
  if ((long)( curMillis - waitMillis) >= 0) {

    // If readSettingIdx == SETTINGS_SIZE or delayInMs == 0 then we no we have reached the end, so restart
    if (readSettingIdx == SETTINGS_SIZE || readSettingBase[readSettingIdx].delayInMs == 0) {
      readSettingIdx = 0;
      return;
    }

    // Set leds
    for (int i = 0; i < CNT(ledPins); i++) {
      uint16_t chk = 1 << i;
      if ((readSettingBase[readSettingIdx].leds & chk) != 0) {
        digitalWrite(ledPins[i], LED_OUT_ON);  
      } else {
        digitalWrite(ledPins[i], LED_OUT_OFF);
      }      
    }

    // And wait
    waitMillis = millis() + (unsigned long) readSettingBase[readSettingIdx].delayInMs;

    // Next index
    readSettingIdx++;
  }
  
}

//
// Setup
//
void setup() {

  Serial.begin(115200);

  clearSettings(settings_1);
  readSettingBase = settings_1;

  clearSettings(settings_2);
  writeSettingBase = settings_2;

  readSettingIdx = writeSettingIdx = 0;

  // Make LED outputs
  for (int i = 0; i < CNT(ledPins); i++) {
    digitalWrite(ledPins[i], LED_OUT_OFF);
    pinMode(ledPins[i], OUTPUT);
  }

}

//
// Loop
//
void loop() {
  handleSerialInput();
  handleLedState();
}
