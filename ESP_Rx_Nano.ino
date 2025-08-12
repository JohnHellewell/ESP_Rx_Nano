#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "driver/ledc.h"
#include "secrets.h" //Wi-Fi credentials
#include "accel_handler.h"
#include "settings.h"

#define SOFTWARE_VERSION "1.1.1"

#define SCL 7 
#define SDA 6 

//LED
#define ONBOARD_LED 8
bool led_on = true;

//UDP
WiFiUDP udp;
const unsigned int localPort = 4200;  
char incomingPacket[255];

#define CH1_PIN 3  //0
#define CH2_PIN 2  //1
#define CH3_PIN 1  //2

unsigned long lastPacketReceived; //used to measure time
bool connected = false;
#define FAILSAFE_DISCONNECT 500 //how long (ms) before failsafe activates

// Channels
#define CH1_PWM LEDC_CHANNEL_1
#define CH2_PWM LEDC_CHANNEL_2
#define CH3_PWM LEDC_CHANNEL_3

#define PWM_FREQ_HZ     50  // 50 Hz = 20 ms period
#define PWM_RES_BITS    LEDC_TIMER_13_BIT  // 13-bit resolution
#define PWM_TIMER       LEDC_TIMER_0
#define PWM_MODE        LEDC_LOW_SPEED_MODE

int DEFAULT_PWM_RANGE[2] = {1000, 2000};
int SERVO_RANGE[2] = {500, 2500};

const bool SERVO_BOT = false; //true if bot is equipped with servo weapon, false if not

const int CH1_DEFAULT = 1500; 
const int CH2_DEFAULT = 1500;
const int CH3_DEFAULT = 1500; //normally would be 1000 for weapon to be off. But ESC is set to bidirectional

int ch1 = CH1_DEFAULT; 
int ch2 = CH2_DEFAULT;
int ch3 = CH3_DEFAULT; 
int killswitch = 0; //0 is OFF (as in robots should be off), 1 is LIMITED (drive enabled, weapon disabled), 2 is ARMED (battle mode)

bool right_motor_reverse = false;
bool left_motor_reverse = true;
bool weapon_reverse = false;

const int SAFE_VARIANCE = 25; //in order to switch from kill switch mode 0 to 1 or 2, channels must be this close to the default range

float z_offset = 2.5;  // Offset to calibrate Z axis. 2.5 is what's typically adjusted
float z_accel = 0.0;

volatile bool flipped = false;
const double FLIPPED_Z_THRESHOLD = 5.0; //when the z acceleration goes above this threshold, bot is considered flipped or unflipped

AccelHandler* accelHandler;


void setup(void) {
  Serial.begin(115200);

  Serial.print("Running software version ");
  Serial.println(SOFTWARE_VERSION);
  
  lastPacketReceived = millis();

  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, HIGH);

  setup_ESCs();
  
  setup_accelerometer();

  xTaskCreate(AccelerometerTask, "AccelMonitor", 4096, NULL, 1, NULL);

  startAP();

  setup_OTA();
}

// Utility to convert microseconds to 13-bit duty
uint32_t usToDuty(uint16_t pulse_us) {
  return (uint32_t)(((uint64_t)pulse_us * 8191) / 20000);
}

void setPWM(uint8_t channel, uint16_t pulse_us) {
  ledc_channel_t ch;
  switch (channel) {
    case 1: ch = CH1_PWM; break;
    case 2: ch = CH2_PWM; break;
    case 3: ch = CH3_PWM; break;
    default: return; // invalid channel
  }

  uint32_t duty = usToDuty(pulse_us);
  ledc_set_duty(PWM_MODE, ch, duty);
  ledc_update_duty(PWM_MODE, ch);
}


void setup_ESCs(){
  // Timer configuration
  ledc_timer_config_t timer_conf = {
    .speed_mode       = PWM_MODE,
    .duty_resolution  = PWM_RES_BITS,
    .timer_num        = PWM_TIMER,
    .freq_hz          = PWM_FREQ_HZ,
    .clk_cfg          = LEDC_AUTO_CLK
  };
  ledc_timer_config(&timer_conf);

  // Channel 1 setup
  ledc_channel_config_t ch1_conf = {
    .gpio_num       = CH1_PIN,
    .speed_mode     = PWM_MODE,
    .channel        = CH1_PWM,
    .intr_type      = LEDC_INTR_DISABLE,
    .timer_sel      = PWM_TIMER,
    .duty           = 0,
    .hpoint         = 0
  };
  ledc_channel_config(&ch1_conf);

  // Channel 2 setup
  ledc_channel_config_t ch2_conf = {
    .gpio_num       = CH2_PIN,
    .speed_mode     = PWM_MODE,
    .channel        = CH2_PWM,
    .intr_type      = LEDC_INTR_DISABLE,
    .timer_sel      = PWM_TIMER,
    .duty           = 0,
    .hpoint         = 0
  };
  ledc_channel_config(&ch2_conf);

  // Channel 3 setup
  ledc_channel_config_t ch3_conf = {
    .gpio_num       = CH3_PIN,
    .speed_mode     = PWM_MODE,
    .channel        = CH3_PWM,
    .intr_type      = LEDC_INTR_DISABLE,
    .timer_sel      = PWM_TIMER,
    .duty           = 0,
    .hpoint         = 0
  };
  ledc_channel_config(&ch3_conf);

  setPWM(1, CH1_DEFAULT);
  setPWM(2, CH2_DEFAULT);
  setPWM(3, CH3_DEFAULT);

  Serial.println("PWM channels started successfully");
}

void setup_accelerometer(){
  accelHandler = new AccelHandler(SDA, SCL);
}

void AccelerometerTask(void *pvParameters) {
    constexpr int SAMPLE_COUNT = 5;
    float readings[SAMPLE_COUNT] = {0.0f};
    int index = 0;
    bool flippedState = flipped; // Local copy to track state changes

    while (true) {
        // Read accelerometer data
        Values v = accelHandler->read();
        readings[index] = v.z;

        // Circular buffer index update
        index = (index + 1) % SAMPLE_COUNT;

        // Compute average Z value
        float sum = 0.0f;
        for (int i = 0; i < SAMPLE_COUNT; i++) {
            sum += readings[i];
        }
        float avg = sum / SAMPLE_COUNT;

        // Flip detection
        if (flippedState) { // Bot is currently marked upside down
            if (avg < -FLIPPED_Z_THRESHOLD) {
                flippedState = false;
                flipped = false;
                Serial.println("FLIPPED! Right side up");
            }
        } else { // Bot is currently marked right side up
            if (avg > FLIPPED_Z_THRESHOLD) {
                flippedState = true;
                flipped = true;
                Serial.println("FLIPPED! Upside down");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


int validate_range(int n, bool is_servo){
  int* range = DEFAULT_PWM_RANGE;
  if(is_servo){
    range = SERVO_RANGE;
  } 
  if(n >= range[0] && n<= range[1]){
    return n;
  } else if(n < range[0]){
    Serial.print("INVALID CHANNEL SIGNAL RECEIVED! received: ");
    Serial.println(n);
    return range[0];
  } else {
    Serial.print("INVALID CHANNEL SIGNAL RECEIVED! received: ");
    Serial.println(n);
    return range[1];
  }
}

bool is_safe_killswitch_change(int v1, int v2, int v3, int mode){
  if(mode==1 || mode==2){
    return abs(v1-CH1_DEFAULT)<=SAFE_VARIANCE && abs(v2-CH2_DEFAULT)<=SAFE_VARIANCE && (mode==1 || abs(v3-CH3_DEFAULT)<=SAFE_VARIANCE);
  } else {
    Serial.print("logic error: is_safe_killswitch_change was given this value as killswitch: ");
    Serial.println(mode);
    return false;
  }
}

void mix_and_write(){
  int forward = ch2 - 1500; //500
  int turn = ch1 - 1500; //500
  int weapon = ch3 - 1500;

  if(FLIPPED_CORRECTION_ENABLED && flipped){ //reverses drive and weapon. Note: turning is always correct and does not need to be flipped
    forward = -forward;

    if(BIDIRECTION_WEAPON){
      weapon = -weapon;
    }
  }

  int left_motor;
  int right_motor;
  int weapon_motor;

  
  // Mixed motor signals
  left_motor = 1500 + forward + turn;
  right_motor = 1500 + forward - turn;
  weapon_motor = 1500 + weapon;
  

  // Clamp to PWM range
  left_motor = constrain(left_motor, 1000, 2000);
  right_motor = constrain(right_motor, 1000, 2000);
  weapon_motor = constrain(weapon_motor, 1000, 2000);


  if(right_motor_reverse){
    right_motor = 2000 - (right_motor-1000);
  }

  if(left_motor_reverse){
    left_motor = 2000 - (left_motor-1000);
  }

  if(MIXING_ENABLED){
    Serial.printf("Motor output (mixed): [%d, %d]\n", right_motor, left_motor);

    setPWM(1, right_motor);
    setPWM(2, left_motor);
    setPWM(3, weapon_motor);
  } else { //unmmixed
    int new_ch1 = turn + 1500;
    int new_ch2 = forward + 1500;
    int new_weapon = weapon + 1500;
    Serial.printf("Ch1 & Ch2 output: [%d, %d]\n", new_ch1, new_ch2);
    
    setPWM(1, new_ch1);
    setPWM(2, new_ch2);
    setPWM(3, new_weapon);

  }

void execute_package(int v1, int v2, int v3, int v4){
  if(v4 != 0 && v4 != 2 && v4 != 1){ //make sure valid killswitch signal is received. If not, activate killswitch and disable bot
      ch1 = CH1_DEFAULT;
      ch2 = CH2_DEFAULT;
      ch3 = CH3_DEFAULT;
      killswitch = 0;
      Serial.print("INVALID KILLSWITCH SIGNAL RECEIVED! received: ");
      Serial.println(v4);
      return;
  }
  v1 = validate_range(v1, false);
  v2 = validate_range(v2, false);
  v3 = validate_range(v3, SERVO_BOT);

  switch(v4){
    case 0: { //killswitch is ON; robot should be immobile
      killswitch=0;
      ch1 = CH1_DEFAULT;
      ch2 = CH2_DEFAULT;
      ch3 = CH3_DEFAULT;
      break;
    }
    case 1: { //limited movement: robot can drive, but weapon is disabled
      if(killswitch == 0){
        //make sure robot is safe to start moving. 
        if(!is_safe_killswitch_change(v1, v2, v3, v4)){
          Serial.println("Robot will not move until drive joystick is at rest");
          return;
        }
        
      }
      killswitch = 1;
      ch1 = v1;
      ch2 = v2;
      ch3 = CH3_DEFAULT;
      break;
    }
    case 2: { //robot is enabled for battle mode
      if(killswitch == 0 || killswitch == 1){
        //make sure robot is safe to start moving. 
        if(!is_safe_killswitch_change(v1, v2, v3, v4)){
          Serial.println("Robot will not move until drive and weapon are at rest");
          return;
        }
        killswitch = 2;
      }
      ch1 = v1;
      ch2 = v2;
      ch3 = v3;
      break;
    }
  }
}

void UDP_packet() {
  int len = 0;
  while (udp.parsePacket()) {
    len = udp.read((uint8_t*)incomingPacket, sizeof(incomingPacket));
  }

  if (len == 8) {
    lastPacketReceived = millis();

    if (!connected) {
      connected = true;
      Serial.println("Connection established; receiving packets");
    }

    if(led_on){
      led_on = !led_on;
      digitalWrite(ONBOARD_LED, LOW);
    } else {
      led_on = !led_on;
      digitalWrite(ONBOARD_LED, HIGH);
    }

    uint16_t* values = (uint16_t*)incomingPacket;
    int v1 = values[0];
    int v2 = values[1];
    int v3 = values[2];
    int v4 = values[3];

    execute_package(v1, v2, v3, v4);
    mix_and_write();

    bool received = true;
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write((uint8_t*)&received, sizeof(received));
    udp.endPacket();
  } else if (connected && millis() - lastPacketReceived >= FAILSAFE_DISCONNECT) {
    connected = false;
    execute_package(CH1_DEFAULT, CH2_DEFAULT, CH3_DEFAULT, 0);
    mix_and_write();
    Serial.println("Connection dropped! Failsafe enabled");
  }
}

void setup_OTA(){
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.setPassword(OTA_PASSWORD); // No password. Put a string in here to add a password
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}

void startAP(){ //begin access point

  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

  // Set Tx power to max
  WiFi.setTxPower(WIFI_POWER_19dBm);
  
  // Optional: print the IP address of the ESP32 AP
  Serial.print("Access Point Started: ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(localPort);
  Serial.println("UDP listening on port " + String(localPort));
}

/*
//connects to Wi-Fi and begins UDP
void connectToWiFi() { 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Set lower WiFi transmit power (e.g., 10 dBm)
  WiFi.setTxPower(WIFI_POWER_20dBm); //lower the transmitter power by 95%. If robots have connection issues, try raising this

  Serial.print("Connecting to WiFi Network ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
    pinMode(ONBOARD_LED, OUTPUT);
    digitalWrite(ONBOARD_LED, HIGH);
    delay(250);
    pinMode(ONBOARD_LED, OUTPUT);
    digitalWrite(ONBOARD_LED, LOW);
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  udp.begin(localPort);
  Serial.println("UDP listening on port " + String(localPort));
} */

void loop() {
  ArduinoOTA.handle(); //checks for incoming OTA programming

  UDP_packet(); //receive latest packet

  //map and write values
  
  delay(10); //power saving
}