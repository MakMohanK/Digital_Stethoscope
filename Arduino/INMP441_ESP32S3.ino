#include <driver/i2s.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "esp_system.h"

// WiFi credentials
const char* ssid = "Your Wifi Name";
const char* password = "Your Wifi Password";

// Flask server URL
const char* serverUrl = "http://<Your Server IP>:8000/upload_audio";  // add IP here of your server

#define pushButton 1
#define I2S_SD 10 // Serial Data (SD)
#define I2S_WS 11 // Word Select (WS)
#define I2S_SCK 12 // Serial Clock (SCK)
#define I2S_PORT I2S_NUM_1

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

  delay(2000);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  delay(2000);

  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);
}

void loop() {
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
    const int bufferSize = 256;  // Smaller buffer size to prevent overflow

    // Dynamically allocated memory for audio data and buffer
    // int16_t *audioData = (int16_t *)malloc(numSamples * sizeof(int16_t));
    // int16_t *buffer = (int16_t *)malloc(bufferSize);  // Buffer for I2S read

    int16_t *audioData = (int16_t *)heap_caps_malloc(numSamples * sizeof(int16_t), MALLOC_CAP_DEFAULT);
    int16_t *buffer = (int16_t *)heap_caps_malloc(bufferSize, MALLOC_CAP_DEFAULT);



    if (audioData == NULL || buffer == NULL) {

        Serial.println("Memory allocation failed");
        return;
    }

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
        Serial.println("Data from Server: "+String(http.getString()));
        if (httpResponseCode > 0) {
            Serial.printf("Server response: %d\n", httpResponseCode);
        } else {
            Serial.printf("Failed to send data. Error: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end();  // Close HTTP connection
    } else {
        Serial.println("WiFi not connected");
    }

    // Free allocated memory
    free(audioData);
    audioData = NULL; // After freeing memory, ensure the pointer is set to NULL 
    buffer = NULL; // After freeing memory, ensure the pointer is set to NULL 
    esp_restart();  // This will reset the ESP32 // this is needed because of memmory issue if you have solution please let me know.
}
