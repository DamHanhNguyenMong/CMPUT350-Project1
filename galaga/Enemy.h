#ifndef ENEMY_H
#define ENEMY_H

#include <memory>
#include <string>
#include <vector>

#include "Bezier.h"
#include "CollisionObject.h"
#include "GameContext.h"

class Enemy : public CMPUT350::CollisionObject
{
public:
    // path is used for bezier control points
    Enemy(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D>& path);
    virtual ~Enemy() = default;

    enum class Mode { entering, inFormation, attacking, returning };  // used to track enemies current mode

    // GameObject Functions
    void Initialize(CMPUT350::GameContext* context) override;
    void Update(CMPUT350::GameContext* context) override;
    void LateUpdate(CMPUT350::GameContext* context) override;
    void ReceiveNotification(const std::string& key) override;
    bool IsAlive() const override;
    void Kill() override;

    // Graphics Object Functions
    void RenderBackground(CMPUT350::GameContext* context) override;
    // void RenderForeground(CMPUT350::GameContext* context) override;
    CMPUT350::Point2D GetLocation() const override { return mLocation; }
    float GetRotation() const override { return mRotation; }

    // Collision Object Functions
    void CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj) override;
    const CMPUT350::Rect& GetBounds() override;
    const std::vector<CMPUT350::Shape>& GetShapes() override;

    // checks if enemy can attack right now - called by galaga
    virtual bool Attack(CMPUT350::GameContext* context, const CMPUT350::Point2D& target) = 0;
    bool isInFormation() const { return (mMode == Mode::inFormation); }
    int getHealth() const { return mHealth; }

protected:
    bool CanAttack() const { return (mAlive && mMode == Mode::inFormation); }  // checks if enemy is available to attack right now
    void FollowPath(const std::vector<CMPUT350::Point2D>& points, Mode mode);
    void TakeDamage(int amount);

    // Shooting helpers for subclasses
    bool ShouldShoot(CMPUT350::GameContext* context, int intervalFrames);
    void ShootBullet(CMPUT350::GameContext* context, const CMPUT350::Point2D& velocity);
    CMPUT350::Point2D PlayerLocation(CMPUT350::GameContext* context) const;  // falls back to mTarget

    CMPUT350::Point2D mPosition;  // this enemy's slot in the formation
    CMPUT350::Point2D mTarget;  // where the player was when the attack began
    float mSpeed = 8.0f;
    int mHealth = 1;
    int mFormationPoints = 50;
    int mAttackPoints = 100;
    int mType = 1;  // enemy type - 1 for enemy 1, 2 for enemy 2, 3 for enemy 3

private:
    CMPUT350::Point2D mLocation;
    float mRotation = 0.0f;
    CMPUT350::Rect mBounds;
    std::vector<CMPUT350::Shape> mShapes;

    CMPUT350::Bezier mCurve;
    Mode mMode = Mode::entering;

    CMPUT350::GameContext* mContext = nullptr;  // not owned
    std::weak_ptr<CMPUT350::GameObject> mSelf;  // passed to Bullet as its "parent"

    // Arc-length table: mTable[i] = length of the curve from t=0 to t=i/samplesPerSegment.
    // to move a fixed distance per frame instead of a fixed change in t.
    std::vector<float> mTable;
    int mNumSegments = 0;
    float mTotalLength = 0.0f;
    float mDistance = 0.0f;  // how far along the current path we are

    int mArrivalFrame = 0;
    int mLastShotFrame = 0;

    void BuildTable(const std::vector<CMPUT350::Point2D>& points);
    float ParameterAtDistance(float distance) const;
    void MoveAlongPath();
    void OnPathFinished(int frame);
    void EnterFormation(int frame);
    void UpdateFormation(int frame);
};


// Specific subclasses
class Enemy1 : public Enemy
{
public:
    Enemy1(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D>& path) : Enemy(loc, path) {
        // speed is 9 so they are faster than a standard enemy, the rest is as base case
        mSpeed = 9.0f;
        mType = 1;
    }
    void LateUpdate(CMPUT350::GameContext* context) override;
    bool Attack(CMPUT350::GameContext* context, const CMPUT350::Point2D& target) override;
    void RenderForeground(CMPUT350::GameContext* context) override;
};

class Enemy2 : public Enemy
{
public:
    Enemy2(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D>& path) : Enemy(loc, path) 
    { 
        mFormationPoints = 80;
        mAttackPoints = 160; 
        mType = 2;
    }

    void LateUpdate(CMPUT350::GameContext* context) override;
    bool Attack(CMPUT350::GameContext* context, const CMPUT350::Point2D& target) override;
    void RenderForeground(CMPUT350::GameContext* context) override;
};

class Enemy3 : public Enemy
{
public:
    Enemy3(CMPUT350::Point2D loc, const std::vector<CMPUT350::Point2D>& path) : Enemy(loc, path)
    {
        mHealth = 2;
        mSpeed = 6.0f;  // Slower but more health
        mFormationPoints = 150;
        mAttackPoints = 400;
        mType = 3;
    }

    void LateUpdate(CMPUT350::GameContext* context) override;
    bool Attack(CMPUT350::GameContext* context, const CMPUT350::Point2D& target) override;
    void RenderForeground(CMPUT350::GameContext* context) override;
};

#endif
