#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <Adafruit_NeoPixel.h>
#include <WebSocketsClient.h>

#define LED_PIN    4
#define LED_COUNT 12
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

char* ssid = "***";
char* password = "***";

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
DynamicJsonDocument root(1024);
DynamicJsonDocument data(1024);

boolean hasBeenSentData = false;
boolean isConnectedToChannels = false;
boolean isPusherMode = false;
uint32_t LEDColour = strip.Color(255, 206, 84);

void setup() {

  Serial.begin(115200);

  Serial.setDebugOutput(true);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(128); 

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  //make the lights red when it's not connected
  colorWipe(strip.Color(255,   0,   0), 10);
  while(WiFiMulti.run() != WL_CONNECTED) {
      delay(100);
    }

  //make the lights green when it's connected to the internet
  colorWipe(strip.Color(0,   255,   0), 10);
  
  webSocket.begin("ws-eu.pusher.com", 80, "/app/5b1e6772b0fa4040f5a6?client=arduino&version=0.0.1&protocol=7");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket.enableHeartbeat(15000, 3000, 2);
 
}

#define USE_SERIAL Serial

void connectToChannel() {
  String json = "{\"event\":\"pusher:subscribe\", \"data\": {\"channel\": \"angel\"} }";
  webSocket.sendTXT(json);
}

void handleEvent(uint8_t * payload) {
 
      deserializeJson(root, payload);
      if (root["event"] == "new-colour") {

        hasBeenSentData = true;
        isPusherMode = false;
        
        String tmp = root["data"];
        deserializeJson(data, tmp);
        String colour = data["responseColour"];
        Serial.println(colour);
        
        if (colour == "pusher") { 
          isPusherMode = true;
        } else if (colour == "pusher-beams") { 
          LEDColour = strip.Color(42, 233, 170); 
        } else if (colour == "pusher-channels") { 
          LEDColour = strip.Color(106, 82, 255);
        } else if (colour == "pusher-chatkit") { 
          LEDColour = strip.Color(255, 132, 115);
        } else {
          String rIn = data["r"];
          String gIn = data["g"];
          String bIn = data["b"];
          
          int r = atoi(rIn.c_str());
          int g = atoi(gIn.c_str());
          int b = atoi(bIn.c_str());
          LEDColour = strip.Color(r,g,b);
        }
        
      }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  USE_SERIAL.printf("[WSc] !\n", payload);
  switch(type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[WSc] Disconnected!\n", payload);
      isConnectedToChannels = false;
      break;
    case WStype_CONNECTED: {
      //change the lights when we're connected to pusher
      isConnectedToChannels = true;
      USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
      connectToChannel();
    }
      break;
    case WStype_TEXT:
      USE_SERIAL.printf("[WSc] get text: %s\n", payload);
      handleEvent(payload);
      // send message to server
      // webSocket.sendTXT("message here");
      break;
    case WStype_BIN:
      USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);
      break;
        case WStype_PING:
            // pong will be send automatically
            USE_SERIAL.printf("[WSc] get ping\n");
            break;
        case WStype_PONG:
            // answer to a ping we send
            USE_SERIAL.printf("[WSc] get pong\n");
            break;
    }
}

void loop() {

  webSocket.loop();

  if (hasBeenSentData == true) {

     if(isPusherMode == true) {
      rainbow(2);  
     } else {
      colorWipe(LEDColour, 200);
     }
    
    
  } else { 
    
    if (isConnectedToChannels == true) {
      colorWipe(strip.Color(0,   0,   255), 100);
    } else {
      //has internet connection
      colorWipe(strip.Color(0,   255,   0), 100);
    }
  }

}




// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, strip.gamma32(color));         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}
