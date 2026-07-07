#include "Map.h"
#include "geometry/Geometry.h"
#include <random>
#include <vector>
#include <utility>
#include <set>
#include <stdexcept>

Map::Map(int nr_points)
{
    if (nr_points < 4)
        throw std::runtime_error("there should be at least four points (three to define the region and one to be a voting station)");

    // we want our random points to be integers in the inclusive range [0, GRID_SIZE]
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> r(0, GRID_SIZE);

    std::vector<std::pair<long long, long long>> points;
    // our random points must be distinct
    // to make sure that we are not running this for loop too much
    // we will set a variable for our duplicate tolerance
    // we will throw if the tolerance is not respecte
    int duplicate_tolerance = nr_points;
    std::set<std::pair<long long, long long>> set_of_points;
    long long x, y;
    for (int i = 0; i < nr_points && duplicate_tolerance >= 0; i++)
    {
        x = r(gen);
        y = r(gen);
        if (set_of_points.count({x, y}) != 0)
        {
            // we are updating our tolerance and we do i--, because we after do continue which will do i++, and it's good cuz we will be at the same index i
            i --;
            duplicate_tolerance --;
            continue;
        }
        points.push_back({x, y});
        set_of_points.insert({x, y});
    }
    // the random points do not respect our duplicate tolerance
    if (duplicate_tolerance < 0)
        throw std::runtime_error("random values have too many duplicates, a possible solution is to increase the GRID_SIZE constant");

    if (!ConvexHull(points, convex_hull, voting_stations, 1, 1))
        throw std::runtime_error("the random values don't create a valid map");
}