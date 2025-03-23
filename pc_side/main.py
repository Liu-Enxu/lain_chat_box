import audio_to_text
import llm
import resp_to_sound
import os
import sys
import socket
import time

PORT = 9999
OUT_PORT = 10000
DISCOVERY_MESSAGE = b"WHERE_IS_MY_GIRLFRIEND"
EXPECTED_RESPONSE = b"IM_HERE"
END_MESSGE = b"DONE"
MAX_TRY = 10

INPUT_FOLDER = "input_files"

# max num of audio files saved in the input folder
MAX_FILES = 10

def timestamp_to_int(filename):
    t = filename[len("received_"):-len(".wav")]
    return int(t.replace(":", "").replace("_", ""))

def clean_old_files(folder):
    all_files = os.listdir(folder)
    all_files.sort(key=timestamp_to_int)
    
    while len(all_files) > MAX_FILES:
        to_be_removed = all_files.pop(0)
        os.remove(os.path.join(folder, to_be_removed))

def find_board():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.settimeout(3)
        
        board_addr = None
        
        for _ in range(MAX_TRY):
            sock.sendto(DISCOVERY_MESSAGE, ("255.255.255.255", PORT))
            
            try:
                data, addr = sock.recvfrom(1024) # addr = (ip, port)
                if data == EXPECTED_RESPONSE and addr[1] == PORT:
                    board_addr = addr
                    print(f"board addr: {board_addr}")
                    break
            except socket.timeout:
                print("timeout, retry...")

    if not board_addr:
        print("can't find board")
        sys.exit()

    return board_addr

########################################################################
# testing as board
def mock_board():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.bind(("", PORT))
        print(f"Mock board listening on UDP port {PORT}...")

        while True:
            data, addr = sock.recvfrom(1024)
            print(f"Received '{data}' from {addr}")
            
            if data == DISCOVERY_MESSAGE:
                sock.sendto(EXPECTED_RESPONSE, addr)
                print(f"Sent expected response to {addr}")
                sys.exit()
########################################################################

def get_audio(board_addr):
    board_ip, board_port = board_addr
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('0.0.0.0', PORT)) # pc bind its port
        s.listen()

        print(f'waiting for input from {board_ip}:{board_port}')
        
        while True:
            conn, addr = s.accept()

            if addr[0] != board_ip:
                print(f"ip not expected: {addr[0]}")
                conn.close()
                continue
            
            timestamp = time.strftime('%Y_%m_%d:%H_%M_%S')
            filename = os.path.join(INPUT_FOLDER, f"received_{timestamp}.wav")
            
            with open(filename, "wb") as f:
                while True:
                    chunk = conn.recv(4096)
                    if not chunk:
                            break
                    if END_MESSGE in chunk:
                        f.write(chunk.split(END_MESSGE)[0])
                        break
                    f.write(chunk)
            print(f'saved received audio: {filename}')

            # optional; limit num of audio files in folder
            clean_old_files(INPUT_FOLDER)

            return filename

def send_audio(file_path, board_ip):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((board_ip, OUT_PORT))
        
        with open(file_path, "rb") as f:
            while chunk := f.read(4096):
                s.sendall(chunk)
        
        # send a packet signaling END
        s.sendall(END_MESSGE)

def main():
    
    os.makedirs("input_files", exist_ok=True)
    os.makedirs("output_files", exist_ok=True)
    
    chat_history = llm.ChatHistory(max_size=10)
    
    board_addr = find_board()
    
    while(True):
        # get audio from board
        input_file = get_audio(board_addr)
        
        # audio -> text
        text = audio_to_text.audio_to_text(input_file)  
        
        # ai gen response
        resp = llm.get_ai_response(text, chat_history)
        
        # resp -> voice
        output_path = resp_to_sound.resp_to_sound(resp)
        
        # send back audio
        send_audio(output_path, board_addr[0])
        
        print(f'sent {output_path} to {board_addr}:10000')

if __name__ == "__main__":
    main()