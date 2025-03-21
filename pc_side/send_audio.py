import socket

PORT = 10000

def send_audio(file_path, board_ip):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((board_ip, PORT))
        
        with open(file_path, "rb") as f:
            while chunk := f.read(4096):
                s.sendall(chunk)
