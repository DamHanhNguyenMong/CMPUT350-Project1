#ifndef PLAYER_H
#define PLAYER_H

#include <memory>
#include <string>
#include <vector>

#include "CollisionObject.h"

class Bullet;

namespace PlayerNotifs 
{
    const std::string MoveLeft = "MoveLeft";
    const std::string MoveRight = "MoveRight";
    const std::string StopLeft = "StopLeft";
    const std::string StopRight = "StopRight";
    const std::string ShootBullet = "ShootBullet";
}

class Player : public CMPUT350::CollisionObject
{
public:
    explicit Player(CMPUT350::Point2D loc);

    // GameObject Functions
    void Initialize(CMPUT350::GameContext* context) override;
    void Update(CMPUT350::GameContext* context) override;
    void LateUpdate(CMPUT350::GameContext* context) override;
    void ReceiveNotification(const std::string& key) override;
    bool IsAlive() const override;
    void Kill() override;

    // Graphics Object Functions
    void RenderBackground(CMPUT350::GameContext* context) override;
    void RenderForeground(CMPUT350::GameContext* context) override;

    CMPUT350::Point2D GetLocation() const override { return mLocation; }
	float GetRotation() const override { return 0.0f; }

    // Collision Object Functions
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    const CMPUT350::Rect& GetBounds() override;
    const std::vector<CMPUT350::Shape>& GetShapes() override;

private:
    void KeepInBounds(CMPUT350::GameContext* context);  // To check bounds when player moves
    void Shoot();

    CMPUT350::GameContext* mContext = nullptr;  // just a reference to game context

    CMPUT350::Point2D mLocation;
    CMPUT350::Rect mBounds;
    std::vector<CMPUT350::Shape> mShapes;

    bool movingLeft = false;
    bool movingRight = false;

    std::weak_ptr<Bullet> mBullet1; // weak pointers for bullet since player dont own the bullets
    std::weak_ptr<Bullet> mBullet2; // but just remember to have those bullets on screen

    static constexpr float PLAYER_SIZE = 40.0f;
    static constexpr float PLAYER_SPEED = 10.0f;
};

#endif