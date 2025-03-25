import ollama
from dotenv import load_dotenv
import os

load_dotenv()

CHARACTER_PROMPT_FIXED = os.getenv("CHARACTER_PROMPT_FIXED")
CHARACTER_PROMPT_SETTINGS = os.getenv("CHARACTER_PROMPT_SETTINGS")

CHARACTER_PROMPT = f"{CHARACTER_PROMPT_FIXED}\n\n{CHARACTER_PROMPT_SETTINGS}"

MODEL = os.getenv("OLLAMA_MODEL")

# FIFO queue for chat history. default saves 10
class ChatHistory:
    
    def __init__(self, max_size=10):
        self.max_size = max_size
        self.history = []

    def add_history(self, user_input, ai_response):
        self.history.append(f"User: {user_input}\nAI: {ai_response}")
        
        while len(self.history) > self.max_size:
            self.history.pop(0)

    def get_history(self):
        return "\n".join(self.history)


def get_ai_response(text, chat_history):
    prompt = f"{CHARACTER_PROMPT}\n\n{chat_history.get_history()}\n\nUser: {text}\nAI:"
    
    response = ollama.chat(model=MODEL, messages=[{"role": "user", "content": prompt}])
    
    ai_response = response["message"]["content"]
    
    chat_history.add_history(text, ai_response)
    
    return ai_response