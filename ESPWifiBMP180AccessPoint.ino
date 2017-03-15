#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "user_interface.h"
#include "ets_sys.h"
#include "osapi.h"
#include <ESP8266WiFiMulti.h>
//Подключение mqtt-библиотеки
#include <MQTTClient.h>

//Connect BMP180
#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;
//Connect Wifi module
ESP8266WiFiMulti WiFiMulti;


WiFiClientSecure net; // sec
ESP8266WebServer server(80);

//Параметры подключения к IBM Bluemix
MQTTClient client;
#define ORG "kwxqcy" // имя организации
#define DEVICE_TYPE "BMP180" // тип устройства
#define DEVICE_NAME = ""; // тип устройства


#define TOKEN "12345678" // - задаешь в IOT хабе
char mqttserver[] = ORG ".messaging.internetofthings.ibmcloud.com"; // подключаемся к Bluemix
char topic[] = "iot-2/evt/status/fmt/json";

char authMethod[] = "use-token-auth";
char token[] = TOKEN;

String clientID;
String deviceID;

char  cID[100];
boolean isBluemixConnected = false;
//----------------------------------------------


String content;
String temperatura;

String netList;
int i = 0;

IPAddress myIP;

int MODE; // Режим загрузки

const int sleepTimeS = 10;

//Функция проверяющая доступность сервиса IBM с подключенным внешнем wifi
void testInternetConnection() {
	const char* host = "kwxqcy.messaging.internetofthings.ibmcloud.com";

	Serial.print("connecting to ");
	Serial.println(host);

	// Use WiFiClient class to create TCP connections
	WiFiClient client;
	const int httpPort = 80;
	if (!client.connect(host, httpPort)) {
		Serial.println("connection failed");
		server.send(200, "text/html", "<h3>Связь с сервером IBM Отсутствует....</h3><br/><a href='/'>Вернуться назад</a>");
		return;
	}

	// We now create a URI for the request
	//String url = "/stan";
	String url = "/";


	Serial.print("Requesting URL: ");
	Serial.println(url);

	// This will send the request to the server
	client.print(String("GET ") + url + " HTTP/1.1\r\n" +
		"Host: " + host + "\r\n" +
		"Connection: close\r\n\r\n");
	delay(10);

	// Read all the lines of the reply from server and print them to Serial
	Serial.println("Respond:");
	while (client.available()) {
		String line = client.readStringUntil('\r');
		Serial.print(line);
		server.send(200, "text/html", "<h3>Связь с сервером IBM подтверждена!</h3><br/><a href='/'>Вернуться назад</a>");
	}
	Serial.println();
	Serial.println("closing connection");
}
//Функция подключения к сервису IBM Bluemix
void connectToBluemix() {

	//String chip = ESP.getChipId();

	clientID = "d:" ORG ":" DEVICE_TYPE ":" "ESP8266-";

	clientID.toCharArray(cID, 50);
	client.begin(mqttserver, 8883, net);
	Serial.println("\nConnecting to IBM Bluemix Client=");
	Serial.println(clientID);
	Serial.println("\n");

	while (!client.connect(cID, authMethod, token)) {
		Serial.print("+");
		delay(1000);
	}
	Serial.println("\nConnected to IBM Bluemix!");
	isBluemixConnected = true;
}
//Функция формирования сообщения в формате JSON для отправки к IBM Bluemix
String buildMqttMessage(float param1, int32_t param2, String DeviceID)
{
	String pl = "{ \"d\" : {\"deviceid\":\"";
	pl += DeviceID;
	pl += "\",\"param1\":\"";
	pl += param1;
	pl += "\",\"param2\":\"";
	pl += param2;
	pl += "\"}}";
	return pl;
}
void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
	Serial.print("incoming: ");
	Serial.print(topic);
	Serial.print(" - ");
	Serial.print(payload);
	Serial.println();
}
//Функция для отправки данных на IBM Bluemix
void sendSensorDataToBluemix() {
	if (client.connected()) {

		deviceID = "ESP8266-";
		String deviceID1 = "ESP826i6-";
		deviceID1 += ESP.getChipId();

		Serial.print("Attempting send message to IBM Bluemix to: \n");
		Serial.print(deviceID1);
		Serial.print("\n");
		
		String payload = buildMqttMessage(bmp.readTemperature(), bmp.readPressure(), deviceID);
		Serial.println((char*)payload.c_str());
		Serial.println("\n");
		client.publish(topic, (char*)payload.c_str());
		Serial.print("Message was sent to Bluemix...\n");

	}
	else {
		Serial.print("failed, rc=");
		Serial.println(" try again in 5 seconds\n");
		connectToBluemix();
	}

	delay(3000);
}

//Подключение специализированного датчика(sensor)
void connectSensor() {
	
	bmp.begin();
	if (!bmp.begin())
	{
		Serial.println("Could not find a valid BMP085 sensor, check wiring!");
		while (1)
		{
		}
	}
	
}


/* Just a little test message.  Go to http://192.168.4.1 in a web browser
* connected to this access point to see it.
*/
void handleRoot() {

	content = "<html><head><style>table, th, td{ border: 1px solid black; width: 500px;}</style></head><body bgcolor='#B0C4DE'>";	
	content += "<h1>IJ Temperature Device:(MODE=";
	content += EEPROM.read(0);
	content += ")</h1>";
		
	content += "</br>";
	content += "<h3>";
	content += "ESP8266 Chip id =";
	content += ESP.getChipId();
	content += "</br>";
	content += "ESP8266 FlashChipId = ";
	content += ESP.getFlashChipId();
	content += "</br>";
	content += "Device Name= ESP8266-";
	content += ESP.getChipId();
	content += "</br>";
	content += "External IP address= ";
	content += ipToString(WiFi.localIP());
	content += "</br>";
	content += "Соединение с Bluemix= ";
	content += isBluemixConnected;
	content += "</h3>";
	
	content += "</br>";
	content += "Temperatura = ";	
	content += bmp.readTemperature();
	content += " Celsius";
	content += "</br>";
	content += "Pressure = ";
	content += bmp.readPressure();
	content += " Pascal";


	content += "</br>";
	content += "<a href='/reboot'>Reboot Device</a>";
	content += "</br>";
	content += "<a href='/scanwifi'>Сканировать доступные WIFI-сети</a>";
	content += "</br>";
	content += "<a href='/testInternet'>Проверить доступность сервера IBM</a>";
	content += "</br>";
	content += "<a href='/testmqtt'>Отправить тестовое MQTT-сообщение</a>";
	content += "</br>";
	content += "<table>";
	content += "<tr><th>Network</th><th>Password</th><th>Action</th></tr>";
	content += netList;
	content += "</table>";

	content += "</body></html>";
	server.send(200, "text/html", content);
}

void rebootDevice() {
	Serial.println("Device will be rebooted...");
	String msg = "<h1>Device is rebooted.../h1>";
	server.send(200, "text/html", msg);
	//ESP.restart();
	//ESP.reset();
	*((int*)0) = 0;
}

void connectToWifi() {
	Serial.println("Starting to connect wifi...");
	String stip = server.arg("wifi");
	String stpsw = server.arg("password");

	char wifi[50];
	char pwd[50];
	stip.toCharArray(wifi, 50);
	stpsw.toCharArray(pwd, 50);

	Serial.print("New wifi= ");
	Serial.println(wifi);
	Serial.print("New pwd= ");
	Serial.println(pwd);

	WiFiMulti.addAP(wifi, pwd);
	
	Serial.print("checking wifi...");
	while (WiFiMulti.run() != WL_CONNECTED) {
		Serial.print(".");
		delay(500);
	}
	Serial.println("Local WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	content = "<!DOCTYPE HTML>\r\n<html>";
	content += "<p>Connected to Newtwork <br> </p>";
	content += stip;
	content += ":";
	content += stpsw;
	content += "</br>";
	content += "<h1>Device is reconnected to: ";
	content += ipToString(WiFi.localIP());
	content += "</h1>";
	content += "<a href='/'>Вернуться к выбору сети</a>";
	content += "</html>";
	
	server.send(200, "text/html", content);
}

void scanNearWifiNetworks() {
	netList = "";
	scanWifiNetworks();
	handleRoot();
}

void setup() {
	
	Serial.begin(115200);
	
	EEPROM.begin(512);

	for (int i = 0; i < 512; i++) {
		EEPROM.write(i, 0);
	}

	if (EEPROM.read(0) == 0) { EEPROM.write(0, 1); EEPROM.commit(); }

	MODE = EEPROM.read(0); // определяем режим загрузки   читаем 0 байт EEPROM- если 0 - режим конфигурации если 1 -рабочий режим
	
	Serial.printf("ESP8266 Chip id = %08X\n", ESP.getChipId());
	Serial.printf("ESP8266 FlashChipId = %08X\n", ESP.getFlashChipId());
	Serial.printf("ESP8266 ResetInfo = ");
	Serial.println(ESP.getResetInfo());
	Serial.printf("SP8266 VCC = ");
	Serial.println(ESP.getVcc());
	Serial.printf("ESP8266 SDK Version = ");
	Serial.println(ESP.getSdkVersion());
	Serial.printf("ESP8266 CoreVersion = ");
	Serial.println(ESP.getCoreVersion());
	Serial.printf("ESP8266 BootVersion = ");
	Serial.println(ESP.getBootVersion());

	if (EEPROM.read(0) == 1)
	{
		Serial.println("DEVICE MODE = 1");
		Serial.print("Configuring access point...");
		WiFi.softAP(get_WIFI_STA_SSID(), get_WIFI_STA_PWD());
		WiFi.mode(WIFI_AP_STA);

		myIP = WiFi.softAPIP();
		Serial.print("AP IP address: ");
		Serial.println(myIP);
		server.on("/", handleRoot);
		server.on("/connect", connectToWifi);
		server.on("/scanwifi", scanNearWifiNetworks);
		server.on("/reboot", rebootDevice);
		server.on("/testInternet", testInternetConnection);

		server.begin();
		Serial.println("HTTP server started");
	}

	connectSensor();
	connectToBluemix();
	delay(1000);
	Serial.println();		
}

void loop() {
	
	server.handleClient();
	sendSensorDataToBluemix();

	/*
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	delay(3000);
	*/
}

void scanWifiNetworks() {

	Serial.println(myIP);

	Serial.print("Scan start ... ");
	int n = WiFi.scanNetworks();
	Serial.print(n);
	Serial.println(" network(s) found");
	
	for (int i = 0; i < n; i++)
	{
		//Serial.println(WiFi.SSID(i));

		netList += "<tr>";
		netList += "<td>";
		netList += WiFi.SSID(i).c_str();
		netList += "</td>";

		
		netList += "<form action = '/connect'>";
		netList += "<input type = 'hidden' name = 'wifi' value='";
		netList += WiFi.SSID(i).c_str();
		netList += "'>";

		netList += "<td>";
		netList += "<input type = 'text' name = 'password'>";		
		netList += "</td>";

		netList += "<td>";
		netList += "<input type = 'submit' value = 'Подключиться'>";
		netList += "</td>";

		netList += "</form>";

		netList += "</tr>";

	}

	Serial.println();
}

String ipToString(IPAddress ip) {
	String s = "";
	for (int i = 0; i<4; i++)
		s += i ? "." + String(ip[i]) : String(ip[i]);
	return s;
}

char* get_WIFI_STA_SSID() {

	char chipId[11];
	clean(chipId);
	sprintf(chipId, "%d", ESP.getChipId());

	char * str = "ESP8266-";
	strcat(str, chipId);
	return str;
}

char * get_WIFI_STA_PWD() {
	return "12345678";
}

void clean(char *var) {
	int i = 0;
	while (var[i] != '\0') {
		var[i] = '\0';
		i++;
	}
}