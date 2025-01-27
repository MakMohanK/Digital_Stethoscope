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

