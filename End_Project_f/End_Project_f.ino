#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

#define TELEGRAM_BUTTON_PIN D5
#define SIGNAL_PIN D6

#define CHAT_0_BUTTON D1
#define CHAT_0_BUTTON_LED D2
#define CHAT_1_BUTTON D3
#define CHAT_1_BUTTON_LED D4
#define MSG_0_BUTTON D5
#define MSG_0_BUTTON_LED D6
#define MSG_1_BUTTON D7
#define MSG_1_BUTTON_LED D8

//define your default values here, if there are different values in config.json, they are overwritten.
char bot_token[60] = "551025608:AAEiyekDcm7rfPIyBAfevEFnNlhzr2t2SRE";
char chat_id_0[20] = "-221709366";
char chat_id_1[20] = "-279773367";
char message_id_0[50] = "Hi there:) call me;)";
char message_id_1[50] = "Hi there:) What's UP?";
WiFiManager wifiManager;


//flag for saving data
bool shouldSaveConfig = false;

//Objects
WiFiClientSecure client;
volatile bool M0PressedFlag = false;
volatile bool M1PressedFlag = false;
volatile bool C0PressedFlag = false;
volatile bool C1PressedFlag = false;


//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  //clean FS, for testing
  //SPIFFS.format();
  Serial.println("Setup started:");


  //read configuration from FS json
  Serial.println("pins init and mounting FS...");
  pinMode(CHAT_0_BUTTON, INPUT);
  pinMode(CHAT_1_BUTTON, INPUT);
  pinMode(MSG_0_BUTTON, INPUT);
  pinMode(MSG_1_BUTTON, INPUT);
  pinMode(CHAT_0_BUTTON_LED, OUTPUT);
  pinMode(CHAT_1_BUTTON_LED, OUTPUT);
  pinMode(MSG_0_BUTTON_LED, OUTPUT);
  pinMode(MSG_1_BUTTON_LED, OUTPUT);
  
  attachInterrupt(CHAT_0_BUTTON, C0Pressed, RISING);
  attachInterrupt(CHAT_1_BUTTON, C1Pressed, RISING);
  attachInterrupt(MSG_0_BUTTON, M0Pressed, RISING);
  attachInterrupt(MSG_1_BUTTON, M1Pressed, RISING);
  
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(bot_token, json["bot_token"]);
          strcpy(chat_id_0, json["chat_id_0"]);
          strcpy(message_id_0, json["message_id_0"]);
          strcpy(chat_id_1, json["chat_id_1"]);
          strcpy(message_id_1, json["message_id_1"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read



  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_bot_token("bot_token", "Bot Token", bot_token, 60);
  WiFiManagerParameter custom_chat_id_0("chat_id_0", "Chat 1", chat_id_0, 20);
  WiFiManagerParameter custom_message_id_0("message_id_0", "Message 1", message_id_0, 50);
  WiFiManagerParameter custom_chat_id_1("chat_id_1", "Chat 2", chat_id_1, 20);
  WiFiManagerParameter custom_message_id_1("message_id_1", "Message 2", message_id_1, 50);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  //wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  
  //add all your parameters here
  wifiManager.addParameter(&custom_bot_token);
  wifiManager.addParameter(&custom_chat_id_0);
  wifiManager.addParameter(&custom_message_id_0);
  wifiManager.addParameter(&custom_chat_id_1);
  wifiManager.addParameter(&custom_message_id_1);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...");
 
  //read updated parameters
  strcpy(bot_token, custom_bot_token.getValue());
  strcpy(chat_id_0, custom_chat_id_0.getValue());
  strcpy(message_id_0, custom_message_id_0.getValue());
  strcpy(chat_id_1, custom_chat_id_1.getValue());
  strcpy(message_id_1, custom_message_id_1.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["bot_token"] = bot_token;
    json["chat_id_0"] = chat_id_0;
    json["message_id_0"] = message_id_0;
    json["chat_id_1"] = chat_id_1;
    json["message_id_1"] = message_id_1;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }



  Serial.print("local ip:");
  Serial.println(WiFi.localIP());

}

void loop() {
    if ( M0PressedFlag ) 
    {
      if( C0PressedFlag){
        sendTelegramMessage(chat_id_0,message_id_0);
      }
      if( C1PressedFlag){
        sendTelegramMessage(chat_id_1,message_id_0);
      }
    }
    if ( M1PressedFlag ) 
    {
      if( C0PressedFlag){
        sendTelegramMessage(chat_id_0,message_id_1);
      }
      if( C1PressedFlag){
        sendTelegramMessage(chat_id_1,message_id_1);
      }
    }
}
void setMSG_LED(int val){
  digitalWrite(MSG_0_BUTTON_LED,val);
  digitalWrite(MSG_1_BUTTON_LED,val);
}
void setCHAT_LED(int val){
  digitalWrite(CHAT_0_BUTTON_LED,val);
  digitalWrite(CHAT_1_BUTTON_LED,val);
}

void C0Pressed() {
  Serial.println("Chat 1 Selected");
  setMSG_LED(HIGH);
  int button = digitalRead(CHAT_0_BUTTON);
  if(button == HIGH)
  {
    C0PressedFlag = true;
  }
  return;
}

void C1Pressed() {
  Serial.println("Chat 2 Selected");
  setMSG_LED(HIGH);
  int button = digitalRead(CHAT_1_BUTTON);
  if(button == HIGH)
  {
    C1PressedFlag = true;
  }
  return;
}


void M0Pressed() {
  Serial.println("Message 1 Selected");
  setCHAT_LED(HIGH);
  int button = digitalRead(MSG_0_BUTTON);
  if(button == HIGH)
  {
    M0PressedFlag = true;
  }
  return;
}

void M1Pressed() {
  Serial.println("Message 2 Selected");
  setCHAT_LED(HIGH);
  int button = digitalRead(MSG_1_BUTTON);
  if(button == HIGH)
  {
    M1PressedFlag = true;
  }
  return;
}

void sendTelegramMessage(String chat_id, String message) {
  UniversalTelegramBot bot(bot_token, client);
  if(bot.sendMessage(chat_id, message, "Markdown")){
    Serial.println("TELEGRAM Successfully sent");
    setCHAT_LED(LOW);
    setMSG_LED(LOW);
  }
  M0PressedFlag = false;
  M1PressedFlag = false;
  C0PressedFlag = false;
  C1PressedFlag = false;
}
