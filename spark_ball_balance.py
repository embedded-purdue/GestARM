import pygame
import sys
import math
from pygame import gfxdraw
import random
import Gest

death_sound_on = True

# Initialize pygame
pygame.init()
pygame.mixer.init()
coin_collect_sound = pygame.mixer.Sound(r"sounds\coin_collect.wav")
rolling_sound = pygame.mixer.Sound(r"sounds\rolling_ball.wav")
level_up_sound = pygame.mixer.Sound(r"sounds\level_up.wav")
pipe_death_sound = pygame.mixer.Sound(r"sounds\pipe_death.wav")
mario_death_sound = pygame.mixer.Sound(r"sounds\mario_death.wav")
mario_death_sound.set_volume(0.5)  # Set volume for mario death sound
rolling_channel = pygame.mixer.Channel(0)  
coin_channel = pygame.mixer.Channel(1)
pipe_death_channel = pygame.mixer.Channel(2)   
mario_death_channel = pygame.mixer.Channel(3)  
level_up_channel = pygame.mixer.Channel(4)

# Constants
WIDTH, HEIGHT = 1000, 900
BOARD_SIZE = 400
BALL_RADIUS = 20
COIN_RADIUS = 15
GRAVITY = 0.6 # Gravity strength, higher is stronger
FRICTION = 0.98 # 0-1, 1 is no friction, 0 is full friction
MAX_TILT = 0.2  # Maximum tilt angle in radians
TILT_SPEED = 0.02
GAME_TIME = 2000 # Displayed // 10
COIN_SCORE_VALUE = 1  # Points for collecting a coin
ROUND_GOAL = 5
BLACK_COLOR_ROUND = 10 # black color of the board when this round is reached
GRAVITY_INCREMENTOR = 0.1  # Increase gravity by this factor each round
FRICTION_MULTIPLIER = 0.9  # Decrease friction by this factor each round

BOARD_COLOR = (100, 70, 40)
BALL_COLOR = (200, 50, 50)
COIN_COLOR = (255, 215, 0)  # Gold color for coins
BACKGROUND_COLOR = (50, 50, 70)
TEXT_COLOR = (255, 255, 255)
EDGE_COLOR = (100, 100, 140)

# Set up the display
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Board Balancing Game")
clock = pygame.time.Clock()
font = pygame.font.SysFont('Arial', 36)
small_font = pygame.font.SysFont('Arial', 24)

def map_value(value, x_min, x_max, y_min, y_max):
    return y_min + (value - x_min) * (y_max - y_min) / (x_max - x_min)

class Ball:
    def __init__(self):
        self.x = 0
        self.y = 0
        self.vx = 0
        self.vy = 0
        self.gravity = GRAVITY
        self.friction = FRICTION
        self.radius = BALL_RADIUS
        
    def update(self, tilt_x, tilt_y):
        # Apply gravity based on tilt
        self.vx += tilt_x * self.gravity
        self.vy += tilt_y * self.gravity
        
        # Apply friction
        self.vx *= self.friction
        self.vy *= self.friction
        
        # Update position
        self.x += self.vx
        self.y += self.vy
        
        # Play rolling sound with volume based on velocity
        rolling_volume = (abs(self.vx) + abs(self.vy)) / 10
        rolling_channel.set_volume(rolling_volume)
        if not rolling_channel.get_busy():  # Only play if not already playing
            rolling_channel.play(rolling_sound, loops=-1)
        
    def draw(self, screen, offset_x, offset_y, tilt_x, tilt_y):
        # Adjust ball position based on tilt for perspective
        perspective_factor = 0.3
        
        draw_x = offset_x + self.x - tilt_x * self.y * perspective_factor
        draw_y = offset_y + self.y - tilt_y * self.x * perspective_factor
        
        # Draw shadow
        shadow_offset = 5
        br_x = -tilt_x + -abs(tilt_y) 
        br_y = -abs(tilt_x) + -tilt_y 
        pygame.draw.circle(screen, (0, 0, 0, 100), 
                         (int(draw_x + shadow_offset - br_x * 8), int(draw_y + shadow_offset - br_y * 8)), 
                         self.radius)
        
        # Draw the ball
        pygame.draw.circle(screen, BALL_COLOR, (int(draw_x), int(draw_y)), self.radius)
        
        # Add highlight
        highlight_pos = (draw_x - self.radius/3, draw_y - self.radius/3)
        pygame.draw.circle(screen, (255, 255, 255, 150), 
                         (int(highlight_pos[0]), int(highlight_pos[1])), 
                         self.radius//3)
        
    def is_off_board(self):
        return (abs(self.x) - 10 > BOARD_SIZE/2 or abs(self.y) - 10 > BOARD_SIZE/2)  # 10 is a grace margin

class Coin:
    def __init__(self):
        self.x = random.uniform(-BOARD_SIZE/2 + 30, BOARD_SIZE/2 - 30)
        self.y = random.uniform(-BOARD_SIZE/2 + 30, BOARD_SIZE/2 - 30)
        self.radius = COIN_RADIUS
        self.collected = False
        
    def draw(self, screen, offset_x, offset_y, tilt_x, tilt_y):
        if self.collected:
            return
            
        # Adjust coin position based on tilt for perspective
        perspective_factor = 0.5
        draw_x = offset_x + self.x - tilt_x * self.x * perspective_factor
        draw_y = offset_y + self.y - tilt_y * self.y * perspective_factor
        
        # Draw coin
        pygame.draw.circle(screen, COIN_COLOR, (int(draw_x), int(draw_y)), self.radius)
        
        # Add some details to make it look like a coin
        pygame.draw.circle(screen, (200, 180, 0), (int(draw_x), int(draw_y)), self.radius - 3)
        pygame.draw.circle(screen, COIN_COLOR, (int(draw_x), int(draw_y)), self.radius - 6)
        
    def check_collision(self, ball_x, ball_y, ball_radius):
        if self.collected:
            return False
            
        distance = math.sqrt((self.x - ball_x)**2 + (self.y - ball_y)**2)
        collision = distance < (self.radius + ball_radius)
        return collision

class Game:
    def __init__(self):
        self.ball = Ball()
        self.coins = []
        self.target_tilt_x = 0
        self.target_tilt_y = 0
        self.current_tilt_x = 0
        self.current_tilt_y = 0
        self.score = 0
        self.round = 0
        self.time = GAME_TIME
        self.game_over = False
        self.reset_game()
        
    @staticmethod
    def map_axis(val, in_min=-20, in_max=20, out_min=-10, out_max=10):
        return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
        
    def reset_game(self):
        self.ball.x = random.uniform(-50, 50)
        self.ball.y = random.uniform(-50, 50)
        self.ball.vx = 0
        self.ball.vy = 0
        self.ball.gravity = GRAVITY
        self.ball.friction = FRICTION
        self.round = 0
        self.target_tilt_x = 0
        self.target_tilt_y = 0
        self.current_tilt_x = 0
        self.current_tilt_y = 0
        self.score = 0
        self.time = GAME_TIME
        self.game_over = False
        self.coins = []
        self.spawn_coin()  # Start with one coin
        
    def spawn_coin(self):
        self.coins.append(Coin())
        
    def update(self):
        if self.game_over:
            return
            
        # Smoothly interpolate current tilt toward target tilt
        self.current_tilt_x += (self.target_tilt_x - self.current_tilt_x) * 0.1
        self.current_tilt_y += (self.target_tilt_y - self.current_tilt_y) * 0.1
        
        # Update ball physics with current tilt
        self.ball.update(self.current_tilt_x, self.current_tilt_y)
        
        # Check for coin collisions
        for coin in self.coins:
            if not coin.collected and coin.check_collision(self.ball.x, self.ball.y, self.ball.radius):
                coin.collected = True
                self.score += COIN_SCORE_VALUE
                self.spawn_coin()  # Spawn a new coin when one is collected
                coin_channel.play(coin_collect_sound)  # Play coin collect sound
        
        # Remove collected coins
        self.coins = [coin for coin in self.coins if not coin.collected]
        
        # Check if ball fell off
        if self.ball.is_off_board():
            rolling_channel.stop()
            if death_sound_on:
                if random.randint(0, 10) == 0:
                    pipe_death_channel.play(pipe_death_sound)
                else:
                    mario_death_channel.play(mario_death_sound)
            self.game_over = True
        else:
            self.time -= 1  # Small score increase for staying alive
            
        if self.time <= 0:
            self.game_over = True

        if self.score >= ROUND_GOAL and self.score > 0:
            self.ball.gravity += GRAVITY_INCREMENTOR
            self.ball.friction = 1 - ((1 - self.ball.friction) * FRICTION_MULTIPLIER)
            self.time = GAME_TIME - self.round * 100
            self.score = 0
            self.round += 1
            level_up_channel.play(level_up_sound)
            
    def draw_board(self, screen, center_x, center_y):
        half_size = BOARD_SIZE // 2
        tilt_x = self.current_tilt_x
        tilt_y = self.current_tilt_y
        abs_x = abs(tilt_x)
        abs_y = abs(tilt_y)
        sign_x = 1 if tilt_x >= 0 else -1
        sign_y = 1 if tilt_y >= 0 else -1
        
        tl_x = -tilt_y   +    abs_x 
        tl_y =  abs_y    +   -tilt_x
        tr_x =  tilt_y   +   -abs_x  
        tr_y =  abs_y    +    tilt_x 
        bl_x =  tilt_y   +    abs_x 
        bl_y = -abs_y    +    tilt_x 
        br_x = -tilt_y   +   -abs_x 
        br_y = -abs_y    +   -tilt_x 
        
        # Calculate the four corners with perspective distortion
        top_left = (
            center_x - half_size + tl_x * half_size * 0.5,
            center_y - half_size + tl_y * half_size * 0.5
        )
        top_right = (
            center_x + half_size + tr_x * half_size * 0.5,
            center_y - half_size + tr_y * half_size * 0.5
        )
        bottom_left = (
            center_x - half_size + bl_x * half_size * 0.5,
            center_y + half_size + bl_y * half_size * 0.5
        )
        bottom_right = (
            center_x + half_size + br_x * half_size * 0.5,
            center_y + half_size + br_y * half_size * 0.5
        )
        
        # Draw the board as a trapezoid
        pygame.gfxdraw.filled_polygon(screen, [
            top_left, top_right, bottom_right, bottom_left
        ], (map_value(-self.round, -BLACK_COLOR_ROUND, 0, 0, BOARD_COLOR[0]) if map_value(-self.round, -BLACK_COLOR_ROUND, 0, 0, BOARD_COLOR[0]) > 0 else 0, 
            map_value(-self.round, -BLACK_COLOR_ROUND, 0, 0, BOARD_COLOR[1]) if map_value(-self.round, -BLACK_COLOR_ROUND, 0, 0, BOARD_COLOR[1]) > 0 else 0,
            map_value(-self.round, -BLACK_COLOR_ROUND, 0, 0, BOARD_COLOR[2]) if map_value(-self.round, -BLACK_COLOR_ROUND, 0, 0, BOARD_COLOR[2]) > 0 else 0)
        )
        
        
        # Draw board edges
        pygame.draw.line(screen, EDGE_COLOR, top_left, top_right, 3)
        pygame.draw.line(screen, EDGE_COLOR, top_right, bottom_right, 3)
        pygame.draw.line(screen, EDGE_COLOR, bottom_right, bottom_left, 3)
        pygame.draw.line(screen, EDGE_COLOR, bottom_left, top_left, 3)
        
    def draw(self, screen):
        # Calculate board position (center of screen)
        board_x = WIDTH // 2
        board_y = HEIGHT // 2
        
        # Draw board with perspective
        self.draw_board(screen, board_x, board_y)
        
        # Draw coins
        for coin in self.coins:
            coin.draw(screen, board_x, board_y, self.current_tilt_x, self.current_tilt_y)
        
        # Draw ball with perspective
        self.ball.draw(screen, board_x, board_y, self.current_tilt_x, self.current_tilt_y)
        
        # Draw score
        score_text = font.render(f"Round {self.round}, Score: {self.score} / {ROUND_GOAL}", True, TEXT_COLOR)
        screen.blit(score_text, (20, 20))
        score_text = font.render(f"Time: {self.time // 10}", True, TEXT_COLOR)
        screen.blit(score_text, (20, 50))
    
        # Draw tilt meter
        self.draw_tilt_meter(screen, WIDTH - 150, 30, self.current_tilt_x, "X")
        self.draw_tilt_meter(screen, WIDTH - 150, 80, self.current_tilt_y, "Y")
        
        # Draw game over message
        if self.game_over:
            game_over_text = font.render("Game Over! Press R to restart", True, TEXT_COLOR)
            text_rect = game_over_text.get_rect(center=(WIDTH//2, HEIGHT//2 - 50))
            screen.blit(game_over_text, text_rect)
            
            final_score_text = font.render(f"Final Score: {self.round * ROUND_GOAL + self.score}", True, TEXT_COLOR)
            score_rect = final_score_text.get_rect(center=(WIDTH//2, HEIGHT//2 + 10))
            screen.blit(final_score_text, score_rect)
    
    def draw_tilt_meter(self, screen, x, y, value, axis):
        # Draw background
        pygame.draw.rect(screen, (50, 50, 50), (x, y, 120, 30))
        
        # Calculate fill width (-1 to 1 range mapped to 0-120)
        fill_width = int((value / MAX_TILT) * 60 + 60)
        fill_width = max(0, min(120, fill_width))
        
        # Draw fill
        color = (255, 0, 0) if abs(value) > MAX_TILT * 0.8 else (0, 200, 0)
        pygame.draw.rect(screen, color, (x, y, fill_width, 30))
        
        # Draw center line
        pygame.draw.line(screen, (255, 255, 255), (x + 60, y), (x + 60, y + 30), 2)
        
        # Draw label
        label = small_font.render(f"{axis}: {value:.2f}", True, TEXT_COLOR)
        screen.blit(label, (x + 5, y + 5))

def key_press_main():
    game = Game()
    
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_r and game.game_over:
                    game.reset_game()
        
        # Get keyboard input for tilting
        keys = pygame.key.get_pressed()
        if not game.game_over:
            if keys[pygame.K_LEFT]:
                game.target_tilt_x = -MAX_TILT 
            elif keys[pygame.K_RIGHT]:
                game.target_tilt_x = MAX_TILT 
            else:
                game.target_tilt_x = 0
                
            if keys[pygame.K_UP]:
                game.target_tilt_y = -MAX_TILT 
            elif keys[pygame.K_DOWN]:
                game.target_tilt_y = MAX_TILT 
            else:
                game.target_tilt_y = 0
        
        # Update game state
        game.update()
        
        # Draw everything
        screen.fill(BACKGROUND_COLOR)
        game.draw(screen)
        
        pygame.display.flip()
        clock.tick(60)

def main():
    game = Game()
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_r and game.game_over:
                    game.reset_game()
      
        if not game.game_over:
            data = Gest.readData()
            rollx = data[3]
            pitchy = data[4]
                                      
            game.target_tilt_x = rollx / 120
            game.target_tilt_y = -pitchy / 120
        else: # Game over state
            game.target_tilt_x = 0
            game.target_tilt_y = 0
            data = Gest.readData()
            flexed = data[5]
            if flexed > 0.5:
                game.reset_game()
        # Update game state and draw
        game.update()
        screen.fill(BACKGROUND_COLOR)
        game.draw(screen)
        pygame.display.flip()
        clock.tick(60)
        
if __name__ == "__main__":
    main()
