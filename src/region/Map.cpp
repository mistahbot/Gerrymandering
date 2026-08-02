#include "Map.h"
#include <random>
#include <vector>
#include <utility>
#include <set>
#include <stdexcept>
#include <map>

Voting_Station::Voting_Station(Point p) : center(p) 
{
    adj.clear();
}
void Voting_Station::add_neighbour(int index)
{
    for (int i = 0; i < int(adj.size()); i ++)
        if (adj[i] == index)
            return;
    adj.push_back(index);
}
const std::vector<int> & Voting_Station::get_neighbours()
{
    return adj;
}
const Point Voting_Station::get_point()
{
    return center;
}

Map::Map(int nr_points)
{
    if (nr_points < 4)
        throw std::runtime_error("there should be at least four points (three to define the region and one to be a voting station)");

    // we want our random points to be integers in the inclusive range [0, GRID_SIZE]
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> r(0, GRID_SIZE);

    std::vector<Point> points;
    // our random points must be distinct
    // to make sure that we are not running this for loop too much
    // we will set a variable for our duplicate tolerance
    // we will throw if the tolerance is not respecte
    int duplicate_tolerance = nr_points;
    std::set<Point> set_of_points;
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

    // we are building the map

    // getting the triangulation
    std::vector<Triangle> delauney = DelauneyTriangulation(points);

    // now i need to focus only on delauney (triangulation) and points (the list of points)
    // a problem until now is that i didnt use indexation, and this makes the code harder to think about it and less efficient
    // so im streamlining the info we have in the voting_station vector

    // when i look at the delauney triangulation to establish adjacency i need to know where those points are located in voting_station
    std::map <Point, int> position;
    for (auto & p : points)
    {
        position[p] = voting_station.size();
        voting_station.push_back(Voting_Station(p));
    }

    // establishing adjacency
    for (auto & t : delauney)
    {
        for (int i = 1; i <= 3; i ++)
        {
            // i and j are just the indexes of the points of the triangle
            int j = i + 1;
            if (j == 4)
                j -= 3;
            
            voting_station[position[t.Get_Point(i)]].add_neighbour(position[t.Get_Point(j)]);
            voting_station[position[t.Get_Point(j)]].add_neighbour(position[t.Get_Point(i)]);
        }
    }
}

const std::vector <Voting_Station> & Map::get_stations()
{
    return voting_station;
}