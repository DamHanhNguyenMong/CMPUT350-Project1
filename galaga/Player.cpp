#include <cassert>
#include "Player.h"
#include "Bullet.h"
#include "Galaga.h"  // To use namespace GalagaNotifs
#include "Enemy.h"

static const bool DEBUG_BOUNDS = false;   // set to false to hide

Player::Player(CMPUT350::Point2D loc) : mLocation(loc)
{
    // bounds and shapes are in local coordinates
    mBounds = CMPUT350::Rect(-PLAYER_SIZE / 2.0f, -PLAYER_SIZE / 2.0f, PLAYER_SIZE, PLAYER_SIZE);

    // shapes
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Rect(-20, -14, 6, 28))); // left wing
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Rect( 14, -14, 6, 28))); // right wing
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Rect(-14,  -6, 28, 16))); // struts / inner wings
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Rect( -4, -20, 8, 40))); // fuselage, nose and engine
}

void Player::Initialize(CMPUT350::GameContext* context)
{
    mContext = context;
    mSelf = context->CurrObject;

    // Register to recieve notifications
    context->mNotificationManager->Register(context->CurrObject, PlayerNotifs::MoveLeft);
    context->mNotificationManager->Register(context->CurrObject, PlayerNotifs::MoveRight);
    context->mNotificationManager->Register(context->CurrObject, PlayerNotifs::StopLeft);
    context->mNotificationManager->Register(context->CurrObject, PlayerNotifs::StopRight);
    context->mNotificationManager->Register(context->CurrObject, PlayerNotifs::ShootBullet);

    context->mNotificationManager->Register(context->CurrObject, GalagaNotifs::RemoveAll);
}

void Player::Update(CMPUT350::GameContext* context) {
    // Key repeat is disabled, so held down key only sends one notification to keep moving, 
    // each moving notification just sets movement direction and update does the movement
    if (movingLeft) {
        mLocation.x -= PLAYER_SPEED;
    }
    if (movingRight) {
        mLocation.x += PLAYER_SPEED;
    }

    KeepInBounds(context);
}

void Player::LateUpdate(CMPUT350::GameContext* context) {}

void Player::ReceiveNotification(const std::string& key) 
{
    if (key == GalagaNotifs::RemoveAll) {
        mAlive = false;
        return;
    }
    if (!mAlive) { return; } // If player is dead no need to handle other notifs

    if (key == PlayerNotifs::MoveLeft) { movingLeft = true; }
    else if (key == PlayerNotifs::MoveRight) { movingRight = true; }
    else if (key == PlayerNotifs::StopLeft) { movingLeft = false; }
    else if (key == PlayerNotifs::StopRight) { movingRight = false; }
    else if (key == PlayerNotifs::ShootBullet) { Shoot(); }
}

void Player::Shoot() 
{
    if (mBullet1.expired()){
        std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>(CMPUT350::Point2D(mLocation.x, mLocation.y - 20),CMPUT350::Point2D(0, -10), true, mSelf); // create new bullet with speed -10

        mContext->mEngineView->AddGameObject(bullet); // add to the game engine
        mBullet1 = bullet; //remember the bullet
    }
    else if (mBullet2.expired()){
        std::shared_ptr<Bullet> bullet = std::make_shared<Bullet>(CMPUT350::Point2D(mLocation.x, mLocation.y - 20),CMPUT350::Point2D(0, -10), true, mSelf); // create new bullet
        // mLocation.y - 20 so bullet shot at the rear not the center of player
        mContext->mEngineView->AddGameObject(bullet); // add to the game engine
        mBullet2 = bullet;
    }
    return;
}

void Player::KeepInBounds(CMPUT350::GameContext* context)
{
    // Keep the ship on the screen - local coordinates
    float minX = PLAYER_SIZE / 2.0f; 
    float maxX = context->ScreenContext->GetWindowWidth() - PLAYER_SIZE / 2.0f;

    if (mLocation.x < minX) {
        mLocation.x = minX;
    }
    if (mLocation.x > maxX) {
        mLocation.x = maxX;
    }

    // mBounds dont change no need to update them as previously was done
    return;
}

void Player::RenderBackground(CMPUT350::GameContext* context) {}

void Player::RenderForeground(CMPUT350::GameContext* context)
{
    auto* g = context->ScreenContext;
    const float x = 0;  // Uses local coordinates, so ship is drawn around 0,0
    const float y = 0;

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
    // player collided with an enemy
    if (std::dynamic_pointer_cast<Enemy>(obj)) { 
        Kill(); 
        return; 
    }

    std::shared_ptr<Bullet> bullet = std::dynamic_pointer_cast<Bullet>(obj); //check if colliding obj is a bullet

    if (bullet != nullptr && !bullet->IsPlayerBullet()) // if its a bullet and not from the player
    {
        Kill(); //kill player
    }

}

void Player::Kill()
{
    if (!mAlive) { return; }  // If player is already dead, they dont need to lose another life
    mAlive = false;

    mContext->mNotificationManager->Notify(GalagaNotifs::PlayerDied);
}

bool Player::IsAlive() const
{
    return mAlive;
}

const CMPUT350::Rect& Player::GetBounds()
{
    return mBounds;
}

const std::vector<CMPUT350::Shape>& Player::GetShapes()
{
    return mShapes;
}
