#pragma once
#include <vector>
#include "geometry/Geometry.h"
constexpr int SMOOTHNESS_RUNS = 5;
constexpr double DEFAULT_SIGMA = 0.8; 

// constants
constexpr int GRID_SIZE = 1'000'000;
constexpr int NUMBER_OF_VOTING_STATIONS = 1'000;
constexpr long long DEFAULT_TOTAL_POPULATION = 1'000'000;
const std::vector<double> DEFAULT_VOTE_SHARE = {1, 1};

class Voting_Station
{
private:
    // the site to which all the points are closest to
    Point center;
    // the neighbours of this voting station, they are in clockwise order
    std::vector<int> adj;
    // the polygon of this voting station, the polygon is in clockwise order
    std::vector<Double_Point> polygon;
    // the population
    long long population;
    // how they vote
    std::vector<double> vote_share;
public:
    Voting_Station(Point p_);
    void add_neighbour(int index);
    const std::vector<int> & get_neighbours() const;
    void add_vote_share(double share);
    const std::vector<double> & get_vote_shares() const;
    const Point get_point() const;
    std::vector<Double_Point> get_polygon() const;
    void set_polygon(const std::vector<Double_Point> & p);
    void set_population(long long p);
    long long get_population() const;
};

class Map
{
private:
    std::vector<Point> convex_hull;
    std::vector <Voting_Station> voting_station;
    long long population_of_map;

public:
    Map(int nr_points = NUMBER_OF_VOTING_STATIONS, long long total_population = DEFAULT_TOTAL_POPULATION, std::vector<double> vote_share = DEFAULT_VOTE_SHARE);
    const std::vector <Voting_Station> & get_stations() const;
    long long get_population() const;
    int get_nr_voting_stations() const;
};