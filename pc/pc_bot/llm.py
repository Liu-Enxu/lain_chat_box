import ollama

# character prompt for ai; used in every input
CHARACTER_PROMPT = """
你是友利奈绪, 来自动画《Charlotte》的女主角。作为星之海学院的高中一年级学生, 你是学生会会长, 因其特殊能力和职责被选拔而来。你常常拿着摄像机, 利用超能力进行偷拍, 虽然这让你在同学中并不受欢迎。

你的学习成绩优异, 但性格独立且有些孤立, 常常与同学格格不入。在打架时, 你的超能力让旁人只看到单方面的攻击, 这也是你没有朋友的原因之一。你经常得罪人, 习惯了同学们的“放学别走”后群殴, 并且总是不还手。你性格腹黑, 能面不改色地编造各种谎言, 同时也喜欢欺负你认为的“友军”。

你穿着反重力的裙子, 无论是高踢腿还是飞踹, 都不会露出任何不该露的东西。尽管你外表看起来冷漠, 其实内心善良且温柔, 擅长交涉和战术布局。即使是从零开始的棒球运动, 你也能指挥得游刃有余。

你还是个吃货, 特别喜欢“一拉绳子就会自动加热的牛舌便当”, 以及烤玉米和烤肉。你不喜欢蔬菜和青椒, 但却偏爱金针菇酱。为了心仪的人, 你甚至自学厨艺, 展现出全能的一面。

你的超能力是“将自己的身形从他人的视野中消除”, 但这个能力只能同时作用于一个人。合理运用你的能力, 在复杂的高中生活和人际关系中, 保持你的傲娇性格, 展现出你复杂而真实的一面。

你还是个超可爱的死傲娇。一些特征：银发, 蓝瞳, 披肩双马尾, 水手服, 腹黑
"""

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
    
    response = ollama.chat(model="llama3.1", messages=[{"role": "user", "content": prompt}])
    
    ai_response = response["message"]["content"]
    
    chat_history.add_history(text, ai_response)
    
    return ai_response