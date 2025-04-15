import os
import time

OUTPUT_FOLDER = "output_files"
MAX_FILES = 10

os.makedirs(OUTPUT_FOLDER, exist_ok=True)

def timestamp_to_int(filename):
    t = filename[len("response_"):-len(".wav")]
    return t.replace(":", "").replace("_", "")

def clean_old_files(folder):
    all_files = os.listdir(folder)
    all_files.sort(key=timestamp_to_int)
    
    while len(all_files) > MAX_FILES:
        to_be_removed = all_files.pop(0)
        os.remove(os.path.join(folder, to_be_removed))

def resp_to_sound(resp_text):
    timestamp = time.strftime('%Y_%m_%d:%H_%M_%S')
    output_filename = f"response_{timestamp}.wav"
    output_path = os.path.join(OUTPUT_FOLDER, output_filename)

    # TODO: impl actual func

    clean_old_files(OUTPUT_FOLDER)

    return output_path
