#include "Enemy.h"

#include <algorithm>
#include <cmath>

#include "Bullet.h"
#include "Galaga.h"  // For notifs
#include "Player.h"

using CMPUT350::Point2D;
using CMPUT350::Rect;

static const bool DEBUG_BOUNDS = false;   // set to false to hide

namespace {
constexpr float PI = 3.14159265f;
constexpr int samples = 100;  // resolution of the arc-length table
constexpr float enemyRadius = 16.0f;  // enemies are 32x32

// 'Sway' is the hover back and forth
constexpr float swayAmount = 20.0f;  // pixels the formation sways left/right
constexpr float swayPeriod = 120.0f;  // frames per full back-and-forth period
constexpr int swayBlend = 15;  // ease into the sway so arrivals don't pop

constexpr float fireCutOffY = 750.0f;  // stop shooting once below this y level - too close to the player
constexpr float bulletSpeed = 6.0f;      // enemy bullet speed, pixels per frame
}

Enemy::Enemy(CMPUT350::Point2D loc, const std::vector<Point2D>& path) : mPosition(loc), mCurve(path) 
{
    if (path.empty()) { mLocation = loc; }
    else { mLocation = path.front(); }

    mBounds = CMPUT350::Rect(-enemyRadius, -enemyRadius, enemyRadius*2, enemyRadius*2);
    mShapes.push_back(CMPUT350::Shape(CMPUT350::Rect(-enemyRadius, -enemyRadius, enemyRadius*2, enemyRadius*2)));
    BuildTable(path);  // Takes bezier control points from path and constructs more precise points to move along in a table
}

void Enemy::Initialize(CMPUT350::GameContext* context) {
    mContext = context;
    mSelf = context->CurrObject;
    context->mNotificationManager->Register(context->CurrObject, GalagaNotifs::RemoveAll);  // to die if remove all is called
}

void Enemy::Update(CMPUT350::GameContext* context) {
    mContext = context;
    const int frame = context->currentFrame;

    if (mMode == Mode::inFormation) {
        UpdateFormation(frame);
        return;
    }

    // Move along path of not already where enemy is supposed to be
    mDistance += mSpeed;
    if (mDistance >= mTotalLength) {
        OnPathFinished(frame);
    } 
    else {
        MoveAlongPath();
    }
}

void Enemy::LateUpdate(CMPUT350::GameContext* context) {}

void Enemy::RenderBackground(CMPUT350::GameContext* context) {}

/*
void Enemy::RenderForeground(CMPUT350::GameContext* context)
{
    context->ScreenContext->DrawRect(mBounds, CMPUT350::Colors::red);

    if (DEBUG_BOUNDS)
        context->ScreenContext->FrameRect(mBounds, 1, CMPUT350::Colors::yellow);
}
*/

void Enemy::CollisionEnter(const std::shared_ptr<CMPUT350::CollisionObject>& obj)
{
    std::shared_ptr<Bullet> bullet = std::dynamic_pointer_cast<Bullet>(obj); //cast to check if the obj is a bullet
    if (bullet != nullptr &&  bullet->IsPlayerBullet()) { // the bullet comes from the player
        TakeDamage(1); // Enemies may have more than 1 health, so dont insta die now
        // Bullet kills itself in its own update
    }
    else if (std::dynamic_pointer_cast<Player>(obj)) {
        Kill();  // If the player runs into the enemy, the enemy dies immediately
    }
}

void Enemy::ReceiveNotification(const std::string& key)
{
    if (key == GalagaNotifs::RemoveAll)
        mAlive = false;
}

void Enemy::Kill()
{
    if (!mAlive) { return; }  // Dont need to kill if already dead
    mAlive = false;
    if (mContext) {
        if (mType == 1) {
            if (isInFormation()) { 
                mContext->mNotificationManager->Notify(GalagaNotifs::Enemy1DiedFormation); 
            }
            else { mContext->mNotificationManager->Notify(GalagaNotifs::Enemy1DiedAttacking); }
        }
        else if (mType == 2) {
            if (isInFormation()) {
                mContext->mNotificationManager->Notify(GalagaNotifs::Enemy2DiedFormation);
            }
            else { mContext->mNotificationManager->Notify(GalagaNotifs::Enemy2DiedAttacking); }
        }
        else if (mType == 3) {
            if (isInFormation()) {
                mContext->mNotificationManager->Notify(GalagaNotifs::Enemy3DiedFormation);
            }
            else { mContext->mNotificationManager->Notify(GalagaNotifs::Enemy3DiedAttacking); }
        }
    }
}

bool Enemy::IsAlive() const
{
    return mAlive;
}

const CMPUT350::Rect& Enemy::GetBounds()
{
    return mBounds;
}

const std::vector<CMPUT350::Shape>& Enemy::GetShapes() 
{
    return mShapes;
}

// Sample the curve and record cumulative lengths
void Enemy::BuildTable(const std::vector<Point2D>& points)
{
    mTable.assign(1, 0.0f);
    mNumSegments = 0;
    mTotalLength = 0.0f;

    // Bezier needs 3n + 1 points - else treat it as an emtpty path
    if (points.size() < 4 || ((points.size() - 1) % 3 != 0)) { return; }

    // gets n from 3n + 1
    mNumSegments = static_cast<int>((points.size() - 1) / 3);
    const int n = samples * mNumSegments;
    mTable.reserve(n + 1);

    Point2D prevPoint = mCurve.GetPoint(0.0f);
    for (int i = 1; i <= n; ++i) {
        Point2D point = mCurve.GetPoint(static_cast<float>(i) / samples);
        mTable.push_back(mTable.back() + static_cast<float>(prevPoint.Distance(point)));
        prevPoint = point;
    }
    mTotalLength = mTable.back();  // iterator
}

// convert distance travelled into the Bezier parameter t
float Enemy::ParameterAtDistance(float distance) const
{
    if (distance <= 0.0f) return 0.0f;
    if (distance >= mTotalLength) { return (static_cast<float>(mNumSegments)); }

    // first table entry that is greater than distance
    auto entry = std::upper_bound(mTable.begin(), mTable.end(), distance);  // binary search - gets an iterator

    size_t sizeIterators = static_cast<size_t>(entry - mTable.begin());
    float point1 = mTable[sizeIterators - 1];  // last and second last points between entry and beginning
    float point2 = mTable[sizeIterators];

    float fraction;
    if (point2 > point1) { fraction = (distance - point1) / (point2 - point1); }
    else { fraction = 0.0f; }

    return (static_cast<float>(sizeIterators - 1) + fraction) / samples;
}

void Enemy::MoveAlongPath()
{
    float t = ParameterAtDistance(mDistance);  // t is bezier parameter
    mLocation = mCurve.GetPoint(t);

    // To point enemy in the direction of travel
    Point2D slope = mCurve.GetSlope(t);
    // if slope is 0,0, then rotation is 0
    if (slope.x != 0.0f || slope.y != 0.0f)
        // sprites point up (-y) at rotation 0 and heading is measured from +x, so use the + pi/2 to start rotation from 90 degrees essentially
        mRotation = std::atan2(slope.y, slope.x) + PI / 2;  // tan uses opp / adj which is y / x
}

void Enemy::FollowPath(const std::vector<Point2D>& points, Mode mode)
{
    mCurve = CMPUT350::Bezier(points);
    BuildTable(points);
    mDistance = 0.0f;
    mMode = mode;

    if (!points.empty()) { mLocation = points.front(); }  // location to start
    if (mode == Mode::attacking && mContext) { mLastShotFrame = mContext->currentFrame; }
}

void Enemy::OnPathFinished(int frame)
{
    if (mMode == Mode::attacking) {
        // Flew off the bottom of the screen - reappear above it and fly back to the slot
        FollowPath({Point2D(mPosition.x, -40), Point2D(mPosition.x, 0), Point2D(mPosition.x, mPosition.y - 60), mPosition}, Mode::returning);
    } else 
    {
        EnterFormation(frame);  // Entering or Returning
    }
}

void Enemy::EnterFormation(int frame)
{
    // enemy is back to position in formation
    mMode = Mode::inFormation;
    mArrivalFrame = frame;
    mRotation = 0.0f;
    mLocation = mPosition;
}

// To 'sway' the entire formation MEOW
void Enemy::UpdateFormation(int frame)
{
    float phase = 2.0f * PI * static_cast<float>(frame) / swayPeriod;
    float blend = std::min(1.0f, static_cast<float>(frame - mArrivalFrame) / swayBlend);
    mLocation = Point2D(mPosition.x + swayAmount * std::sin(phase) * blend, mPosition.y);
}

// SHOOTING HELPERS
bool Enemy::ShouldShoot(CMPUT350::GameContext* context, int intervalFrames)
{
    if (mMode != Mode::attacking || mLocation.y > fireCutOffY) { return false; }
    if ((context->currentFrame - mLastShotFrame) < intervalFrames) { return false; }  // Not enough time between last shot and now to shoot yet
    
    mLastShotFrame = context->currentFrame;
    return true;
}

// creates a new bullet
void Enemy::ShootBullet(CMPUT350::GameContext* context, const Point2D& velocity)
{
    // false indicates it is an enemy bullet and not a player bullet
    context->mEngineView->AddGameObject(std::make_shared<Bullet>(mLocation, velocity, false, mSelf));
}

Point2D Enemy::PlayerLocation(CMPUT350::GameContext* context) const
{
    // Find the player from game objects, and get their location if they are alive
    for (const auto& object : *context->mEngineView) {
        if (auto player = std::dynamic_pointer_cast<Player>(object)) {
            if (player->IsAlive()) {
                return player->GetLocation();
            }
        }   
    }

    // Otherwise return where the player was when the attack began
    return mTarget;
}

void Enemy::TakeDamage(int amount)
{
    mHealth -= amount;
    if (mHealth <= 0) { Kill(); }
}


// =============== ENEMY 1 ==========================
bool Enemy1::Attack(CMPUT350::GameContext* context, const Point2D& target)
{
    // Enemy 1 will moove towards center screen, then zig zag out before exiting at the bottom
    if (!CanAttack()) { return false; }
    mTarget = target;

    const Point2D position = GetLocation();
    const float windowWidth = context->ScreenContext->GetWindowWidth();
    const float windowHeight = context->ScreenContext->GetWindowHeight();

    // direction of zig of a zig zag
    float zig = -1.0f;  // on right side of screen
    if (mPosition.x < windowWidth / 2) { zig = 1.0f; }  // home position in formation is on left half of screen

    // Zig-zag: swing out, cross back, swing out again, then exit at the bottom
    FollowPath({position,
                Point2D(position.x + zig * 200, position.y + 100), Point2D(position.x + zig * 200, position.y + 300), Point2D(position.x, position.y + 400),
                Point2D(position.x - zig * 200, position.y + 500), Point2D(position.x - zig * 200, windowHeight - 100), Point2D(position.x - zig * 100, windowHeight + 60)},
                Mode::attacking);
    return true;
}

void Enemy1::LateUpdate(CMPUT350::GameContext* context)
{
    // One bullet straight down every 1.5 seconds at 30 fps - 45 fps
    if (ShouldShoot(context, 45))
        ShootBullet(context, Point2D(0, bulletSpeed));
}

void Enemy1::RenderForeground(CMPUT350::GameContext* context)
{
    // Drawn around (0,0) - local coordinates
    auto* g = context->ScreenContext;
    g->DrawRect(Rect(-16, -6, 10, 12), CMPUT350::Colors::gray);  // wings
    g->DrawRect(Rect(6, -6, 10, 12), CMPUT350::Colors::gray);
    g->DrawRect(Rect(-6, -12, 12, 24), CMPUT350::Colors::yellow);  // body
    g->DrawRect(Rect(-4, -8, 3, 3), CMPUT350::Colors::red);  // eyes
    g->DrawRect(Rect(1, -8, 3, 3), CMPUT350::Colors::red);
}


// =============== ENEMY 2 ==========================
bool Enemy2::Attack(CMPUT350::GameContext* context, const Point2D& target)
{
    if (!CanAttack()) { return false; }
    mTarget = target;

    const Point2D position = GetLocation();
    const float windowWidth = context->ScreenContext->GetWindowWidth();
    const float windowHeight = context->ScreenContext->GetWindowHeight();

    float swing = -1.0f;
    if (mPosition.x < windowWidth / 2) { swing = 1.0f; }  // on left side of screen, swing right first

    // swing away from the player first, then swoop across to the player's column
    FollowPath({position,
                Point2D(position.x - swing * 250, position.y + 200),
                Point2D(target.x, target.y - 300),
                Point2D(target.x, windowHeight + 60)},
                Mode::attacking);
    return true;
}

void Enemy2::LateUpdate(CMPUT350::GameContext* context)
{
    // Aimed shot at wherever the player every 40 frames
    if (ShouldShoot(context, 40)) {
        Point2D targetDist = PlayerLocation(context) - GetLocation();  // distance to target (player)
        float len = std::sqrt(targetDist.x * targetDist.x + targetDist.y * targetDist.y);

        if (len > 0.0f)
            ShootBullet(context, Point2D(targetDist.x / len * bulletSpeed, targetDist.y / len * bulletSpeed));
    }
}

void Enemy2::RenderForeground(CMPUT350::GameContext* context)
{
    // Designed kind of like a butterfly
    auto* g = context->ScreenContext;
    g->DrawRect(Rect(-16, -12, 12, 10), CMPUT350::Colors::magenta);  // upper wings
    g->DrawRect(Rect(4, -12, 12, 10), CMPUT350::Colors::magenta);
    g->DrawRect(Rect(-14, 0, 10, 10), CMPUT350::Colors::cyan);  // lower wings
    g->DrawRect(Rect(4, 0, 10, 10), CMPUT350::Colors::cyan);
    g->DrawRect(Rect(-3, -12, 6, 24), CMPUT350::Colors::grey);  // body
    g->DrawRect(Rect(-2, -10, 4, 4), CMPUT350::Colors::yellow);  // head
}


// =============== ENEMY 3 ==========================
bool Enemy3::Attack(CMPUT350::GameContext* context, const Point2D& target)
{
    if (!CanAttack()) { return false; }
    mTarget = target;

    const Point2D position = GetLocation();
    const float windowWidth = context->ScreenContext->GetWindowWidth();
    const float windowHeight = context->ScreenContext->GetWindowHeight();

    float swing = -1.0f;
    if (mPosition.x < windowWidth / 2) { swing = 1.0f; }

    // wide sweep out to the side, then a hooked dive at the player's column
    FollowPath({position,
                Point2D(position.x + swing * 300, position.y - 50), Point2D(position.x + swing * 300, position.y + 300), Point2D(target.x, position.y + 350),
                Point2D(target.x - swing * 200, windowHeight - 200), Point2D(target.x - swing * 100, windowHeight - 50), Point2D(target.x, windowHeight + 60)},
                Mode::attacking);
    return true;
}

void Enemy3::LateUpdate(CMPUT350::GameContext* context)
{
    // 3 bullet spread every 60 frames (2 secs)
    if (ShouldShoot(context, 60)) {
        ShootBullet(context, Point2D(-2.5f, bulletSpeed));
        ShootBullet(context, Point2D(0.0f, bulletSpeed));
        ShootBullet(context, Point2D(2.5f, bulletSpeed));
    }
}

void Enemy3::RenderForeground(CMPUT350::GameContext* context)
{
    auto* g = context->ScreenContext;
    // Body colour shows remaining health - blue at full health, red once it's been hit
    auto bodyColor = CMPUT350::Colors::blue;
    if (getHealth() < 2) { bodyColor = CMPUT350::Colors::red; }

    g->DrawRect(Rect(-16, -16, 6, 8), CMPUT350::Colors::yellow);  // horns
    g->DrawRect(Rect(10, -16, 6, 8), CMPUT350::Colors::yellow);
    g->DrawRect(Rect(-12, -12, 24, 24), bodyColor);  // body
    g->DrawRect(Rect(-8, 8, 16, 6), CMPUT350::Colors::white);  // jaw
    g->DrawCircle(Point2D(0, -2), 4, CMPUT350::Colors::red);  // eye
}