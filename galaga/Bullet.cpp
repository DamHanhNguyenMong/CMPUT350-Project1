#include "Bullet.h"
#include "Enemy.h"
#include "Player.h"

static const bool DEBUG_BOUNDS = false;   // set to false to hide

Bullet::Bullet(CMPUT350::Point2D location, CMPUT350::Point2D heading, bool player, std::weak_ptr<CMPUT350::GameObject> parent)
    : mLocation(location),
      mPreviousLocation(location),  // hasn't moved yet
      mHeading(heading),
      mPlayerBullet(player),
      mParent(parent),
      mBounds(location, 0, 0)
{
    // Collision shape in LOCAL space: a directed line from the tail to the head
    // Each frame the bullet moves by mHeading, so the tail is at -heading
    CMPUT350::Point2D tail(-heading.x, -heading.y);
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Line(tail, CMPUT350::Point2D(0, 0))));
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

    // Only kill when leaving the screen in the direction of travel
    if ((mHeading.y < 0 && mLocation.y < 0) ||
        (mHeading.y > 0 && mLocation.y > context->ScreenContext->GetWindowHeight()) ||
        (mHeading.x < 0 && mLocation.x < 0) ||
        (mHeading.x > 0 && mLocation.x > context->ScreenContext->GetWindowWidth()))
    {
        Kill();
    }

}

void Bullet::LateUpdate(CMPUT350::GameContext* context)
{
    // Collisions are done by now. Die if we hit something (the hit circle is
    // still drawn this frame, because removal happens next frame)
    if (mHit) {
        Kill(); // kill the bullet if mHit true
    }

    // When the parent dies, Die with the parent
    std::shared_ptr<CMPUT350::GameObject> parent = mParent.lock();
    if (parent == nullptr || !parent->IsAlive()) {
        Kill();
    }
}

void Bullet::RenderBackground(CMPUT350::GameContext* context)
{
}

void Bullet::RenderForeground(CMPUT350::GameContext* context)
{
    // Slightly different colors for player and enemy bullets
    CMPUT350::Point2D tail(-mHeading.x, -mHeading.y);
    CMPUT350::RGBColor color = mPlayerBullet ? CMPUT350::RGBColor(255, 255, 0)    // yellow
                                             : CMPUT350::RGBColor(255, 160, 0);   // orange
    // Drawing is in LOCAL space; the engine applies the location/rotation
    if (mHit) {
        // After a collision, just a circle at the point of collision
        context->ScreenContext->DrawCircle(mHitPoint, 6, color);
    }
    else {
        context->ScreenContext->DrawLine(tail, CMPUT350::Point2D(0, 0), 4, color);
    }

    if (DEBUG_BOUNDS) {
        CMPUT350::Rect localBounds(tail, CMPUT350::Point2D(0, 0));   // min/max constructor, always positive size
        context->ScreenContext->FrameRect(localBounds, 1, CMPUT350::Colors::white);
    }
}

CMPUT350::Point2D Bullet::GetLocation() const
{
    return mLocation;
}

float Bullet::GetRotation() const
{
    return 0.0f; // No Rotation
}

void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    // No point supplied: treat the bullet's head (local origin) as the hit point
    CollisionEnter(obj, CMPUT350::Point2D(0, 0));
}

void Bullet::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj, const CMPUT350::Point2D& collisionPoint)
{
    // Check if the colliding obj is the parent, we dont collide with the parent
    std::shared_ptr<CMPUT350::GameObject> parent = mParent.lock();
    if (parent != nullptr && parent == obj) {
        return;
    }

    // The target obj is obly valid if: Player bullets only hit enemies and enemy bullets only hit the player
    bool validTarget =
        (mPlayerBullet && std::dynamic_pointer_cast<Enemy>(obj) != nullptr) ||  // parent and obj are both not Enemy
        (!mPlayerBullet && std::dynamic_pointer_cast<Player>(obj) != nullptr); // parent and obj are both not Player
    if (!validTarget) {
        return;
    }

    // If there are many collision point (bullet's hit point != collision point), keep the one with largest y (lowest on screen)
    if (!mHit || collisionPoint.y > mHitPoint.y) {
        mHitPoint = collisionPoint;
        mHit = true;
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

const std::vector<CMPUT350::Shape>& Bullet::GetShapes()
{
    return mShapes;
}
