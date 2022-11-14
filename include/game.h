#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GameLevel.h"

enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(150.0f, 25.0f);

// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);

// class for game set-up
class Game
{
    public:
        GameState State;

        bool Keys[1024]; // user input
        unsigned int Width, Height; // game board size

        std::vector<GameLevel> Levels;
        unsigned int           Level;

        // constructor & deconstructor
        Game(unsigned int width, unsigned int height);
        ~Game();

        void Init();
        void ProcessInput(float dt);
        void Update(float dt);
        void Render();
};

#endif