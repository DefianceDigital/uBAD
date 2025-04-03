#include <EEPROM.h>
#include "Keyboard.h"
#include <avr/io.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>

volatile uint8_t resetCause = GPIOR0; // bootloader stores MCUSR value in GPIOR0 to preserve it

static bool debug = false; // enable/disable SerialX
//#define productionVersion
#ifdef productionVersion
  // use ISP port for UART
  SoftwareSerial SerialX(MISO, MOSI); // RX, TX
  #define SoftSerial

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

uint16_t savedCode() {
    // Read each byte separately
    uint8_t lowByte = EEPROM.read(0x02);   // Low byte at address 2
    uint8_t highByte = EEPROM.read(0x03);  // High byte at address 3

    /*if(debug){
      SerialX.print("SC LOW: "); SerialX.println(lowByte);
      SerialX.print("SC HIGH: "); SerialX.println(highByte);
    }*/

    // Combine into a single uint16_t
    uint16_t code = (highByte << 8) | lowByte;

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
    //EEPROM.update(1, 0x02); // tell bootloader to erase EEPROM in case of reset
    delayMicroseconds(random(0, 255));
    eraseEEPROM();
    asm volatile ("  jmp 0"); // restart application from beginning
  }
}

void eraseEEPROM() {
  int size = EEPROM.length() - 1; // don't erase bootkill byte
  if(debug){
    SerialX.println("Erasing EEPROM");
  }
  EEPROM.update(0, 0xFF); // erase first
  for (int i = 2; i < size; i++) {
    EEPROM.update(i, 0xFF); // write all eeprom to 0xFF
  }
  EEPROM.update(1, 0xFF); // erase last
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

  printCredentials(password);

  reValidate(); // fault injection prevention

  if(strlen(pim) > 0){
    printCredentials(pim);
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

void manualUnlock(){
  digitalWrite(YELLOW_PIN, HIGH);

  delay(250);

  Keyboard.begin();

  reValidate(); // fault injection prevention

  delay(250);

  loadCredentials();

  reValidate(); // fault injection prevention

  printCredentials(password);

  reValidate(); // fault injection prevention

  if(strlen(pim) > 0){
    printCredentials(pim);
  }

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

  Serial.println("Select Decryption Mode:");
  Serial.println("--> For Automatic VeraCrypt-Windows System Decryption - Enter 'A'");
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
      if(selection == 'A'){
        encMode = 0x01;
      } else if(selection == 'a'){
        encMode = 0x01;
      }else if(selection == 'M'){
        encMode = 0x02;
      } else if(selection == 'm'){
        encMode = 0x02;
      } else {
        Serial.println("Invalid Mode Selection");
        Serial.flush();
      }
    }
  }

  if(EEPROM.read(1023) != 0xBB){ // do not allow changes once set to true
    Serial.println("Permanently Disable Firmware Updates? (Enter 'Y' or 'N')");
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

  Serial.println("Enter Decryption Password (64 characters or less)");
  Serial.flush();
  while(!hasPass){
    if(Serial.available()){
      delay(25);
      for(uint8_t x = 0; x < 64; x++){
        char value = Serial.read();
        if((value != '\r') && (value != '\n') && (value != 0xFF)){
          EEPROM.update(x+4, value);
        } else {
          EEPROM.update(x+4, 0x00);
        }
      }
      EEPROM.update(68, 0x00); //null terminated password
      while(Serial.available()){
        Serial.read();
      }
      hasPass = true;
    }
  }
  Serial.println("Enter VeraCrypt System PIM (Enter '*' if none)");
  Serial.flush();
  while(1){
    if(Serial.available()){
      delay(25);
      for(uint8_t y = 0; y < 16; y++){
        char value = Serial.read();
        if((value != '\r') && (value != '\n') && (value != 0xFF)){
          EEPROM.update(y+69, value);
        } else {
          EEPROM.update(y+69, 0x00);
        }
      }
      while(Serial.available()){
        Serial.read();
      }

      if(debug){
        SerialX.println("New credentials obtained");
      }

      EEPROM.update(0, encMode); // set se encryption mode
      EEPROM.update(1, 0x00); // set number of failed attempts
      uint8_t low = lowByte(switchCode());
      uint8_t high = highByte(switchCode());
      EEPROM.update(2, low); // set switch code part 1
      EEPROM.update(3, high); // set switch code part 2
      if(disableUpdates){
        EEPROM.update(1023, 0xBB); // prevent updates via bootloader
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
}

void seedRNG(){ // use entropy to generate random seed
  unsigned int ar1 = analogRead(A4);
  unsigned int ar2 = analogRead(A5);

  unsigned int tr; // future raw internal temperature sensor reading
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3)); // set 1.1v reference and MUX
  ADCSRA |= _BV(ADEN);  // enable ADC
  delay(20); // wait for voltages to stabilize
  while(bit_is_set(ADCSRA, ADSC)); // detect EOC
  tr = ADCW; // store raw reading
  
  unsigned int seed = ar1 ^ ar2 ^ tr; // combine all 3 entropy sources by XOR mixing

  randomSeed(seed); // seed the RNG

  if(debug){
    SerialX.print("Analog Reading 1: "); SerialX.println(ar1);
    SerialX.print("Analog Reading 2: "); SerialX.println(ar2);
    SerialX.print("Raw Temp Reading: "); SerialX.println(tr);
    SerialX.print("Random Seed: "); SerialX.println(seed);
  }
}

void printCredentials(char credentials[64]){
  uint8_t len = strlen(credentials);
  for(uint8_t i = 0; i < len; i++){
    char toPrint = credentials[i];
    if(toPrint != 0x00){
      reValidate(); // fault injection prevention
      Keyboard.print(toPrint);
      delay(10);
    }
  }
  delay(500); // delay after sending credentials
  Keyboard.write(KEY_RETURN);
  delay(500); // delay after sending enter
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

  seedRNG(); // start random number generator with entropy

  initPins();
  delay(250);
  if(debug){
    SerialX.println("Initialized Pins");
  }

  if((EEPROM.read(0) != 0xFF) && (savedCode() == switchCode())){
    delay(random(0, 1000)); // here to make it hard to determine if code was accepted or erase is happening by monitoring leds electronically

    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(YELLOW_PIN, HIGH);

    if(debug){
      SerialX.println("Valid credentials Found");
    }
    EEPROM.update(1, 0x00); // this only changes after incorrect dip switch

    if(EEPROM.read(0) == 0x01){
      reValidate(); // fault injection prevention
      unlockVeracrypt();
    } else if(EEPROM.read(0) == 0x02){
      reValidate(); // fault injection prevention
      manualUnlock();
    }
  } else {
    if(debug){
      SerialX.println("Invalid DIP Code or Credentials");
      SerialX.print("Current EEPROM.read(0): "); SerialX.println(EEPROM.read(0));
      SerialX.print("Current Switch Code: "); SerialX.println(switchCode());
      loadCredentials();
    }

    // give chance to fix failed switch code if this is 1st failure 
    if(EEPROM.read(1) == 0){
      EEPROM.update(1, 0x01);
      digitalWrite(RED_PIN, HIGH); 
      digitalWrite(YELLOW_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      while(1); // stop here
    }
    //EEPROM.update(1, 0x02); // tell bootloader to erase EEPROM too(in case of reset attempt)

    eraseEEPROM();
    
    setCredentials();
  }
}

void loop() {
  // no loop 
}