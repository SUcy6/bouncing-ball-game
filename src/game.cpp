#include "game.h"

Game::Game(unsigned int width, unsigned int height): State(GAME_ACTIVE), Keys(), Width(width), Height(height){


}

Game::~Game() {}

void Game::Init() {

}

void Game::Update(float dt) {

}

void Game::ProcessInput(float dt) {

}

void Game::Render() {
    
}

// compile: 
// clang++ ./src/*.cpp ./src/glad.c -I ./include/ -I ./thirdparty/old/glm -o lab -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo