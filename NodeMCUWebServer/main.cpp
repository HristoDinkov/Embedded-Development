#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <fauxmoESP.h>


#define lampPin 5
#define buttonPin 4

const char* ssid = "TP-LINK_A498";
const char* password = "to$hkoekotka";

fauxmoESP fauxmo;

ESP8266WebServer server(80);   //Web server object. Will be listening in port 80 (default for HTTP)

bool lamp = false;
bool isPressed = false;

long start = 0;

void setup()
{

    Serial.begin(115200);
    WiFi.begin(ssid, password); //Connect to the WiFi network

    pinMode(lampPin, OUTPUT);
    pinMode(buttonPin, INPUT);
    digitalWrite(lampPin, lamp);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot); //Associate the handler function to the path

    server.onNotFound(handleNotFound);

    server.begin();                                       //Start the server
    Serial.println("Server listening");

    fauxmo.addDevice("Lights");
    fauxmo.addDevice("Lamp");
    fauxmo.onMessage(callback);

}

void loop()
{

    server.handleClient();    //Handling of incoming requests

    fauxmo.handle();

    int button = digitalRead(buttonPin);
    //Serial.println(button);
    if(button == 1 && isPressed == false && millis() - start > 1000)
    {
        start = millis();
        lamp = !lamp;
        digitalWrite(lampPin, lamp);
        isPressed = true;
    }else if(button == 0 && isPressed)
    {
      isPressed = false;
    }

}

void handleRoot()
{
    String lampVar = server.arg("lamp");

    if(lampVar == "false")
    {
        lamp = false;
    }
    else if(lampVar == "true")
    {
        lamp = true;
    }

    digitalWrite(lampPin, lamp);

    server.send(200, "text/plain", "Lamp is: " + lamp);
}

void handleNotFound()
{

    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i=0; i<server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);

}


void callback(uint8_t device_id, const char * device_name, bool state)
{
  Serial.print("Device "); Serial.print(device_name);
  Serial.print(" state: ");
  if (state)
  {
    Serial.println("ON");
  }
  else
  {
    Serial.println("OFF");
  }

  //Switching action on detection of device name

  if ( (strcmp(device_name, "Lights") == 0) || (strcmp(device_name, "Lamp") == 0) )
  {
    if (!state)
    {
      lamp = false;
    }
    else
    {
      lamp = true;
    }
    digitalWrite(lampPin, lamp);
  }
}
