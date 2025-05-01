from Gest import readData
import pyautogui
import time
import numpy as np

# Configuration - adjust these for optimal performance
SENSITIVITY = 1           # Higher = faster cursor movement
DEAD_ZONE = 3             # Smaller = more sensitive to small movements
SMOOTHING_FACTOR = 0.3    # Lower = more responsive (0.1-0.3 recommended)
CLICK_THRESHOLD = 4       # Flex sensor value to trigger click
POLL_RATE = 60            # Target updates per second (max your device can handle)

# Initialize
screen_width, screen_height = pyautogui.size()
pyautogui.FAILSAFE = True
pyautogui.PAUSE = 0       # Remove pyautogui's built-in delay

# State variables
last_pos = np.array(pyautogui.position())
last_click_time = time.time()
click_delay = 0.5         # Minimum time between clicks

def process_movement(roll, pitch):
    """Optimized movement processing with numpy"""
    global last_pos
    
    # Apply deadzone (vectorized for speed)
    roll = 0 if abs(roll) < DEAD_ZONE else roll
    pitch = 0 if abs(pitch) < DEAD_ZONE else pitch
    
    # Calculate movement (vectorized)
    movement = np.array([roll, pitch]) * SENSITIVITY
    
    # Apply exponential smoothing
    new_pos = last_pos + movement
    smoothed_pos = last_pos + (new_pos - last_pos) * SMOOTHING_FACTOR
    
    # Constrain to screen bounds
    smoothed_pos = np.clip(smoothed_pos, [10, 10], [screen_width-1 - 10, screen_height-1 - 10])
    
    last_pos = smoothed_pos
    return smoothed_pos.astype(int)

def handle_click(flex_value):
    """Optimized click handling"""
    global last_click_time
    current_time = time.time()
    
    if flex_value > 0.5 and (current_time - last_click_time) > click_delay:
        pyautogui.click()
        last_click_time = current_time
        return True
    return False

def main():
    print(f"Starting optimized mouse control (Target: {POLL_RATE}Hz)")
    print("Press Ctrl+C to stop")
    
    # Warm-up the serial connection
    for _ in range(10):
        readData()
    
    try:
        while True:
            start_time = time.perf_counter()
            
            try:
                # Get only the data we need
                _, _, _, roll, pitch, flex, _ = readData()
                
                # Process movement
                x, y = process_movement(roll, pitch)
                pyautogui.moveTo(x, y)
                
                # Handle click
                handle_click(flex)
                
            except (TypeError, ValueError):
                continue
            
            # Dynamic sleep to maintain target poll rate
            elapsed = time.perf_counter() - start_time
            sleep_time = max(0, (1/POLL_RATE) - elapsed)
            time.sleep(sleep_time)
            
    except KeyboardInterrupt:
        print("\nStopping mouse control")

if __name__ == "__main__":
    main()