#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <cmath>
#include <iostream>
#include <algorithm>  // for std::min

namespace CMPUT350 {

struct Point2D {
    float x, y;
    // Constructor
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}

    // Finds the scalar distance between the point and 'other'
    double Distance(const Point2D &other) const {
        // Calculate distance between (x,y) of this point and (x,y) of other
        // Note: always returns a positive value
        double xDistance = x - other.x;
        double yDistance = y - other.y;
        double distanceSquared = pow(xDistance, 2) + pow(yDistance, 2);
        return sqrt(distanceSquared);
    }

    // Overload + operator -> add this (x,y) point to the 'other' (x,y) point and returns it as a new point
    Point2D operator+(const Point2D &other) const {
        return Point2D((x + other.x), (y + other.y));
    }

    // Overload + operator -> add the float value to each x and y coordinates and return a new point
    Point2D operator+(const float &other) const {
        return Point2D((x + other), (y + other));
    }

    // Overload - operator -> subtract the 'other' (x,y) point from the (x,y) point and return a new point
    Point2D operator-(const Point2D &other) const {
        return Point2D((x - other.x), (y - other.y));
    }

    // Overload - operator -> subtract the float value from each the x and y coordinates and return a new point
    Point2D operator-(const float &other) const {
        return Point2D((x - other), (y - other));
    }

    // Overload * operator -> multiply each of the x and y values by scalar and return a new point
    Point2D operator*(const float &scalar) const {
        return Point2D((x * scalar), (y * scalar));
    }

    // Overload += operator -> add the scalar to the x and y values of the point
    Point2D &operator+=(const float &scalar) {
        x += scalar;
        y += scalar;
        return *this;
    }

    // Overload += operator -> add the 'other's x and y values to the point's x and y
    Point2D &operator+=(const Point2D &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    // Overload -= operator -> subtract the 'other' x and y values from the points x and y, respectively
    Point2D &operator-=(const Point2D &other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    // Overload == operator -> return true if x = other.x and y = other.y (if points values match)
    bool operator==(const Point2D &other) const {
        if ((x == other.x) && (y == other.y)) { return true; }
        return false;
    }

    // Overload *= operator -> multiply both the x and y values of the point by scalar
    Point2D &operator*=(const float &scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    // Overload /= operator -> divide both the x and y values of the point by scalar
    Point2D &operator/=(const float &scalar) {
        if (scalar == 0) { x = 0; y = 0; }  // Set point to (0,0) if scalar is 0
        else {
            x /= scalar;
            y /= scalar;
        }
        return *this;
    }

    // Overload * operator -> assuming this is shorthand for dot product
    float operator*(const Point2D &other) const {
        return Dot(other);
    }

    // Gets dot product of the point by the given point b
    float Dot(Point2D b) const {
        // (x1 * x2) + (y1 * y2)
        return float((x * b.x) + (y * b.y));
    }

    // Gets dot product of the given points a and b
    static float Dot(Point2D a, Point2D b) {
        // (x_a * x_b) + (y_a * y_b)
        return float((a.x * b.x) + (a.y * b.y));
    }

    // Gets modified cross product of the given points : a x b = (a.x)(b.y) - (b.x)(a.y)
    static float Cross(Point2D a, Point2D b) {
        /* 
        Note: The magnitude of this value gives the area of the parallelogram made from the two points as corners
        Second note: This is also the determinant of the two vectors and can be used for checking line intersection
        */
        return ((a.x * b.y) - (a.y *b.x));
    }

    // Normalizes the given point -> divide x and y by the length of the vector from (0,0)
    void Normalize() {
        double magnitude = Distance(Point2D(0,0));
        // To prevent division by zero
        if (magnitude != 0) { 
            x /= magnitude;
            y /= magnitude;
        }
    }
};

// Overload << operator -> return the point in a readable way to ostream
static std::ostream &operator<<(std::ostream &os, const Point2D &p) {
    // format:(p.x, p.y)
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}

// Overload * operator -> multiplies point x and y values by number and returns a new point
static Point2D operator*(float number, const Point2D &rhs) {
    return Point2D(rhs.x*number, rhs.y*number);
}

struct Line {
    Point2D p1, p2;

    // Constructors
    Line(Point2D p1 = {0, 0}, Point2D p2 = {0, 0}) : p1(p1), p2(p2) {}
    Line(float x1, float y1, float x2, float y2) : p1(x1, y1), p2(x2, y2) {}

    // Calculates length of the line
    float Length() const {
        return p1.Distance(p2);
    }

    // Returns the closest point on the line to p
    Point2D ClosestPoint(const Point2D &p) const {
        // Just in case check the points on the line are different
        if (p1 == p2) { return p1; }

        Point2D lineVector = p2 - p1;
        Point2D pointVector = p - p1;  // Gets a direction vector from p1 to the point

        // A dot B / B dot B => denominator is length squared
        float t = Point2D::Dot(pointVector, lineVector) / Point2D::Dot(lineVector, lineVector);

        // t value needs to be between 0 and 1 otherwise it goes off the line - t is a parameterization
        if (t < 0) { t = 0; }
        else if (t > 1) { t = 1; }

        return (p1 + lineVector * t);
    }

    /**
     * @brief Checks if two lines intersect, and if so, finds the intersection point
     * 
     * @param other
     * @param crossingPoint
     * 
     * Uses slope vectors to check if the line and 'other' overlap, and returns true or false accordingly. If the 
     * lines overlap on just one point, then crossingPoint is set to that point. If the lines are collinear (parallel
     * and overlapping), crossingPoint is set to the midpoint between the two innermost overlapping points of the lines.
     */
    bool Crosses(Line other, Point2D &crossingPoint) const {
        // Based on lecture 6 and linked stack overflow
        // Get slope vectors for each line - x = x2 - x1, y = y2 - y1
        Point2D slopeLine = Point2D(p2.x - p1.x, p2.y - p1.y);
        Point2D slopeOther = Point2D(other.p2.x - other.p1.x, other.p2.y - other.p1.y);

        float slopeCross = Point2D::Cross(slopeLine, slopeOther);  // Cross product of the two slopes
        float pointDiffCross = Point2D::Cross((other.p1 - p1), slopeLine);  // Cross product of the difference of points by slope of the line

        // Case 1: The lines are collinear and may or may not overlap
        if (slopeCross == 0 && pointDiffCross == 0) {
            // Represent the second's points line in terms of the first based on p1 + t0 * slopeLine
            if (!(slopeLine == Point2D(0, 0))) { 
                float t0 = Point2D::Dot(other.p1 - p1, slopeLine) / Point2D::Dot(slopeLine, slopeLine);
                float t1 = Point2D::Dot(other.p2 - p1, slopeLine) / Point2D::Dot(slopeLine, slopeLine);

                // If [t0, t1] intersects with [0, 1] then the lines intersect
                // First two in the if statement check if one endpoint overlaps the other line, second two check if other fully contains line
                if ((0 <= t0 && t0 <= 1) || (0 <= t1 && t1 <= 1) || (t0 <= 0 && t1 >= 0) || (t1 <= 0 && t0 >= 1)) {
                    // The lines overlap, in this implementation, return the midpoint between the two inner points
                    float upperMidpoint = fmin(1.0f, fmax(t0, t1));
                    float lowerMidpoint = fmax(0.0f, fmin(t0, t1));
                    crossingPoint = p1 + (slopeLine * ((lowerMidpoint + upperMidpoint) / 2.0f));  // finds crossing point from parameterization

                    return true;
                }
                return false;  // Lines are collinear but do not overlap
            }

             // Special case: slopeLine is the zero vector - the line is made up of two of the same point
             // We know p1 == p2 by this point
            if (p1 == other.p1 && other.p1 == other.p2) { crossingPoint = p1; return true; } // All 4 points are just the zero vector or the same
            
            if (slopeOther == Point2D(0, 0)) { return false; }  // other is different from p1 but is also just one point (don't overlap)

            // The line is made up of two of the same point -> check if that point lies along the line other
            // Represent p1 as a point on the line other: other.p1 + t0 * slopeOther = p1
            float t0 = Point2D::Dot(p1 - other.p1, slopeOther) / Point2D::Dot(slopeOther, slopeOther);
            if (0 <= t0 && t0 <= 1) {
                // p1 == p2 and it sits somewhere on other, crossing point is just p1
                crossingPoint = p1;
                return true;
            }
            return false;
        }
        
        // Case 2: The lines are parallel and disjoint (do not overlap)
        if (slopeCross == 0 && pointDiffCross != 0) {
            return false;  // The lines do not overlap
        }

        // Case 3: The line segments may meet but we have to check - implicitly slopeCross != 0
        // Calculate t = ( (other.p1 - p1) x slopeOther ) / (slopeLine x slopeOther) based on lecture notes
        float t = (Point2D::Cross(other.p1 - p1, slopeOther) / Point2D::Cross(slopeLine, slopeOther));
        // Calculate u = ( (other.p1 - p1) x slopeLine ) / (slopeLine x slopeOther) derived based on lecture notes
        float u = (Point2D::Cross(other.p1 - p1, slopeLine) / Point2D::Cross(slopeLine, slopeOther));

        // If both t and u are between 0 and 1, then the lines intersect and we can find the intersection point using t and u:
        if (0 <= t && t <= 1 && 0 <= u && u <= 1) {
            // Find intersection point as point where p1 + t*slopeLine = other.p1 + u*slopeOther by just pluging values into either side of that equation
            crossingPoint = p1 + (t*slopeLine);
            return true;
        }
        
        // The lines are not parallel but they also do not intersect
        return false;
    }
};

// Overload << operator -> print line information to ostream
static std::ostream &operator<<(std::ostream &os, const Line &l) {
    os << l.p1 << " -> " << l.p2;
    return os;
}

struct Circle {
    Point2D center;
    float radius;

    Circle(Point2D c = {0, 0}, float r = 0) : center(c), radius(r) {}

    Circle(float x, float y, float r) : center(x, y), radius(r) {}
};

struct Rect {
    Point2D topLeft;
    float width, height;

    // Constructors
    Rect(float left, float top, float width, float height)
        : topLeft(Point2D(left, top)), width(width), height(height) {}

    Rect(Point2D tl = {0.0f, 0.0f}, float w = 0.0f, float h = 0.0f) : topLeft(tl), width(w), height(h) {}

    // Creates bounding box around p1 and p2 with positive width/height
    // Note to myself: remember 0,0 is the top left corner
    Rect(Point2D p1, Point2D p2)
        : topLeft(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
          width(fabs(p1.x - p2.x)),
          height(fabs(p1.y - p2.y)) {}

    Rect(Point2D center, float radius)
        : topLeft(center.x - radius, center.y - radius), width(2 * radius), height(2 * radius) {}

    // Overload |= operator for two rectangles: create a new rectangle of the smallest bounding box of both rectangles
    Rect &operator|=(const Rect &other) {
        // Find the rightmost point
        float largestX = fmax(topLeft.x + width, other.topLeft.x + other.width);
        float largestY = fmax(topLeft.y + height, other.topLeft.y + other.height);

        // Set topLeft to leftmost and highest point
        topLeft = Point2D(fmin(topLeft.x, other.topLeft.x), fmin(topLeft.y, other.topLeft.y));

        // Set new values
        width = largestX - topLeft.x;
        height = largestY - topLeft.y;
        
        return *this;
    }

    // Overload |= operator for a rectanlgle and another point -> return the smallest bounding box of the rectangle and given point
    Rect &operator|=(const Point2D &other) {
        if (this->IsInside(other)) {
            return *this;  // The point is within the rectangle
        }

        // Find rightmost point
        float largestX = fmax(topLeft.x + width, other.x);
        float largestY = fmax(topLeft.y + height, other.y);

        // Set topLeft to the smallest of the x and y values of both the rect and point
        topLeft = Point2D(fmin(other.x, topLeft.x), fmin(other.y, topLeft.y));

        // Set new values
        width = largestX - topLeft.x;
        height = largestY - topLeft.y;

        return *this;
    }

    // Overload |= operator for a rectangle and a line -> return the smallest bounding box of the 6 points (4 from the rect, 2 from the line)
    Rect &operator|=(const Line &other) {
        // Call |= operator for each point of line
        *this |= other.p1;
        *this |= other.p2;
        return *this;
    }

    // Overload &= operator for two rects -> return the largest rect that fits completely within the other two
    Rect &operator&=(const Rect &other) {
        // Gets a point made of the larger of the two topLeft points x and y values
        float smallestX = fmax(topLeft.x, other.topLeft.x);
        float smallestY = fmax(topLeft.y, other.topLeft.y);
        
        // Gets a point made of the smallest of the rightmost / bottommost points
        float largestX = fmin(topLeft.x + width, other.topLeft.x + other.width);
        float largestY = fmin(topLeft.y + height, other.topLeft.y + other.height);

        // Check that the rectangles actually overlap - if they dont, the 'smallest' values will be larger than the 'largest' values
        if (smallestX > largestX || smallestY > largestY) {
            *this = Rect();  // The rectangles don't overlap, so the returned rectangle should reprent a null rectangle
            this->width = -1;  // Set the width to negative to indicate that the rectangle is null
            return *this;
        }

        // Set values
        topLeft = Point2D(smallestX, smallestY);
        width = largestX - smallestX;
        height = largestY - smallestY;

        return *this;
    }

    // Overload += operator -> shift entire rectangle by the x and y of the point
    Rect &operator+=(const Point2D &other) {
        topLeft.x += other.x;
        topLeft.y += other.y;
        return *this;
    }

    // Overload + operator -> shift entire rectangle by the x and y of the point and return this as a new rectangle
    Rect operator+(const Point2D &other) const {
        return Rect(topLeft.x + other.x, topLeft.y + other.y, width, height);
    }

    // Sets the rectangle to a new rectangle inset by the given amount -> assuming each side is inset but the given amount
    void Inset(float inset) {
        width -= inset*2;  // multiplied by 2 to remove inset amount from both left and right
        height -= inset*2;
        topLeft.x += inset;
        topLeft.y += inset;
    }

    // Checks if the given point is within the rectangle
    bool IsInside(const Point2D &p) const {
        if (p.x < topLeft.x || p.x > topLeft.x + width || p.y < topLeft.y || p.y > topLeft.y + height) {
            return false;   // x is out of bounds
        }
        // Both x and y are in bounds
        return true;
    }
};

// Overload << operator -> returns the topLeft, width, and height values to ostream
static std::ostream &operator<<(std::ostream &os, const Rect &l) {
    os << "Top Left: " << l.topLeft << ", width: " << l.width << ", height: " << l.height;
    return os;
}

}  // namespace CMPUT350

#endif  // MATHUTIL_H
