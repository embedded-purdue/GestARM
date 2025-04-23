import pygame
import sys
import math
from pygame import gfxdraw
import random

# Initialize pygame
pygame.init()

# Constants
WIDTH, HEIGHT = 800, 600
BOARD_SIZE = 400
BALL_RADIUS = 20
GRAVITY = 0.5
FRICTION = 0.98
MAX_TILT = 0.2  # Maximum tilt angle in radians
TILT_SPEED = 0.02
BOARD_COLOR = (100, 70, 40)
BALL_COLOR = (200, 50, 50)
BACKGROUND_COLOR = (50, 50, 70)
TEXT_COLOR = (255, 255, 255)
EDGE_COLOR = (70, 50, 30)

# Set up the display
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Board Balancing Game with Perspective")
clock = pygame.time.Clock()
font = pygame.font.SysFont('Arial', 36)
small_font = pygame.font.SysFont('Arial', 24)

class Ball:
    def __init__(self):
        self.x = 0
        self.y = 0
        self.vx = 0
        self.vy = 0
        self.radius = BALL_RADIUS
        
    def update(self, tilt_x, tilt_y):
        # Apply gravity based on tilt
        self.vx += tilt_x * GRAVITY
        self.vy += tilt_y * GRAVITY
        
        # Apply friction
        self.vx *= FRICTION
        self.vy *= FRICTION
        
        # Update position
        self.x += self.vx
        self.y += self.vy
        
    def draw(self, screen, offset_x, offset_y, tilt_x, tilt_y):
        # Adjust ball position based on tilt for perspective
        perspective_factor = 0.3
        draw_x = offset_x + self.x - tilt_x * self.y * perspective_factor
        draw_y = offset_y + self.y - tilt_y * self.x * perspective_factor
        
        # Draw shadow
        shadow_offset = 5 * (1 + (abs(tilt_x) + abs(tilt_y)) * 2)
        pygame.draw.circle(screen, (0, 0, 0, 100), 
                         (int(draw_x + shadow_offset), int(draw_y + shadow_offset)), 
                         self.radius)
        
        # Draw the ball
        pygame.draw.circle(screen, BALL_COLOR, (int(draw_x), int(draw_y)), self.radius)
        
        # Add highlight
        highlight_pos = (draw_x - self.radius/3, draw_y - self.radius/3)
        pygame.draw.circle(screen, (255, 255, 255, 150), 
                         (int(highlight_pos[0]), int(highlight_pos[1])), 
                         self.radius//3)
        
    def is_off_board(self):
        return (abs(self.x) > BOARD_SIZE/2 or abs(self.y) > BOARD_SIZE/2)

class Game:
    def __init__(self):
        self.ball = Ball()
        self.target_tilt_x = 0
        self.target_tilt_y = 0
        self.current_tilt_x = 0
        self.current_tilt_y = 0
        self.score = 0
        self.game_over = False
        self.reset_game()
        
    def reset_game(self):
        self.ball.x = random.uniform(-50, 50)
        self.ball.y = random.uniform(-50, 50)
        self.ball.vx = 0
        self.ball.vy = 0
        self.target_tilt_x = 0
        self.target_tilt_y = 0
        self.current_tilt_x = 0
        self.current_tilt_y = 0
        self.score = 0
        self.game_over = False
        
    def update(self):
        if self.game_over:
            return
            
        # Smoothly interpolate current tilt toward target tilt
        self.current_tilt_x += (self.target_tilt_x - self.current_tilt_x) * 0.1
        self.current_tilt_y += (self.target_tilt_y - self.current_tilt_y) * 0.1
        
        # Update ball physics with current tilt
        self.ball.update(self.current_tilt_x, self.current_tilt_y)
        
        # Check if ball fell off
        if self.ball.is_off_board():
            self.game_over = True
        else:
            self.score += 1
            
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
        ], BOARD_COLOR)
        
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
        
        # Draw ball with perspective
        self.ball.draw(screen, board_x, board_y, self.current_tilt_x, self.current_tilt_y)
        
        # Draw score
        score_text = font.render(f"Score: {self.score}", True, TEXT_COLOR)
        screen.blit(score_text, (20, 20))
        
        # Draw tilt information
        tilt_text = small_font.render(
            f"Tilt X: {self.current_tilt_x:.3f}  Tilt Y: {self.current_tilt_y:.3f}", 
            True, TEXT_COLOR
        )
        screen.blit(tilt_text, (20, 60))
        
        # Draw tilt meter
        self.draw_tilt_meter(screen, WIDTH - 150, 30, self.current_tilt_x, "X")
        self.draw_tilt_meter(screen, WIDTH - 150, 80, self.current_tilt_y, "Y")
        
        # Draw game over message
        if self.game_over:
            game_over_text = font.render("Game Over! Press R to restart", True, TEXT_COLOR)
            text_rect = game_over_text.get_rect(center=(WIDTH//2, HEIGHT//2 - 50))
            screen.blit(game_over_text, text_rect)
            
            final_score_text = font.render(f"Final Score: {self.score}", True, TEXT_COLOR)
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

if __name__ == "__main__":
    main()
