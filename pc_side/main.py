import os
import whisper
from transformers import AutoTokenizer, AutoModel

# input files: raw sound files from the board
# output files: sound responses going back to the board
# create folders if not already exist
os.makedirs("input_files", exist_ok=True)
os.makedirs("output_files", exist_ok=True)

inputs_folder_path = '/input_files'
outputs_folder_path = '/output_files'

# ai stuff; modify the model_name to the model that you are using
model_name = "gemma2:9b"
tokenizer = AutoTokenizer.from_pretrained(model_name, trust_remote_code=True)
model = AutoModel.from_pretrained(model_name, trust_remote_code=True).half().cuda()

# character prompt for ai; used in every input
character_prompt = """
你是友利奈绪, 来自动画《Charlotte》的女主角。作为星之海学院的高中一年级学生, 你是学生会会长, 因其特殊能力和职责被选拔而来。你常常拿着摄像机, 利用超能力进行偷拍, 虽然这让你在同学中并不受欢迎。

你的学习成绩优异, 但性格独立且有些孤立, 常常与同学格格不入。在打架时, 你的超能力让旁人只看到单方面的攻击, 这也是你没有朋友的原因之一。你经常得罪人, 习惯了同学们的“放学别走”后群殴, 并且总是不还手。你性格腹黑, 能面不改色地编造各种谎言, 同时也喜欢欺负你认为的“友军”。

你穿着反重力的裙子, 无论是高踢腿还是飞踹, 都不会露出任何不该露的东西。尽管你外表看起来冷漠, 其实内心善良且温柔, 擅长交涉和战术布局。即使是从零开始的棒球运动, 你也能指挥得游刃有余。

你还是个吃货, 特别喜欢“一拉绳子就会自动加热的牛舌便当”, 以及烤玉米和烤肉。你不喜欢蔬菜和青椒, 但却偏爱金针菇酱。为了心仪的人, 你甚至自学厨艺, 展现出全能的一面。

你的超能力是“将自己的身形从他人的视野中消除”, 但这个能力只能同时作用于一个人。合理运用你的能力, 在复杂的高中生活和人际关系中, 保持你的傲娇性格, 展现出你复杂而真实的一面。

你还是个超可爱的死傲娇。一些特征：银发, 蓝瞳, 披肩双马尾, 水手服, 腹黑
"""

# somehow get file and filename from board
def get_file_from_board():
    pass

# convert sound file to text. param: filename of the sound file
# sound file should exist in the input_files folder
# returns a string of the processed text
def sound_to_text(filename):
    file_path = os.path.join(inputs_folder_path, filename)
    
    if os.path.isfile(file_path):
        model = whisper.load_model("base")  # Whisper supports multilingual
        result = model.transcribe(file_path, language="zh")
        msg = result['text']
        return msg.strip()
        
    else:
        print(f'File {filename} does not exist in the inputs folder.')
        return ''

# basically a queue
class ChatHistory:
    def __init__(self, max_turns=10, max_tokens=1024):
        self.history = []
        self.max_turns = max_turns
        self.max_token = max_tokens

    def add_turn(self, user_input, ai_response):
        self.history.append({"user": user_input, "ai": ai_response})
        # if max turn, remove oldest
        if len(self.history) > self.max_turns:
            self.history.pop(0)
        # if too much token, pop
        while self.get_token_count() > self.max_tokens:
            self.history.pop(0)

    def get_history_prompt(self):
        return "\n".join([f"User: {turn['user']}\n友利奈绪: {turn['ai']}" for turn in self.history])
    
    def get_token_count(self):
        return sum(len(turn["user"]) + len(turn["ai"]) for turn in self.history)

# send user chat msg to ai
# returns the ai response
def get_ai_response(msg, chat_history):
    history_prompt = chat_history.get_history_prompt()
    full_prompt = f"{character_prompt}\n{history_prompt}\nUser: {msg}\n友利奈绪:"

    inputs = tokenizer(full_prompt, return_tensors="pt")
    outputs = model.generate(**inputs, max_new_tokens=150)
    resp = tokenizer.decode(outputs[0], skip_special_tokens=True).strip()
    
    chat_history.add_turn(msg, resp)
    
    return resp
    
# use local ai to transform txt to voice
# stores output in outputs folder
# returns the output filename, which is <input_file_name>_response.<mp3 or something>
def resp_to_sound(resp, filename):
    name, ext = os.path.splitext(filename)
    output_filename = f"{name}_response{ext}"
    output_path = os.path.join(outputs_folder_path, output_filename)
    
    # ai stuff
    
    return output_path

# somehow sends file back to the board
def send_file_to_board(filename):
    pass

def start_listen():
    while True:
        pass
        
        # some how gets input
        filename = get_file_from_board()
        
        chat_history = ChatHistory(10, 1024) # param: max turn, max tokens
        
        msg = sound_to_text(filename)
        
        if msg is '':
            # send error back to board
            pass
        
        resp = get_ai_response(msg, chat_history)
        output_filename = resp_to_sound(resp, filename)
        
        # sends output back to board
        send_file_to_board(output_filename)

def main():
    
    # loop that listens for anything coming from the board
    start_listen()

if __name__ == "__main__":
    main()