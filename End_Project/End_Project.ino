/*****************
 * Project description 
 */
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
void init_wifi(String ssid,String password);

/*********
 * Configuration 
 */
#define TELEGRAM_BUTTON_PIN D5
#define SIGNAL_PIN D6
#define BOT_TOKEN "551025608:AAEiyekDcm7rfPIyBAfevEFnNlhzr2t2SRE"  // Telegramm BOT token - BOT Father
#define CHAT_ID "-221709366" // Chat ID of where the message will go
char ssid[] = "MTA-Students";
char password[] = "";  

/******
 * Objects
 */
 WiFiClientSecure client;
 UniversalTelegramBot bot(BOT_TOKEN, client);
 volatile bool telegramButtonPressedFlag = false;
 int messageID = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TELEGRAM_BUTTON_PIN, INPUT);
  pinMode(SIGNAL_PIN, OUTPUT);
  digitalWrite(SIGNAL_PIN, HIGH);
  attachInterrupt(TELEGRAM_BUTTON_PIN, telegramButtonPressed, RISING);
  //init_wifi("PS2H","20150423");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  //digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
}

void loop() {
  if ( telegramButtonPressedFlag ) {
    sendTelegramMessage();
  }
}

void init_wifi(String ssid,String password){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  //WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  //digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP().toString());
}

void telegramButtonPressed() {
  Serial.println("telegramButtonPressed");
  int button = digitalRead(TELEGRAM_BUTTON_PIN);
  if(button == HIGH)
  {
    telegramButtonPressedFlag = true;
  }
  return;
}

void sendTelegramMessage() {
  String message = "Message ID:";
  //String t = messageID;
  //message.concat(t);
  //messageID++;
  //"SSID:  ";
  //message.concat(ssid);
  //message.concat("\n");
  //message.concat("IP: ");
  //message.concat(ipAddress);
  //message.concat("\n");
  if(bot.sendMessage(CHAT_ID, message, "Markdown")){
    Serial.println("TELEGRAM Successfully sent");
  }
  telegramButtonPressedFlag = false;
}


