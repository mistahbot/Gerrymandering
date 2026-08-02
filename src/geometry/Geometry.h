#pragma once
#include <vector>
#include <utility>

class Point
{
public:
    long long x, y;
    Point(const std::pair <long long, long long> & p) : x(p.first), y(p.second) {};
    Point(long long x_, long long y_) : x(x_), y(y_) {};
    bool operator<(const Point & other) const;
    bool operator==(const Point & other) const;
};

int Orientation(Point a, Point b, Point c);
bool ConvexHull(std::vector <Point> points, std::vector <Point> & convex_hull_points, std::vector <Point> & interior_points, bool non_line_non_point, int needed_interior_points);

class Edge
{
public:
    Point a, b;
    Edge(const Point & a_, const Point & b_) : a(a_), b(b_) {};
    bool operator<(const Edge & other) const;
    bool operator==(const Edge & other) const;
};

// The points are in clockwise order
class Triangle
{
private:
    Point a, b, c;
public:
    Triangle (const Point & a_, const Point & b_, const Point & c_);
    Point Get_Point(int index) const;
};

__int128_t Matrix_Determinant(std::vector<std::vector<long long>> a);
bool is_in_Circumcircle(const Triangle & t, const Point & p);

std::vector<Triangle> DelauneyTriangulation(std::vector<Point> p);