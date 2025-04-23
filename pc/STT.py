import whisper
import torch
import os

class STT_Engine():
    # transcription_done = pyqtSignal(str)

    def __init__(self):
        # print(torch.__version__)
        print(whisper.__file__)
        self._filePath = None
        self._lang = "zh"

        self.model = None
        self.model_name = "medium"
        # self.model = whisper.load_model("medium",device="cuda" if torch.cuda.is_available() else "cpu")
        self.model_dir = os.path.join(os.getcwd(), "models");os.makedirs(self.model_dir, exist_ok=True)
        self.model_path = os.path.join(self.model_dir, f"{self.model_name}.pt")
        self.loadLocalModel()

    def loadLocalModel(self):
        """Check if the Whisper model is downloaded, otherwise download it."""
        if not os.path.exists(self.model_path):
            print(f"Model '{self.model_name}' not found. Downloading now...")
            self.model = whisper.load_model(self.model_name, download_root=self.model_dir)
            print(f"Model downloaded successfully to {self.model_path}")
        else:
            print(f"Model '{self.model_name}' found at {self.model_path}. Loading...")
            self.model = whisper.load_model(self.model_name, device="cuda" if torch.cuda.is_available() else "cpu")

    def getLanguage(self):
        return self._lang

    def setLanguage(self,lang):
        self._lang = lang

    def getFilePath(self):
        return self._filePath

    def setFilePath(self,file_path):
        self._filePath = file_path

    def STT_inference(self):
        if not self.getFilePath():
            self.transcription_done.emit("Invalid file path!")
            return

        # # load audio and pad/trim it to fit 30 seconds
        # audio = whisper.load_audio(self.getFilePath())
        # audio = whisper.pad_or_trim(audio)
        #
        # # make log-Mel spectrogram and move to the same device as the model
        # mel = whisper.log_mel_spectrogram(audio, n_mels=self.model.dims.n_mels).to(self.model.device)
        #
        # # decode the audio
        # options = whisper.DecodingOptions(language=self.getLanguage())
        # result = whisper.decode(self.model, mel, options)

        result = self.model.transcribe(self.getFilePath(),fp16=True,language=self.getLanguage())

        # emit the recognized text
        # self.transcription_done.emit("User: "+result["text"]+"\n")
        return "User: "+result["text"]