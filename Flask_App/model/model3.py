import os
import numpy as np
import joblib
import librosa

# Load the XGB Regressor model
xgb_model = joblib.load("model/xgb_regressor.pkl")  # Replace with actual path


classes = ['artifact', 'murmur', 'normal','extrahls','extrastole']

def stretch(audio, rate):
    """
    Apply time stretching to the audio signal.
    """
    try:
        return librosa.effects.time_stretch(audio, rate)
    except Exception as e:
        print(f"Error in stretching: {e}")
        return audio

def extract_features(audio, sr, n_mfcc=52, n_fft=2048, hop_length=512):
    """
    Extracts MFCC and other audio features.
    """
    # Extract various audio features
    mfccs = np.mean(librosa.feature.mfcc(y=audio, sr=sr, n_mfcc=n_mfcc, n_fft=n_fft, hop_length=hop_length).T, axis=0)

    # Combine all features into a single vector
    return np.hstack([mfccs])

def preprocess_audio(audio_path, target_sr=22050, duration=10, n_mfcc=52):
    """
    Preprocess a single audio file to generate a feature vector of size 52.
    Includes augmentation with stretching.
    """
    n_fft = 2048
    hop_length = 512
    input_length = target_sr * duration
    try:
        # Load the audio file
        audio, sr = librosa.load(audio_path, sr=target_sr, duration=duration)

        # Pad or truncate to the target length
        if len(audio) < input_length:
            pad_width = input_length - len(audio)
            audio = np.pad(audio, (0, pad_width), mode='constant')
        elif len(audio) > input_length:
            audio = audio[:input_length]

        # Extract features from the original audio
        feature_vector = extract_features(audio, sr, n_mfcc, n_fft, hop_length)

        # Data augmentation - stretching
        augmented_features = []
        # for stretch_factor in [0.8, 1.2]:
        #     stretched_audio = stretch(audio, stretch_factor)
        #     augmented_features.append(extract_features(stretched_audio, sr, n_mfcc, n_fft, hop_length))

        # Combine original and augmented features
        all_features = [feature_vector] + augmented_features
        return np.array(all_features)

    except Exception as e:
        print(f"Error processing audio file: {e}")
        return None

def predict_class_with_confidence(audio_path, model=xgb_model):
    features = preprocess_audio(audio_path)
    if features is not None:
        print(f"Feature shape: {features.shape}")  # Debug feature shape
        
        # Use only the original feature vector (first 52)
        original_features = features[0]
        original_features = original_features.reshape(1, -1)  # Ensure correct shape for XGBoost
        
        # Get the predicted value (regression)
        prediction = model.predict(original_features)
        
        # Assuming predicted_value is continuous and we want to estimate confidence
        predicted_value = prediction[0]
        
        # Confidence: Use absolute value of the prediction for the confidence (this can be adjusted as per your task)
        confidence = 1 - (abs(predicted_value) / (1 + abs(predicted_value)))  # Example of confidence calculation

        return predicted_value, confidence
    else:
        print("Failed to extract features for prediction.")
        return None, None

if __name__ == "__main__":
    audio_file = "model/output_audio.wav"  # Replace with actual file path
    if os.path.exists(audio_file):
        predicted_value, confidence = predict_class_with_confidence(audio_file, xgb_model)
        if predicted_value is not None:
            print(f"Predicted value: {predicted_value}")
            print(f"Confidence: {confidence * 100:.2f}%")
            print(f"Predicted Class: {classes[int(round(predicted_value))]}")
        else:
            print("Prediction failed.")
    else:
        print("Audio file not found!")
