#include <EEPROM.h>
#include "Keyboard.h"
#include <avr/io.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>

volatile uint8_t resetCause = GPIOR0; // bootloader stores MCUSR value in GPIOR0 to preserve it

bool debug = false; // enable/disable SerialX
#define productionVersion
#ifdef productionVersion
  // use ISP port for UART
  SoftwareSerial SerialX(MISO, MOSI); // RX, TX

  #define RED_PIN A0
  #define YELLOW_PIN A1
  #define GREEN_PIN A2
#else
  // non-production versions have UART access
  #define SerialX Serial1

  #define RED_PIN 15
  #define YELLOW_PIN A1
  #define GREEN_PIN A3
#endif

uint16_t switchCode() {
  uint16_t code = 0;
  for(uint8_t i = 0; i < 10; i++){
    uint8_t bit = i;
    uint8_t pin = i+2;
    if (digitalRead(pin) == HIGH) { // high is low and low is high because "ON" will read low on pin
      // Clear the bit to 0
      code &= ~(1 << bit);
    } else {
      // Set the bit to 1
      code |= (1 << bit);
    }
  }
  return code;
}

uint16_t savedCode(){
  uint16_t code = (EEPROM.read(3) << 8) | EEPROM.read(2);
  return code;
}

void initPins(){
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  for(uint8_t i = 2; i < 12; i ++){
    pinMode(i, INPUT_PULLUP);
  }
}

// This function is specifically for fault injection prevention and should bbe called often
void reValidate(){
  delayMicroseconds(random(0, 255));
  if(switchCode() != savedCode()){
    delayMicroseconds(random(0, 255));
    uint8_t switchFailures = EEPROM.read(1);
    delayMicroseconds(random(0, 255));
    EEPROM.update(1, 0x02); // tell bootloader to erase EEPROM in case of reset
    delayMicroseconds(random(0, 255));
    eraseEEPROM();
    asm volatile ("  jmp 0"); // restart application from beginning
  }
}

void eraseEEPROM() {
  int size = EEPROM.length() - 1; // don't erase bootkill byte
  digitalWrite(YELLOW_PIN, HIGH);
    if(debug){
      SerialX.println("Erasing EEPROM");
    }
    EEPROM.update(0, 0xFF); // erase first
    for (int i = 2; i < size; i++) {
      EEPROM.update(i, 0xFF); // write all eeprom to 0xFF
    }
    EEPROM.update(1, 0xFF); // erase last
    digitalWrite(YELLOW_PIN, LOW);
}


char password[65];
char pim[16];
void loadCredentials(){
  for(uint8_t i = 0; i < 65; i++){
    password[i] = EEPROM.read(i+4);
  }
  for(uint8_t i = 0; i < 16; i++){
    pim[i] = EEPROM.read(i+69);
  }
  if(debug){
    uint16_t dip = (EEPROM.read(3) << 8) | EEPROM.read(2);
    SerialX.print("Encryption Type: ["); SerialX.print(EEPROM.read(0)); SerialX.println("]");
    SerialX.print("Switch Code Fail Count: ["); SerialX.print(EEPROM.read(1)); SerialX.println("]");
    SerialX.print("DIP Code: ["); SerialX.print(dip); SerialX.println("]");
    SerialX.print("Password: ["); SerialX.print(password); SerialX.println("]");
    SerialX.print("PIM: ["); SerialX.print(pim); SerialX.println("]");
    SerialX.print("Bootkill Byte: ["); SerialX.print(EEPROM.read(1023)); SerialX.println("]");
  }
}

void setupInterrupts(){
  UDINT = 0; // Clear all interrupt flags initially
  UDIEN = (1 << WAKEUPE); // wake-up interrupt enabled (only one that works)
}

bool usbActive = false;
uint8_t inactiveCount = 0;
bool isRestarted(){
  if (UDINT & (1 << WAKEUPI)) {
    if(!usbActive){
      usbActive = true;
      if(debug){
        SerialX.println("Wake-up CPU Interrupt Detected");
        SerialX.flush();
      }      
    }
    inactiveCount = 0;
    UDINT &= ~(1 << WAKEUPI); // Acknowledge the interrupt
    return false;
  } else {
    if(usbActive){
      usbActive = false;
      if(debug){
        SerialX.println("Wake-up CPU Interrupt NOT Detected");
        SerialX.flush();
      }
      return true;
    } else {
      return false;
    }
  }
}

void unlockVeracrypt(){
  delay(15000); // Initial delay

  reValidate(); // fault injection prevention

  Keyboard.begin();

  reValidate(); // fault injection prevention

  setupInterrupts();

  reValidate(); // fault injection prevention

  loadCredentials();

  reValidate(); // fault injection prevention

  Keyboard.print(password);
  Keyboard.flush();
  delay(500); // short delay after sending password
  Keyboard.write(KEY_RETURN);
  Keyboard.flush();
  delay(500); // delay after sending enter

  reValidate(); // fault injection prevention

  if(strlen(pim) > 0){
    Keyboard.print(pim);
    Keyboard.flush();
    delay(500); // short delay before sending pim
    Keyboard.write(KEY_RETURN);
    Keyboard.flush();
    delay(500); // short delay after sending Enter
  }

  uint16_t waitCount = 0;
  bool yellowState = 0;

  // second restart flag is pass accepted
  while(!isRestarted()){
    delay(100);
  }
  while(isRestarted()){
    delay(100);
  }
  if(debug){
    SerialX.println("Password Accepted");
    SerialX.flush();
  }

  // 3rd restart flag is windows loading
  while(!isRestarted()){
    waitCount++;
    if(waitCount % 5 ==0){ // every 5 periods (500ms)
      if(!yellowState){ // if yellow off
        digitalWrite(YELLOW_PIN, HIGH);
        yellowState = 1;
      } else {
        yellowState = 0;
        digitalWrite(YELLOW_PIN, LOW);
      }
    }
    delay(50);
  }
  while(isRestarted()){
    delay(100);
  }
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
  if(debug){
    SerialX.println("Windows Started");
    SerialX.flush();
  }

  while(1){
    if(isRestarted()){
      while(isRestarted());
      asm volatile ("  jmp 0");
    }
    delay(100);
  }
}

void unlockLuks(){
  if (resetCause & (1 << PORF)) {
    if(debug){
      SerialX.print("Power-on Reset");
    }
    delay(10000);

    reValidate(); // fault injection prevention

    Keyboard.begin();

    reValidate(); // fault injection prevention

    setupInterrupts();

    reValidate(); // fault injection prevention

    loadCredentials();

    reValidate(); // fault injection prevention

    uint16_t waitCount = 0;
    bool yellowState = 0;
    while(!isRestarted()){
      waitCount++;
      if(waitCount % 5 ==0){ // every 5 periods (500ms)
        if(!yellowState){ // if yellow off
          digitalWrite(YELLOW_PIN, HIGH);
          yellowState = 1;
        } else {
          yellowState = 0;
          digitalWrite(YELLOW_PIN, LOW);
        }
      }
      delay(50);
    }
    while(isRestarted()){
      delay(100);
    }
    if(debug){
      Serial1.println("Decryption Screen Loading");
      Serial1.flush();
    }
    wdt_enable(WDTO_1S);
    while(1); // reset
  } else if (resetCause & (1 << WDRF)) {
    if(debug){
      SerialX.print("Watchdog Reset");
    }
    delay(60000);

    reValidate(); // fault injection prevention

    Keyboard.begin();

    reValidate(); // fault injection prevention

    setupInterrupts();

    reValidate(); // fault injection prevention

    loadCredentials();

    reValidate(); // fault injection prevention

    Keyboard.print(password);
    Keyboard.flush();
    delay(500); // short delay after sending password
    Keyboard.write(KEY_RETURN);
    Keyboard.flush();
    delay(500); // delay after sending enter

    // No way to confim correct password with Luks
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    if(debug){
      SerialX.println("Password Entered");
      SerialX.flush();
    }

    while(1){
    if(isRestarted()){
      while(isRestarted());
      GPIOR0 = 0b101; // power on reset (and brownout)
      asm volatile ("  jmp 0");
    }
    delay(100);
  }
  }
}

void manualUnlock(){
  digitalWrite(YELLOW_PIN, HIGH);

  delay(250);

  Keyboard.begin();

  reValidate(); // fault injection prevention

  delay(250);

  loadCredentials();

  reValidate(); // fault injection prevention

  Keyboard.print(password);
  Keyboard.flush();

  reValidate(); // fault injection prevention

  delay(500); // short delay after sending password

  reValidate(); // fault injection prevention

  if(strlen(pim) > 0){
    Keyboard.print(pim);
    Keyboard.flush();
    delay(500); // short delay afte entering pim
  }
  Keyboard.write(KEY_RETURN);
  Keyboard.flush();
  delay(500); // delay after sending enter

  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);

  while(1); 
}

void setCredentials(){
  uint16_t waitCount = 0;
  bool redState = 0;
  bool disableUpdates = 0;

  Serial.begin(115200);
  while(!Serial){
    waitCount++;
    if(waitCount % 5 ==0){ // every 5 periods (500ms)
      if(!redState){ // if red off
        digitalWrite(RED_PIN, HIGH);
        redState = 1;
      } else {
        redState = 0;
        digitalWrite(RED_PIN, LOW);
      }
    }
    delay(50);
  }
  digitalWrite(RED_PIN, HIGH);(!Serial);

  setupInterrupts(); 

  uint8_t encMode = 0;
  bool hasPass = false;
  bool hasUpdateChoice = false;

  if(debug){
    SerialX.println("Waiting for new credentials");
  }

  Serial.println("Select Encryption Mode:");
  Serial.println("--> For Automatic VeraCrypt System Decryption (Windows) - Enter 'W'");
  Serial.println("--> For Automatic Luks System Decryption (Linux) - Enter 'L'");
  Serial.println("--> For Manual Decryption (Enters Credentials on Plugin) - Enter 'M'");
  Serial.flush();

  while(!Serial.available()){
    waitCount++;
    if(waitCount % 5 ==0){ // every 5 periods (500ms)
      if(!redState){ // if red off
        digitalWrite(RED_PIN, HIGH);
        redState = 1;
      } else {
        redState = 0;
        digitalWrite(RED_PIN, LOW);
      }
    }
    delay(50);
  }
  digitalWrite(RED_PIN, HIGH);

  while(encMode == 0){
    if(Serial.available()){
      delay(25);
      char selection = Serial.read();
      while(Serial.available()){Serial.read();} // clear serial buffer
      if(selection == 'W'){
        encMode = 0x01;
      } else if(selection == 'w'){
        encMode = 0x01;
      } else if(selection == 'L'){
        encMode = 0x02;
      } else if(selection == 'l'){
        encMode = 0x02;
      }else if(selection == 'M'){
        encMode = 0x03;
      } else if(selection == 'm'){
        encMode = 0x03;
      } else {
        Serial.println("Invalid Mode Selection");
        Serial.flush();
      }
    }
  }

  if(EEPROM.read(1023) != 0xBB){ // do not allow changes once set to true
    Serial.println("Disable Firmware Security Patches? (Enter 'y' or 'n'))");
    Serial.flush();
    while(hasUpdateChoice == 0){
      if(Serial.available()){
        delay(25);
        char selection = Serial.read();
        while(Serial.available()){Serial.read();} // clear serial buffer
        if(selection == 'Y'){
          disableUpdates = true;
          hasUpdateChoice = true;
        } else if(selection == 'y'){
          disableUpdates = true;
          hasUpdateChoice = true;
        } else if(selection == 'N'){
          hasUpdateChoice = true;
        } else if(selection == 'n'){
          hasUpdateChoice = true;
        } else {
          Serial.println("Invalid Selection");
          Serial.flush();
        }
      }
    }
  }

  Serial.println("Enter Desired Password (64 characters or less)");
  Serial.flush();
  while(!hasPass){
    if(Serial.available()){
      delay(25);
      for(uint8_t x = 0; x < 64; x++){
        char value = Serial.read();
        if((value != '\r') && (value != '\n') && (value != 0xFF)){
          EEPROM.write(x+4, value);
        } else {
          EEPROM.write(x+4, 0x00);
        }
      }
      EEPROM.write(68, 0x00); //null terminated password
      while(Serial.available()){
        Serial.read();
      }
      hasPass = true;
    }
  }
  
  if(encMode == 0x01 || encMode == 0x03){ // Luks doesn't have PIM option
    Serial.println("Enter Desired PIM (Press '*' if none)");
    Serial.flush();
    while(1){
      if(Serial.available()){
        delay(25);
        for(uint8_t y = 0; y < 16; y++){
          char value = Serial.read();
          if((value != '\r') && (value != '\n') && (value != 0xFF)){
            EEPROM.write(y+69, value);
          } else {
            EEPROM.write(y+69, 0x00);
          }
        }
        while(Serial.available()){
          Serial.read();
        }

        if(debug){
          SerialX.println("New credentials obtained");
        }

        EEPROM.write(0, encMode); // set se encryption mode
        EEPROM.write(1, 0x00); // set number of failed attempts
        uint8_t low = lowByte(switchCode());
        uint8_t high = highByte(switchCode());
        EEPROM.write(2, low); // set switch code part 1
        EEPROM.write(3, high); // set switch code part 2
        if(disableUpdates){
          EEPROM.write(1023, 0xBB); // preven updates via bootloader
        }
        
        loadCredentials();

        Serial.println("Configuration complete");
        Serial.flush();
            
        Serial.end();;
        digitalWrite(RED_PIN, LOW); // red off

        while(1){
          if(isRestarted()){
            while(isRestarted());
            asm volatile ("  jmp 0");
          } else {
            digitalWrite(YELLOW_PIN, LOW);
            digitalWrite(GREEN_PIN, HIGH);
            delay(500);
            digitalWrite(YELLOW_PIN, HIGH);
            digitalWrite(GREEN_PIN, LOW);
            delay(500);
          }
        }
      }
    }
  } else { // Luks doesn't use PIM
    for(uint8_t y = 0; y < 16; y++){
      EEPROM.write(y+69, 0x00);
    }

    if(debug){
      SerialX.println("New credentials obtained");
    }

    EEPROM.write(0, encMode); // set se encryption mode
    EEPROM.write(1, 0x00); // set number of failed attempts
    uint8_t low = lowByte(switchCode());
    uint8_t high = highByte(switchCode());
    EEPROM.write(2, low); // set switch code part 1
    EEPROM.write(3, high); // set switch code part 2

    loadCredentials();

    /*Serial.print("password: "); Serial.println(password);
    Serial.print("pim: "); Serial.println(pim);
    Serial.flush();*/

    EEPROM.write(1023, 0xBB); // lock bootloader when credentials are stored (anykey-bootloader)

    Serial.println("Configuration complete");
    Serial.flush();
            
    Serial.end();;
    digitalWrite(RED_PIN, LOW); // red off

    while(1){
      if(isRestarted()){
        while(isRestarted());
        asm volatile ("  jmp 0");
      } else {
        digitalWrite(YELLOW_PIN, LOW);
        digitalWrite(GREEN_PIN, HIGH);
        delay(500);
        digitalWrite(YELLOW_PIN, HIGH);
        digitalWrite(GREEN_PIN, LOW);
        delay(500);
      }
    }
  }
}

void setup() {
  #ifndef productionVersion // production boards don't have these 
    DDRB &= ~(1 << PB0); // disable RXLED
    DDRD &= ~(1 << PD5); // disable TXLED
  #endif
  
  if(debug){
    #ifdef SoftSerial
      SerialX.begin(9600); // SoftwareSerial can't support 115200 baud
    #else
      SerialX.begin(115200);
    #endif
  }

  initPins();
  if(debug){
    SerialX.println("Initialized Pins");
  }

  if((EEPROM.read(0) != 0xFF) && (savedCode() == switchCode())){
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(YELLOW_PIN, HIGH);

    if(debug){
      SerialX.println("Valid credentials Found");
    }
    EEPROM.update(1, 0x00);

    if(EEPROM.read(0) == 0x01){
      reValidate(); // fault injection prevention
      unlockVeracrypt();
    } else if(EEPROM.read(0) == 0x02){
      reValidate(); // fault injection prevention
      unlockLuks();
    } else if(EEPROM.read(0) == 0x03){
      reValidate(); // fault injection prevention
      manualUnlock();
    }
  } else {
    digitalWrite(RED_PIN, HIGH); 
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);

    if(debug){
      SerialX.println("Invalid DIP Code or Credentials");
      loadCredentials();
    }

    // give chance to fix failed switch code if this is 1st failure (2 seconds)
    if(EEPROM.read(1) == 0){
      EEPROM.write(1, 0x01);
      delay(2000);
    }
    EEPROM.write(1, 0x02); // tell bootloader to erase EEPROM if reset

    eraseEEPROM();
    
    setCredentials();
  }
}

void loop() {
  // no loop 
}