from Gest import readData
import pyautogui
import time
import numpy as np
import ctypes
import win32api
import win32con

# Configuration - optimized for Minecraft
SENSITIVITY = 0.07          # Higher = faster cursor movement
DEAD_ZONE = 1.5           # Smaller = more sensitive to small movements
SMOOTHING_FACTOR = 0.15   # Lower = more responsive
POLL_RATE = 120           # Target updates per second
CLICK_THRESHOLD = 4       # Flex sensor value to trigger click

# Initialize
pyautogui.FAILSAFE = False
pyautogui.PAUSE = 0

# Windows API for direct mouse input
user32 = ctypes.windll.user32

# State variables
last_movement = np.zeros(2)
last_click_time = time.time()
click_delay = 0.3

def move_mouse(dx, dy):
    """Direct input method that works with Minecraft"""
    user32.mouse_event(win32con.MOUSEEVENTF_MOVE, int(dx), int(dy), 0, 0)

def process_movement(roll, pitch):
    """Calculate relative mouse movement with improved Minecraft response"""
    global last_movement
    
    # Apply deadzone with exponential scaling
    roll = 0 if abs(roll) < DEAD_ZONE else roll * 1.5
    pitch = 0 if abs(pitch) < DEAD_ZONE else pitch * 1.5
    
    # Non-linear sensitivity curve (better for small movements)
    movement = np.array([
        np.sign(roll) * (abs(roll)**1.3),
        np.sign(pitch) * (abs(pitch)**1.3)
    ]) * SENSITIVITY
    
    # Apply smoothing
    smoothed = last_movement + (movement - last_movement) * SMOOTHING_FACTOR
    last_movement = smoothed
    
    return smoothed

def handle_click(flex_value):
    """Direct input click handling for Minecraft"""
    global last_click_time
    current_time = time.time()
    
    if flex_value > CLICK_THRESHOLD:
        if (current_time - last_click_time) > click_delay:
            user32.mouse_event(win32con.MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0)
            last_click_time = current_time
        return True
    else:
        if win32api.GetKeyState(win32con.VK_LBUTTON) < 0:
            user32.mouse_event(win32con.MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
        return False

def main():
    print("Starting Minecraft-optimized mouse control")
    print("Press Ctrl+C to stop")
    
    # Set higher process priority (Windows only)
    try:
        ctypes.windll.kernel32.SetPriorityClass(ctypes.windll.kernel32.GetCurrentProcess(), 0x00008000)
    except:
        pass
    
    # Warm-up
    for _ in range(10):
        readData()
    
    try:
        while True:
            start_time = time.perf_counter()
            
            try:
                *_, roll, pitch, flex = readData()
                
                # Process movement
                dx, dy = process_movement(roll, pitch)
                if dx != 0 or dy != 0:
                    move_mouse(dx, dy)
                
                # Handle click
                handle_click(flex)
                
            except (TypeError, ValueError):
                continue
            
            # Maintain target poll rate
            elapsed = time.perf_counter() - start_time
            time.sleep(max(0, (1/POLL_RATE) - elapsed))
            
    except KeyboardInterrupt:
        # Ensure mouse button is released on exit
        if win32api.GetKeyState(win32con.VK_LBUTTON) < 0:
            user32.mouse_event(win32con.MOUSEEVENTF_LEFTUP, 0, 0, 0, 0)
        print("\nStopping Minecraft mouse control")

if __name__ == "__main__":
    print("5 seconds to switch to the Minecraft window...")
    time.sleep(5)  # Give time to switch to the game window
    main()