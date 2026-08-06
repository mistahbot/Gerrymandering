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

class Double_Point
{
public:
    double x, y;
    Double_Point(const std::pair <double, double> & p) : x(p.first), y(p.second) {};
    Double_Point(double x_, double y_) : x(x_), y(y_) {};
    Double_Point(const Point & p) : x(p.x), y(p.y) {};
};

int Orientation(Point a, Point b, Point c);
double Double_Orientation(Double_Point, Double_Point b, Double_Point c);
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

std::vector<Triangle> DelaunayTriangulation(std::vector<Point> p);

Double_Point Circumcenter(const Triangle & t);

void CutPolygon(std::vector<Double_Point> & polygon, Double_Point a, Double_Point b, Double_Point p);