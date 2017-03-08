#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

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

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
* connected to this access point to see it.
*/
void handleRoot() {

	content = "<html><head><style>table, th, td{ border: 1px solid black; width: 500px;}</style></head><body bgcolor='#66ffff'>";	
	content += "<h1>IJ Temperature Device:</h1>";
	content += "</br>";
	content += "Temperatura = ";	
	content += bmp.readTemperature();
	content += " Celsius";
	content += "</br>";
	content += "Pressure = ";
	content += bmp.readPressure();
	content += " Pascal";

	content += "<table>";
	content += "<tr><th>Network</th><th>Password</th></tr>";
	content += netList;
	content += "</table>";

	content += "</body></html>";
	server.send(200, "text/html", content);
}

void connectToWifi() {

	String stip = server.arg("wifi");
	String stpsw = server.arg("password");
	Serial.println("wifi: " + stip);
	Serial.println("password: " + stpsw);

	content = "<!DOCTYPE HTML>\r\n<html>";
	content += "<p>Connect to Newtwork <br> </p>";
	
	content += stip;
	content += ":";
	content += stpsw;
	content += "</br>";

	content += "<a href='/'>Вернуться к выбору сети</a>";
	content += "</html>";
	server.send(200, "text/html", content);
}
void setup() {

	bmp.begin();
	if (!bmp.begin())
	{
		Serial.println("Could not find a valid BMP085 sensor, check wiring!");
		while (1)
		{
		}
	}
	delay(1000);
	Serial.begin(115200);
	Serial.println();
	Serial.print("Configuring access point...");
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid, password);
	WiFi.mode(WIFI_AP_STA);

	myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
	server.on("/", handleRoot);
	server.on("/connect", connectToWifi);
	server.begin();
	Serial.println("HTTP server started");
}

void loop() {
	i++;
	Serial.println(i);
	scanWifiNetworks();
	server.handleClient();
	delay(15000);
}

void scanWifiNetworks() {

	Serial.println(myIP);

Serial.print("Scan start ... ");
	int n = WiFi.scanNetworks();
	Serial.print(n);
	Serial.println(" network(s) found");
	
	netList = "";
	
	for (int i = 0; i < n; i++)
	{
		//Serial.println(WiFi.SSID(i));

		netList += "<tr>";
		netList += "<td>";
		netList += WiFi.SSID(i).c_str();
		netList += "</td>";

		netList += "<td>";
		

		netList += "<form action = '/connect'>";
		netList += "<input type = 'hidden' name = 'wifi' value='";
		netList += WiFi.SSID(i).c_str();	
		netList += "'>";
		netList += "<input type = 'text' name = 'password'>";
		netList += "<input type = 'submit' value = 'Подключиться'>";
		netList += "</form>";

		
		netList += "</td>";
		netList += "</tr>";

	}

	Serial.println();
}