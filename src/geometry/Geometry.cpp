#include "Geometry.h"
#include <set>
#include <algorithm>

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
    for (int i = 0; i < int(points.size()); i ++)
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