#include "DrawContext.h"

namespace CMPUT350 {

DrawContext::DrawContext(std::shared_ptr<sf::RenderWindow> window, std::shared_ptr<sf::Font> font)
    : mWindow(window), mFont(font) {}  // Sets the window to window

/**
 * @brief Draws text with the given size and colour, centered around the given position
 *
 * @param text The text to draw to the window (string)
 * @param pixelSize The size with which to make the text in pixels
 * @param p The position to draw the text at - centered around this point
 * @param c The colour of the text as a RGBColor object
 *
 * This function creates an sf::Text object, using the given pixelSize and text input, as
 * well the font the instance of DrawContext is using. It finds the midpoint of the rectangle
 * that would bound the text, and sets the origin to be that point. This way, when the position
 * is set to the given position, it sets the center of the text to this position. The text is
 * then drawn to the window.
 */
void DrawContext::DrawCenteredText(const std::string &text, int pixelSize, Point2D p, RGBColor c) {
    // Need to crete text object - constructor for sf::Text (const Font &font, String string="", unsigned int characterSize=30) 
    sf::Text drawableText = sf::Text(*mFont, text, pixelSize);
    
    // Centered around point p
    sf::FloatRect boundingRect = drawableText.getLocalBounds();  // Gets a rectangle that bounds the text
    float xCenter = boundingRect.position.x + (boundingRect.size.x / 2.0f);
    float yCenter = boundingRect.position.y + (boundingRect.size.y / 2.0f);

    drawableText.setOrigin(sf::Vector2f(xCenter, yCenter));  // Text position is based on the center of the text
    drawableText.setPosition(sf::Vector2f(p.x, p.y));  // Text is set to position given

    // Add color
    drawableText.setFillColor(sf::Color(c.r, c.g, c.b));

    // Draw to window
    mWindow->draw(drawableText);
}

/**
 * @brief Draws text with the given size, position, and colour
 *
 * @param text The text to draw to the window (string)
 * @param pixelSize The size with which to make the text in pixels
 * @param p The position to draw the text at
 * @param c The colour of the text as a RGBColor object
 *
 * This function creates an sf::Text object, using the given pixelSize and text input, as
 * well the font the instance of DrawContext is using. It sets the position and colour of the
 * text, and then draws it to the window.
 */
void DrawContext::DrawText(const std::string &text, int pixelSize, Point2D p, RGBColor c) {
    // Need to create text object - constructor for sf::Text (const Font &font, String string="", unsigned int characterSize=30) 
    sf::Text drawableText = sf::Text(*mFont, text, pixelSize);

    // Set color and position
    drawableText.setPosition(sf::Vector2f(p.x, p.y));  // Text is set to position given
    drawableText.setFillColor(sf::Color(c.r, c.g, c.b));

    // Draw to window
    mWindow->draw(drawableText);
}

/**
 * @brief Draws a circle with a specified radius and colour about a point
 * 
 * @param p The point around which to draw the circle (Point2D)
 * @param radius The radius of the circle to be drawn
 * @param c The colour of the circle, as an RGBColor object
 * 
 * This functions creates an sf::CircleShape around the given point, with the given
 * radius, and sets the colour of the circle to the given colour. It sets the origin
 * to be the middle of the circle instead of the top left corner, so that the circle
 * is drawn around the middle of the given point. It then draws the circle to the window.
 */
void DrawContext::DrawCircle(Point2D p, float radius, RGBColor c) {
    // Draw a circle -> sf::CircleShape
    sf::CircleShape drawableCircle = sf::CircleShape(radius);  // default constructor (float radius=0, std::size_t pointCount=30)
    
    // SFML draws about the top left corner, but we want to draw about the point
    drawableCircle.setOrigin(sf::Vector2f (radius, radius));  // Sets origin to the midpoint of the circle


    // Set attributes
    drawableCircle.setPosition(sf::Vector2f(p.x, p.y));
    drawableCircle.setFillColor(sf::Color (c.r, c.g, c.b));

    // Draw to window
    mWindow->draw(drawableCircle);
}

/**
 * @brief Draws a rectangle with a specified colour
 *
 * @param r The rectangle to be drawn, given as a Rect object
 * @param c The color of the rectangle, specified as an RGBColor object.
 *
 * This function creates an sf::RectangleShape based on the given Rect and uses 
 * it to draw a rectangle to the window. It specifies the rectangle to have the
 * given colour, c, and sets its location to the location of the Rect, r.
 */
void DrawContext::DrawRect(Rect r, RGBColor c) {
    // Draw a rectanle -> sf::RectangleShape
    sf::RectangleShape drawableRectangle = sf::RectangleShape(sf::Vector2f (r.width, r.height)); // constructor RectangleShape (Vector2f size={})

    // Draws around the top left point - rect position is top left corner so coordinates dont need to be modified
    drawableRectangle.setPosition(sf::Vector2f (r.topLeft.x, r.topLeft.y));
    drawableRectangle.setFillColor(sf::Color (c.r, c.g, c.b));

    // Draw to window
    mWindow->draw(drawableRectangle);
}

/**
 * @brief Draws the outline of a rectangle with a specified width and colour
 * 
 * @param r The rectangle of which to draw given as a Rect object
 * @param width The width of the outline in pixels
 * @param c The colour of the outline, as an RGBColor object
 * 
 * This functions creates an sf::RectangleShape and sets its outline to be of the
 * specified width and colour, width and c respectively. It then sets the fill
 * colour to be transparent so that only the outline is visible, then sets a 
 * position for the shape to be drawn and draws it to the window.
 * 
 * Note: This function draws the thickness growing inward. If we wanted the thickness
 * growing outward width should be positive.
 */
void DrawContext::FrameRect(Rect r, float width, RGBColor c) {
    // Draws just the outline of a rectangle
    sf::RectangleShape drawableFrame = sf::RectangleShape(sf::Vector2f (r.width, r.height));

    // Have to set a thickness and colour for the outline - there is no outline by default
    drawableFrame.setOutlineThickness(-width);
    drawableFrame.setOutlineColor(sf::Color (c.r, c.g, c.b));

    // Remove fill colour
    drawableFrame.setFillColor(sf::Color::Transparent);
    drawableFrame.setPosition(sf::Vector2f (r.topLeft.x, r.topLeft.y));

    // Draw to window
    mWindow->draw(drawableFrame);
}

/**
 * @brief Draws a line between two points with a specified width and color.
 *
 * @param from The starting point of the line (Point2D).
 * @param to The ending point of the line (Point2D).
 * @param width The width of the line in pixels.
 * @param c The color of the line, specified as an RGBColor object.
 *
 * This function calculates 4 points to be used to draw a rectangle representing
 * the line. It finds the slope of the line, and uses the negative inverse of that
 * slope as the slope along the edges, perpendicular to the line itself, to calculate
 * the points from 'from' and 'to'. It draws the line as a rectangle between those
 * 4 points with the given width and colour.
 */
void DrawContext::DrawLine(Point2D from, Point2D to, float width, RGBColor c) {
    // Construct shape with 4 points - calculate 2 along the perpendicular line at the end of line that runs through
    // 'from' and calculate the other 2 along the perpendicular line at the end of the line that runs through 'to'
    sf::ConvexShape drawableLine = sf::ConvexShape(4); // constructor (std::size_t pointCount=0) - set 4 points

    // end.x is the change in y between 'to' an 'from', end.x is the *negative* change in y
    Point2D end = Point2D(from.y - to.y, to.x - from.x);  // -(y2 - y1) = y1 - y2
    // Normalize end then multiply by w / 2
    end.Normalize();
    end *= (width / 2.0f); 

    // 4 points, two calculated from each 'from' and 'to'
    sf::Vector2f p1 = sf::Vector2f(from.x - end.x, from.y - end.y);  // p1 = from - end
    sf::Vector2f p2 = sf::Vector2f(from.x + end.x, from.y + end.y);  // p2 = from + end
    sf::Vector2f p3 = sf::Vector2f(to.x + end.x, to.y + end.y);  // p3 = to + end
    sf::Vector2f p4 = sf::Vector2f(to.x - end.x, to.y - end.y);  // p4 = to - end

    // Set these 4 points to make up drawableLine
    drawableLine.setPoint(0, p1);
    drawableLine.setPoint(1, p2);
    drawableLine.setPoint(2, p3);
    drawableLine.setPoint(3, p4);

    // Set colour
    drawableLine.setFillColor(sf::Color (c.r, c.g, c.b));

    // Draw the line
    mWindow->draw(drawableLine);

}

int DrawContext::GetWindowWidth() { return mWindow->getSize().x; }

int DrawContext::GetWindowHeight() { return mWindow->getSize().y; }

}  // namespace CMPUT350
