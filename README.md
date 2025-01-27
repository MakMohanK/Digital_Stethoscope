# Flask Audio Classification API

## Overview

This repository provides a Flask-based API for audio classification. It processes raw PCM audio data, converts it to WAV format, and predicts its class using a pre-trained model. The results include both the predicted class and its confidence level, which are stored in a JSON file.

## Features

- Accepts raw PCM audio data via a POST request.
- Converts PCM data to WAV format with optional amplification.
- Predicts the audio class and confidence using a pre-trained model.
- Stores the predicted results in a JSON file.

## Prerequisites

Before running the application, ensure the following dependencies are installed:

- Python 3.x
- Flask
- NumPy

Install the required Python packages using:

```bash
pip install flask numpy
```

## File Structure

- `app.py`: Main Flask application.
- `model/model3.py`: Contains the `predict_class_with_confidence` function for making predictions.
- `data.json`: Stores the predicted class and confidence.
- `model/output_audio.wav`: Temporary file for storing the converted WAV audio.

## API Endpoints

### 1. Root Endpoint

**URL:** `/`

**Method:** GET

**Description:**

- Returns the contents of the `data.json` file, which contains the latest prediction results.

**Response:**

- `200 OK`: JSON data with the predicted class and confidence.
- `500 Internal Server Error`: Error loading the JSON file.

### 2. Upload Audio Endpoint

**URL:** `/upload_audio`

**Method:** POST

**Description:**

- Accepts raw PCM audio data and processes it to:
  - Convert it to WAV format.
  - Predict the class and confidence.
  - Save the results to `data.json`.

**Request Body:**

- Raw PCM audio data.

**Response:**

- `200 OK`: Returns the predicted class and confidence as a string.
- `400 Bad Request`: No audio data received.

## Usage

### Running the Application

Start the Flask server with the following command:

```bash
python app.py
```

The application will be accessible at `http://0.0.0.0:8000`.

### Example Requests

#### 1. Fetch the Latest Prediction

```bash
curl http://0.0.0.0:8000/
```

#### 2. Upload Audio for Prediction

```bash
curl -X POST --data-binary @your_audio_file.pcm http://0.0.0.0:8000/upload_audio
```

Replace `your_audio_file.pcm` with the path to your raw PCM audio file.

## Core Functions

### `pcm_to_wav`

Converts raw PCM audio data to WAV format with optional amplification.

**Parameters:**

- `pcm_data`: Raw PCM audio data.
- `sample_rate`: Sampling rate of the audio (default: 16000 Hz).
- `num_channels`: Number of audio channels (default: 1).
- `sample_width`: Sample width in bytes (default: 2 for 16-bit audio).
- `amplification_factor`: Factor to amplify the audio (default: 10.0).

### `predict_class_with_confidence`

A function from the `model/model3.py` module, used to predict the class and confidence of the audio.

**Parameters:**

- `audio_file`: Path to the WAV file.

**Returns:**

- `predicted_value`: Predicted class index.
- `confidence`: Confidence level of the prediction.

## Classes

The application can classify audio into the following categories:

- `Artifact`
- `Murmur`
- `Normal`
- `Extrahls`
- `Extrastole`

## Output

The prediction results are stored in `data.json` in the following format:

```json
{
  "class": "Normal",
  "confidence": 98.5
}
```

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments

- The `model3` module for audio classification.
- Flask for the web framework.
- NumPy for data processing.
- 

# Digital Stethoscope with ESP32 and Flask Server Integration

## Overview

This project implements a digital stethoscope using an ESP32 microcontroller. The device records audio signals (heart sounds) through an I2S interface and sends the audio data to a Flask server for classification and prediction. The project integrates features like WiFi connectivity, OLED display feedback, and HTTP communication.

## Features

- Records audio using the ESP32's I2S interface.
- Sends recorded audio to a Flask server for prediction.
- Displays the system's status and results on an OLED screen.
- Dynamically allocates memory for audio processing.
- Automatic system restart after audio recording and data transmission.

## Hardware Requirements

1. **ESP32 Microcontroller**
2. **I2S Microphone**
   - SD (Serial Data) connected to GPIO 10
   - WS (Word Select) connected to GPIO 11
   - SCK (Serial Clock) connected to GPIO 12
3. **OLED Display** (Adafruit SH110X-compatible)
   - I2C connections (SDA, SCL)
4. **Push Button** (connected to GPIO 1)
5. Power supply for the ESP32

## Software Requirements

1. **Arduino IDE**
   - ESP32 Board Package
   - Required libraries:
     - `WiFi.h`
     - `Wire.h`
     - `Adafruit_GFX.h`
     - `Adafruit_SH110X.h`
     - `ArduinoWebsockets.h`
2. **Flask Server**
   - Set up to handle audio data via HTTP POST requests.

## Setup Instructions

### Hardware Setup

1. Connect the I2S microphone to the ESP32:
   - SD -> GPIO 10
   - WS -> GPIO 11
   - SCK -> GPIO 12
2. Connect the OLED display to the ESP32 via I2C.
3. Connect a push button to GPIO 1.

### Software Setup

1. Install the required libraries in the Arduino IDE.
2. Update the following configurations in the code:
   - WiFi credentials:
     ```cpp
     const char* ssid = "YOUR_WIFI_SSID";
     const char* password = "YOUR_WIFI_PASSWORD";
     ```
   - Flask server URL:
     ```cpp
     const char* serverUrl = "http://<FLASK_SERVER_IP>:8000/upload_audio";
     ```
3. Upload the code to the ESP32 using the Arduino IDE.
4. Start the Flask server on the specified IP and port.

## Usage

1. Power up the ESP32.
2. The OLED display will show initialization messages and attempt to connect to the WiFi network.
3. Once connected, press the push button to start recording audio.
4. The device records 4 seconds of audio and sends it to the Flask server.
5. The Flask server processes the audio and sends back the predicted class and confidence score.
6. The OLED display shows the prediction result.

## Code Highlights

### I2S Configuration

The I2S interface is configured to record audio data:

```cpp
const i2s_config_t i2s_config = {
  .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
  .sample_rate = 44100,
  .bits_per_sample = i2s_bits_per_sample_t(16),
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
  .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
  .dma_buf_count = 10,
  .dma_buf_len = 1024,
  .use_apll = false
};
```

### Audio Transmission

The audio data is sent to the Flask server using HTTP POST:

```cpp
HTTPClient http;
http.begin(serverUrl);
http.addHeader("Content-Type", "application/octet-stream");
int httpResponseCode = http.POST((uint8_t*)audioData, numSamples * sizeof(int16_t));
```

### OLED Feedback

System status and results are displayed on the OLED:

```cpp
display.setCursor(0, 1);
display.println("Device initialised");
display.display();
```

## Troubleshooting

- **WiFi Connection Issues**:
  - Ensure the WiFi credentials are correct.
  - Verify that the ESP32 is within range of the WiFi network.
- **Server Communication Errors**:
  - Check that the Flask server is running and accessible.
  - Ensure the server URL is correctly configured.
- **Memory Allocation Failures**:
  - Use `heap_caps_malloc` for better memory management on the ESP32.

## Future Enhancements

- Add support for multiple audio sample rates.
- Implement error handling for server communication.
- Store predictions locally for offline usage.

## Acknowledgments

This project uses libraries and tools from:

- Adafruit
- Arduino
- Flask (for server-side processing)

