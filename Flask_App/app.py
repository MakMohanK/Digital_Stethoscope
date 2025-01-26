from flask import Flask, request
import wave
import numpy as np
import time
import os
import json

file_path = 'data.json'

# Data to be saved


from model.model3 import predict_class_with_confidence

audio_file = "model/output_audio.wav" 

classes = ['artifact', 'murmur', 'normal','extrahls','extrastole']

app = Flask(__name__)

def pcm_to_wav(pcm_data, sample_rate=32000, num_channels=1, sample_width=2, amplification_factor=10.0):
    """
    Converts raw PCM data into a WAV file with optional amplification.

    :param pcm_data: The raw PCM audio data.
    :param sample_rate: The sample rate of the audio.
    :param num_channels: Number of channels (1 for mono, 2 for stereo).
    :param sample_width: Sample width in bytes (2 bytes for 16-bit audio).
    :param amplification_factor: Factor by which to amplify the audio.
    :return: None (Saves the audio as a .wav file).
    """
    # Convert PCM data to a numpy array for processing
    pcm_array = np.frombuffer(pcm_data, dtype=np.int16)

    # Apply amplification
    pcm_array = np.clip(pcm_array * amplification_factor, -32768, 32767).astype(np.int16)

    # Convert back to bytes
    amplified_pcm_data = pcm_array.tobytes()

    # Create a wave file
    with wave.open(audio_file, 'wb') as wav_file:
        wav_file.setnchannels(num_channels)  # Set number of channels
        wav_file.setsampwidth(sample_width)  # Set sample width (2 bytes for 16-bit audio)
        wav_file.setframerate(sample_rate)   # Set sample rate (e.g., 16000 Hz)
        wav_file.writeframes(amplified_pcm_data)  # Write the amplified PCM data to the file

    print("WAV file created successfully!")

# Define a route for the root URL
@app.route('/')
def hello_world():
    try:
        with open(file_path, 'r') as json_file:
            loaded_data = json.load(json_file)
        # Pass the loaded data to the template
        return loaded_data
    except Exception as e:
        # Handle any errors (e.g., file not found)
        return f"Error loading data: {e}", 500

@app.route('/upload_audio', methods=['POST'])
def upload_audio():
    audio_data = request.data  # Get the raw audio data
    # print(audio_data)
    if audio_data:
        pcm_to_wav(audio_data)
        time.sleep(0.2) # wait for 2 sec to get running process free

        result = None
        if os.path.exists(audio_file):
            predicted_value, confidence = predict_class_with_confidence(audio_file)
            if predicted_value is not None:
                print(f"Predicted value: {predicted_value}")
                print(f"Confidence: {confidence * 100:.2f}%")
                print(f"Predicted Class: {classes[int(round(predicted_value))]}")
                result = str(classes[int(round(predicted_value))]) + "|" + "Conf: "+str(int(confidence*100))+"%"

                # Save data to JSON file
                data = {
                    "class": classes[int(round(predicted_value))],
                    "confidence": float(confidence*100)
                }

                with open(file_path, 'w') as json_file:
                    json.dump(data, json_file)

                print("Data saved to JSON file.")
        else:
            print("Prediction failed.")

        return result, 200
    else:
        return "No audio data received", 400

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000, debug=True)
