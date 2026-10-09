#include "Bezier.h"

namespace CMPUT350 {

// Constructor
Bezier::Bezier(const std::vector<Point2D>& pts) : mPoints(pts) {
}

// Instance method
Point2D Bezier::GetPoint(float t) const {
    return GetPoint(mPoints, t);
}

// Instance method
Point2D Bezier::GetSlope(float t) const {
    return GetSlope(mPoints, t);
}

// Static method: calculate the point
Point2D Bezier::GetPoint(const std::vector<Point2D>& pts, float t) {
    // Check the number of points is at least 4 and is a multiple of 3 plus 1
    if (pts.size() < 4 || (pts.size() - 1) % 3 != 0) {
        return Point2D(0, 0);
    }
    // Number of segments
    int numSegments = static_cast<int>((pts.size() - 1) / 3);
    // Clamp t to the valid range [0, numSegments]
    if (t < 0) 
        t = 0;
    if (t > numSegments)    
        t = static_cast<float>(numSegments);
    
    int segment = static_cast<int>(t);
    float localT = t - segment; // local t in [0,1] for the current segment

    // Check if segment is the last one, adjust localT to 1.0
    if (segment == numSegments) {
        segment--;
        localT = 1.0f;
    }

    // Get the four points of this segment
    Point2D p0 = pts[segment * 3];
    Point2D p1 = pts[segment * 3 + 1];
    Point2D p2 = pts[segment * 3 + 2];
    Point2D p3 = pts[segment * 3 + 3];

    float u = 1.0f - localT;

    float b0 = u * u * u;
    float b1 = 3.0f * u * u * localT;
    float b2 = 3.0f * u * localT * localT;
    float b3 = localT * localT * localT;

    return p0 * b0 + p1 * b1 + p2 * b2 + p3 * b3;
}

// Static method: calculate the slope
Point2D Bezier::GetSlope(const std::vector<Point2D>& pts, float t) {
    // Check the number of points is at least 4 and is a multiple of 3 plus 1
    if (pts.size() < 4 || (pts.size() - 1) % 3 != 0) {
        return Point2D(0, 0);
    }
    // Number of segments
    int numSegments = static_cast<int>((pts.size() - 1) / 3);
    // Clamp t to the valid range [0, numSegments]
    if (t < 0) 
        t = 0;
    if (t > numSegments)    
        t = static_cast<float>(numSegments);
    
    int segment = static_cast<int>(t);
    float localT = t - segment; // local t in [0,1] for the current segment

    // Check if segment is the last one, adjust localT to 1.0
    if (segment == numSegments) {
        segment--;
        localT = 1.0f;
    }

    // Get the four points of this segment
    Point2D p0 = pts[segment * 3];
    Point2D p1 = pts[segment * 3 + 1];
    Point2D p2 = pts[segment * 3 + 2];
    Point2D p3 = pts[segment * 3 + 3];

    float u = 1.0f - localT;

    float b0 = 3.0f * u * u;
    float b1 = 6.0f * u * localT;
    float b2 = 3.0f * localT * localT;

    return (p1 - p0) * b0 + (p2 - p1) * b1 + (p3 - p2) * b2;
}

} // namespace CMPUT350