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
    delete ball;
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
    // update objects
    ball->Move(dt, this->Width);
        
    // check for collisions
    this->DoCollisions();

    // check loss condition, reset the game
    if (ball->Position.y >= this->Height){
        this->ResetLevel();
        this->ResetPlayer();
    }
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

// collision detection
bool CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(Ball &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);

// calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target) {
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}

// bool CheckCollision(Ball &one, GameObject &two) // AABB - Circle collision
// {
//     // get center point circle first 
//     glm::vec2 center(one.Position + one.Radius);
//     // calculate AABB info (center, half-extents)
//     glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
//     glm::vec2 aabb_center(
//         two.Position.x + aabb_half_extents.x, 
//         two.Position.y + aabb_half_extents.y
//     );
//     // get difference vector between both centers
//     glm::vec2 difference = center - aabb_center;
//     glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
//     // add clamped value to AABB_center and we get the value of box closest to circle
//     glm::vec2 closest = aabb_center + clamped;
//     // retrieve vector between center circle and closest point AABB and check if length <= radius
//     difference = closest - center;
//     return glm::length(difference) < one.Radius;
// }  

bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

Collision CheckCollision(Ball &one, GameObject &two) // AABB - Circle collision
{
    // get center point circle first 
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;

    if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

void Game::DoCollisions() {
    for (GameObject &box : this->Levels[this->Level].Bricks) {
        if (!box.Destroyed) {
            Collision collision = CheckCollision(*ball, box);
            if (std::get<0>(collision)) // if collision is true
            {
                // destroy block if not solid
                if (!box.IsSolid)
                    box.Destroyed = true;
                // collision resolution
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if (dir == LEFT || dir == RIGHT) // horizontal collision
                {
                    ball->Velocity.x = -ball->Velocity.x; // reverse horizontal velocity
                    // relocate
                    float penetration = ball->Radius - std::abs(diff_vector.x);
                    if (dir == LEFT)
                        ball->Position.x += penetration; // move ball to right
                    else
                        ball->Position.x -= penetration; // move ball to left;
                }
                else // vertical collision
                {
                    ball->Velocity.y = -ball->Velocity.y; // reverse vertical velocity
                    // relocate
                    float penetration = ball->Radius - std::abs(diff_vector.y);
                    if (dir == UP)
                        ball->Position.y -= penetration; // move ball bback up
                    else
                        ball->Position.y += penetration; // move ball back down
                }               
            }
        }
    }

    // check collisions for player pad (unless stuck)
    Collision result = CheckCollision(*ball, *player);
    if (!ball->Stuck && std::get<0>(result))
    {
        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = player->Position.x + player->Size.x / 2.0f;
        float distance = (ball->Position.x + ball->Radius) - centerBoard;
        float percentage = distance / (player->Size.x / 2.0f);
        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = ball->Velocity;
        ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength; 
        //Ball->Velocity.y = -Ball->Velocity.y;
        ball->Velocity = glm::normalize(ball->Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
        // fix sticky paddle
        ball->Velocity.y = -1.0f * abs(ball->Velocity.y);
    }
}  

void Game::ResetLevel()
{
    if (this->Level == 0)
        this->Levels[0].Load("levels/one.lvl", this->Width, this->Height / 2);
    else if (this->Level == 1)
        this->Levels[1].Load("levels/two.lvl", this->Width, this->Height / 2);
    else if (this->Level == 2)
        this->Levels[2].Load("levels/three.lvl", this->Width, this->Height / 2);
    else if (this->Level == 3)
        this->Levels[3].Load("levels/four.lvl", this->Width, this->Height / 2);
}

void Game::ResetPlayer()
{
    // reset player/ball stats
    player->Size = PLAYER_SIZE;
    // set to middle
    player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    ball->Reset(player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}

// compile: 
// clang++ -std=c++17 ./src/*.cpp ./src/glad.c -I ./include/ -I ./thirdparty/old/glm -o lab -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo