#include <ESP8266WiFi.h>
#include <stdlib.h>



// Daten fÃ¼r den Netzwerkzugriff
char ssid[32] = "WLAN-SSID";
char pwd[64] = "WLAN-Passwort";
char host[32] = "Wetterstation1";

// IP-Adresse oder Domain des Webservers
const char* server = "192.168.0.100";

// Pfad auf dem Webserver (URL)
const char* script = "/wx/wlan.php?t=";

WiFiClient client;


void setup() {
  Serial.begin(9600);

  // Setze Netzwerkinformationen
  WiFi.hostname(host);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pwd);

  // Verbinde mit dem Netzwerk
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  Serial.println("Moin!");

  // Teste, ob das Netzwerk funktioniert
  WiFiClient client;
  if (!client.connect(server,80)) {
    Serial.println("HTTP-Verbindung geht nicht");
  }

  // Setze den Port fÃ¼r den Temperatursensor
  dht.setup(2); // data pin 2

  readdht();
}

void readdht() {
  delay(dht.getMinimumSamplingPeriod());

  // Lese Daten aus dem 
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  const char* dhtstatus = dht.getStatusString();
  float heat = dht.computeHeatIndex(temperature, humidity, false);

  // Verbinde mit dem Webserver
  WiFiClient client;
  const int httpPort = 80;
  int erg;
  do
  {
    erg = client.connect(server, httpPort);
  } while (erg!=1);

  // Baue URL-String zusammen und setze Messwerte ein
  String url = script;
  url += temperature;
  url += "&f=";
  url += humidity;
  url += "&s=";
  url += dhtstatus;
  url += "&e=";
  url += heat;
  url += "&id=1";

  // Baue HTTP-Verbindung auf
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" + "Connection: close\r\n\r\n");

  // Lies ein, was der Webserver zurÃ¼cksendet. Dieses Ergebnis
  // kÃ¶nnte verwendet werden, um zu prÃ¼fen, ob die Ãœbertragung
  // der Messwerte funktioniert hat.
  while(client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  client.stop();
}



void loop() {

  // Lege den Sensor schlafen fÃ¼r 5 Minuten
  ESP.deepSleep(300000000, WAKE_RF_DISABLED); // entspricht 300 s

  // Wenn der Sensor nicht schlafen gelegt werden soll, 
  // kann hiermit auch einfach 5 Minuten gewartet werden.
  delay(300000);
  readdht();
}


