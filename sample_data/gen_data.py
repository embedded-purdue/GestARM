import csv
import random

sensor_polling_rate = 100  # Hz
num_samples = 1000

accel_max = 100
accel_min = -100

gyro_max = 180
gyro_min = -180

flex_max = 100
flex_min = 0

rand_max = 10
rand_min = -10

noiseBool = True

def noise():
    return random.randint(rand_min, rand_max) if noiseBool else 0


# Null mpu

data = []

for i in range(num_samples):
    x = noise()
    y = noise()
    z = noise()
    p = noise()
    r = noise()
    w = noise()
    data.append([i, x, y, z, p, r, w])

file_path = 'sample_data/mpu_null.csv'

with open(file_path, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(data)

print(f"Data has been written to {file_path}")


# Only Accel mpu

# Graph of x accel:
# _/\  _
#    \/ 

data = []
accel = 0

for i in range(num_samples):
    if i <= 1 * num_samples // 6:
        accel = 0
    elif i <= 2 * num_samples // 6:
        accel += accel_max * 6 / num_samples
    elif i <= 3 * num_samples // 6:
        accel -= accel_max * 6 / num_samples
    elif i <= 4 * num_samples // 6:
        accel += accel_min * 6 / num_samples
    elif i <= 5 * num_samples // 6:
        accel -= accel_min * 6 / num_samples
    elif i <= 6 * num_samples // 6:
        accel = 0
        
    x = noise() + int(accel)
    y = noise()
    z = noise()
    p = noise()
    r = noise()
    w = noise()
    
    if x > accel_max:
        x = accel_max
    elif x < accel_min:
        x = accel_min
    
    data.append([i, x, y, z, p, r, w])
    
file_path = 'sample_data/mpu_accel.csv'

with open(file_path, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(data)
    
print(f"Data has been written to {file_path}")


# Only Gyro mpu

# Graph of p gyro:
# _/\  _
#    \/

data = []
gyro = 0

for i in range(num_samples):
    if i <= 1 * num_samples // 6:
        gyro = 0
    elif i <= 2 * num_samples // 6:
        gyro += gyro_max * 6 / num_samples
    elif i <= 3 * num_samples // 6:
        gyro -= gyro_max * 6 / num_samples
    elif i <= 4 * num_samples // 6:
        gyro += gyro_min * 6 / num_samples
    elif i <= 5 * num_samples // 6:
        gyro -= gyro_min * 6 / num_samples
    elif i <= 6 * num_samples // 6:
        gyro = 0
        
    x = noise()
    y = noise()
    z = noise()
    p = noise() + int(gyro)
    r = noise()
    w = noise()
    
    if x > gyro_max:
        x = gyro_max
    elif x < gyro_min:
        x = gyro_min
    
    data.append([i, x, y, z, p, r, w])
    
file_path = 'sample_data/mpu_gyro.csv'

with open(file_path, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(data)
    
print(f"Data has been written to {file_path}")


# Null Flex sensor

data = []

for i in range(num_samples):
    f = noise()
    
    data.append([i, f])
    
file_path = 'sample_data/flex_null.csv'

with open(file_path, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(data)
    
print(f"Data has been written to {file_path}")

# Only Flex sensor

# Graph of flex:
# _/\_

data = []
flex = 0

for i in range(num_samples):
    if i <= 1 * num_samples // 4:
        flex = 0
    elif i <= 2 * num_samples // 4:
        flex += flex_max * 4 / num_samples
    elif i <= 3 * num_samples // 4:
        flex -= flex_max * 4 / num_samples
    elif i <= 4 * num_samples // 4:
        flex = 0
        
    f = noise() + int(flex)
    
    if f > flex_max:
        f = flex_max
    elif f < flex_min:
        f = flex_min
    
    data.append([i, f])
    
file_path = 'sample_data/flex_flex.csv'

with open(file_path, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerows(data)
    
print(f"Data has been written to {file_path}")