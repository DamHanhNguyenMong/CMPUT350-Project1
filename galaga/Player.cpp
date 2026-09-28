#include <cassert>
#include "Player.h"
#include "Bullet.h"

Player::Player(CMPUT350::Point2D loc)
{
    // TODO: Update code
    mLocation = loc;
    mBounds = CMPUT350::Rect(mLocation.x - 20, mLocation.y - 20, 40, 40);


}

void Player::Initialize(CMPUT350::GameContext* context)
{
}

void Player::Update(CMPUT350::GameContext* context)
{
}

void Player::LateUpdate(CMPUT350::GameContext* context)
{
}

bool Player::HandleKeyEvent(CMPUT350::GameContext* context, char key)
{
    if (key == 'a' || key == 'A')
    {
        mLocation.x -= PLAYER_SPEED;
    }
    else if (key == 'd' || key == 'D')
    {
        mLocation.x += PLAYER_SPEED;
    }
    else if (key == ' ') {
        if (mBullet1.expired()){
            std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>(mLocation,CMPUT350::Point2D(0, -5), true); // create new bullet with speed -5

            context->mEngineView->AddGameObject(bullet); // add the the game engine
            mBullet1 = bullet; //remember the bullet
        }

        else if (mBullet2.expired()){
            std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>(CMPUT350::Point2D(mLocation.x, mLocation.y - 20),CMPUT350::Point2D(0, -5), true); // create new bullet
            // mLocation.y - 20 so bullet shot at the rear not the center of player
            context->mEngineView->AddGameObject(bullet); // add the the game engine
            mBullet2 = bullet;
        }
    }
    // Update mBounds
    mBounds = CMPUT350::Rect(mLocation.x -20, mLocation.y -20, 40,40);
    return false;
}

void Player::RenderBackground(CMPUT350::GameContext* context)
{
}

void Player::RenderForeground(CMPUT350::GameContext* context)
{
    // NOTE: js a sample. We can make it prettier :)
    context->ScreenContext->DrawRect(
        CMPUT350::Rect(mLocation.x - 20, mLocation.y, 40, 10),
        CMPUT350::Colors::blue
    );

    context->ScreenContext->DrawLine(
        CMPUT350::Point2D(mLocation.x, mLocation.y - 15),
        CMPUT350::Point2D(mLocation.x - 20, mLocation.y + 10),
        5,
        CMPUT350::Colors::blue
    );

    context->ScreenContext->DrawLine(
        CMPUT350::Point2D(mLocation.x, mLocation.y - 15),
        CMPUT350::Point2D(mLocation.x + 20, mLocation.y + 10),
        5,
        CMPUT350::Colors::blue
    );
}

void Player::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    std::shared_ptr<Bullet> bullet = std::dynamic_pointer_cast<Bullet>(obj); //check if colliding obj is a bullet

    if (bullet != nullptr && !bullet->IsPlayerBullet()) // if its a bullet and not from the player
    {
        Kill(); //kill player
        bullet->Kill(); //kill bullet
    }
}

void Player::Kill()
{
    mAlive = false;
}

bool Player::IsAlive() const
{
    // TODO: Update code
    return mAlive;
}

const CMPUT350::Rect& Player::GetBounds()
{
    // TODO: Update code
    return mBounds;
}
