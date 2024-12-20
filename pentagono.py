import os
os.environ["SDL_AUDIODRIVER"] = "dummy"
import pygame
import random
import time

# Inicializar Pygame
pygame.init()

# Configurações da tela
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Desafio do Pentágono Fictício")

# Cores
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (255, 0, 0)
GREEN = (0, 255, 0)
BLUE = (0, 0, 255)

# Fonte
font = pygame.font.Font(None, 36)

def draw_text(text, color, x, y, center=False):
    """Desenhar texto na tela."""
    text_surface = font.render(text, True, color)
    text_rect = text_surface.get_rect()
    if center:
        text_rect.center = (x, y)
    else:
        text_rect.topleft = (x, y)
    screen.blit(text_surface, text_rect)

def stage_password():
    """Etapa 1: Tela para adivinhar a senha."""
    password = "segredo"
    input_text = ""
    running = True
    while running:
        screen.fill(WHITE)
        draw_text("Etapa 1: Adivinhe a senha", BLACK, WIDTH // 2, 50, center=True)
        draw_text("Dica: É algo que ninguém gosta de compartilhar.", BLACK, WIDTH // 2, 100, center=True)
        draw_text(f"Sua tentativa: {input_text}", BLACK, WIDTH // 2, 200, center=True)
        draw_text("Pressione Enter para confirmar", BLUE, WIDTH // 2, 250, center=True)
        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RETURN:
                    if input_text == password:
                        return True
                    else:
                        return False
                elif event.key == pygame.K_BACKSPACE:
                    input_text = input_text[:-1]
                else:
                    input_text += event.unicode
    return False

def stage_enigma():
    """Etapa 2: Resolver um enigma."""
    enigmas = [
        ("O que tem um pescoço, mas não tem cabeça?", "garrafa"),
        ("Quanto mais você tira, maior fica. O que é?", "buraco"),
        ("O que sempre sobe, mas nunca desce?", "idade")
    ]
    question, answer = random.choice(enigmas)
    input_text = ""
    running = True
    while running:
        screen.fill(WHITE)
        draw_text("Etapa 2: Resolva o enigma", BLACK, WIDTH // 2, 50, center=True)
        draw_text(question, BLACK, WIDTH // 2, 100, center=True)
        draw_text(f"Sua resposta: {input_text}", BLACK, WIDTH // 2, 200, center=True)
        draw_text("Pressione Enter para confirmar", BLUE, WIDTH // 2, 250, center=True)
        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RETURN:
                    if input_text.lower() == answer:
                        return True
                    else:
                        return False
                elif event.key == pygame.K_BACKSPACE:
                    input_text = input_text[:-1]
                else:
                    input_text += event.unicode
    return False

def stage_firewall():
    """Etapa 3: Evitar o firewall com cálculo rápido."""
    num1 = random.randint(1, 10)
    num2 = random.randint(1, 10)
    correct_answer = num1 + num2
    input_text = ""
    start_time = time.time()

    running = True
    while running:
        elapsed_time = time.time() - start_time
        screen.fill(WHITE)
        draw_text("Etapa 3: Evite o firewall!", BLACK, WIDTH // 2, 50, center=True)
        draw_text(f"Quanto é {num1} + {num2}?", BLACK, WIDTH // 2, 100, center=True)
        draw_text(f"Sua resposta: {input_text}", BLACK, WIDTH // 2, 200, center=True)
        draw_text(f"Tempo restante: {max(0, 5 - int(elapsed_time))}s", RED, WIDTH // 2, 250, center=True)
        pygame.display.flip()

        if elapsed_time > 5:
            return False

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RETURN:
                    if input_text.isdigit() and int(input_text) == correct_answer:
                        return True
                    else:
                        return False
                elif event.key == pygame.K_BACKSPACE:
                    input_text = input_text[:-1]
                else:
                    input_text += event.unicode
    return False

def main():
    """Função principal."""
    running = True
    while running:
        screen.fill(WHITE)
        draw_text("Bem-vindo ao Desafio do Pentágono Fictício!", BLACK, WIDTH // 2, 100, center=True)
        draw_text("Pressione Enter para começar", BLUE, WIDTH // 2, 200, center=True)
        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RETURN:
                    # Etapas do jogo
                    if not stage_password():
                        draw_text("Você falhou na Etapa 1!", RED, WIDTH // 2, HEIGHT // 2, center=True)
                        pygame.display.flip()
                        pygame.time.wait(2000)
                        return
                    if not stage_enigma():
                        draw_text("Você falhou na Etapa 2!", RED, WIDTH // 2, HEIGHT // 2, center=True)
                        pygame.display.flip()
                        pygame.time.wait(2000)
                        return
                    if not stage_firewall():
                        draw_text("Você falhou na Etapa 3!", RED, WIDTH // 2, HEIGHT // 2, center=True)
                        pygame.display.flip()
                        pygame.time.wait(2000)
                        return

                    draw_text("Parabéns! Você completou o desafio!", GREEN, WIDTH // 2, HEIGHT // 2, center=True)
                    pygame.display.flip()
                    pygame.time.wait(3000)
                    return

# Executar o jogo
main()
pygame.quit()
