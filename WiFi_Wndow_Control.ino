#include <DHT.h>
#include <ESP8266WiFi.h>

// Define DHT sensor type and pin
#define DHTPIN 2    // GPIO2 (D4)
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Define motor control pins for L298N
#define IN1 5    // GPIO5 (D1)
#define IN2 4    // GPIO4 (D2)

// Wi-Fi credentials
const char* ssid = "...";       // Replace with your Wi-Fi name
const char* password = "...";       // Replace with your Wi-Fi password

WiFiServer server(80);  // Web server on port 80

void setup() {
  // Start serial communication
  Serial.begin(9600);
  delay(1000);

  // Initialize DHT sensor
  dht.begin();

  // Initialize motor control pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  stopMotor();

  // Connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start the web server
  server.begin();
  Serial.println("Web Server Started");
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New Client Connected");
    String currentLine = "";

    // Read the HTTP request
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          // Send the HTTP response
          if (currentLine.length() == 0) {
            sendWebPage(client);
            break;
          } else {
            if (currentLine.startsWith("GET /open")) {
              Serial.println("Opening window...");
              openWindow();
            } else if (currentLine.startsWith("GET /close")) {
              Serial.println("Closing window...");
              closeWindow();
            }
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client Disconnected");
  }
}

// Function to send the web page
void sendWebPage(WiFiClient& client) {
  // Read temperature
  float temperature = dht.readTemperature();

  // Start sending the HTML response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<h1>Temperature and Motor Control</h1>");

  // Display temperature
  if (isnan(temperature)) {
    client.println("<p>Failed to read from DHT sensor!</p>");
  } else {
    client.print("<p>Temperature: ");
    client.print(temperature);
    client.println(" °C</p>");
  }

  // Add control buttons
  client.println("<button onclick=\"location.href='/open'\">Open</button>");
  client.println("<button onclick=\"location.href='/close'\">Close</button>");
  client.println("</html>");
}

// Function to open the window
void openWindow() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

// Function to close the window
void closeWindow() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

// Function to stop the motor
void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}
