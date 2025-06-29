// Use "ESP32 Dev Module" as board

#include "WiFi.h"
#include "nimbleCon.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"

bool btLED = 0;

// Set your Static IP address
IPAddress local_IP(192, 168, 8, 10);
// Set your Gateway IP address
IPAddress gateway(192, 168, 8, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(1, 1, 1, 1);   // optional
IPAddress secondaryDNS(1, 0, 0, 1); // optional

// Replace with your network credentials (STATION)
const char* ssid = "The Strokes";
const char* password = "benimble";

unsigned long previousMillis = 0;
unsigned long interval = 30000;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Set web server port number to 80
//WiFiServer server(80);

// Variable to store the HTTP request
String header;

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void serveWebserverHomepage(WiFiClient& client) {
    //HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
    //and a content-type so the client knows what's coming, then a blank line:
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();

    // Display the HTML web page
    client.println("<!DOCTYPE html><html>");
    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<link rel=\"icon\" href=\"data:,\">");
    // CSS to style the on/off buttons
    // Feel free to change the background-color and font-size attributes to fit your preferences
    client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
    client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
    client.println(".button2 {background-color: #555555;}</style></head>");

    // Web Page Heading
    client.println("<body><h1>ESP32 Web Server</h1>");

    //The HTTP response ends with another blank line
   client.println();
}

Pendant readPayload(WiFiClient& client) {
    Pendant pendant = {};
    uint8_t payload[9];  // Expect exactly 9 bytes (4 for position, 4 for force, 1 for flags)
    int bytesRead = 0;

    while (bytesRead < sizeof(payload) && client.available()) {
        payload[bytesRead++] = client.read();
    }

    // Convert received bytes into 32-bit integers (assuming little-endian)
    int32_t posCmd;
    int32_t forceCmd;
    memcpy(&posCmd, &payload[0], 4);
    memcpy(&forceCmd, &payload[4], 4);

    //int32_t forceCmd = payload[4] | (payload[5] << 8) | (payload[6] << 16) | (payload[7] << 24);
    uint8_t flags = payload[8];


    // Assign to pendant structure
    actuator.positionCommand = posCmd;
    actuator.forceCommand = forceCmd;
    actuator.activated = true;
    actuator.airOut = flags & 0x01;
    actuator.airIn  = flags & 0x02;

    // for(int i =0; i<sizeof(payload); i++) {
    //   Serial.print(payload[i]);
    //   Serial.print(" ");
    // }

    // Serial.println();
    // Serial.print("position command: ");
    // Serial.println(actuator.positionCommand);
    // Serial.print("force command: ");
    // Serial.println(actuator.forceCommand);

    return pendant;
}


// void setup() {
//   // put your setup code here, to run once:
  
//   Serial.begin(115200);

//   // Configures static IP address
//   if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
//     Serial.println("STA Failed to configure");
//   }

//   initWiFi();
//   Serial.print("RRSI: ");
//   Serial.println(WiFi.RSSI());

//   server.begin();

//   initNimbleSDK();

//   /*ledcWrite(8, 50);
//   ledcWrite(9, 50);
//   ledcWrite(10, 50);
//   ledcWrite(11, 50);*/
// }

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  // Define WebSocket event handler
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                AwsEventType type, void *arg, uint8_t *data, size_t len) {
    Serial.println("Saw event");
    if (type == WS_EVT_CONNECT) {
      Serial.println("WebSocket client connected");
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.println("WebSocket client disconnected");
    } else if (type == WS_EVT_DATA) {
      if (len == 9) {
        actuator.positionCommand = *(int32_t*)&data[0];
        actuator.forceCommand = *(int32_t*)&data[4];
        uint8_t flags = data[8];
        actuator.airOut = flags & 0x01;
        actuator.airIn  = flags & 0x02;

        // Debug output
        // Serial.printf("[WS] Pos: %ld, Force: %ld, Flags: 0x%02X\n",
        //               actuator.positionCommand, actuator.forceCommand, flags);
      }
    }
  });

  // Add WebSocket to server
  server.addHandler(&ws);
  server.begin();
  Serial.println("WebSocket server started");
  Serial.println(WiFi.localIP().toString() + "/ws");
}

void loop() {
  // put your main code here, to run repeatedly:
  
  unsigned long currentMillis = millis();
  
  // // if WiFi is down, try reconnecting
  // if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
  //   Serial.print(millis());
  //   Serial.println("Reconnecting to WiFi...");
  //   WiFi.disconnect();
  //   WiFi.reconnect();
  //   previousMillis = currentMillis;
  // }

  // WiFiClient client = server.available();   // Listen for incoming clients

  // if (client) {                             // If a new client connects,

  //   //Serial.println("client connected");
  //   currentTime = millis();
  //   previousTime = currentTime;
  //   //Serial.println("New Client.");          // print a message out in the serial port
  //   String currentLine = "";                // make a String to hold incoming data from the client

  //   while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
  //     currentTime = millis();

  //     if (client.available()) {             // if there's bytes to read from the client,
  //       char c = client.read();             // read a byte, then
  //       header += c;                        // save, but for now ignore the header
  //       if (c == '\n') {                    // if the byte is a newline character
  //         // if the current line is blank, you got two newline characters in a row.
  //         // that's the end of the client HTTP request, so send a response:
  //         if (currentLine.length() == 0) {
  //           Pendant inputPendant = readPayload(client);
  //           //Serial.printf("Received position: 0x%04X, force: 0x%04X\n", inputPendant.positionCommand, inputPendant.forceCommand);
            
  //           serveWebserverHomepage(client);

  //           // Break out of the while loop
  //           break;
  //         } else { // if you got a newline, then clear currentLine
  //           currentLine = "";
  //         }
  //       } else if (c != '\r') {  // if you got anything else but a carriage return character,
  //         currentLine += c;      // add it to the end of the currentLine
  //       }
  //     }
  //   }

  //   // Clear the header variable
  //   header = "";
  //   // Close the connection
  //   client.stop();
  //   // Serial.println("Client disconnected.");
  //   // Serial.println("");
  // }

  // ***************** Do stuff to the values to be sent below this line. Use no delays.
  
  // // Check actuator and pendant serial ports for complete packets and update structs.
  // if(readFromPend())  // Read values from pendant. If the function returns true, the values were updated so update the pass-through values.
  // { // DEMO: Pass through data from pendant to actuator
  //   actuator.positionCommand = pendant.positionCommand;
  //   actuator.forceCommand = pendant.forceCommand;
  //   actuator.airIn = pendant.airIn;
  //   actuator.airOut = pendant.airOut;
  //   actuator.activated = pendant.activated;
  // }
  
  readFromAct(); // Read values from actuator. If the function returns true, the values were updated. Otherwise there was nothing new.

  // // This DEMO code pauses the actuator (in a very crude way) when the encoder button is pressed (it will jump to whatever position the pendant is commanding at the moment the button is released)
  // if(digitalRead(ENC_BUTT)) // Encoder button reads low when pressed.
  // {
  //   driveLEDs(encoder.getCount());  // Show LEDs as demo
  // }else
  // {
  //   driveLEDs(0);   // Blank LEDs when button is pressed
  //   actuator.forceCommand = 0;  // Set force command to 0 when button is pressed.
  // }

// ***************** Do stuff to the values to be sent above this line. Use no delays.

  // Check if it's time to send a packet.
  if(checkTimer()) sendToAct();
  
  // pendant.present ? ledcWrite(PEND_LED, 50) : ledcWrite(PEND_LED, 0);  // Display pendant connection status on LED.
  // actuator.present ? ledcWrite(ACT_LED, 50) : ledcWrite(ACT_LED, 0);  // Display actuator connection status on LED.
}
