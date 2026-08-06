#pragma once
#include <vector>
#include "geometry/Geometry.h"

// constants
constexpr int GRID_SIZE = 1'000'000;
constexpr int NUMBER_OF_VOTING_STATIONS = 1'000;

class Voting_Station
{
private:
    // the site to which all the points are closest to
    Point center;
    // the neighbours of this voting station
    std::vector<int> adj;
    // the polygon of this voting station, the polygon is in clockwise order
    std::vector<Double_Point> polygon;
public:
    Voting_Station(Point p_);
    void add_neighbour(int index);
    const std::vector<int> & get_neighbours() const;
    const Point get_point() const;
    std::vector<Double_Point> get_polygon() const;
    void set_polygon(const std::vector<Double_Point> & p);
};

class Map
{
private:
    std::vector<Point> convex_hull;
    std::vector <Voting_Station> voting_station;

public:
    Map(int nr_points = NUMBER_OF_VOTING_STATIONS);
    const std::vector <Voting_Station> & get_stations();
};