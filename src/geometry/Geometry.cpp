#include "Geometry.h"
#include <set>
#include <algorithm>
#include <stdexcept>
#include <map>

// 0 means that the points are collinear
// -1 means that the orientation is clockwise/right
// 1 means that the orientations is counter-clockwise/left
int Orientation(std::pair<long long, long long> a, std::pair<long long, long long> b, std::pair<long long, long long> c)
{
    long long data = (b.first - a.first) * (c.second - b.second) - (c.first - b.first) * (b.second - a.second);
    if (data > 0)
        return 1;
    else if (data < 0)
        return -1;
    else
        return 0;
}

// This ConvexHull includes collinear points in the polygon structure
bool ConvexHull(std::vector<std::pair<long long, long long>> points, std::vector<std::pair<long long, long long>> &convex_hull_points, std::vector<std::pair<long long, long long>> &interior_points, bool non_line_non_point, int needed_interior_points)
{
    // emptying the two reference vectors
    convex_hull_points.clear();
    interior_points.clear();

    // prepping for convex hull, by sorting the points
    std::sort(points.begin(), points.end(), [](std::pair<long long, long long> a, std::pair<long long, long long> b)
              { return (a.first < b.first) || (a.first == b.first && a.second < b.second); });

    // here, only the indexes of the points from the vector points will be retained
    std::vector<int> upper_hull, lower_hull;

    // calculating the upper_hull
    for (int i = 0; i < int(points.size()); i++)
    {
        while (upper_hull.size() >= 2 && Orientation(points[upper_hull[upper_hull.size() - 2]], points[upper_hull[upper_hull.size() - 1]], points[i]) > 0)
            upper_hull.pop_back();
        upper_hull.push_back(i);
    }

    // calculating the lower_hull
    for (int i = int(points.size()) - 1; i >= 0; i--)
    {
        while (lower_hull.size() >= 2 && Orientation(points[lower_hull[lower_hull.size() - 2]], points[lower_hull[lower_hull.size() - 1]], points[i]) > 0)
            lower_hull.pop_back();
        lower_hull.push_back(i);
    }

    // i'm not sure if this is optimal, but it doesnt take much compute to just use a set to merge the two hulls
    // the thing is that im calculating the convex hull to contain the collinear points and this can make a point appear on both hulls, i think
    std::set<int> polygon_points;
    while (upper_hull.size())
    {
        if (polygon_points.count(upper_hull.back()) == 0)
            convex_hull_points.push_back(points[upper_hull.back()]);
        polygon_points.insert(upper_hull.back());
        upper_hull.pop_back();
    }
    while (lower_hull.size())
    {
        if (polygon_points.count(lower_hull.back()) == 0)
            convex_hull_points.push_back(points[lower_hull.back()]);
        polygon_points.insert(lower_hull.back());
        lower_hull.pop_back();
    }
    for (int i = 0; i < int(points.size()); i++)
        if (polygon_points.count(i) == 0)
            interior_points.push_back(points[i]);

    // now we need to make sure that we have a valid convex hull

    // we are checking if he have the minimum needed interior points
    if (needed_interior_points > int(interior_points.size()))
        return false;
    // we are checking the non_line_non_point condition
    if (non_line_non_point)
    {
        if (convex_hull_points.size() < 3)
            return false;
        bool ok = false;
        for (int i = 1; i < int(convex_hull_points.size()) - 1; i++)
            if (Orientation(convex_hull_points[0], convex_hull_points[i], convex_hull_points[convex_hull_points.size() - 1]) != 0)
                ok = true;
        if (!ok)
            return false;
    }
    return true;
}

bool Point::operator==(const Point &other) const
{
    return (x == other.x && y == other.y);
}

bool Point::operator<(const Point &other) const
{
    return (x < other.x || (x == other.x && y < other.y));
}

bool Edge::operator==(const Edge &other) const
{
    // (a,b)=(b,a) in this case
    return ((a == other.a && b == other.b) || (a == other.b && b == other.a));
}

bool Edge::operator<(const Edge &other) const
{
    // (a,b)=(b,a) in this case
    return ((std::min(a, b) < std::min(other.a, other.b)) || (std::min(a, b) == std::min(other.a, other.b) && std::max(a, b) < std::max(other.a, other.b)));
}

Triangle::Triangle(const Point &a_, const Point &b_, const Point &c_) : a(a_), b(b_), c(c_)
{
    // points should be in clockwise order
    int o = Orientation({a.x, a.y}, {b.x, b.y}, {c.x, c.y});
    if (o == 0)
        throw std::runtime_error("degenerate triangle, the points are collinear");
    if (o == 1)
        std::swap(b, c);
}

// index must be 1, 2, or 3
Point Triangle::Get_Point(int index) const
{
    if (index < 1 || index > 3)
        throw std::runtime_error("index of triangle point should be between 1 and 3 (inclusive)");
    if (index == 1)
        return a;
    if (index == 2)
        return b;
    return c;
}

__int128_t Matrix_Determinant(std::vector<std::vector<long long>> a)
{
    // checking validity of matrix
    int n = a.size();
    for (auto x : a)
        if (n != int(x.size()))
            throw std::runtime_error("the matrix a must have the same number of rows and columns");

    // base case
    if (n == 1)
        return a[0][0];
    else if (n == 2)
    {
        return __int128_t(a[0][0]) * a[1][1] - __int128_t(a[0][1]) * a[1][0];
    }

    // classic determinant calculation, im selecting the first row here
    __int128_t det = 0;
    for (int j = 0; j < n; j++)
    {
        // creating a new matrix without the first row and the (j+1)-th column (the j column)
        std::vector<std::vector<long long>> new_a;
        new_a.resize(n - 1);
        for (int i = 1; i < n; i++)
        {
            new_a[i - 1].clear();
            for (int k = 0; k < n; k++)
                if (k != j)
                    new_a[i - 1].push_back(a[i][k]);
        }

        det += __int128_t(a[0][j]) * (j % 2 == 0 ? 1 : -1) * Matrix_Determinant(new_a);
    }
    return det;
}

bool is_in_Circumcircle(const Triangle &t, const Point &p)
{
    // creating the matrix to determine if p is in the circumcircle of t
    std::vector<std::vector<long long>> a;
    a.resize(4);
    Point points[4]{t.Get_Point(1), t.Get_Point(2), t.Get_Point(3), p};
    for (int i = 0; i < 4; i++)
        a[i] = {points[i].x * points[i].x + points[i].y * points[i].y, points[i].x, points[i].y, 1};

    // Important to note, that we are looking for negative because the points, in the triangle, are in clockwise order
    if (Matrix_Determinant(a) < 0)
        return true;
    return false;
}

std::vector<Triangle> BowyerWatson(const std::vector<Point> &p)
{
    if (p.size() < 3)
        throw std::runtime_error("need at least three points to calculate BowyerWatson");

    // Trying to create the super triangle
    // I will look what is the smallest square that contains all the points
    // And the super triangle will contain that square
    long long lowest_x, highest_x, lowest_y, highest_y;
    lowest_x = highest_x = p[0].x;
    lowest_y = highest_y = p[0].y;
    for (int i = 1; i < int(p.size()); i++)
    {
        lowest_x = std::min(lowest_x, p[i].x);
        highest_x = std::max(highest_x, p[i].x);
        lowest_y = std::min(lowest_y, p[i].y);
        highest_y = std::max(highest_y, p[i].y);
    }

    // Creating the points of the super triangle

    long long dx = highest_x - lowest_x, dy = highest_y - lowest_y;
    Point a{lowest_x - 1, lowest_y - 1};
    Point b{lowest_x - 1, highest_y + dx + 2};
    Point c{highest_x + dy + 2, lowest_y - 1};

    // Adding the super triangle

    std::vector<Triangle> triangulation;
    triangulation.push_back(Triangle(a, b, c));

    // Starting the incremental process
    for (const Point &x : p)
    {
        // this will helps us know what is the polygon formed by the bad triangles
        std::map<Edge, int> bad_edges;
        // we will keep the indices of the bad triangles so that we can eliminate them
        std::vector<int> triangles_to_eliminate;
        int index = 0;

        // Looking for bad triangles
        for (Triangle &t : triangulation)
        {
            if (is_in_Circumcircle(t, x))
            {
                // Taking note of all the bad edges
                bad_edges[{t.Get_Point(1), t.Get_Point(2)}]++;
                bad_edges[{t.Get_Point(2), t.Get_Point(3)}]++;
                bad_edges[{t.Get_Point(3), t.Get_Point(1)}]++;

                // Marking this for elimination
                triangles_to_eliminate.push_back(index);
            }
            index ++;
        }
        // eliminating bad triangles
        while (triangles_to_eliminate.size())
        {
            triangulation.erase(triangulation.begin() + triangles_to_eliminate.back());
            triangles_to_eliminate.pop_back();
        }

        for (auto e : bad_edges)
            if (e.second == 1)
            {
                // creating the new triangle formed by this edge and the new point
                triangulation.push_back(Triangle(e.first.a, e.first.b, x));
            }
    }

    // elimating the super vertices

    // we will keep the indices of the bad triangles so that we can eliminate them
    std::vector<int> triangles_to_eliminate;
    int index = 0;
    for (Triangle &t : triangulation)
    {
        if (a == t.Get_Point(1) || a == t.Get_Point(2) || a == t.Get_Point(3))
            triangles_to_eliminate.push_back(index);
        else if (b == t.Get_Point(1) || b == t.Get_Point(2) || b == t.Get_Point(3))
            triangles_to_eliminate.push_back(index);
        else if (c == t.Get_Point(1) || c == t.Get_Point(2) || c == t.Get_Point(3))
            triangles_to_eliminate.push_back(index);

        index++;
    }
    // eliminating the triangles with super vertices
    while (triangles_to_eliminate.size())
    {
        triangulation.erase(triangulation.begin() + triangles_to_eliminate.back());
        triangles_to_eliminate.pop_back();
    }

    return triangulation;
}