#include "Enemy.h"
#include "Bullet.h"

Enemy::Enemy(CMPUT350::Point2D loc)
{
    // TODO: Update code
    mLocation=loc;
    mBounds = CMPUT350::Rect(loc.x -20, loc.y -20, 40,40); //since loc is the center of the rect, we get top left by minus 20
}

void Enemy::Initialize(CMPUT350::GameContext* context)
{
}

void Enemy::Update(CMPUT350::GameContext* context)
{
}

void Enemy::LateUpdate(CMPUT350::GameContext* context)
{
}

bool Enemy::HandleKeyEvent(CMPUT350::GameContext* context, char key)
{
}

void Enemy::RenderBackground(CMPUT350::GameContext* context)
{
}

void Enemy::RenderForeground(CMPUT350::GameContext* context)
{
    context->ScreenContext->DrawRect(mBounds, CMPUT350::Colors::red);
}

void Enemy::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    std::shared_ptr<Bullet> bullet = std::dynamic_pointer_cast<Bullet>(obj); //cast to check if the obj is a bullet
    if (bullet!= nullptr &&  bullet->IsPlayerBullet()) { // the bullet comes from the player
        Kill(); //If its a bullet, kill the enemy
        bullet->Kill(); // bullet also dies
    }
}

void Enemy::Kill()
{
    mAlive = false;
}

bool Enemy::IsAlive() const
{
    // TODO: Update code
    return mAlive;
}

const CMPUT350::Rect& Enemy::GetBounds()
{
    // TODO: Update code
    return mBounds;
}
