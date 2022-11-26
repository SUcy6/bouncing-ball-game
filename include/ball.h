#ifndef BALL_H
#define BALL_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GameObject.h"
#include "texture.h"

class Ball : public GameObject
{
    public:
        Ball();
        Ball(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite);
        ~Ball();

        // move the ball within the window
        glm::vec2 Move(float dt, unsigned int window_width);
        // reset to origin
        void Reset(glm::vec2 position, glm::vec2 velocity);

        float Radius;
        bool Stuck; 
        bool Sticky, PassThrough;
};

#endif