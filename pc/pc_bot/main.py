# import audio_to_text
# import llm
# import resp_to_sound
import os
import sys
import socket
import time
import psutil
import netifaces as ni

UDP_PORT = 9998
IN_PORT = 9999
OUT_PORT = 10000

BROADCAST_IP1 = "255.255.255.255"
BROADCAST_IP2 = "192.168.0.255"

DISCOVERY_MESSAGE = b"WHERE_IS_MY_GIRLFRIEND"
EXPECTED_RESPONSE = b"IM_HERE"
END_MESSAGE = b"DONE"
TCP_READY = b"TCP"

MAX_TRY = 10

# max num of audio files saved in the input folder
MAX_FILES = 10
INPUT_FOLDER = os.path.join(os.getcwd(),"input_files")
OUTPUT_FOLDER = os.path.join(os.getcwd(),"output_files")


# def timestamp_to_int(filename):
#     t = filename[len("received_"):-len(".wav")]
#     return int(t.replace(":", "").replace("_", ""))

# def clean_old_files(folder):
#     all_files = os.listdir(folder)
#     all_files.sort(key=timestamp_to_int)

#     while len(all_files) > MAX_FILES:
#         to_be_removed = all_files.pop(0)
#         os.remove(os.path.join(folder, to_be_removed))

# def get_audio(board_addr):
#     board_ip, board_port = board_addr

#     with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#         s.bind(('0.0.0.0', PORT)) # pc bind its port
#         s.listen()

#         print(f'waiting for input from {board_ip}:{board_port}')

#         conn, addr = s.accept()
#         with conn:
#             print(f'{addr} connected')

#             timestamp = time.strftime('%Y_%m_%d:%H_%M_%S')
#             filename = os.path.join(INPUT_FOLDER, f"received_{timestamp}.wav")

#             with open(filename, "wb") as f:
#                 while True:
#                     chunk = conn.recv(4096)
#                     if END_MESSAGE in chunk:
#                         f.write(chunk.split(END_MESSAGE)[0])
#                         break
#                     f.write(chunk)
#             print(f'saved received audio: {filename}')

#             # optional; limit num of audio files in folder
#             clean_old_files(INPUT_FOLDER)

#             return filename

def send_audio(file_name, board_ip):
    final_path = os.path.join(OUTPUT_FOLDER,file_name)
    if not os.path.exists(final_path):
        print(f"Error: File '{final_path}' does not exist.")
        return
    print(f"File: {final_path}")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((board_ip, OUT_PORT))

        with open(final_path, "rb") as f:
            while chunk := f.read(3072):
                s.sendall(chunk)

        # send a packet signaling END
        time.sleep(1)
        s.sendall(END_MESSAGE)
        print("File sent.")

def get_correct_ip():
    for iface, addrs in psutil.net_if_addrs().items():
        if "Wi-Fi" in iface or "Wireless" in iface:  # Ensure it's the Wi-Fi adapter
            for addr in addrs:
                if addr.family == 2 and addr.address.startswith("192.168.0."):  # family = 2 --> AF_INET = IPv4
                    return addr.address

def find_board():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.settimeout(1)

        # local/PC
        local_ip = get_correct_ip()  # Get the IP address associated with the hostname
        print("local IP is: " + local_ip)
        sock.bind((local_ip, UDP_PORT))
        print("sock name after bind: "+str(sock.getsockname()))

        # hostname = socket.gethostname()
        # x_local_ip = socket.gethostbyname(hostname)
        # print("wrong local IP is: " + x_local_ip)

        board_addr = None

        for _ in range(MAX_TRY):
            sock.sendto(DISCOVERY_MESSAGE, (BROADCAST_IP1, UDP_PORT))
            # sock.bind(("192.168.0.27", UDP_PORT))
            try:
                for _ in range(5):
                    data, addr = sock.recvfrom(1024) # addr = (ip, port)
                    if data.strip() == EXPECTED_RESPONSE and addr[1] == UDP_PORT:
                        board_addr = addr
                        print(f"Receiving response: {data.strip()}")
                        print(f"board addr: {board_addr}")
                        # terminate UDP channel
                        time.sleep(1)
                        sock.sendto(END_MESSAGE, board_addr)
                        return board_addr
            except socket.timeout:
                print("timeout, retry...")
            time.sleep(0.5) # have to, otherwise keep receiving self-msg before board msg parsed here

    if not board_addr:
        print("can't find board")
        return None


def main():
    
    os.makedirs("input_files", exist_ok=True)
    os.makedirs("output_files", exist_ok=True)
    
    # chat_history = llm.ChatHistory(max_size=10)

    # connect board
    board_addr = find_board()
    if not board_addr: return
    # send audio
    send_audio("oof.wav",board_addr[0])

    # while(True):
    #     # get audio from board
    #     input_file = get_audio(board_addr)
        
    #     # audio -> text
    #     text = audio_to_text.audio_to_text(input_file)  
        
    #     # ai gen response
    #     resp = llm.get_ai_response(text, chat_history)
        
    #     # resp -> voice
    #     output_path = resp_to_sound.resp_to_sound(resp)
        
    #     # send back audio
    #     send_audio(output_path, board_addr[0])
        
    #     print(f'sent {output_path} to {board_addr}:10000')

if __name__ == "__main__":
    main()