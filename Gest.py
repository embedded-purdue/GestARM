import serial
import re


try:
    ser = serial.Serial("COM3", 115200, timeout=0.1)
except Exception as e:
    print("Serial port error:", e)
    ser = None
float_pattern = re.compile(r"[-+]?\d*\.\d+|\d+")
    
def readData():
    while True:
        if ser:
                try:
                    if ser.in_waiting:
                        # Read a line from the serial port; expected format: "flag,x,y,z,w1,w2"
                        line = ser.readline()
                        line = line.decode("utf-8").strip()
                        # Skip empty lines
                        if line:
                            parts = line.split(',')
                            parts = float_pattern.findall(line)          
                            parts = [float(x) for x in parts[:6]]
                            if len(parts) >= 6:
                                try:
                                    # We ignore the first value (gripper flag) and the last two (wrist data).
                                    x_val = float(parts[0])
                                    y_val = float(parts[1])
                                    z_val = float(parts[2])
                                    roll = float(parts[3])
                                    pitch = float(parts[4])
                                    mV = float(parts[5])
                                    flexed = 1 if mV > 700 else 0
                                    return x_val, y_val, z_val, roll, pitch, flexed, mV
                                except ValueError:
                                    print("Error: Invalid float conversion in data:", line)
                except Exception as e:
                    print("Serial read error:", e)    

if __name__ == "__main__":
    while True:
        readData()
        data = readData()
        print(data)