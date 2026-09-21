
#ifndef GAMEENGINE_H
#define GAMEENGINE_H

namespace CMPUT350 {
class GameEngine;
}

#include "EngineView.h"
#include "GameObject.h"
#include "MathUtil.h"
#include "GameContext.h"
#include <SFML/Graphics.hpp>

namespace CMPUT350 {

class DrawContext;

class GameEngine : public EngineView {
public:
    GameEngine(unsigned int width, unsigned int height, const std::string& name);
    ~GameEngine();

    GameEngine(const GameEngine&) = delete;             // Prevent copy-construction
    GameEngine(GameEngine&&) = delete;                  // Prevent move-construction
    GameEngine& operator=(const GameEngine&) = delete;  // Prevent assignment
    GameEngine& operator=(GameEngine&&) = delete;       // Prevent move-assignment

    void AddGameObject(std::shared_ptr<GameObject> gameObject);

    void Run();

private:
    GameContext mContext;
    std::shared_ptr<sf::RenderWindow> mWindow;
    // We use shared pointers because The engine shares ownership of this game object
    std::vector<std::shared_ptr<GameObject>> mGameObjects; // objects currently in the game
    std::vector<std::shared_ptr<GameObject>> mObjectsToAdd; // objects waiting to be added in the next frame

    //	std::shared_ptr<sf::Font> mFont;
};

}  // namespace CMPUT350

#endif  // GAMEENGINE_H
