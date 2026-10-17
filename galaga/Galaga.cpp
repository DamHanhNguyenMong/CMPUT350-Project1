
#include "Galaga.h"

#include <SFML/Window/Keyboard.hpp>
#include <vector>
#include <memory>

#include "Stars.h"
#include "Enemy.h"
#include "Player.h"
#include "MathUtil.h"

using CMPUT350::Point2D;


// ============== GALAGA CLASS =========================================
Galaga::Galaga(int width, int height) : width(width), height(height)
{
    mRequestedState = std::make_unique<GalagaWaiting>();  // create initial state and request to transition to it, Initialize will handle that
}

void Galaga::Initialize(CMPUT350::GameContext* context)
{
    mContext = context;  // Set the game context ptr

    // Set notification keys for pressing 5 (add coin) and pressing 1 (start game) to EngineView
    mContext->mEngineView->InstallKeyDownNotification(static_cast<int>(sf::Keyboard::Key::Num5), GalagaNotifs::InsertCoin);
    mContext->mEngineView->InstallKeyDownNotification(static_cast<int>(sf::Keyboard::Key::Num1), GalagaNotifs::StartGame);

    // Instal player movement and shooting notifications
    mContext->mEngineView->InstallKeyDownNotification(static_cast<int>(sf::Keyboard::Key::A), PlayerNotifs::MoveLeft);
    mContext->mEngineView->InstallKeyUpNotification(static_cast<int>(sf::Keyboard::Key::A), PlayerNotifs::StopLeft);

    mContext->mEngineView->InstallKeyDownNotification(static_cast<int>(sf::Keyboard::Key::D), PlayerNotifs::MoveRight);
    mContext->mEngineView->InstallKeyUpNotification(static_cast<int>(sf::Keyboard::Key::D), PlayerNotifs::StopRight);

    mContext->mEngineView->InstallKeyDownNotification(static_cast<int>(sf::Keyboard::Key::Space), PlayerNotifs::ShootBullet);

    mContext->mNotificationManager->Register(mContext->CurrObject, GalagaNotifs::InsertCoin);
    mContext->mNotificationManager->Register(mContext->CurrObject, GalagaNotifs::StartGame);
    mContext->mNotificationManager->Register(mContext->CurrObject, GalagaNotifs::EnemyDied);
    mContext->mNotificationManager->Register(mContext->CurrObject, GalagaNotifs::PlayerDied);

    // Stars are created upon initialization
    mContext->mEngineView->AddGameObject(std::make_shared<Stars>(250, CMPUT350::Rect(0, 0, width, height)));

    applyState();  // enter the Waiting state requested by the constructor
}

void Galaga::Update(CMPUT350::GameContext* context)
{
    mContext = context;
    if (!mCurrentState) return;  // There is no current set state, nothing to do
    
    mUpdating = true;
    mCurrentState->update(*this);  // Updates the current state
    mUpdating = false;

    applyState();  // Transition to next state if a next state is given
}

void Galaga::RenderForeground(CMPUT350::GameContext* context)
{
    mContext = context;
    if (mCurrentState) {
        mCurrentState->draw(*this);
    }
}

void Galaga::ReceiveNotification(const std::string& key)
{
    // For every state
    if (key == GalagaNotifs::EnemyDied) {
        // If an enemy is killed, reduce the number of enemies alive and increase score
        --mEnemiesAlive;
        score += 100;
    } 
    else if (key == GalagaNotifs::PlayerDied) {
        // The player has died, lose a life - if all lives are lost is handled later
        mPlayerAlive = false;
        --lives;
    }

    // State-specific notifications
    if (mCurrentState) {
        mCurrentState->onNotification(*this, key);
    }
    if (!mUpdating) {
        // Only transition to next state if not currently in the middle of updating current state
        applyState();
    }
}

// Sets the requested state, does not make the actual transition
void Galaga::setState(std::unique_ptr<GalagaState> nextState) { 
    mRequestedState = std::move(nextState); 
}

// Takes the requested state and makes the transition from the current state
void Galaga::applyState()
{
    while (mRequestedState) {
        if (mCurrentState) { 
            mCurrentState->exit(*this);  // Remove the current state
        }

        mCurrentState = std::move(mRequestedState);

        // start() can also call setState, which would transition into a new state right after the first transition,
        // so we use a loop to allow this to happen directly
        mCurrentState->start(*this);
    }
}


// Check if enemies are all in formation or not - has an attack phase started / not yet ended?
bool Galaga::enemyOutOfFormation() const
{
    for (const auto& object : *mContext->mEngineView)
        if (auto enemy = std::dynamic_pointer_cast<Enemy>(object))
            //if (enemy->IsAlive() && !enemy->isInFormation()) { return true; }  // NEED TO BE ADDED TO ENEMY MEOW

    return false;
}

// 'spendCoin' lets Galaga know whether or not it can start the game and 'spends' the coin if it can
bool Galaga::spendCoin()
{
    if (!isCoin) { return false; }

    isCoin = false;  // Spen the coin and start the game
    return true;
}

// Sets beginning of game information
void Galaga::newGame()
{
    score = 0;
    lives = 3;
    currLevel = 1;
    mEnemiesAlive = 0;
    mPlayerAlive = false;
}

void Galaga::spawnPlayer()
{
    mContext->mEngineView->AddGameObject(std::make_shared<Player>(Point2D(width / 2.0f, 900)));

    mPlayerAlive = true;
}

// Each wave fills 8 slots of the 4x10 formation
void Galaga::spawnEnemies(int wave, int indexInWave)
{
    const float separator = 600.0f / 9.0f;  // Separator between enemies
    int slot = wave * shipsPerWave + indexInWave;  // gets specific of the 40 enemy spots (wave indicates the row)

    // 84 is the margin from the edge, 10 in each row over 4 rows, 100 is the margin from the top
    Point2D enemyPosition(84 + (slot % 10) * separator, 100 + (slot / 10) * separator);

    // Cubic Bezier: start off screen, alternating which side per wave, then fly in and end at enemyPosition
    bool leftSide = (wave % 2 == 0);  // If true, enemies come from left, if false, enemies come from right

    float startX;
    if (leftSide) { startX = -50.0f; }
    else { startX = width + 50.0f; }

    float sign;
    if (leftSide) { sign = 1.0f; }
    else { sign = -1.0f; }

    // Creates a set of points that enemy can use to call bezier
    std::vector<Point2D> path = {
        Point2D(startX, 200),  // Start point
        Point2D(startX + sign * 400, 700),
        Point2D(width / 2.0f - sign * 300, 600),
        enemyPosition,  // End point
    };

    // MEOW ========== I NEED TO ADD TO ENEMY FILES WAY TO USE THE POINTS
    mContext->mEngineView->AddGameObject(std::make_shared<Enemy>(enemyPosition, path));
    ++mEnemiesAlive;
}

// count is number of enemies in the attack, picks that number of enemies that are currently in formation and sends them to attack
void Galaga::startAttack(int count)
{
    std::vector<std::shared_ptr<Enemy>> enemiesInFormation;
    for (const auto& object : *mContext->mEngineView) {
        // for each entity in engine view
        if (auto enemy = std::dynamic_pointer_cast<Enemy>(object)) {
            //if (enemy->IsAlive() && enemy->isInFormation()) {  // enemy isInFormation needs to be written MEOW!!
             //   enemiesInFormation.push_back(enemy);
            //}
        }
    }

    // For each of the requested number of attackers, and while there are still enemies available to attack
    for (int e = 0; e < count && !enemiesInFormation.empty(); ++e) {
        size_t index = (mAttackCounter++ * 3) % enemiesInFormation.size();  // multiplying by 3 just makes the enemies picked more random
        //enemiesInFormation[index]->startAttack();  // ENEMIES NEED A START ATTACK METHOD ALSO MEOW
        // erase uses an iterator not the index itself
        enemiesInFormation.erase(enemiesInFormation.begin() + index);
    }
}

// The player has dies - check if lives all lost or not
void Galaga::playerDeath()
{
    if (lives > 0) {
        setState(std::make_unique<GalagaRespawn>());
    }
    else {
        setState(std::make_unique<GalagaGameOver>());
    }
}

// Player and Enemy register for remove all notif in their own initialize methods
void Galaga::clearField()
{
    mContext->mNotificationManager->Notify(GalagaNotifs::RemoveAll);
    mEnemiesAlive = 0;
    mPlayerAlive = false;
}

void Galaga::drawText(const std::string& text, float x, float y)
{
    // TODO: draw with mContext->GUIContext (engine never sets an offset on it) once I've seen DrawContext.h
    (void)text; (void)x; (void)y;  // MEOWWW
}

void Galaga::drawDisplay()
{
    drawText("SCORE: " + std::to_string(score), 20, 20);
    drawText("LIVES: " + std::to_string(lives), width - 150.0f, 20);
}



// ============== GALAGA BASE STATE ====================================
void GalagaState::start(Galaga& galaga) {
    mStartFrame = galaga.frame();
    enter(galaga);
}

// Returns current amount of frames spent in state
int GalagaState::framesInState(const Galaga& galaga) const { 
    return galaga.frame() - mStartFrame; 
}


// ============= GALAGA WAITING STATE =================================
void GalagaWaiting::onNotification(Galaga& galaga, const std::string& key) {
    if (key == GalagaNotifs::InsertCoin) {
        galaga.insertCoin();
    } 
    else if (key == GalagaNotifs::StartGame && galaga.spendCoin()) {
        galaga.newGame();
        galaga.setState(std::make_unique<GalagaStartup>());  // Transition to startup state
    }
}

// To draw text on screen
void GalagaWaiting::draw(Galaga& galaga) {
    galaga.drawText("GALAGA", galaga.getWidth() / 2.0f - 60, 300);
    
    if (!galaga.getCoin())
    {
        // A coin hasn't been entered yet
        galaga.drawText("Credits: 0", 20, galaga.getHeight() - 40.0f);
        galaga.drawText("Press 5 To Insert Coin", galaga.getWidth() / 2.0f - 120, 500);
    }
    else
    {
        // There is a coin entered
        galaga.drawText("Credits = 1", 20, galaga.getHeight() - 40.0f);
        galaga.drawText("Press 1 To Start", galaga.getWidth() / 2.0f - 120, 500);
    }
}


// ============== GALAGA STARTUP STATE =================================
void GalagaStartup::enter(Galaga& galaga) {
    // If the player is not currently spawned, spawn them
    if (!galaga.playerAlive()) { galaga.spawnPlayer(); }
}

// Wait a few seconds, then begin the waves
void GalagaStartup::update(Galaga& galaga) {
    if (framesInState(galaga) > 2 * fps) {
        galaga.setState(std::make_unique<GalagaWaves>());
    }
}

void GalagaStartup::draw(Galaga& galaga) {
    galaga.drawText("Level: " + std::to_string(galaga.getLevel()), galaga.getWidth() / 2.0f - 60, 500);
    galaga.drawDisplay();
}


// ============== GALAGA WAVES STATE ===================================
void GalagaWaves::update(Galaga& galaga) {
    // waves are done - stop spawning things
    if (mWave >= totalWaves) {
        if (!galaga.enemyOutOfFormation()) {
            galaga.setState(std::make_unique<GalagaGamePlay>());  // Request to start game play
            return;
        }

    }

    if (galaga.frame() < mNextSpawnFrame) { return; }  // Dont spawn things too soon after spawning previous things

    // mShipInWave is which of the 8 ships per wave ( indexed 0 to 7 )
    if (mShipInWave < shipsPerWave) {
        galaga.spawnEnemies(mWave, mShipInWave++);
        mNextSpawnFrame = galaga.frame() + fps / 4;  // Just sets a gap between enemy ships
    }
    else {
        // All of the ships for the current wave have spawned
        mShipInWave = 0;  // reset
        ++mWave;
        mNextSpawnFrame = galaga.frame() + fps * 2;  // Wait between waves
    }
}

void GalagaWaves::draw(Galaga& galaga) { galaga.drawDisplay(); }


// ============== GALAGA GAMEPLAY STATE ================================
void GalagaGamePlay::enter(Galaga& galaga) {
    mNextAttackFrame = galaga.frame() + fps * 2;
    if (!galaga.playerAlive()) {
        galaga.playerDeath();  // player died during the waves
    }
}

void GalagaGamePlay::update(Galaga& galaga) {
    if (!galaga.playerAlive()) { 
        // Player died
        galaga.playerDeath(); 
        return; 
    }

    if (galaga.enemiesAlive() <= 0) {
        galaga.nextLevel();
        galaga.setState(std::make_unique<GalagaStartup>());
        return;
    }

    if (galaga.frame() >= mNextAttackFrame) {
        galaga.startAttack(2);
        mNextAttackFrame = galaga.frame() + 2 * fps;
    }
}

void GalagaGamePlay::draw(Galaga& galaga) { galaga.drawDisplay(); }


// ============== GALAGA RESPAWN STATE =================================
void GalagaRespawn::update(Galaga& galaga) {
    if (mResumeFrame < 0) {
        if (galaga.enemyOutOfFormation()) { return; }  // Wait until enemies are in their positions before respawning
        if (!galaga.playerAlive()) { galaga.spawnPlayer(); }
        mResumeFrame = galaga.frame() + 2 * fps;

    } 
    else if (galaga.frame() >= mResumeFrame) {
        // Adds a short pause before resuming game play
        galaga.setState(std::make_unique<GalagaGamePlay>());
    }
}

void GalagaRespawn::draw(Galaga& galaga) {
    galaga.drawText("Lives Left: " + std::to_string(galaga.getLives()), galaga.getWidth() / 2.0f - 90, 500);
    galaga.drawDisplay();
}


// ============== GALAGA GAMEOVER STATE ================================
void GalagaGameOver::update(Galaga& galaga) {
    // Wait a secb before going back to start screen
    if (framesInState(galaga) > 4 * fps) {
        galaga.clearField();
        galaga.setState(std::make_unique<GalagaWaiting>());
    }
}

void GalagaGameOver::draw(Galaga& galaga) {
    galaga.drawText("Game Over!", galaga.getWidth() / 2.0f - 80, 450);
    galaga.drawText("Final Score: " + std::to_string(galaga.getScore()), galaga.getWidth() / 2.0f - 100, 520);
}