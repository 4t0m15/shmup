#pragma once
#include <raylib.h>

// Game constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;
const int PLAYER_SIZE = 30;
const int PLAYER_SPEED = 5;

struct Player {
    Rectangle rect;
    Color color;
};

void InitGame(Player& player);
void UpdateGame(Player& player);
void DrawGame(const Player& player);
