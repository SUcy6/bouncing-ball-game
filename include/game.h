#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};


// class for game set-up
class Game
{
    public:
        GameState State;

        bool Keys[1024]; // user input
        unsigned int Width, Height; // game board size

        // constructor & deconstructor
        Game(unsigned int width, unsigned int height);
        ~Game();

        void Init();
        void ProcessInput(float dt);
        void Update(float dt);
        void Render();
};

#endif