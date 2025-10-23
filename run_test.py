import os
import time
import subprocess
import signal

# Test duration
HOURS = 0
MINUTES = 1
SECONDS = 0

if __name__ == "__main__":

    write = os.path.abspath("build/write")
    read = os.path.abspath("build/read")

    # Start write process
    write_proc = subprocess.Popen(["taskset", "-c", "1", write], cwd=os.path.dirname(write))
    time.sleep(3) 

    # Start read process, read output text
    read_proc = subprocess.Popen(["taskset", "-c", "2", read], stdout=subprocess.PIPE, text=True , cwd=os.path.dirname(read))

    # Sleep for a while
    time.sleep( (HOURS * 3600) + (MINUTES * 60) + SECONDS ) 

    # Stop reader
    read_proc.send_signal(signal.SIGINT)
    read_proc.wait()

    # Stop writer
    write_proc.terminate()

    # Generate Diagram
    subprocess.run(["python3", "gen_diagram.py"])
    print("Test completed!")


