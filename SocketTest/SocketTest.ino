    #include <WiFi.h>
    #include <WebServer.h>
    #include <WebSocketsServer.h>
    

    WebServer server(80);
    WebSocketsServer webSocket = WebSocketsServer(81);
    
    int interval = 10000;
    unsigned long previousMillis = 0;

    void setup() 
    {
      Serial.begin(115200);

      WiFi.begin("Wolffden", "wolffresidence1322");
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
      }
      Serial.println("Connected to WiFi");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      server.begin();
      webSocket.begin();
    }

    
    void loop() {
      server.handleClient();
      webSocket.loop();

      unsigned long now = millis();
      if(now - previousMillis > interval)
      {
        String str = String(random(100));
        int str_len = str.length() + 1;
        char char_array[str_len];
        str.toCharArray(char_array, str_len);
        webSocket.broadcastTXT(char_array);
        previousMillis = now;
      }
    }