#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pca9685 = Adafruit_PWMServoDriver(0x40);

#define SERVOMIN 94
#define SERVOMAX 500

#define SER0 0

// Wifi Setup
const char* ssid = "Wolffden";
const char* password = "wolffresidence1322";
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Decode HTTP GET value
String valueString = String(5);
int pos1 = 0;
int pos2 = 0;

// Current Time
unsigned long currentTime = millis();
// Previous Time
unsigned long previousTime = 0;
// Define timeout time in milliseconds
const long timeoutTime = 2000;

int servoMinimums[12] = {94};
int servoMaximums[12] = {500};
float servoConversions[12] = {0};
int sleepAngle[12] = {90, 150, 0, 90, 30, 180, 90, 150, 0, 90, 30, 180};

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connectiong to WiFi ..");
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }

  Serial.println(WiFi.localIP());
  server.begin();
}


int pwm0;

void setup() {
  Serial.begin(115200);
  initWiFi();

  Serial.println("PCA9685 Servo Test");

  bool wireBeginRet = Wire.begin(20,21);
  Serial.println("Wire Begin " + wireBeginRet);
  pca9685.begin();

  pca9685.setPWMFreq(50);

}

void loop() {

  // Listen for incoming clients
  WiFiClient client = server.available();   
  
  // Client Connected
  if (client) 
  {                             
    // Set timer references
    currentTime = millis();
    previousTime = currentTime;
    
    // Print to serial port
    Serial.println("New Client."); 
    
    // String to hold data from client
    String currentLine = ""; 
    
    // Do while client is cponnected
    while (client.connected() && currentTime - previousTime <= timeoutTime) 
    { 
      currentTime = millis();
      if (client.available()) 
      {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n')                    // if the byte is a newline character
        {
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {        
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK) and a content-type
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
 
            // Display the HTML web page
            
            // HTML Header
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            // CSS - Modify as desired
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto; }");
            client.println("table, th, td { border: 1px solid black; }");
            client.println(".slider { -webkit-appearance: none; width: 300px; height: 25px; border-radius: 10px; background: #ffffff; outline: none;  opacity: 0.7;-webkit-transition: .2s;  transition: opacity .2s;}");
            client.println(".slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; border-radius: 50%; background: #ff3410; cursor: pointer; }</style>");
            
            // Get JQuery
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");
                     
            // Page title
            client.println("</head><body style=\"background-color:#70cfff;\">");

            // Table Setup
            client.println("<table> <tr> <th>Servo #</th> <th>Minimum PWM</th> <th>Maximum PWM</th> <th>Position</th> <th>Adjustment</th> </tr>");

            for(int servoNumber = 0; servoNumber < 12; servoNumber++)
            {
              client.println("<tr>");
              client.print("<td> Servo ");
              client.print(servoNumber+1);
              client.println("</td>");
              client.print("<td>");
              client.print(<input servoMinimums[servoNumber]);
              client.println("</td>");
              client.print("<td>");
              client.print(servoMaximums[servoNumber]);
              client.println("</td>");

              client.print("<td>");
              // Position display1
              client.print("<h2 style=\"color:#ffffff;\">Position: <span id=\"servoPos" + String(servoNumber) + "\"></span>&#176;</h2>");
              client.println("</td>");

              client.print("<td>");
              // Slider control
              client.print("<input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" id=\"servoSlider" + String(servoNumber) + "\" onchange=\"servo(this.value)\" value=\""+valueString+"\"/>");
              client.println("</td>");

              client.println("</tr>");
            }

            client.println("</table>");
            
            
            //<h1 style=\"color:#ff3410;\">Servo Control</h1>");
            
             
                     
            
            
            // Javascript
            client.println("<script>");
            client.println("document.addEventListener(\"DOMContentLoaded\", function() {");
            client.println("const sliders = document.querySelectorAll(\".slider\");");

            client.println("sliders.forEach((slider, index) => {");
            client.println("const positionSpan = document.getElementById(\"servoPos\" + index);");

                  // Set initial position
            client.println("positionSpan.innerHTML = slider.value;");

                  // Update position display and send value on input
            client.println("slider.oninput = function () {");
            client.println("positionSpan.innerHTML = this.value;");
            client.println("servo(index, this.value);");   // Pass servo index and value
            client.println("};");
            client.println("});");

            client.println("function servo(servoNumber, pos) {");
            client.println("$.get(\"/?servo=\" + servoNumber + \"&value=\" + pos);");
            client.println("}");
            client.println("});");
            
            // client.println("var servoP = document.getElementById(\"servoPos\"); servoP.innerHTML = slider.value;");
            // client.println("slider.oninput = function() { slider.value = this.value; servoP.innerHTML = this.value; }");
            // client.println("$.ajaxSetup({timeout:1000}); function servo(pos) { ");
            // client.println("$.get(\"/?value=\" + pos + \"&\"); {Connection: close};}</script>");
            client.println("</script>");
            
            // End page
            client.println("</body></html>");     
            
            // GET data
            if(header.indexOf("GET /?servo=")>=0)
            {
              int servoIndexStart = header.indexOf("servo=") + 6;
              int servoIndexEnd = header.indexOf("&", servoIndexStart);
              String servoNumber = header.substring(servoIndexStart, servoIndexEnd);
              int servoNum = servoNumber.toInt();

              int valueIndexStart = header.indexOf("value=") + 6;
              int valueIndexEnd = header.indexOf(" ", valueIndexStart);
              String valueStr = header.substring(valueIndexStart, valueIndexEnd);
              int value = valueStr.toInt();

              
              pwm0 = map(value, 0, 180, SERVOMIN, SERVOMAX);
              pca9685.setPWM(servoNum, 0, pwm0);

              Serial.print("Servo ");
              Serial.print(servoNum);
              Serial.print(" set to value ");
              Serial.print(value);
              Serial.print(" (PWM: ");
              Serial.print(pwm0);
              Serial.println(")"); 
            }         
            // The HTTP response ends with another blank line
            client.println();
            
            // Break out of the while loop
            break;
          
          }
          else
          { 
            // New lline is received, clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')  // if you got anything else but a carriage return character,
        {
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  // for(int posDegrees = 0; posDegrees <= 180; posDegrees++)
  // {
  //   pwm0 = map(posDegrees, 0, 180, SERVOMIN, SERVOMAX);
  //   pca9685.setPWM(SER0, 0, pwm0);
  //   Serial.print("Motor 0 = ");
  //   Serial.println(posDegrees);
  //   delay(300);
  // }

  // for(int posDegrees = 180; posDegrees >= 0; posDegrees--)
  // {
  //   pwm0 = map(posDegrees, 0, 180, SERVOMIN, SERVOMAX);
  //   pca9685.setPWM(SER0, 0, pwm0);
  //   Serial.print("Motor 0 = ");
  //   Serial.println(posDegrees);
  //   delay(300);
  // }

  //delay(90);
}
