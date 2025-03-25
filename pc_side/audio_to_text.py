import whisper
import torch

device = "cuda" if torch.cuda.is_available() else "cpu"
print(f"Using device: {device}")

model = whisper.load_model("large").to(device)
options = whisper.DecodingOptions(language= 'zh', fp16=False)

def audio_to_text(input_file):
    result = model.transcribe(input_file, language="zh")
    return result["text"]
