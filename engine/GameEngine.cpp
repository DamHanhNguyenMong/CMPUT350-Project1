#include "GameEngine.h"
#include "GameContext.h"
#include "CollisionObject.h"
#include "GraphicsObject.h"
#include "DrawContext.h"
#include "FontData.h"

namespace CMPUT350 {


/**
 * @brief Creates and initializes the game engine.
 *
 * @param width The width of the game window in pixels.
 * @param height The height of the game window in pixels.
 * @param name The title displayed on the game window.
 *
 * Creates the SFML window, sets the frame rate limit, loads the game font,
 * creates the DrawContext, and initializes the GameContext with access to
 * the engine and drawing context.
 */
GameEngine::GameEngine(unsigned int width, unsigned int height, const std::string& name) {
    // Sample font loading code
    //	if (!mFont->openFromMemory(&_font, _font_len))
    //	{
    //		fprintf(stderr, "WARNING: Font did not load.\n");
    //	}
    
    // Create SMFL window
        mWindow = std::make_shared<sf::RenderWindow>(sf::VideoMode({width,height}), name); // the game window
        mWindow->setFramerateLimit(30); //maximum 30 FPS

        // Create the font object used for drawing text
        mFont = std::make_shared<sf::Font>();

        // Load the embedded font data into the font object
        if (!mFont->openFromMemory(&_font, _font_len))
        {
            fprintf(stderr, "Font did not load\n");
        }

        // Create the drawing context using the game window and loaded font
        mDrawContext = std::make_shared<DrawContext>(mWindow, mFont);

        // Give the game context access to the engine and drawing context
        mContext.mEngineView = this;
        mContext.ScreenContext = mDrawContext.get(); //get the raw pointer DrawContext*
        // This gives GameContext access to DrawContext but GameEngine still owns it

}

/**
 * @brief Destroys the game engine and closes the game window.
 *
 * This function releases the window resource by closing the SFML window.
 */
GameEngine::~GameEngine() {
    // Cleanup resources
    // mWindow->close();
    // Destructor: close the window. The window itself will also be cleaned
        if (mWindow) {
            mWindow->close();
        }
}

/**
 * @brief Adds a game object to the engine.
 *
 * @param gameObject The game object to be added.
 *
 * The object is placed in a temporary list and will be added and
 * initialized at the beginning of the next game frame.
 */
void GameEngine::AddGameObject(std::shared_ptr<GameObject> gameObject) {
        // Objects are not to enter the game immediatly but stored in a temp vector first
        mObjectsToAdd.push_back(gameObject);
    }

/**
 * @brief Runs the main game loop.
 *
 * This function repeatedly processes the game until the game window
 * is closed. Each frame removes dead objects, adds newly created
 * objects, processes input events, updates objects, checks collisions,
 * performs late updates, and renders the game.
 *
 * @return void
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
                // loop through each game object, handle events based on each
                for (auto& gameObject : mGameObjects) {
                    gameObject->HandleKeyEvent(&mContext, static_cast<char>(keyPressed->unicode));
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
                // 1. A is completely to the LEFT of B
                // 2. A is completely to the RIGHT of B
                // 3. A is completely ABOVE B
                // 4. A is completely BELOW B

                bool noCollision =
                    rectA.topLeft.x + rectA.width <= rectB.topLeft.x ||
                    rectB.topLeft.x + rectB.width <= rectA.topLeft.x ||
                    rectA.topLeft.y + rectA.height <= rectB.topLeft.y ||
                    rectB.topLeft.y + rectB.height <= rectA.topLeft.y;

                if (!noCollision)
                {
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
