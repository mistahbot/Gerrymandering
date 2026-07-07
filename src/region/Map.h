#pragma once
#include <vector>
#include <utility>

// constants
constexpr int GRID_SIZE = 1'000'000;
constexpr int NUMBER_OF_VOTING_STATIONS = 1'000;

class Map
{
private:
    std::vector <std::pair <long long, long long>> convex_hull, voting_stations;

public:
    Map(int nr_points = NUMBER_OF_VOTING_STATIONS);
};