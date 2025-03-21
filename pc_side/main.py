import get_audio
import audio_to_text
import llm
import resp_to_sound
import send_audio
import os
import socket

PORT = 9999
DISCOVERY_MESSAGE = b"WHERE_IS_MY_GIRLFRIEND"
EXPECTED_RESPONSE = b"IM_HERE"
MAX_TRY = 10

def find_board():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.settimeout(3)
        
        board_addr = None
        for _ in range(MAX_TRY):
            sock.sendto(DISCOVERY_MESSAGE, ("255.255.255.255", PORT))
            
            try:
                data, addr = sock.recvfrom(1024)
                if data == EXPECTED_RESPONSE:
                    board_addr = addr
                    print(f"board addr: {board_addr}")
                    break
            except socket.timeout:
                print("timeout, retry...")

    if not board_addr:
        raise Exception("can't find board :(")

    return board_addr

def main():
    
    os.makedirs("input_files", exist_ok=True)
    os.makedirs("output_files", exist_ok=True)
    
    chat_history = llm.ChatHistory(max_size=10)
    
    board_addr = find_board()
    
    while(True):
        # get audio from board
        input_file = get_audio.get_audio(board_addr)
        
        # audio -> text
        text = audio_to_text.audio_to_text(input_file)  
        
        # ai gen response
        resp = llm.get_ai_response(text, chat_history)
        
        # resp -> voice
        output_path = resp_to_sound.resp_to_sound(resp)
        
        # send back audio
        send_audio.send_audio(output_path, board_addr[0])
        
        print(f'sent {output_path} to {board_addr}')

if __name__ == "__main__":
    main()