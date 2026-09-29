#include "Bullet.h"
#include "Enemy.h"
#include "Player.h"

Bullet::Bullet(CMPUT350::Point2D location, CMPUT350::Point2D heading, bool player)
{
    mLocation = location;
    mPreviousLocation = location; //When first created, bullet hasnt moved yet
    mHeading = heading;
    mPlayerBullet = player;
    mBounds = CMPUT350::Rect(location, 0, 0);
}

bool Bullet::IsPlayerBullet()
{
    // TODO: Update
    return mPlayerBullet;
}

void Bullet::Initialize(CMPUT350::GameContext* context)
{
}

void Bullet::Update(CMPUT350::GameContext* context)
{
    mPreviousLocation = mLocation; //Make the current as the prev location

    //Move the bullet
    mLocation += mHeading;

    //Update bounding box
    mBounds = CMPUT350::Rect(mPreviousLocation, mLocation);

}

void Bullet::LateUpdate(CMPUT350::GameContext* context)
{
}

bool Bullet::HandleKeyEvent(CMPUT350::GameContext* context, char key)
{
    return false; // no key handling from bullets
}

void Bullet::RenderBackground(CMPUT350::GameContext* context)
{
}

void Bullet::RenderForeground(CMPUT350::GameContext* context)
{
    context->ScreenContext->DrawLine(mPreviousLocation, mLocation, 2, CMPUT350::Colors::white);
}

void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    // Check which objects is the bullet colliding with
    std::shared_ptr<Enemy> enemy = std::dynamic_pointer_cast<Enemy>(obj);
    std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(obj);

    // If the bullet coming from the player and the colliding obj is an enemy
    if (mPlayerBullet && enemy != nullptr)
    {
        Kill(); // kill the bullet
        enemy->Kill(); // kill the enemy also
    }
    else if (!mPlayerBullet && player != nullptr) //otherwise, the colliding obj is a player and the bullet not from the player
    {
        Kill(); // kill the bullet
        player->Kill(); //kill the player
    }
}

void Bullet::Kill()
{
    mAlive = false;
}

bool Bullet::IsAlive() const
{
    // TODO: Update code
    return mAlive;
}

const CMPUT350::Rect& Bullet::GetBounds()
{
    // TODO: Update code
    return mBounds;
}
