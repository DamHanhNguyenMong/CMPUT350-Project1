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

        mWindow->setKeyRepeatEnabled(false);

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

namespace {
    // Helper: Rotate a point
    Point2D RotatePoint (Point2D p, float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);

        return Point2D(p.x * c - p.y * s, p.x * s + p.y * c);
    }
    // Helper: Conver to world coordinates by rotating the point and adding the location
    Point2D ToWorld(Point2D p, Point2D location, float rotation) {
        return location + RotatePoint(p, rotation);
    }

    // Helper: world -> local: move back by the location, then rotate the opposite way
    Point2D ToLocal(Point2D p, Point2D location, float rotation) {
        return RotatePoint(p - location, -rotation);
    }

    // Helper: Transform a Line
   Line WorldLine(const Line& l, Point2D location, float rotation) {
    return Line(ToWorld(l.p1, location, rotation), ToWorld(l.p2, location, rotation));
}

    // Helper: Transform a Circle
    Circle WorldCircle(const Circle& c, Point2D location, float rotation) {
        return Circle(ToWorld(c.center, location, rotation), c.radius);
    }

    // Helper: rotate and translate the corners
    std::vector<Point2D> WorldCorners(const Rect& r, Point2D location, float rotation) {
        std::vector<Point2D> corners;
        corners.push_back(ToWorld(r.topLeft, location, rotation));
        corners.push_back(ToWorld(Point2D(r.topLeft.x + r.width, r.topLeft.y), location, rotation));
        corners.push_back(ToWorld(Point2D(r.topLeft.x + r.width, r.topLeft.y + r.height), location, rotation));
        corners.push_back(ToWorld(Point2D(r.topLeft.x, r.topLeft.y + r.height), location, rotation));
        return corners;
    }

    // Helper: Circle x Circle
    bool CircleCircle(const Circle& c1, const Circle& c2) {
        // (p1.x-p2.x)^2+(p1.y-p2.y)^2<=(r1+r2)^2
        return c1.center.Distance(c2.center) <= c1.radius + c2.radius;
    }

    // Helper: Point inside a rotated rectangle
    // A point is inside a (rotated) rect if it is on the same side of all 4 edges.
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
                if (Point2D::Cross(edge, p - a) >= 0) { // this corner is inside or on the edge
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

    // Circle x Rect
    bool CircleRect(const Circle& c, const std::vector<Point2D>& corners) {
        if (PointInRect(c.center, corners)) 
            return true; // center inside the rect
        for (int i = 0; i < 4; i++) {  // or the circle touches an edge
            Line edge(corners[i], corners[(i + 1) % 4]);
            // If the the center closest point to the edge is smaller than the radius then return true
            if (edge.ClosestPoint(c.center).Distance(c.center) <= c.radius) 
                return true;
        }
        return false;
    }

    // Helper: Line x Circle and return the FIRST hit point
    bool LineCircleHit(const Line& l, const Circle& c, Point2D& hit) {
        // Line starts inside the circle: the first contact is the start itself
        if (l.p1.Distance(c.center) <= c.radius) { hit = l.p1; return true; }

        float length = l.Length();
        if (length == 0) 
            return false; // a single point outside the circle

        Point2D dir = (l.p2 - l.p1) * (1.0f / length); // unit direction p1 -> p2
        float t = Point2D::Dot(c.center - l.p1, dir); // distance along the line to the point nearest the center
        Point2D nearest = l.p1 + dir * t; // that nearest point (not clamped)
        float d = static_cast<float>(nearest.Distance(c.center)); // Distance between the nearest point to the center
        // If the distance larger than the radius
        if (d > c.radius) return false; // line misses the circle

        // The line enters the circle back units before the nearest point (Pythagoras)
        // nearest is the middle of the chord that the line cuts through the circle BUT nit where the line first touches the circle
        // The first touch (the entry point) is some distance before nearest along the line, and that distance is back
        // So since center, nearest and the entry point form a right triangle
        float back = std::sqrt(c.radius * c.radius - d * d);
        float tEnter = t - back; // distance from p1 to the entry point
        if (tEnter < 0 || tEnter > length) 
            return false; // circle is behind p1 or beyond p2
        // hit = l1 + distance * direction unit vector
        hit = l.p1 + dir * tEnter;
        return true;
    }

    // Helper: Line x rect and return the FIRST hit point
    // The hit point is the intersect point closest to p1 among all the edges
    bool LineRectHit(const Line& l, const std::vector<Point2D>& corners, Point2D& hit) {
        // If the p1 inside the rect, first hit is p1
        if (PointInRect(l.p1, corners)) { hit = l.p1; return true; }

        bool found = false;
        double bestDist = 0;
        for (int i = 0; i < 4; i++) {
            Line edge(corners[i], corners[(i + 1) % 4]);
            Point2D p;
            if (l.Crosses(edge, p)) { // If l crosses edge, p is the interesect point
                double dist = l.p1.Distance(p); // Distance between p1 and p
                if (!found || dist < bestDist) { // keep the crossing closest to p1
                    bestDist = dist;
                    hit = p;
                    found = true;
                }
            }
        }
        return found;
    }

    // Helper: Order shapes so that Line < Circle < Rect. That way we only write 6 cases, not 9
    int Rank(ShapeType t) {
        if (t == ShapeType::kLine) return 0;
        if (t == ShapeType::kCircle) return 1;
        return 2;
    }

    // Returns true if the two shapes overlap
    // If a line is involved, hasHit is set to true and hit holds the first
    // contact point along that line, in WORLD coordinates
    bool ShapesCollide(const Shape& sa, Point2D posA, float rotA,
                    const Shape& sb, Point2D posB, float rotB,
                    bool& hasHit, Point2D& hit) {
        hasHit = false;

        // Put the "lower" shape type first (swap A and B if needed)
        if (Rank(sa.t) > Rank(sb.t)) {
            return ShapesCollide(sb, posB, rotB, sa, posA, rotA, hasHit, hit);
        }

        if (sa.t == ShapeType::kLine) {
            // Transform la
            Line la = WorldLine(sa.shape.line, posA, rotA);
            hasHit = true;
            if (sb.t == ShapeType::kLine) {
                // Transform lb
                Line lb = WorldLine(sb.shape.line, posB, rotB);
                // Return the hit point of the 2 lines
                return la.Crosses(lb, hit);
            }
            if (sb.t == ShapeType::kCircle) {
                // Return the hit point of the line and the circle
                return LineCircleHit(la, WorldCircle(sb.shape.circle, posB, rotB), hit);
            }
            // Otherwise, return the hit point of the line and rect
            return LineRectHit(la, WorldCorners(sb.shape.rect, posB, rotB), hit);
        }

        if (sa.t == ShapeType::kCircle) {
            // Transform circle ca
            Circle ca = WorldCircle(sa.shape.circle, posA, rotA);
            if (sb.t == ShapeType::kCircle) {
                // Return true if they collide
                return CircleCircle(ca, WorldCircle(sb.shape.circle, posB, rotB));
            }
            // Return true if they collide
            return CircleRect(ca, WorldCorners(sb.shape.rect, posB, rotB));
        }

        // Rect x Rect
        return RectRect(WorldCorners(sa.shape.rect, posA, rotA), WorldCorners(sb.shape.rect, posB, rotB));
    }
} // namespace

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

        // Since when Initialize calls AddGameObject, it does a pushback. We don't want to perform this 
        // on a vector we are iterating. So instead we iterate on pending
        std::vector<std::shared_ptr<GameObject>> pending;
        pending.swap(mObjectsToAdd); // mObjectsToAdd is now empty

        for (auto& gameObject : pending) {
            mGameObjects.push_back(gameObject);
            gameObject->Initialize(&mContext);
        }


        // 2. Process events
        while (const auto event = mWindow->pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                mWindow->close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                // Check if the pressed key has a registered notification
                int key = static_cast<int>(keyPressed->code);
                auto it = mKeyDownNotifications.find(key); // find the key in the key down notifications map
                if (it != mKeyDownNotifications.end()) {
                    for (const std::string& msg : it->second)
                        mNotificationManager.Notify(msg);
                }
            }
            else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                // Check if the released key has a registered notification
                int key = static_cast<int>(keyReleased->code);
                auto it = mKeyUpNotifications.find(key); // find the key in the key down notifications map
                if (it != mKeyUpNotifications.end()) {
                    for (const std::string& msg : it->second)
                        mNotificationManager.Notify(msg);
                }
            }
        }


        // 3. Update game objects
        for (auto& gameObject : mGameObjects) {
            gameObject->Update(&mContext);
        }

        // 4. Process collision events
        // Cast ONCE per frame, instead of inside the nested loops
        std::vector<std::shared_ptr<CollisionObject>> colliders;
        for (auto& gameObject : mGameObjects) {
            std::shared_ptr<CollisionObject> c = std::dynamic_pointer_cast<CollisionObject>(gameObject);
            if (c != nullptr) {
                colliders.push_back(c); // all the collision objects
            }
        }

        for (size_t a = 0; a < colliders.size(); ++a) {
            std::shared_ptr<CollisionObject> objA = colliders[a];

            for (size_t b = a + 1; b < colliders.size(); ++b) { // Avoid checking colllide with themselves
                std::shared_ptr<CollisionObject> objB = colliders[b];

                // Skip anything that already died this frame
                if (!objA->IsAlive()) break;
                if (!objB->IsAlive()) continue;

                // CollisionObject is already a GraphicsObject, so no second cast is needed
                Point2D posA = objA->GetLocation();
                Point2D posB = objB->GetLocation();
                float rotA = objA->GetRotation();
                float rotB = objB->GetRotation();

                // Compare every shape of A with every shape of B until one pair overlaps
                bool collided = false;
                bool hasHit = false;
                Point2D hitWorld;
                const std::vector<Shape>& shapesA = objA->GetShapes();
                const std::vector<Shape>& shapesB = objB->GetShapes();
                // Loop through each shape of A and B 
                for (size_t i = 0; i < shapesA.size() && !collided; ++i) {
                    for (size_t j = 0; j < shapesB.size() && !collided; ++j) {
                        collided = ShapesCollide(shapesA[i], posA, rotA,
                                                shapesB[j], posB, rotB,
                                                hasHit, hitWorld);
                    }
                }

                if (!collided) continue; // If no collision, skip the loop

                // Exactly one notification per object for this pair
                if (hasHit) {
                    // The hit point is in world coordinates; each object gets it in its own local space
                    objA->CollisionEnter(objB, ToLocal(hitWorld, posA, rotA));
                    objB->CollisionEnter(objA, ToLocal(hitWorld, posB, rotB));
                } else {
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
    
