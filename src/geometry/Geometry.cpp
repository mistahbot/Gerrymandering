#include "Geometry.h"
#include <set>
#include <algorithm>
#include <stdexcept>
#include <map>

// 0 means that the points are collinear
// -1 means that the orientation is clockwise/right
// 1 means that the orientations is counter-clockwise/left
int Orientation(Point a, Point b, Point c)
{
    long long data = (b.x - a.x) * (c.y - b.y) - (c.x - b.x) * (b.y - a.y);
    if (data > 0)
        return 1;
    else if (data < 0)
        return -1;
    else
        return 0;
}

// =0 means that the points are collinear
// <0 means that the orientation is clockwise/right
// >0 means that the orientations is counter-clockwise/left
double Double_Orientation(Double_Point a, Double_Point b, Double_Point c)
{
    return (b.x - a.x) * (c.y - b.y) - (c.x - b.x) * (b.y - a.y);
}

// This ConvexHull includes collinear points in the polygon structure
bool ConvexHull(std::vector<Point> points, std::vector<Point> &convex_hull_points, std::vector<Point> &interior_points, bool non_line_non_point, int needed_interior_points)
{
    // emptying the two reference vectors
    convex_hull_points.clear();
    interior_points.clear();

    // prepping for convex hull, by sorting the points
    std::sort(points.begin(), points.end(), [](Point a, Point b)
              { return (a.x < b.x) || (a.x == b.x && a.y < b.y); });

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

std::vector<Triangle> DelaunayTriangulation(std::vector<Point> p)
{
    if (p.size() < 3)
        throw std::runtime_error("need at least three points to calculate BowyerWatson");

    std::vector<Triangle> triangulation;
    std::vector<Point> convex_hull;
    std::vector<Point> interior_points;

    // Calculating the convex hull
    if (ConvexHull(p, convex_hull, interior_points, 1, 0) == 0)
        throw std::runtime_error("cant build a convex hull out of the points for the delaunay triangulation");

    // Creating a initial triangulation (it will be probably wrong)
    for (int i = 2; i < int(convex_hull.size()); i++)
        triangulation.push_back(Triangle(convex_hull[0], convex_hull[i - 1], convex_hull[i]));

    bool finished_fixing_triangulation = false;
    // Fixing triangulation
    while (!finished_fixing_triangulation)
    {
        finished_fixing_triangulation = true;
        // Adding the edges
        std::map<Edge, int> edges;
        for (Triangle &t : triangulation)
        {
            edges[{t.Get_Point(1), t.Get_Point(2)}]++;
            edges[{t.Get_Point(2), t.Get_Point(3)}]++;
            edges[{t.Get_Point(3), t.Get_Point(1)}]++;
        }

        // Searching at possible delaunay errors
        // We use Lawson's flipping
        // we check each quadrilateral if it's valid
        for (auto e : edges)
            if (e.second > 1)
            {
                // we now know the edge in common of 2 triangles
                // searching those two triangles

                std::vector<Point> third_points;
                std::vector<int> triangles_to_eliminate_maybe;
                int index = 0;
                for (Triangle &t : triangulation)
                {
                    if (Edge({t.Get_Point(1), t.Get_Point(2)}) == e.first)
                    {
                        third_points.push_back(t.Get_Point(3));
                        triangles_to_eliminate_maybe.push_back(index);
                    }
                    if (Edge({t.Get_Point(2), t.Get_Point(3)}) == e.first)
                    {
                        third_points.push_back(t.Get_Point(1));
                        triangles_to_eliminate_maybe.push_back(index);
                    }
                    if (Edge({t.Get_Point(3), t.Get_Point(1)}) == e.first)
                    {
                        third_points.push_back(t.Get_Point(2));
                        triangles_to_eliminate_maybe.push_back(index);
                    }
                    index ++;
                }
                if (third_points.size() != 2)
                    throw std::runtime_error("something went wrong in the lawsons's flipping");

                // now let's check the quadrilateral
                if (is_in_Circumcircle(Triangle(third_points[0], e.first.a, e.first.b), third_points[1]))
                {
                    finished_fixing_triangulation = false;
                    // this means that we need to flip the diagonal of the quadrilateral

                    while (triangles_to_eliminate_maybe.size())
                    {
                        triangulation.erase(triangulation.begin() + triangles_to_eliminate_maybe.back());
                        triangles_to_eliminate_maybe.pop_back();
                    }
                    triangulation.push_back(Triangle(third_points[0], third_points[1], e.first.a));
                    triangulation.push_back(Triangle(third_points[0], third_points[1], e.first.b));
                }
            }
    }

    // Starting the incremental process
    for (const Point &x : interior_points)
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
            index++;
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

    return triangulation;
}

Double_Point Circumcenter(const Triangle & t)
{
    // i'm using the formulas i deduced in Circumcircle.md

    Point n(t.Get_Point(1)), m(t.Get_Point(2)), p(t.Get_Point(3));

    double coef_1 = n.x - m.x;
    double coef_2 = n.y - m.y;
    double coef_3 = n.x - p.x;
    double coef_4 = n.y - p.y;
    double const_1 = (n.x + m.x) / 2.0 * (n.x - m.x) + (n.y + m.y) / 2.0 * (n.y - m.y);
    double const_2 = (n.x + p.x) / 2.0 * (n.x - p.x) + (n.y + p.y) / 2.0 * (n.y - p.y);

    Double_Point c{0, 0};
    double det = coef_1 * coef_4 - coef_2 * coef_3;
    if (det == 0)
        throw std::runtime_error("got determinant is 0 while calculating a circumcenter");
    c.x = (const_1 * coef_4 - const_2 * coef_2) / det;
    c.y = (coef_1 * const_2 - const_1 * coef_3) / det;
    return c;
}

// we are cutting the polygon with the line formed by ab, and choosing the convex in the same halfplane (obtained by cutting the plane with the line ab) as p
void CutPolygon(std::vector<Double_Point> & polygon, Double_Point a, Double_Point b, Double_Point p)
{
    double op = Double_Orientation(a, b, p);
    std::vector<Double_Point> new_polygon;
    for (int i = 1; i <= int(polygon.size()); i ++)
    {
        // good_half_plane tells us for each of the two points if they are in the same half plane as p
        bool good_half_plane[2] = {0, 0};
        double o0, o1;
        o0 = Double_Orientation(a, b, polygon[i - 1]);
        o1 = Double_Orientation(a, b, polygon[(i == int(polygon.size()) ? 0 : i)]);

        if (op == 0)
            throw std::runtime_error("whilst cutting the polygon, there is ambiguity on which half plane needs to be choosen");

        if (o0 == 0 || (o0 > 0 && op > 0) || (o0 < 0 && op < 0))
            good_half_plane[0] = 1;
        if (o1 == 0 || (o1 > 0 && op > 0) || (o1 < 0 && op < 0))
            good_half_plane[1] = 1;

        // now we can update the new polygon
        // we need to add the latter point if its in the right half plane
        // we dont need to do it for the former point because for another edge, that point will be the latter
        // we also need to see if the line cuts the edge

        if (good_half_plane[0] != good_half_plane[1])
        {
            // we need to find the intersection between the lines (polygon[i], polygon[i - 1]) and (a, b)
            double t = o0 / (o0 - o1);
            new_polygon.push_back(Double_Point(polygon[i - 1].x + t * (polygon[(i == int(polygon.size()) ? 0 : i)].x - polygon[i - 1].x), polygon[i - 1].y + t * (polygon[(i == int(polygon.size()) ? 0 : i)].y - polygon[i - 1].y)));
        }

        if (good_half_plane[1])
            new_polygon.push_back(polygon[(i == int(polygon.size()) ? 0 : i)]);
    }

    polygon = new_polygon;
}