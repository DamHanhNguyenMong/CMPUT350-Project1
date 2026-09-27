#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <cmath>
#include <iostream>

namespace CMPUT350 {

struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}

    double Distance(const Point2D &other) const {
        // Calculate distance between (x,y) of this point and (x,y) of other
        // Note: always returns a positive value
        double xDistance = x - other.x;
        double yDistance = y - other.y;
        double distanceSquared = pow(xDistance, 2) + pow(yDistance, 2);
        return sqrt(distanceSquared);
    }

    Point2D operator+(const Point2D &other) const {
        // Overload + operator -> add this (x,y) point to the 'other' (x,y) point
        return Point2D((x + other.x), (y + other.y));
        //return *this;
    }

    Point2D operator+(const float &other) const {
        // Overload + operator -> add the float value to each x and y coordinates??
        return Point2D((x + other), (y + other));
        //return *this;
    }

    Point2D operator-(const Point2D &other) const {
        // Overload - operator -> subtract the (x,y) point from the 'other' (x,y) point
        //return *this;
        return Point2D((x - other.x), (y - other.y));
    }

    Point2D operator-(const float &other) const {
        // Overload - operator -> subtract the float value from each the x and y coordinates
        //return *this;
        return Point2D((x - other), (y - other));
    }

    Point2D operator*(const float &scalar) const {
        // Overload * operator -> multiply each of the x and y values by scalar
        //return *this;
        return Point2D((x * scalar), (y * scalar));
    }

    Point2D &operator+=(const float &scalar) {
        // Overload += operator
        x += scalar;
        y += scalar;
        return *this;
    }

    Point2D &operator+=(const Point2D &other) {
        // Overload += operator
        x += other.x;
        y += other.y;
        return *this;
    }

    Point2D &operator-=(const Point2D &other) {
        // Overload -= operator
        x -= other.x;
        y -= other.y;
        return *this;
    }

    bool operator==(const Point2D &other) const {
        // Overload == operator -> return true if x = other.x and y = other.y
        if ((x == other.x) && (y == other.y)) { return true; }
        return false;
    }

    Point2D &operator*=(const int &scalar) {
        // Overload *= operator -> multiply both the x and y values by scalar
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Point2D &operator/=(const int &scalar) {
        // Overload /= operator -> divide both the x and y values by scalar
        x /= scalar;
        y /= scalar;
        return *this;
    }

    float operator*(const Point2D &other) const {
        // Overload * operator -> assuming this is shorthand for dot product
        return Dot(other);
    }

    float Dot(Point2D b) const {
        // Gets dot product of this by the given point b
        // (x1 * x2) + (y1 * y2)
        return float((x * b.x) + (y * b.y));
    }

    static float Dot(Point2D a, Point2D b) {
        // Gets dot product of the given points a and b
        // (x_a * x_b) + (y_a * y_b)
        return float((a.x * b.x) + (a.y * b.y));
    }

    static float Cross(Point2D a, Point2D b) {
        /* 
        Gets modified cross product of the given points : a x b = (a.x)(b.y) - (b.x)(a.y)
        Note: The magnitude of this value gives the area of the rectangle made 
        from the two points as corners
        Second note: This is also the determinant of the two vectors and can be used for checking line intersection
        */
        return ((a.x * b.y) - (a.y *b.x));
    }

    void Normalize() {
        // Normalizes the given point -> divide x and y by the length of the vector from (0,0)
        double magnitude = Distance(Point2D(0,0));
        // To prevent division by zero
        if (magnitude != 0) { 
            x /= magnitude;
            y /= magnitude;
        }
    }
};

static std::ostream &operator<<(std::ostream &os, const Point2D &p) {
    // Overload << operator -> return the point in a readable way to os stream
    // format:(p.x, p.y)
    os << "(" << p.x << ", " << p.y << ")";
    return os;
}

static Point2D operator*(float number, const Point2D &rhs) {
    // Overload * operator -> multiplies point x and y values by number and returns the modified point
    //return rhs;
    return Point2D(rhs.x*number, rhs.y*number);
}

struct Line {
    Point2D p1, p2;

    Line(Point2D p1 = {0, 0}, Point2D p2 = {0, 0}) : p1(p1), p2(p2) {}
    Line(float x1, float y1, float x2, float y2) : p1(x1, y1), p2(x2, y2) {}

    float Length() const {
        // Calculates length of the line
        //return 0;
        return p1.Distance(p2);
    }

    Point2D ClosestPoint(const Point2D &p) const {
        // Returns whichever of p1 and p2 of the line are closest to p?
        // Note: If distance to p1 and p2 are the same, p1 is returned
        //return p;
        if (p.Distance(p1) > p.Distance(p2)) { return p2; }
        else { return p1; }
    }

    bool Crosses(Line other, Point2D &crossingPoint) const {
        // If this line and 'other' cross, set crossingPoint to the point where they cross and return true, otherwise return false?

        // Find orientation of each end point with respect to the other line
        // Example: orientation of other.p1 with respect to line
        int oline1p1 = orientation(p1, p2, other.p1);
        int oline1p2 = orientation(p1, p2, other.p2);
        int oline2p1 = orientation(other.p1, other.p2, p1);
        int oline2p2 = orientation(other.p1, other.p2, p2);

        // Finds the slopes of the lines for use finding intersection points later
        float slopeLine = (p2.y - p1.y) / (p2.x - p1.x);
        float slopeOther = (other.p2.y - other.p1.y) / (other.p2.x - other.p1.x);
        // Note: need to account for vertical lines and a division by zero risk

        // If any orientations are 0, the lines projections are collinear and they may or may not intersect
        if (oline1p1 == 0 || oline1p2 == 0 || oline2p1 == 0 || oline2p2 == 0) {
            // Check if the lines overlap -> the x or y value of one of the lines lies between those of the other line
            if ((fmin(p1.x, p2.x) <= other.p1.x && other.p1.x <= fmax(p1.x, p2.x)) || (fmin(p1.x, p2.x) <= other.p2.x && other.p2.x <= fmax(p1.x, p2.x)) ||
                (fmin(p1.y, p2.y) <= other.p1.y && other.p1.y <= fmax(p1.y, p2.y)) || (fmin(p1.y, p2.y) <= other.p2.y && other.p2.y <= fmax(p1.y, p2.y))) {
                    
                // The lines intersect at 1 or more points; this implementation returns the midpoint of the inner two points
                 
                // Special case where the lines are collinear AND are completely vertical (x1 = x2 = x3 = x4)
                if (p1.x == other.p1.x && p2.x == other.p2.x && p1.x == other.p2.x) {
                    float upperMidpoint = fmin(fmax(p1.y, p2.y), fmax(other.p1.y, other.p2.y));  // Finds the smaller of the two largest points
                    float lowerMidpoint = fmax(fmin(p1.y, p2.y), fmin(other.p1.y, other.p2.y));  // Finds the larger of the two smaller points
                    crossingPoint.y = (upperMidpoint + lowerMidpoint) / 2;  // y is the middle of the two midpoints
                    crossingPoint.x = (crossingPoint.y - p1.y) * slopeLine;  // Calculated from rearranged line equation: x = (y - b) / m
                    // Note: need to handle the case that either or both of the lines are the same two points, and so slope would be 0
                }
                // The lines are not completely vertical
                else { 
                    float upperMidpoint = fmin(fmax(p1.x, p2.x), fmax(other.p1.x, other.p2.x));  // Finds the smaller of the two largest points
                    float lowerMidpoint = fmax(fmin(p1.x, p2.x), fmin(other.p1.x, other.p2.x));  // Finds the larger of the two smaller points
                    crossingPoint.x = (upperMidpoint + lowerMidpoint) / 2;  // x is the middle of the two midpoints
                    crossingPoint.y = (slopeLine*crossingPoint.x) + p1.y;  // Calculated from line equation as y = mx + b
                }

                return true;
            }

            // Lines are along the same projection but do not overlap
            return false;
        }

        // If other.p1 and other.p2 are on opposite sides of the line, and p1 and p2 are on opposite
        // sides of other, then the lines intersect
        if (oline1p1 != oline1p2 && oline2p1 != oline2p2) {
            // Find the intersection point - equate line equations, then solve for x based on that:
            // x = (other.p1.y - p1.y) / (line_slope - other_slope) -> derived from y = mx + b forms of each line
            crossingPoint.x = (other.p1.y - p1.y) / (slopeLine - slopeOther);
            crossingPoint.y = (slopeLine*crossingPoint.x) + p1.y;  // Plug x into line equation y = mx + b

            return true;
        }

        // Otherwise, the lines are neither collinear, nor do they intersect at a single point
        return false;
    }

    int orientation(Point2D op1, Point2D op2, Point2D op3) const {
        // Helper function for finding orientation of 3 points
        // References orientation of points based on: https://www.geeksforgeeks.org/dsa/orientation-3-ordered-points/
        // Creates 'slope points' based on the slope of two given points: the x value is the change in x of the two 
        // points and the y value is the change in y of the two points, then finds the cross product of the 'slope points'
        // Returns:
        //      0 if points are collinear
        //      1 if orientation is clockwise
        //      2 if orientation is counterclockwise

        Point2D point12 = Point2D((op2.x - op1.x), (op2.y - op1.y));  // based on the slope between op1 and op2
        Point2D point23 = Point2D((op3.x - op2.x), (op3.y - op2.y));  // based on the slope between op2 and op3

        // essentially a cross product of the slope between op1 and op2, and the slope between op2 and op3
        float crossProd = Point2D::Cross(point12, point23);

        // if the crossProd of slopes is 0, the points are collinear -> their projections overlap
        if (crossProd == 0) { return 0; }
        
        // if the crossProd < 0, the points have a clockwise orientation
        if (crossProd < 0) { return 1; }

        // if the crossProd > 0, the points have a counterclockwise orientation
        if (crossProd > 0) { return 2; }

        return 0;  // Just as a backup so the compiler is happy
    }
};

static std::ostream &operator<<(std::ostream &os, const Line &l) {
    // Overload << operator -> print line information to os stream
    os << l.p1 << ", " << l.p2;
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

    Rect(float left, float top, float width, float height)
        : topLeft(Point2D(left, top)), width(width), height(height) {}

    Rect(Point2D tl = {0, 0}, int w = 0, int h = 0) : topLeft(tl), width(w), height(h) {}

    // Creates bounding box around p1 and p2 with positive width/height
    // Note to myself: remember 0,0 is the top left corner
    Rect(Point2D p1, Point2D p2)
        : topLeft(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
          width(fabs(p1.x - p2.x)),
          height(fabs(p1.y - p2.y)) {}

    Rect(Point2D center, float radius)
        : topLeft(center.x - radius, center.y - radius), width(2 * radius), height(2 * radius) {}

    Rect &operator|=(const Rect &other) {
        // Overload or = operator for two rectangles
        return *this;
    }
    Rect &operator|=(const Point2D &other) {
        // TODO: write this code
        return *this;
    }
    Rect &operator|=(const Line &other) {
        // TODO: write this code
        return *this;
    }
    Rect &operator&=(const Rect &other) {
        // TODO: write this code
        return *this;
    }

    Rect &operator+=(const Point2D &other) {
        // Overload += operator -> shift entire rectangle by the point
        topLeft.x += other.x;
        topLeft.y += other.y;
        return *this;
    }

    Rect operator+(const Point2D &other) const {
        // Overload + operator -> shift entire rectangle by the point and return this as a new rectange
        //return *this;
        return Rect(Point2D(topLeft.x + other.x, topLeft.y + other.y), width, height);
    }

    void Inset(float inset) {
        // Sets the rectangle to a new rectangle inset by the given amount -> assuming each side is inset
        width -= inset*2;  // multiplied by 2 to remove inset amount from both left and right
        height -= inset*2;
        topLeft.x += inset;
        topLeft.y += inset;
    }

    bool IsInside(const Point2D &p) const {
        // Checks if the given point is within the rectangle
        if (p.x < topLeft.x || p.x > topLeft.x + width || p.y < topLeft.y || p.y > topLeft.y + height) {
            return false;   // x is out of bounds
        }
        // Both x and y are in bounds
        return true;
    }
};

static std::ostream &operator<<(std::ostream &os, const Rect &l) {
    // Overload << operator -> returns the topLeft, width, and height values
    os << "Top Left: " << l.topLeft << ", width: " << l.width << ", height: " << l.height;
    return os;
}

}  // namespace CMPUT350

#endif  // MATHUTIL_H
