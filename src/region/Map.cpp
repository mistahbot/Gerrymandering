#include "Map.h"
#include <random>
#include <vector>
#include <utility>
#include <set>
#include <stdexcept>
#include <map>
#include <cmath>
#include <algorithm>

Voting_Station::Voting_Station(Point p) : center(p)
{
    adj.clear();
}
void Voting_Station::add_neighbour(int index)
{
    for (int i = 0; i < int(adj.size()); i++)
        if (adj[i] == index)
            return;
    adj.push_back(index);
}
void Voting_Station::add_vote_share(double share)
{
    vote_share.push_back(share);
}
const std::vector<int> &Voting_Station::get_neighbours() const
{
    return adj;
}
const std::vector<double> &Voting_Station::get_vote_shares() const
{
    return vote_share;
}
const Point Voting_Station::get_point() const
{
    return center;
}

std::vector<Double_Point> Voting_Station::get_polygon() const
{
    return polygon;
}

void Voting_Station::set_polygon(const std::vector<Double_Point> &p)
{
    polygon = p;
}

Map::Map(int nr_points, long long total_population, std::vector<double> vote_share)
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
            i--;
            duplicate_tolerance--;
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
    std::vector<Triangle> delaunay = DelaunayTriangulation(points);

    // now i need to focus only on delaunay (triangulation) and points (the list of points)
    // a problem until now is that i didnt use indexation, and this makes the code harder to think about it and less efficient
    // so im streamlining the info we have in the voting_station vector

    // when i look at the delaunay triangulation to establish adjacency i need to know where those points are located in voting_station
    std::map<Point, int> position;
    for (auto &p : points)
    {
        position[p] = voting_station.size();
        voting_station.push_back(Voting_Station(p));
    }

    // using a copy
    // because i want to sort them so they are in clockwise order
    std::vector<std::vector<int>> adj_copy(points.size());
    // establishing adjacency
    for (auto &t : delaunay)
    {
        for (int i = 1; i <= 3; i++)
        {
            // i and j are just the indexes of the points of the triangle
            int j = i + 1;
            if (j == 4)
                j -= 3;

            adj_copy[position[t.Get_Point(i)]].push_back(position[t.Get_Point(j)]);
            adj_copy[position[t.Get_Point(j)]].push_back(position[t.Get_Point(i)]);
        }
    }

    // creating the polygon of each station

    // initiating each voronoi cell as the convex hull, which will then be cut by the mediators
    std::vector<Point> interior_points;
    ConvexHull(points, convex_hull, interior_points, 1, 0);

    // Before doing the polygon, i will also sort the voting station's neighbour to be in clockwise order
    // Doing this now because ive just calculated the convex hull and i might need this information
    for (int i = 0; i < int(points.size()); i++)
    {
        sort(adj_copy[i].begin() + 1, adj_copy[i].end(),
            [&points, &adj_copy, &i](int a, int b)
            {
                double oa = Double_Orientation(points[i], points[adj_copy[i][0]], points[a]);
                double ob = Double_Orientation(points[i], points[adj_copy[i][0]], points[b]);

                if (oa <= 0 && ob > 0)
                    return 1;
                if (ob <= 0 && oa > 0)
                    return 0;

                if (Double_Orientation(points[i], points[a], points[b]) < 0)
                    return 1;
                return 0;
            });

        // for on hull points, the two neighbours on the convex hull are not connected
        for (int p = 0; p < int(adj_copy[i].size()) - 1; p ++)
        {
            int voting_station_a = adj_copy[i][p];
            int voting_station_b = adj_copy[i][p + 1];
            bool found_edge = false;
            for (int j : adj_copy[voting_station_a])
                if (j == voting_station_b)
                {
                    found_edge = true; 
                    break;
                }
            
            if (!found_edge && voting_station_a != voting_station_b)
            {
                std::rotate(adj_copy[i].begin(), adj_copy[i].begin() + p + 1, adj_copy[i].end());
                break;
            }
        }

        for (int p = 0; p < int(adj_copy[i].size()); p ++)
            voting_station[i].add_neighbour(adj_copy[i][p]);
    }

    std::vector<Double_Point> double_convex_hull;
    for (auto &p : convex_hull)
        double_convex_hull.push_back(p);
    for (int i = 0; i < int(voting_station.size()); i++)
        voting_station[i].set_polygon(double_convex_hull);

    // cutting the polygon
    std::vector<Double_Point> aux_polygon;
    Double_Point midpoint{0, 0}, aux_point{0, 0};
    for (auto &t : delaunay)
    {
        for (int i = 1; i <= 3; i++)
        {
            // i and j are just the indexes of the points of the triangle
            int j = i + 1;
            if (j == 4)
                j -= 3;

            // finding the midpoint
            midpoint.x = (t.Get_Point(i).x + t.Get_Point(j).x) / 2.0;
            midpoint.y = (t.Get_Point(i).y + t.Get_Point(j).y) / 2.0;

            // finding the point which forms the line equivalent to the mediator (together with the midpoint)
            // we do this by finding the perpendicular of the midpoint->t.Get_Point(i), it could have also been j instead of i
            aux_point.x = midpoint.x - (t.Get_Point(i).y - midpoint.y);
            aux_point.y = midpoint.y + (t.Get_Point(i).x - midpoint.x);

            // cutting the polygon of the i-th point in the triangle
            aux_polygon = voting_station[position[t.Get_Point(i)]].get_polygon();
            CutPolygon(aux_polygon, midpoint, aux_point, t.Get_Point(i));
            voting_station[position[t.Get_Point(i)]].set_polygon(aux_polygon);

            // cutting the polygon of the j-th point in the triangle
            aux_polygon = voting_station[position[t.Get_Point(j)]].get_polygon();
            CutPolygon(aux_polygon, midpoint, aux_point, t.Get_Point(j));
            voting_station[position[t.Get_Point(j)]].set_polygon(aux_polygon);
        }
    }

    // setting up the population
    // the heuristic is
    // units_i = min(100,max(0.01, mean_area/area_i))^0.5
    // then population_i = units_i/sum of units
    double mean_area = Convex_Polygon_Area(double_convex_hull) / nr_points;
    double total_units = 0;
    for (int i = 0; i < int(voting_station.size()); i++)
        total_units += std::sqrt(std::min(100.0, std::max(0.01, mean_area / Convex_Polygon_Area(voting_station[i].get_polygon()))));
    long long sum_of_population = 0;
    for (int i = 0; i < int(voting_station.size()); i++)
    {
        long long current_population = total_population * (std::sqrt(std::min(100.0, std::max(0.01, mean_area / Convex_Polygon_Area(voting_station[i].get_polygon())))) / total_units);
        // in the last case i just do the difference because otherwise i would have rounding errors
        if (i == int(voting_station.size()) - 1)
            voting_station[i].set_population(total_population - sum_of_population);
        else
            voting_station[i].set_population(current_population);
        sum_of_population += current_population;
    }
    population_of_map = total_population;

    // creating the noise which determines political leaning
    std::vector<double> noise(voting_station.size());
    // setting the initial noise be 1 - the percentage of the population
    // the 1 - maintains the heuristic high population (which is correlated with less area) means more left political leaning
    for (int i = 0; i < int(voting_station.size()); i++)
        noise[i] = 1 - double(voting_station[i].get_population()) / total_population;
    // doing smoothness operation, ill do it 5 times (this is just heuristic)
    for (int smoothness = 1; smoothness <= SMOOTHNESS_RUNS; smoothness++)
    {
        std::vector<double> new_noise(voting_station.size());
        // the heuristic is the noise for a voting station becomes half of the current noise and half of the mean of the neighbours noise
        for (int i = 0; i < int(voting_station.size()); i++)
        {
            new_noise[i] = 0;
            const auto &n = voting_station[i].get_neighbours();
            for (int j : n)
                new_noise[i] += noise[j];
            new_noise[i] /= n.size();
            new_noise[i] /= 2;
            new_noise[i] += noise[i] / 2;
        }
        noise = new_noise;
    }

    // we need to determine what is the order
    std::vector<int> order_of_noise;
    for (int i = 0; i < int(voting_station.size()); i++)
        order_of_noise.push_back(i);
    std::sort(order_of_noise.begin(), order_of_noise.end(), [&noise](int a, int b)
              { return noise[a] < noise[b]; });

    if (vote_share.size() <= 1)
        throw std::runtime_error("there must be at least two parties");
    // making sure the vote share sums up to 1
    // cuz im not sure if it respects the condition
    // but i can always make it respect the condition
    // if the total sum is not zero, and each element is not negative
    // by just making each element be the percentage it is of the total sum
    double sum_of_vote_share_elements = 0;
    for (int i = 0; i < int(vote_share.size()); i++)
    {
        if (vote_share[i] < 0)
            throw std::runtime_error("can't have negative vote_share");
        sum_of_vote_share_elements += vote_share[i];
    }
    if (sum_of_vote_share_elements == 0)
        throw std::runtime_error("can't have all parties with zero share of the vote");
    for (int i = 0; i < int(vote_share.size()); i++)
        vote_share[i] = vote_share[i] / sum_of_vote_share_elements;

    // the whole idea is to just put the parties on a normal distribution
    // they own a segment of the normal distribution
    // and then for each voting station we found its positions, and the voters are spread using another normal distribution
    // which has standard deviation sigma, rather than 1
    // so thats why there are some allignments calculations which are made

    std::vector<double> party_delimitations;
    double sum_of_vote_share = vote_share[0];
    // this is just a heuristic
    double sigma_vote_distribution = DEFAULT_SIGMA;
    for (int i = 1; i < int(vote_share.size()); i++)
    {
        party_delimitations.push_back(std::sqrt(1 + sigma_vote_distribution * sigma_vote_distribution) * Inverse_NormalDistribution_CDF(sum_of_vote_share));
        sum_of_vote_share += vote_share[i];
    }

    // finding the vote share for each cell
    sum_of_population = 0;
    for (int i = 0; i < int(voting_station.size()); i++)
    {
        double position_of_voting_station = Inverse_NormalDistribution_CDF((sum_of_population + voting_station[order_of_noise[i]].get_population() / 2.0) / total_population);
        double previous_value = 0;

        for (int j = 0; j < int(party_delimitations.size()); j++)
        {
            double current_value = NormalDistribution_CDF((party_delimitations[j] - position_of_voting_station) / sigma_vote_distribution);
            voting_station[order_of_noise[i]].add_vote_share(current_value - previous_value);
            previous_value = current_value;
        }
        voting_station[order_of_noise[i]].add_vote_share(1.0 - previous_value);

        sum_of_population += voting_station[order_of_noise[i]].get_population();
    }
}

const std::vector<Voting_Station> &Map::get_stations() const
{
    return voting_station;
}

void Voting_Station::set_population(long long p)
{
    population = p;
}

long long Voting_Station::get_population() const
{
    return population;
}

long long Map::get_population() const
{
    return population_of_map;
}

int Map::get_nr_voting_stations() const
{
    return voting_station.size();
}
