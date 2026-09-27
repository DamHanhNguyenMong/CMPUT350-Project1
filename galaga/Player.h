#ifndef PLAYER_H
#define PLAYER_H

#include "CollisionObject.h"
#include <memory>

class Bullet;

class Player : public CMPUT350::CollisionObject
{
public:
    Player(CMPUT350::Point2D loc);

    // GameObject Functions
    void Initialize(CMPUT350::GameContext* context) override;
    void Update(CMPUT350::GameContext* context) override;
    void LateUpdate(CMPUT350::GameContext* context) override;
    bool HandleKeyEvent(CMPUT350::GameContext* context, char key) override;
    bool IsAlive() const override;
    void Kill() override;

    // Graphics Object Functions
    void RenderBackground(CMPUT350::GameContext* context) override;
    void RenderForeground(CMPUT350::GameContext* context) override;


    // Collision Object Functions
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    const CMPUT350::Rect& GetBounds() override;
private:
    CMPUT350::Point2D mLocation;
    CMPUT350::Rect mBounds;

    std::weak_ptr<Bullet> mBullet1; // weak pointers for bullet since player dont own the bullets
    std::weak_ptr<Bullet> mBullet2; // but just remember to have those bullets on screen

    static constexpr float PLAYER_SIZE = 40.0f;
    static constexpr float PLAYER_SPEED = 5.0f;
};

#endif
