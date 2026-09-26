#include "GameEngine.h"
#include "GameContext.h"
#include "CollisionObject.h"
#include "GraphicsObject.h"


/// @brief
namespace CMPUT350 {
#include "FontData.h"

GameEngine::GameEngine(unsigned int width, unsigned int height, const std::string& name) {
    // Sample font loading code
    //	if (!mFont->openFromMemory(&_font, _font_len))
    //	{
    //		fprintf(stderr, "WARNING: Font did not load.\n");
    //	}
    
    // Create SMFL window
        mWindow = std::make_shared<sf::RenderWindow>(sf::VideoMode({width,height}), name); // the game window
        mWindow->setFramerateLimit(30); //maximum 30 FPS
        mContext.mEngineView = this; // set engine view

}

GameEngine::~GameEngine() {
    // Cleanup resources
    // mWindow->close();
    // Destructor: close the window. The window itself will also be cleaned
        if (mWindow) {
            mWindow->close();
        }
}

void GameEngine::AddGameObject(std::shared_ptr<GameObject> gameObject) {
        // Objects are not to enter the game immediatly but stored in a temp vector first
        mObjectsToAdd.push_back(gameObject);
    }

/**
 * @method Run
 * @arguments None
 * @description Gives control to the game engine. Will not return until the game window is closed or
 * all objects have been destroyed.
 */
void GameEngine::Run() {
    while (mWindow->isOpen())  // window is open
    {
        // 0. Remove any objects that are now dead

        //Store alive objects in a vector
        std::vector<std::shared_ptr<GameObject>> aliveObjects;
        //Loop through each object inside the game engine
        for (auto& gameObject:mGameObjects) {
            if (gameObject->IsAlive()) { //Check if the object is still alive
                aliveObjects.push_back(gameObject); //If it is, push to the vector
            }
        }

        mGameObjects.swap(aliveObjects); // renew the game engine objects with the alive ones

        // 1. Activate and initialize any objects added during the last frame
        for (auto& gameObject : mObjectsToAdd) {
            mGameObjects.push_back(gameObject);
            gameObject->Initialize(&mContext);
        }
        mObjectsToAdd.clear(); // Clear the waiting 


        // 2. Process events
        while (const auto event = mWindow->pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                mWindow->close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::TextEntered>()) {
                if (keyPressed->unicode == 'p') {

                } 
            }
        }


        // 3. Update game objects
        for (auto& gameObject : mGameObjects) {
            gameObject->Update(&mContext);
        }

        // 4. Process collision events

        //Loop through each object: We comppare by pairs of objects in the vector and check if they collide
        for (size_t a = 0; a<mGameObjects.size(); ++a) {
            // Cast to check if the obj we are checking is a collision object
            std::shared_ptr<CollisionObject> objA = std::dynamic_pointer_cast<CollisionObject>(mGameObjects[a]);

            //If not, skip it
            if (objA==nullptr) {
                continue;
            }
            for (size_t b = a+1; b<mGameObjects.size(); ++b) {
                // Cast to check if the obj we are checking is a collision object
                std::shared_ptr<CollisionObject> objB = std::dynamic_pointer_cast<CollisionObject>(mGameObjects[b]);
                //If not, skip it
                if (objB==nullptr) {
                    continue;
                }

                //Get their boundaries by get the ref of the rect without changing the value (const)
                const Rect& rectA = objA->GetBounds();
                const Rect& rectB = objB->GetBounds();

                // Check if the two rectangles overlap:
                // 1. Horizontal: Top left of A is on the left of top right of B
                // 2. Top right of A is on the right of top left of B
                // Same thing but vertically

                if (rectA.topLeft.x < rectB.topLeft.x + rectB.width &&
                    rectA.topLeft.x + rectA.width > rectB.topLeft.x &&
                    rectA.topLeft.y < rectB.topLeft.y + rectB.height &&
                    rectA.topLeft.y + rectA.height > rectB.topLeft.y) {
                    objA->CollisionEnter(objB);
                    objB->CollisionEnter(objA);
                }
            }

        }

        // 5. Late updates
        // Loop through each object and update
        for (auto& gameObject : mGameObjects) {
            gameObject->LateUpdate(&mContext);
        } 

        // Clear window
        mWindow->clear(sf::Color::Black);

        // 6. Render background
        for (auto& gameObject : mGameObjects) {
            // Check if the object is the graphic object
            std::shared_ptr<GraphicsObject> graphicsObject = std::dynamic_pointer_cast<GraphicsObject>(gameObject);
            if (graphicsObject == nullptr) {
                continue;
            }
            graphicsObject->RenderBackground(&mContext); // render the graphic object
        }


        // 7. Render foreground
        for (auto& gameObject : mGameObjects) {
            // Check if the object is the graphic object
            std::shared_ptr<GraphicsObject> graphicsObject = std::dynamic_pointer_cast<GraphicsObject>(gameObject);
            if (graphicsObject == nullptr) {
                continue;
            }
            graphicsObject->RenderForeground(&mContext); // render the graphic object
        }

        // Actually render to window
        mWindow->display();
    }
}

// Sample code for processing events

// bool GameEngine::ProcessEvents(GameContext *context)
//{
//	while (const std::optional event = mWindow->pollEvent())
//	{
//		if (event->is<sf::Event::Closed>())
//		{
//		}
//		else if (event->is<sf::Event::Resized>())
//		{
//		}
//		else if (const auto* keyPressed = event->getIf<sf::Event::TextEntered>())
//		{
//			// use keyPressed->unicode to get character
//		}
//	}
// }

}  // namespace CMPUT350
