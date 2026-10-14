#include <cassert>
#include "Player.h"
#include "Bullet.h"

static const bool DEBUG_BOUNDS = false;   // set to false to hide

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
            std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>(mLocation,CMPUT350::Point2D(0, -10), true); // create new bullet with speed -10

            context->mEngineView->AddGameObject(bullet); // add the the game engine
            mBullet1 = bullet; //remember the bullet
        }

        else if (mBullet2.expired()){
            std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>(CMPUT350::Point2D(mLocation.x, mLocation.y - 20),CMPUT350::Point2D(0, -10), true); // create new bullet
            // mLocation.y - 20 so bullet shot at the rear not the center of player
            context->mEngineView->AddGameObject(bullet); // add the the game engine
            mBullet2 = bullet;
        }
    }
    // Keep the ship on the screen
    float minX = 20; //includes left rear of the ship
    float maxX = context->ScreenContext->GetWindowWidth() - 20; // includes right rear of the ship
    if (mLocation.x < minX) {
        mLocation.x = minX;
    }
    if (mLocation.x > maxX) {
        mLocation.x = maxX;
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
    auto* g = context->ScreenContext;
    float x = mLocation.x;
    float y = mLocation.y;

    // Outer wings (blue) with red tips, like the Galaga fighter
    g->DrawRect(CMPUT350::Rect(x - 20, y - 8, 6, 22), CMPUT350::Colors::blue);
    g->DrawRect(CMPUT350::Rect(x + 14, y - 8, 6, 22), CMPUT350::Colors::blue);
    g->DrawRect(CMPUT350::Rect(x - 20, y - 14, 6, 6), CMPUT350::Colors::red);
    g->DrawRect(CMPUT350::Rect(x + 14, y - 14, 6, 6), CMPUT350::Colors::red);

    // Struts connecting the wings to the body
    g->DrawRect(CMPUT350::Rect(x - 14, y + 2, 28, 8), CMPUT350::Colors::white);

    // Inner wings, stepping up toward the fuselage
    g->DrawRect(CMPUT350::Rect(x - 11, y - 6, 22, 8), CMPUT350::Colors::white);

    // Central fuselage and nose
    g->DrawRect(CMPUT350::Rect(x - 4, y - 16, 8, 32), CMPUT350::Colors::white);
    g->DrawRect(CMPUT350::Rect(x - 2, y - 20, 4, 6), CMPUT350::Colors::white);

    // Cockpit
    g->DrawRect(CMPUT350::Rect(x - 2, y - 8, 4, 10), CMPUT350::Colors::blue);

    // Red accent stripes on the inner wings
    g->DrawRect(CMPUT350::Rect(x - 11, y - 6, 4, 8), CMPUT350::Colors::red);
    g->DrawRect(CMPUT350::Rect(x + 7, y - 6, 4, 8), CMPUT350::Colors::red);

    // Engine glow at the rear
    g->DrawCircle(CMPUT350::Point2D(x, y + 17), 3, CMPUT350::Colors::red);

    // Debug hitbox
    if (DEBUG_BOUNDS)
        g->FrameRect(mBounds, 1, CMPUT350::Colors::white);
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
