/*****************
 * 
 * 
 * 
 * 
 */
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
void init_wifi(String ssid,String password);

char ssid[] = "PS2H";
char password[] = "20150423";


void setup() {
  Serial.begin(115200);
  init_wifi("PS2H","20150423");
}

void loop() {
  // put your main code here, to run repeatedly:

}

void init_wifi(String ssid,String password){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP().toString());
}

