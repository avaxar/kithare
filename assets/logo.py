import pygame
pygame.init()

WIDTH, HEIGHT = 946, 946
logo = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
banner = pygame.Surface((WIDTH * 4, HEIGHT), pygame.SRCALPHA)
FACTOR = 1 / 16.0

BLUE1 = (2, 5, 190)
BLUE2 = (20, 60, 220)
BLUE3 = (30, 110, 255)
BLUE4 = (45, 150, 255)

ORANGE1 = (240, 120, 0)
ORANGE2 = (255, 140, 0)


def translate(c):
    return round(c[0] * FACTOR * (WIDTH - 1)), round(c[1] * FACTOR * (HEIGHT - 1))


def poly(points, color):
    pygame.draw.polygon(logo, color, list(map(translate, points)))
    pygame.draw.polygon(banner, color, list(map(translate, points)))


# A1 piece
poly([
    (2, 7),
    (5, 7),
    (5, 5),
    (2, 6)
], BLUE2)

# A2 piece
poly([
    (2, 8),
    (5, 8),
    (5, 10),
    (2, 9)
], BLUE2)

# B1 piece
poly([
    (2, 5),
    (5, 4),
    (5, 1),
    (2, 1)
], ORANGE1)

# B2 piece
poly([
    (2, 10),
    (5, 11),
    (5, 14),
    (2, 14)
], ORANGE1)

# C1 piece
poly([
    (6, 7),
    (9, 7),
    (13, 3),
    (9, 4)
], BLUE3)

# C2 piece
poly([
    (6, 8),
    (9, 8),
    (13, 12),
    (9, 11)
], BLUE3)

# D1 piece
poly([
    (10, 3),
    (14, 2),
    (15, 1),
    (12, 1)
], ORANGE2)

# D2 piece
poly([
    (10, 12),
    (14, 13),
    (15, 14),
    (12, 14)
], ORANGE2)

# T1 piece
poly([
    (0, 7.5),
    (1, 6),
    (1, 9),
], BLUE1)

# T2 piece
poly([
    (10, 7.5),
    (15, 3),
    (15, 12),
], BLUE4)

pygame.image.save(logo, "logo.png")
pygame.image.save(banner, "banner.png")
