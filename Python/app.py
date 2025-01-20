from flask import Flask, request
import wave
import struct
app = Flask(__name__)

def pcm_to_wav(pcm_data, sample_rate=16000, num_channels=1, sample_width=2):
    """
    Converts raw PCM data into a WAV file.

    :param pcm_data: The raw PCM audio data.
    :param sample_rate: The sample rate of the audio.
    :param num_channels: Number of channels (1 for mono, 2 for stereo).
    :param sample_width: Sample width in bytes (2 bytes for 16-bit audio).
    :return: None (Saves the audio as a .wav file).
    """
    # Create a wave file
    with wave.open('output_audio.wav', 'wb') as wav_file:
        wav_file.setnchannels(num_channels)  # Set number of channels
        wav_file.setsampwidth(sample_width)  # Set sample width (2 bytes for 16-bit audio)
        wav_file.setframerate(sample_rate)   # Set sample rate (e.g., 16000 Hz)
        wav_file.writeframes(pcm_data)       # Write the raw PCM data to the file

    print("WAV file created successfully!")

@app.route('/upload_audio', methods=['POST'])
def upload_audio():
    audio_data = request.data  # Get the raw audio data
    # print(audio_data)
    if audio_data:
        pcm_to_wav(audio_data)
        with open("received_audio.raw", "wb") as f:
            f.write(audio_data)
        return "Audio data received", 200
    else:
        return "No audio data received", 400

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000, debug=True)
