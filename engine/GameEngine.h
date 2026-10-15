
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
#include <unordered_map>
#include <string>
#include "NotificationManager.h"
#include <memory>
#include <vector>

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

    void AddGameObject(std::shared_ptr<GameObject> gameObject) override;

    void Run();

    void InstallKeyDownNotification(int key, const std::string& notification) override;

    void InstallKeyUpNotification(int key, const std::string& notification) override;

    EngineView::const_iterator cbegin() const override;
    EngineView::const_iterator cend() const override;
    EngineView::const_iterator begin() const override;
    EngineView::const_iterator end() const override;

private:
    GameContext mContext;
    std::shared_ptr<sf::RenderWindow> mWindow;
    std::shared_ptr<sf::Font> mFont;
    std::shared_ptr<DrawContext> mDrawContext; // A DrawCOntect Member
    // We use shared pointers because The engine shares ownership of this game object
    std::vector<std::shared_ptr<GameObject>> mGameObjects; // objects currently in the game
    std::vector<std::shared_ptr<GameObject>> mObjectsToAdd; // objects waiting to be added in the next frame

    std::unordered_map<int, std::vector<std::string>> mKeyDownNotifications; // maps key codes to notification names
    std::unordered_map<int, std::vector<std::string>> mKeyUpNotifications; // maps key codes to notification names
    NotificationManager mNotificationManager; // manages the notification system for the engine
    std::shared_ptr<DrawContext> mGUIContext; // A DrawContext for GUI elements

    //	std::shared_ptr<sf::Font> mFont;
};

}  // namespace CMPUT350

#endif  // GAMEENGINE_H
