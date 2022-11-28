#include "game.h"
#include "resource_manager.hpp"
#include "SpriteRenderer.h"
#include "shader.h"
#include "ball.h"
#include "PostProcessor.h"

SpriteRenderer *renderer;
GameObject *player;
Ball *ball; 
ParticleGenerator *particles;
PostProcessor   *effects;
float ShakeTime = 0.0f;

Game::Game(unsigned int width, unsigned int height): State(GAME_ACTIVE), Keys(), Width(width), Height(height){}

Game::~Game() {
    delete renderer;
    delete player;
    delete ball;
    delete particles;
    delete effects;
}

void Game::Init() {
    // load shaders
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
    ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.frag", nullptr, "particle");
    ResourceManager::LoadShader("shaders/post_processing.vs", "shaders/post_processing.frag", nullptr, "postprocessing");
    
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").Use().SetMatrix4("projection", projection);

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
    ResourceManager::LoadTexture("textures/star.png", true, "particle");
    // texture for powerup
    ResourceManager::LoadTexture("textures/powerup_speed.png", true, "powerup_speed");
    ResourceManager::LoadTexture("textures/powerup_sticky.png", true, "powerup_sticky");
    ResourceManager::LoadTexture("textures/powerup_increase.png", true, "powerup_increase");
    ResourceManager::LoadTexture("textures/powerup_confuse.png", true, "powerup_confuse");
    ResourceManager::LoadTexture("textures/powerup_chaos.png", true, "powerup_chaos");
    ResourceManager::LoadTexture("textures/powerup_passthrough.png", true, "powerup_passthrough");

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

    particles = new ParticleGenerator(
        ResourceManager::GetShader("particle"), 
        ResourceManager::GetTexture("particle"), 
        500
    );

    effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width*2, this->Height*2); // need to double

}

void Game::Update(float dt) {
    // update objects
    ball->Move(dt, this->Width);
        
    // check for collisions
    this->DoCollisions();

    particles->Update(dt, *ball, 2, glm::vec2(ball->Radius / 2.0f));

    if (ShakeTime > 0.0f){
        ShakeTime -= dt;
        if (ShakeTime <= 0.0f) {effects->Shake = false;}
    }
    
    // update PowerUps
    this->UpdatePowerUps(dt);

    // check loss condition, reset the game
    if (ball->Position.y >= this->Height){
        this->Lives --;
        if (this->Lives == 0){
            this->ResetLevel();
            this->ResetPlayer();
        }
        this->ResetPlayer();
    }

    // check win condition
    if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
    {
        this->ResetLevel();
        this->ResetPlayer();
        effects->Chaos = true;
        this->State = GAME_WIN;
    }
}

void Game::ProcessInput(float dt) {

    if (this->State == GAME_MENU)
    {
        if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
        {
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
        }
        if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
        {
            this->Level = (this->Level + 1) % 4;
            this->KeysProcessed[GLFW_KEY_W] = true;
        }
        if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
        {
            if (this->Level > 0)
                --this->Level;
            else
                this->Level = 3;
            this->KeysProcessed[GLFW_KEY_S] = true;
        }
    }

    if (this->State == GAME_WIN)
    {
        if (this->Keys[GLFW_KEY_ENTER])
        {
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
            effects->Chaos = false;
            this->State = GAME_MENU;
        }
    }

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
    // Texture2D myTexture;
    // myTexture = ResourceManager::GetTexture("face");
    // renderer->DrawSprite(myTexture, glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    if(this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN)
    {   
        effects->BeginRender();

            // draw background
            Texture2D bgTexture;
            bgTexture = ResourceManager::GetTexture("background");
            renderer->DrawSprite(bgTexture, glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

            // draw level
            this->Levels[this->Level].Draw(*renderer);

            // draw player
            player->Draw(*renderer);

            // draw PowerUps
            for (PowerUp &powerUp : this->PowerUps){
                if (!powerUp.Destroyed){
                    powerUp.Draw(*renderer); 
                }  
            }

            // draw particles	
            particles->Draw();

            // draw ball
            ball->Draw(*renderer);

        effects->EndRender();
        effects->Render(glfwGetTime());
    }
    if (this->State == GAME_MENU)
    {
    }
    if (this->State == GAME_WIN)
    {
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

void ActivatePowerUp(PowerUp &powerUp){
    if (powerUp.Type == "speed")
    {
        ball->Velocity *= 1.2;
    }
    else if (powerUp.Type == "sticky")
    {
        ball->Sticky = true;
        player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if (powerUp.Type == "pass-through")
    {
        ball->PassThrough = true;
        ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if (powerUp.Type == "pad-size-increase")
    {
        player->Size.x += 50;
    }
    else if (powerUp.Type == "confuse")
    {
        if (!effects->Chaos)
            effects->Confuse = true; // only activate if chaos wasn't already active
    }
    else if (powerUp.Type == "chaos")
    {
        if (!effects->Confuse)
            effects->Chaos = true;
    }
} 

void Game::DoCollisions() {
    for (GameObject &box : this->Levels[this->Level].Bricks) {
        if (!box.Destroyed) {
            Collision collision = CheckCollision(*ball, box);
            if (std::get<0>(collision)) // if collision is true
            {
                // destroy block if not solid
                // shake it if solid
                if (!box.IsSolid){
                    box.Destroyed = true;
                    this->SpawnPowerUps(box);
                }
                else{
                    ShakeTime = 0.05f;
                    effects->Shake = true;
                }

                // collision resolution
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if (!(ball->PassThrough && !box.IsSolid)){
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
    }

    for (PowerUp &powerUp : this->PowerUps){
        if (!powerUp.Destroyed)
        {
            if (powerUp.Position.y >= this->Height)
                powerUp.Destroyed = true;
            if (CheckCollision(*player, powerUp))
            {	// collided with player, now activate powerup
                ActivatePowerUp(powerUp);
                powerUp.Destroyed = true;
                powerUp.Activated = true;
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

        ball->Stuck = ball->Sticky;
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

    this->Lives = 3;
}

void Game::ResetPlayer()
{
    // reset player/ball stats
    player->Size = PLAYER_SIZE;
    // set to middle
    player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    ball->Reset(player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}

// power up section
bool ShouldSpawn(unsigned int chance){
    unsigned int random = rand() % chance;
    return random == 0;
}

void Game::SpawnPowerUps(GameObject &block){
    if (ShouldSpawn(75)) // 1 in 75 chance
        this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position,ResourceManager::GetTexture("powerup_speed")));
    if (ShouldSpawn(75))
        this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("powerup_sticky")));
    if (ShouldSpawn(75))
        this->PowerUps.push_back(
            PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_passthrough")));
    if (ShouldSpawn(75))
        this->PowerUps.push_back(
            PowerUp("pad-size-increase", glm::vec3(1.0f, 0.6f, 0.4), 0.0f, block.Position, ResourceManager::GetTexture("powerup_increase")));
    if (ShouldSpawn(15)) // negative powerups should spawn more often
        this->PowerUps.push_back(
            PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_confuse")));
    if (ShouldSpawn(15))
        this->PowerUps.push_back(
            PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_chaos")));
}  

bool isOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type){
    for (const PowerUp &powerUp : powerUps)
    {
        if (powerUp.Activated)
            if (powerUp.Type == type)
                return true;
    }
    return false;
} 

void Game::UpdatePowerUps(float dt){
    for (PowerUp &powerUp : this->PowerUps)
    {
        powerUp.Position += powerUp.Velocity * dt;
        if (powerUp.Activated)
        {
            powerUp.Duration -= dt;

            if (powerUp.Duration <= 0.0f)
            {
                // remove powerup from list (will later be removed)
                powerUp.Activated = false;
                // deactivate effects
                if (powerUp.Type == "sticky")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "sticky"))
                    {	// only reset if no other PowerUp of type sticky is active
                        ball->Sticky = false;
                        player->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "pass-through")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "pass-through"))
                    {	// only reset if no other PowerUp of type pass-through is active
                        ball->PassThrough = false;
                        ball->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "confuse")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "confuse"))
                    {	// only reset if no other PowerUp of type confuse is active
                        effects->Confuse = false;
                    }
                }
                else if (powerUp.Type == "chaos")
                {
                    if (!isOtherPowerUpActive(this->PowerUps, "chaos"))
                    {	// only reset if no other PowerUp of type chaos is active
                        effects->Chaos = false;
                    }
                }                
            }
        }
    }
    this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
        [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
    ), this->PowerUps.end());

}  







// compile: 
// clang++ -std=c++17 ./src/*.cpp ./src/glad.c -I ./include/ -I ./thirdparty/old/glm -o lab -lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo