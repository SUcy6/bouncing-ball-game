#include "game.h"
#include "resource_manager.hpp"
#include "SpriteRenderer.h"
#include "shader.h"

SpriteRenderer *renderer;

Game::Game(unsigned int width, unsigned int height): State(GAME_ACTIVE), Keys(), Width(width), Height(height){}

Game::~Game() {
    delete renderer;
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
    ResourceManager::LoadTexture("textures/profile.png", true, "face");
}

void Game::Update(float dt) {

}

void Game::ProcessInput(float dt) {

}

void Game::Render() {
    Texture2D myTexture;
    myTexture = ResourceManager::GetTexture("face");
    renderer->Draw(myTexture, glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}

// compile: 
// clang++ ./src/*.cpp ./src/glad.c -I ./include/ -I ./thirdparty/old/glm -o lab -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo