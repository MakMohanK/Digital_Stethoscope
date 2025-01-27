#include <WiFi.h>
#include <Wire.h>
#include <driver/i2s.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ArduinoWebsockets.h>
#include "esp_system.h"

// WiFi credentials
const char* ssid = "YOUR WIFI NAME";
const char* password = "YOUR WIFI PASSWORD";

// Flask server URL
const char* serverUrl = "http://<YOUR SERVER IP>:8000/upload_audio"; // change to your server ip address

#define pushButton 1
#define I2S_SD 10 // Serial Data (SD)
#define I2S_WS 11 // Word Select (WS)
#define I2S_SCK 12 // Serial Clock (SCK)
#define I2S_PORT I2S_NUM_1

// OLED display settings
#define i2c_Address 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// #define bufferCnt 10
// #define bufferLen 1024
// int16_t sBuffer[1024];

void i2s_install() {
  // Set up I2S Processor configuration
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    // .sample_rate = 16000,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 10,
    .dma_buf_len = 1024,
    .use_apll = false
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin() {
  // Set I2S pin configuration
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

void setup() {
  Serial.begin(115200);
  pinMode(pushButton, INPUT);

   // Initialize I2C communication
  Wire.begin(19, 20);  // Change to your SDA, SCL pins if necessary

    // Initialize OLED display
  if (!display.begin(i2c_Address, true)) {
    Serial.println(F("OLED initialization failed!"));
    while (true);  // Halt if display initialization fails
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("---------------------"); 
  display.println("---------------------");
  display.println("---------------------");
  display.println("       DIGITAL       ");
  display.println("     STETHOSCOPE     ");
  display.println("---------------------");
  display.println("---------------------");
  display.println("---------------------");
  display.display();
  delay(3000);

  // Initialization message
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Device initialised");
  display.setCursor(0, 12);
  display.println("Connecting to WiFi...");
  display.display();

  delay(2000);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    display.println("....");
    display.display();
  }
  
  delay(1000);

  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);

  display.setCursor(0, 36);
  display.println("Connected to WiFi");
  Serial.println("Connected to WiFi");
  display.display();

  delay(2000);
  display.clearDisplay();
}

void loop() {
    // Display heading
  display.clearDisplay();

  // display.setTextSize(1);
  display.setCursor(0, 1);
  display.println("---------------------");
  display.println("       DIGITAL       ");
  display.println("     STETHOSCOPE     ");
  display.println("---------------------");
  display.println("---------------------");
  display.println(" Click the button to ");
  display.println("   get prediction    ");
  display.println("---------------------");
  display.display();
  // read the switch event
  int buttonState = digitalRead(pushButton);
  if (buttonState == HIGH){
    Serial.printf("Free heap before allocation: %d bytes\n", ESP.getFreeHeap());
    recordAndSendAudio();
  }
  delay(100);  // delay in between reads for stability
}

void recordAndSendAudio() {
    const int durationSec = 4;  // Duration of audio to record in seconds
    const int sampleRate = 16000;  // Sample rate in Hz
    const int numSamples = durationSec * sampleRate;
    const int bufferSize = 64;  // Smaller buffer size to prevent overflow

    // Dynamically allocated memory for audio data and buffer
    // int16_t *audioData = (int16_t *)malloc(numSamples * sizeof(int16_t));
    // int16_t *buffer = (int16_t *)malloc(bufferSize);  // Buffer for I2S read

    int16_t *audioData = (int16_t *)heap_caps_malloc(numSamples * sizeof(int16_t), MALLOC_CAP_DEFAULT);
    int16_t *buffer = (int16_t *)heap_caps_malloc(bufferSize, MALLOC_CAP_DEFAULT);

    if (audioData == NULL || buffer == NULL) {
        Serial.println("Memory allocation failed");
        return;
    }

    display.clearDisplay();

    // display.setTextSize(1);
    display.clearDisplay();
    display.setCursor(0, 1);
    display.println("---------------------");
    display.println("       DIGITAL       ");
    display.println("     STETHOSCOPE     ");
    display.println("---------------------");
    display.println("---------------------");
    display.println("      STARTED        ");
    display.println("  Audio Recording    ");
    display.println("---------------------");
    display.display();

    delay(3000);

    Serial.println("Recording audio...");

    size_t bytesRead = 0;
    int index = 0;
    while (index < numSamples) {
        // Non-blocking I2S read with a timeout
        esp_err_t result = i2s_read(I2S_PORT, buffer, bufferSize, &bytesRead, 100 / portTICK_RATE_MS);
        if (result == ESP_OK && bytesRead > 0) {
            // Copy audio data from buffer to audioData array
            for (size_t i = 0; i < bytesRead / sizeof(int16_t); i++) {
                if (index < numSamples) {
                    audioData[index++] = buffer[i];
                }
            }
        }
    }
    // Free allocated memory
    free(buffer);
    
    display.setCursor(0, 1);
    display.clearDisplay();
    display.println("---------------------");
    display.println("       DIGITAL       ");
    display.println("     STETHOSCOPE     ");
    display.println("---------------------");
    display.println("---------------------");
    display.println("      STOPPED        ");
    display.println("  Audio Recording    ");
    display.println("---------------------");
    display.display();
    Serial.println("Audio recording completed.");

    // Check WiFi status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi is connected. Sending data to server...");

        HTTPClient http;
        http.begin(serverUrl);  // Use your server's URL
        http.addHeader("Content-Type", "application/octet-stream");

        Serial.println(serverUrl);

        // Send audio data using HTTP POST
        int httpResponseCode = http.POST((uint8_t*)audioData, numSamples * sizeof(int16_t));  // Send data to the server
        // Display the server response
        String serverResponse = http.getString();
        Serial.println("Data from Server: "+serverResponse);
        display.clearDisplay();

        // display.setTextSize(1);
        display.setCursor(0, 1);
        display.clearDisplay();
        display.println("---------------------");
        display.println("   MODEL RESPONSE    ");
        display.println("---------------------");
        display.println("---------------------");
        display.println(serverResponse);
        display.println("---------------------");
        display.display();

        if (httpResponseCode > 0) {
            Serial.printf("Server response: %d\n", httpResponseCode);
        } else {
            Serial.printf("Failed to send data. Error: %s\n", http.errorToString(httpResponseCode).c_str());
            display.setCursor(0, 1);
            display.clearDisplay();
            display.println("---------------------");
            display.println("   MODEL RESPONSE    ");
            display.println("---------------------");
            display.println("Something Went Wrong!");
            display.println(" Make Sure Server is ");
            display.println("    Up and Running   ");
            display.println("---------------------");
            display.display();
        }
        http.end();  // Close HTTP connection
    } else {
        Serial.println("WiFi not connected");
    }

    // Free allocated memory
    free(audioData);
    audioData = NULL; // After freeing memory, ensure the pointer is set to NULL 
    buffer = NULL; // After freeing memory, ensure the pointer is set to NULL 
    delay(8000); 
    esp_restart();  // This will reset the ESP32
}
