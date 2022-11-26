#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GameLevel.h"
#include "ball.h"
#include "ParticleGenerator.h"
#include "PostProcessor.h"
#include "PowerUp.h"

enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

// collision directions
enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

// Initial size of the player paddle
const glm::vec2 PLAYER_SIZE(150.0f, 25.0f);

// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);

// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);

// Radius of the ball object
const float BALL_RADIUS = 12.5f;

// Defines a Collision typedef that represents collision data
// a tuple to store: whether collide, collide direction, R vector
typedef std::tuple<bool, Direction, glm::vec2> Collision; 

// class for game set-up
class Game
{
    public:
        GameState State;

        bool Keys[1024]; // user input
        unsigned int Width, Height; // game board size

        std::vector<GameLevel> Levels;
        unsigned int           Level;

        std::vector<PowerUp>    PowerUps;

        // constructor & deconstructor
        Game(unsigned int width, unsigned int height);
        ~Game();

        void Init();
        void ProcessInput(float dt);
        void Update(float dt);
        void Render();
        void DoCollisions();
        
        // reset
        void ResetLevel();
        void ResetPlayer();

        // powerups
        void SpawnPowerUps(GameObject &block);
        void UpdatePowerUps(float dt);
};

#endif