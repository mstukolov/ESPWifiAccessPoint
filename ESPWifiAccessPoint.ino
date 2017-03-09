#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <EEPROM.h>


#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;


/* Set these to your desired credentials. */
const char *ssid = "IJnewDev";
const char *password = "12345678";

ESP8266WebServer server(80);

//Connect BMP180
#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

String content;
String temperatura;

String netList;
int i = 0;

IPAddress myIP;

int MODE; // Режим загрузки


/* Just a little test message.  Go to http://192.168.4.1 in a web browser
* connected to this access point to see it.
*/
void handleRoot() {

	content = "<html><head><style>table, th, td{ border: 1px solid black; width: 500px;}</style></head><body bgcolor='#66ffff'>";	
	content += "<h1>IJ Temperature Device:(MODE=";
	content += EEPROM.read(0);
	content += ")</h1>";
		
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

	content += "<table>";
	content += "<tr><th>Network</th><th>Password</th><th>Action</th></tr>";
	content += netList;
	content += "</table>";

	content += "</body></html>";
	server.send(200, "text/html", content);
}

void rebootDevice() {
	Serial.println("Device will be reconnected");
	
	WiFiMulti.addAP("Maks", "carter2017!");
	Serial.print("checking wifi...");
	while (WiFiMulti.run() != WL_CONNECTED) {
		Serial.print(".");
		delay(500);
	}
	Serial.println("Local WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
	
	String msg = "<h1>Device is reconnected. New IP: ";
	msg += ipToString(WiFi.localIP());
	msg += "</h1>";
	delay(1000);
	server.send(200, "text/html", msg);
}

void connectToWifi() {

	String stip = server.arg("wifi");
	String stpsw = server.arg("password");

	char wifi[50];
	char pwd[50];
	stip.toCharArray(wifi, 50);
	stpsw.toCharArray(pwd, 50);

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
	if (EEPROM.read(0) == 0) { EEPROM.write(0, 1); EEPROM.commit(); }
	//delay(5000);
	MODE = EEPROM.read(0); // определяем режим загрузки   читаем 0 байт EEPROM- если 0 - режим конфигурации если 1 -рабочий режим
	
	Serial.print("Started mode:");
	Serial.println(EEPROM.read(0));

	if (EEPROM.read(0) == 1)
	{
		Serial.println("DEVICE MODE = 1");
		Serial.print("Configuring access point...");
		WiFi.softAP(ssid, password);
		WiFi.mode(WIFI_AP_STA);

		myIP = WiFi.softAPIP();
		Serial.print("AP IP address: ");
		Serial.println(myIP);
		server.on("/", handleRoot);
		server.on("/connect", connectToWifi);
		server.on("/scanwifi", scanNearWifiNetworks);
		server.on("/reboot", rebootDevice);
		server.begin();
		Serial.println("HTTP server started");
	}

	if (EEPROM.read(0) == 0)
	{
		Serial.println("DEVICE MODE = 0");
		Serial.println("Connecting to Local Wifi Network");
	}

	bmp.begin();
	if (!bmp.begin())
	{
		Serial.println("Could not find a valid BMP085 sensor, check wiring!");
		while (1)
		{
		}
	}
	delay(1000);
	Serial.println();
		
}

void loop() {
	
	server.handleClient();
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