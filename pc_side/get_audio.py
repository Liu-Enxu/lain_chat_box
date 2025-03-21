import os
import socket
import time

INPUT_FOLDER = "input_files"

# max num of audio files saved in the input folder
MAX_FILES = 10

def timestamp_to_int(filename):
    t = filename[len("received_"):-len(".wav")]
    return t.replace(":", "").replace("_", "")

def clean_old_files(folder):
    all_files = os.listdir(folder)
    all_files.sort(key=timestamp_to_int)
    
    while len(all_files) > MAX_FILES:
        to_be_removed = all_files.pop(0)
        os.remove(os.path.join(folder, to_be_removed))

def get_audio(board_addr):
    board_ip, board_port = board_addr
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((board_ip, board_port))
        s.listen()

        print(f'waiting for input from {board_ip}:{board_port}')
        
        conn, addr = s.accept()
        with conn:
            print(f'{addr} connected')
            
            timestamp = time.strftime('%Y_%m_%d:%H_%M_%S')
            filename = os.path.join(INPUT_FOLDER, f"received_{timestamp}.wav")
            
            with open(filename, "wb") as f:
                while chunk := conn.recv(4096):
                    f.write(chunk)
            
            print(f'saved received audio: {filename}')

            # optional; limit num of audio files in folder
            clean_old_files(INPUT_FOLDER)

            return filename
