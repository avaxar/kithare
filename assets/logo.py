import pygame


success_init, failed_init = pygame.init()

WIDTH, HEIGHT = 1025, 1025
FACTOR = 1 / 16.0
logo = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
rgb = lambda r, g, b: (int(r * 255), int(g * 255), int(b * 255))
translate = lambda x, y: (round(x * (WIDTH - 1)), round(y * (HEIGHT - 1)))
poly = lambda points, r, g, b: pygame.draw.polygon(logo, rgb(r, g, b), [translate(x * FACTOR, y * FACTOR) for x, y in points])

# A piece
poly([
    (2, 2),
    (5, 2),
    (5, 12),
    (2, 12)
], 1, 0, 0)

# B piece
poly([
    (2, 13),
    (5, 13),
    (5, 14),
    (2, 14)
], 1, 0.25, 0.25)

# C piece
poly([
    (11, 2),
    (14, 2),
    (11, 5),
    (8, 5)
], 0, 1, 0)

# D piece
poly([
    (7, 6),
    (10, 6),
    (9, 7),
    (6, 7)
], 0.25, 1, 0.25)

# E piece
poly([
    (6, 9),
    (9, 9),
    (12, 12),
    (9, 12)
], 0, 0, 1)

# F piece
poly([
    (10, 13),
    (13, 13),
    (14, 14),
    (11, 14)
], 0.25, 0.25, 1)

pygame.image.save(logo, "logo.png")
