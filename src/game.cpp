#include "game.h"
#include "resource_manager.hpp"
#include "SpriteRenderer.h"
#include "shader.h"
#include "ball.h"

SpriteRenderer *renderer;
GameObject *player;
Ball *ball; 


Game::Game(unsigned int width, unsigned int height): State(GAME_ACTIVE), Keys(), Width(width), Height(height){}

Game::~Game() {
    delete renderer;
    delete player;
}

void Game::Init() {
    // load shaders
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");

    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);

    // set render-specific controls
    Shader myShader;
    myShader = ResourceManager::GetShader("sprite");
    renderer = new SpriteRenderer(myShader);

    // load textures
    ResourceManager::LoadTexture("textures/background.jpg", true, "background");
    ResourceManager::LoadTexture("textures/profile.png", true, "face");
    ResourceManager::LoadTexture("textures/block.png", false, "block");
    ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");

    // load levels
    // 4 levels:
    // Standard
    // A few small gaps
    // Space invader
    // Bounce galore

    GameLevel one; 
    one.Load("levels/one.lvl", this->Width, this->Height/2);

    GameLevel two; 
    two.Load("levels/two.lvl", this->Width, this->Height/2);

    GameLevel three; 
    three.Load("levels/three.lvl", this->Width, this->Height/2);

    GameLevel four; 
    four.Load("levels/four.lvl", this->Width, this->Height/2);

    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Level = 0; // set to the first level initially

    // configure game objects
    glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"), glm::vec3(0.49f, 0.188f, 0.188f));

    // configure ball objects
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
    ball = new Ball(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}

void Game::Update(float dt) {
    ball->Move(dt, this->Width);
}

void Game::ProcessInput(float dt) {
    if (this->State == GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;

        // move playerboard
        if (this->Keys[GLFW_KEY_A])
        {
            if (player->Position.x >= 0.0f)
                player->Position.x -= velocity;
                if (ball->Stuck){
                    ball->Position.x -= velocity;
                }
        }
        if (this->Keys[GLFW_KEY_D])
        {
            if (player->Position.x <= this->Width - player->Size.x)
                player->Position.x += velocity;
                if (ball->Stuck){
                    ball->Position.x += velocity;
                }
        }
        if (this->Keys[GLFW_KEY_SPACE])
        {
            ball->Stuck = false;
        }
    }
}

void Game::Render() {
    Texture2D myTexture;
    myTexture = ResourceManager::GetTexture("face");
    renderer->DrawSprite(myTexture, glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    if(this->State == GAME_ACTIVE)
    {
        // draw background
        Texture2D bgTexture;
        bgTexture = ResourceManager::GetTexture("background");
        renderer->DrawSprite(bgTexture, glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

        // draw level
        this->Levels[this->Level].Draw(*renderer);

        // draw player
        player->Draw(*renderer);

        // draw ball
        ball->Draw(*renderer);
    }
}

// compile: 
// clang++ ./src/*.cpp ./src/glad.c -I ./include/ -I ./thirdparty/old/glm -o lab -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo