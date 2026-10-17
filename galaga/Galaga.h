#pragma once

#include <memory>
#include <string>

#include "Player.h"
#include "GameEngine.h"
#include "DrawContext.h"

#include "GameContext.h"
#include "GameObject.h"
#include "MathUtil.h"

// Notification names shared between Galaga, Player, and Enemy
namespace GalagaNotifs {
    const std::string InsertCoin = "InsertCoin";
    const std::string StartGame = "StartGame";
    const std::string PlayerDied = "PlayerDied";  // sent when player dies
    const std::string RemoveAll = "RemoveAll";  // sent by Galaga to remove all player/enemies

    const std::string Enemy1DiedFormation = "Enemy1DiedFormation";
    const std::string Enemy1DiedAttacking = "Enemy1DiedAttacking";
    const std::string Enemy2DiedFormation = "Enemy2DiedFormation";
    const std::string Enemy2DiedAttacking = "Enemy2DiedAttacking";
    const std::string Enemy3DiedFormation = "Enemy3DiedFormation";
    const std::string Enemy3DiedAttacking = "Enemy3DiedAttacking";
};

// Global information
constexpr int totalWaves = 5;
constexpr int shipsPerWave = 8;
constexpr int fps = 30;

class GalagaState;

// Game object - game starts with the instantiation of a galaga game object
// Holds the current state and also relevant game information
class Galaga : public CMPUT350::GraphicsObject
{
public:
    Galaga(int width, int height);

    // override game object methods
    void Initialize(CMPUT350::GameContext* context) override;
    void Update(CMPUT350::GameContext* context) override;
    void RenderBackground(CMPUT350::GameContext* context) override {}
    void RenderForeground(CMPUT350::GameContext* context) override;
    void ReceiveNotification(const std::string& key) override;

    CMPUT350::Point2D GetLocation() const override { return CMPUT350::Point2D(0, 0); }
    float GetRotation() const override { return 0.0f; }

    // state machine - transition to a new state
    void setState(std::unique_ptr<GalagaState> nextState);

    // To access Galaga object data
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool getCoin() const { return isCoin; }  // a 'coin' is inserted before starting game, checks if coin has been inserted
    int getScore() const { return score; }
    int getLives() const { return lives; }
    int getLevel() const { return currLevel; }

    int frame() const { return mContext->currentFrame; }

    int enemiesAlive() const { return mEnemiesAlive; }
    bool playerAlive() const { return mPlayerAlive; }  // If the player is currently dead, not all lives are lost

    bool enemyOutOfFormation() const;  // To check if enemies are in formation - pre/post attacking player

    // Methods states need to access
    void insertCoin() { isCoin = true; }
    bool spendCoin();  // Starts the game essentially

    void newGame();
    void nextLevel() { ++currLevel; }

    void spawnPlayer();
    void spawnEnemies(int wave, int indexInWave);

    void startAttack(int count);
    void playerDeath();  // Player died - check if need to respawn or lose game

    void clearField();
    void drawText(const std::string& text, float x, float y);
    void drawCenteredText(const std::string& text, float x, float y);
    void drawDisplay();

private:
    void applyState();  // Only Galaga object actually transitions to states, other objects just request to change state

    std::unique_ptr<GalagaState> mCurrentState;  // current state held by unique pointer since only one object of each state needs to exist
    std::unique_ptr<GalagaState> mRequestedState;  // Next state, before Galaga has transitioned to it
    CMPUT350::GameContext* mContext = nullptr;
    
    bool mUpdating = false;  // So that state transitions dont happen before previous state has finished what it needs to do

    int width;
    int height;
    bool isCoin = false;
    int score = 0;
    int lives = 3;  // How many lives left, starting at total lives 
    int currLevel = 1;

    int mEnemiesAlive = 0;
    bool mPlayerAlive = false;  // Set to true when game starts

    int mAttackCounter = 0;  // MEOW
};


// Each individual state inherits from GalagaState
class GalagaState 
{
public:
    void start(Galaga& galaga);  // records the entry frame, then calls enter()

    virtual void enter(Galaga&) {}
    virtual void update(Galaga&) = 0;
    virtual void onNotification(Galaga&, const std::string&) {}
    virtual void draw(Galaga&) {}
    virtual void exit(Galaga&) {}

	virtual ~GalagaState() = default;

protected:
    int framesInState(const Galaga& galaga) const;

private:
    int mStartFrame = 0;
};

// Waiting state - before game starts
class GalagaWaiting : public GalagaState
{
// Start state - game is waiting for 'coins to be entered' - player must press 5 to insert coins and 1 to start game
public:
    void update(Galaga&) override {}
    void onNotification(Galaga&, const std::string&) override;
    void draw(Galaga&) override;
};

// Startup state - adds enemies and starts the game
class GalagaStartup : public GalagaState
{
public:
    void enter(Galaga&) override;
    void update(Galaga&) override;
    void draw(Galaga&) override;
    // Doesnt need onNotification since it directly transitions into game
};


// State to transition into next wave?
class GalagaWaves : public GalagaState
{
public:
    void update(Galaga&) override;
    void draw(Galaga&) override;

private:
    int mWave = 0;
    int mShipInWave = 0;
    int mNextSpawnFrame = 0;
};


// Main game play state
class GalagaGamePlay : public GalagaState
{
public:
    void enter(Galaga&) override;
    void update(Galaga&) override;
    void draw(Galaga&) override;

private:
    int mNextAttackFrame = 0;
};


// Game over state - cleanup and whatnot
class GalagaGameOver : public GalagaState
{
public:
    void update(Galaga&) override;
    void draw(Galaga&) override;
};


// Added a player respawn state
class GalagaRespawn: public GalagaState
{
public:
    void update(Galaga&) override;
    void draw(Galaga&) override;

private:
    int mResumeFrame = -1;  // Wait until enemies are home and player respawned
};