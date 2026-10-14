#include "GameEngine.h"
#include "GameContext.h"
#include "CollisionObject.h"
#include "GraphicsObject.h"
#include "DrawContext.h"
#include "FontData.h"
#include <cmath>
#include <algorithm>
#include <vector>

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
        
        // Give the game context access to the notification manager
        mContext.mNotificationManager = &mNotificationManager;
        mContext.currentFrame = 0;
        // Create a separate DrawContext for GUI elements
        mGUIContext = std::make_shared<DrawContext>(mWindow, mFont);
        mContext.GUIContext = mGUIContext.get();

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
 * @brief Installs a notification for a key press event.
 *
 * @param key The key code to listen for.
 * @param notification The notification name to send when the key is pressed.
 *
 * This function registers a notification that will be sent to all
 * registered game objects when the specified key is pressed.
 */
void GameEngine::InstallKeyDownNotification(int key, const std::string& notification) {
    // Store the key and notification in the key down notifications map
    mKeyDownNotifications[key].push_back(notification);
}

/**
 * @brief Installs a notification for a key release event.
 *
 * @param key The key code to listen for.
 * @param notification The notification name to send when the key is released.
 *
 * This function registers a notification that will be sent to all
 * registered game objects when the specified key is released.
 */
void GameEngine::InstallKeyUpNotification(int key, const std::string& notification) {
    // Store the key and notification in the key up notifications map
    mKeyUpNotifications[key].push_back(notification);
}

/**
 * @brief Returns a constant iterator to the beginning of the game objects.
 *
 * @return A constant iterator pointing to the first game object.
 */
EngineView::const_iterator GameEngine::cbegin() const {
    return mGameObjects.cbegin();
}

EngineView::const_iterator GameEngine::cend() const {
    return mGameObjects.cend();
}

EngineView::const_iterator GameEngine::begin() const {
    return mGameObjects.begin();
}

/**
 * @brief Returns a constant iterator to the end of the game objects.
 *
 * @return A constant iterator pointing past the last game object.
 */
EngineView::const_iterator GameEngine::end() const {
    return mGameObjects.end();
}

// Helper: Rotate a point
Point2D RotatePoint (Point2D p, float angle) {
    float x = p.x * std::cos(angle) - p.y * std::sin(angle);
    float y = p.x * std::sin(angle) + p.y * std::cos(angle);

    return Point2D(x, y);
}
// Conver to world coordinates by rotating the point and adding the location
Point2D ToWorld(Point2D p, Point2D location, float rotation) {
    return location + RotatePoint(p, rotation);
}

// Helper: Circle x Circle
bool CircleCircle(const Circle& c1, const Circle& c2) {
    // (p1.x-p2.x)^2+(p1.y-p2.y)^2<=(r1+r2)^2
    return c1.center.Distance(c2.center) <= c1.radius + c2.radius;
}

// Helper: Line x Circle
bool LineCircle(const Line& line, const Circle& c) {
    // Check distance between the p and c and the radius of the circle
    return line.ClosestPoint(c.center).Distance(c.center) <= c.radius;
}

// Helper: Line x Line
bool LineLine(const Line& line1, const Line& line2) {
    Point2D crossingPoint; // This will hold the crossing point if the lines cross
    return line1.Crosses(line2, crossingPoint);
}

// Helper: Line with width: creates an axis-aligned bounding box around the line
Rect LineBounds(const Line& line, float width) {
    float half = width / 2.0f;

    float minX = std::min(line.p1.x, line.p2.x) - half;
    float maxX = std::max(line.p1.x, line.p2.x) + half;
    float minY = std::min(line.p1.y, line.p2.y) - half;
    float maxY = std::max(line.p1.y, line.p2.y) + half;

    return Rect(Point2D(minX, minY), Point2D(maxX, maxY));
}

// Helper: Get four corners of the rectangle
std::vector<Point2D> RectCorners(const Rect& r) {
    return {
        r.topLeft,
        Point2D(r.topLeft.x + r.width, r.topLeft.y),
        Point2D(r.topLeft.x + r.width, r.topLeft.y + r.height),
        Point2D(r.topLeft.x, r.topLeft.y + r.height)
    };
}
// Helper: rotate and translate the corners
std::vector<Point2D> WorldCorners(const Rect& r, Point2D location, float rotation) {
    std::vector<Point2D> corners = RectCorners(r); // get the 4 corners

    for (Point2D& p : corners) {
        p = ToWorld(p, location, rotation); // rotate and translate each corner to world coordinates
    }

    return corners;
}

// Helper: Point inside a rotated rectangle
bool PointInRect(Point2D p, const std::vector<Point2D>& corners) {
    bool positive = false;
    bool negative = false;
    // For each edge, check the cross-product sign
    for (int i = 0; i < 4; i++){
        Point2D a = corners[i];
        Point2D b = corners[(i + 1) % 4];

        Point2D edge = b - a;
        Point2D toPoint = p - a;

        float cross = Point2D::Cross(edge, toPoint);

        if (cross > 0) positive = true;
        if (cross < 0) negative = true;
    }

    return !(positive && negative);
}

// Helper: Check if two rotated rectangles intersect
bool SeparatedByEdge(const std::vector<Point2D>& cornersA, const std::vector<Point2D>& cornersB) {
    // Check each edge of rectangle A to see if all points of rectangle B are on one side
    for (int i = 0; i < 4; i++) {
        Point2D a = cornersA[i];
        Point2D b = cornersA[(i + 1) % 4];

        Point2D edge = b - a;

        bool allOutside = true;
        // Check if all points of rect B are on the side the the current edge of rect A
        for (const Point2D& p : cornersB) {
            if (Point2D::Cross(edge, p - a) >= 0) { 
                allOutside = false; 
                break; 
            }
        }
        if (allOutside) return true;   // B entirely on the outer side of this edge
    }
    return false;
}

bool RectRect(const std::vector<Point2D>& cornersA, const std::vector<Point2D>& cornersB) {
    if (SeparatedByEdge(cornersA, cornersB))
        return false;

    if (SeparatedByEdge(cornersB, cornersA))
        return false;

    return true;
}

// Helper: Transform a Line
Line WorldLine(const Line& line, Point2D location, float rotation) {
    return Line(
        ToWorld(line.p1, location, rotation),
        ToWorld(line.p2, location, rotation)
    );
}

// Helper: Transform a Circle
Circle WorldCircle(const Circle& circle, Point2D location, float rotation) {
    return Circle(
        ToWorld(circle.center, location, rotation),
        circle.radius
    );
}

// !!! ASSSUMED THAT each shape's coordinates are local to the object's rotation center !!!

// Line x Rect
bool LineRect(const Line& line, const std::vector<Point2D>& corners) {
    // If either endpoint is inside the rectangle, they collide
    if (PointInRect(line.p1, corners) || PointInRect(line.p2, corners)) {
        return true;
    }

    // Otherwise, check whether the line crosses any rectangle edge
    for (int i = 0; i < 4; i++) {
        Line edge(corners[i], corners[(i + 1) % 4]);
        // Check if the line collide with the current edge
        if (LineLine(line, edge))
            return true;
    }
    return false;
}

// Circle x Rect
bool CircleRect(const Circle& circle, const std::vector<Point2D>& corners) {
    // Circle center inside the rectangle
    if (PointInRect(circle.center, corners))
        return true;

    // Check the distance to each rectangle edge.
    for (int i = 0; i < 4; i++) {
        Line edge(corners[i], corners[(i + 1) % 4]);

        if (LineCircle(edge, circle))
            return true;
    }

    return false;
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
        
        ++mContext.currentFrame;
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
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                // Check if the pressed key has a registered notification
                int key = static_cast<int>(keyPressed->code);
                auto it = mKeyDownNotifications.find(key); // find the key in the key down notifications map
                // If found, notify the registered objects
                if (it != mKeyDownNotifications.end()) {
                    mNotificationManager.Notify(it->second);
                }
            }
            else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                // Check if the released key has a registered notification
                int key = static_cast<int>(keyReleased->code);
                auto it = mKeyUpNotifications.find(key);
                // If found, notify the registered objects
                if (it != mKeyUpNotifications.end()) {
                    mNotificationManager.Notify(it->second);
                }
            }
        }


        // 3. Update game objects
        for (auto& gameObject : mGameObjects) {
            gameObject->Update(&mContext);
        }

        // 4. Process collision events

        //Loop through each object: We comppare by pairs of objects in the vector and check if they collide
        for (size_t a = 0; a < mGameObjects.size(); ++a) {
            std::shared_ptr<CollisionObject> objA = std::dynamic_pointer_cast<CollisionObject>(mGameObjects[a]);

            if (objA == nullptr)
                continue;

            // Get object A's location and rotation
            auto graphicsA = std::dynamic_pointer_cast<GraphicsObject>(objA);
            Point2D posA = graphicsA->GetLocation();
            float rotA = graphicsA->GetRotation();

            const auto& shapesA = objA->GetShapes();

            for (size_t b = a + 1; b < mGameObjects.size(); ++b)
            {
                std::shared_ptr<CollisionObject> objB = std::dynamic_pointer_cast<CollisionObject>(mGameObjects[b]);

                if (objB == nullptr)
                    continue;

                // Get object B's location and rotation
                auto graphicsB = std::dynamic_pointer_cast<GraphicsObject>(objB);
                Point2D posB = graphicsB->GetLocation();
                float rotB = graphicsB->GetRotation();

                const auto& shapesB = objB->GetShapes();

                bool objectsCollide = false;

                // Compare every shape belonging to A with every shape belonging to B
                for (const Shape& shapeA : shapesA) {
                    for (const Shape& shapeB : shapesB) {
                        bool collided = false;

                        // Line x Line
                        if (shapeA.t == ShapeType::kLine && shapeB.t == ShapeType::kLine) {
                            // Transform the two lines according to their positions and rotations
                            Line lineA = WorldLine(shapeA.shape.line, posA, rotA);
                            Line lineB = WorldLine(shapeB.shape.line, posB, rotB);

                            collided = LineLine(lineA, lineB);
                        }

                        // Line x Circle
                        else if (shapeA.t == ShapeType::kLine && shapeB.t == ShapeType::kCircle) {
                            Line lineA = WorldLine(shapeA.shape.line, posA, rotA);
                            Circle circleB = WorldCircle(shapeB.shape.circle, posB, rotB);

                            collided = LineCircle(lineA, circleB);
                        }

                        // Circle x Line
                        else if (shapeA.t == ShapeType::kCircle && shapeB.t == ShapeType::kLine) {
                            Circle circleA = WorldCircle(shapeA.shape.circle, posA, rotA);
                            Line lineB = WorldLine(shapeB.shape.line, posB, rotB);

                            collided = LineCircle(lineB, circleA);
                        }

                        // Circle x Circle
                        else if (shapeA.t == ShapeType::kCircle && shapeB.t == ShapeType::kCircle) {
                            Circle circleA = WorldCircle(shapeA.shape.circle, posA, rotA);
                            Circle circleB = WorldCircle(shapeB.shape.circle, posB, rotB);

                            collided = CircleCircle(circleA, circleB);
                        }

                        // Rect x Rect
                        else if (shapeA.t == ShapeType::kRect && shapeB.t == ShapeType::kRect) {
                            auto cornersA = WorldCorners(
                                shapeA.shape.rect, posA, rotA);

                            auto cornersB = WorldCorners(
                                shapeB.shape.rect, posB, rotB);

                            collided = RectRect(cornersA, cornersB);
                        }

                        // Line x Rect
                        else if (shapeA.t == ShapeType::kLine && shapeB.t == ShapeType::kRect) {
                            Line lineA = WorldLine(shapeA.shape.line, posA, rotA);

                            auto cornersB = WorldCorners(
                                shapeB.shape.rect, posB, rotB);

                            collided = LineRect(lineA, cornersB);
                        }

                        // Rect x Line
                        else if (shapeA.t == ShapeType::kRect && shapeB.t == ShapeType::kLine) {
                            Line lineB = WorldLine(shapeB.shape.line, posB, rotB);

                            auto cornersA = WorldCorners(
                                shapeA.shape.rect, posA, rotA);

                            collided = LineRect(lineB, cornersA);
                        }

                        // Circle x Rect
                        else if (shapeA.t == ShapeType::kCircle && shapeB.t == ShapeType::kRect) {
                            Circle circleA = WorldCircle(shapeA.shape.circle, posA, rotA);
                            // Get the Rect corners
                            auto cornersB = WorldCorners(shapeB.shape.rect, posB, rotB);

                            collided = CircleRect(circleA, cornersB);
                        }

                        // Rect x Circle
                        else if (shapeA.t == ShapeType::kRect && shapeB.t == ShapeType::kCircle) {
                            Circle circleB = WorldCircle(shapeB.shape.circle, posB, rotB);

                            auto cornersA = WorldCorners(shapeA.shape.rect, posA, rotA);

                            collided = CircleRect(circleB, cornersA);
                        }

                        if (collided) {
                            objectsCollide = true;
                            break;
                        }
                    }

                    if (objectsCollide)
                        break;
                }

                // Notify each object once per object pair.
                if (objectsCollide) {
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
            // Set the context offset and rotation for the graphics object
            mDrawContext->SetContextOffset(graphicsObject->GetLocation());
            mDrawContext->SetContextRotation(graphicsObject->GetRotation());
            
            graphicsObject->RenderBackground(&mContext); // render the graphic object
        }


        // 7. Render foreground
        for (auto& gameObject : mGameObjects) {
            // Check if the object is the graphic object
            std::shared_ptr<GraphicsObject> graphicsObject = std::dynamic_pointer_cast<GraphicsObject>(gameObject);
            if (graphicsObject == nullptr) {
                continue;
            }
            // Set the context offset and rotation for the graphics object
            mDrawContext->SetContextOffset(graphicsObject->GetLocation());
            mDrawContext->SetContextRotation(graphicsObject->GetRotation());

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
