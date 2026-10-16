#ifndef BULLET_H
#define BULLET_H

#include <memory>
#include <vector>
#include "CollisionObject.h"
#include "GameContext.h"

class Bullet : public CMPUT350::CollisionObject
{
public:
    Bullet(CMPUT350::Point2D location, CMPUT350::Point2D heading, bool player, std::weak_ptr<CMPUT350::GameObject> parent);
    bool IsPlayerBullet();

    // GameObject Functions
    void Initialize(CMPUT350::GameContext* context) override;
    void Update(CMPUT350::GameContext* context) override;
    void LateUpdate(CMPUT350::GameContext* context) override;
    // bool HandleKeyEvent(CMPUT350::GameContext* context, char key) override;
    bool IsAlive() const override;
    void Kill() override;

    // Graphics Object Functions
    void RenderBackground(CMPUT350::GameContext* context) override;
    void RenderForeground(CMPUT350::GameContext* context) override;
    CMPUT350::Point2D GetLocation() const override;
    float GetRotation() const override;

    // Collision Object Functions
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj,
                        const CMPUT350::Point2D& collisionPoint) override;
    const CMPUT350::Rect& GetBounds() override;
    const std::vector<CMPUT350::Shape>& GetShapes() override;
private:
    CMPUT350::Point2D mLocation; // world position of the bullet's head
    CMPUT350::Point2D mPreviousLocation; // position last frame
    CMPUT350::Point2D mHeading; // movement per frame
    bool mPlayerBullet;
    std::weak_ptr<CMPUT350::GameObject> mParent;  // who fired this bullet
    
    bool mHit = false; // true once a valid collision happened
    CMPUT350::Point2D mHitPoint; // collision point, in LOCAL coordinates

    CMPUT350::Rect mBounds;
    std::vector<CMPUT350::Shape> mShapes;
};
#endif // BULLET_H
