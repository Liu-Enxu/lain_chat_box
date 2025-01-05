import os

# input files: raw sound files from the board
# output files: sound responses going back to the board
os.makedirs("input_files", exist_ok=True)
os.makedirs("output_files", exist_ok=True)

inputs_folder_path = '/input_files'
outputs_folder_path = '/output_files'

# convert sound file to text. param: filename of the sound file
# sound file should exist in the input_files folder
# returns a string of the processed text
def sound_to_text(filename):
    file_path = os.path.join(inputs_folder_path, filename)
    
    if os.path.isfile(file_path):
        # convert to text
        msg = ''
        pass
    
        return msg
        
    else:
        print(f'File {filename} does not exist in the inputs folder.')

# send user chat msg to ai
# returns the ai response
def get_ai_response(msg):
    # ai output -> resp
    resp = ''
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

def start_listen():
    while True:
        pass
        
        # some how gets input
        filename = ''
        msg = sound_to_text(filename)
        resp = get_ai_response(msg)
        output = resp_to_sound(resp, filename)
        
        # sends output back to board

def main():
    
    # loop that listens for anything coming from the board
    start_listen()

if __name__ == "__main__":
    main()