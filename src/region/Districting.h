#pragma once
#include "region/Map.h"
#include <vector>

const std::vector<double> DEFAULT_PARTY_WEIGHTS = {1, 1};

class Districting
{
private:
    Map map;
    int nr_districts;
    // if the value is -1, then it means that the voting station doesn't have a district assigned
    // otherwise it's the index of the district;
    std::vector <int> which_district;

    // data needed for improving the gerrymandering

    std::vector<std::vector<long long>> party_votes;
    std::vector<std::vector<long long>> station_votes;
    std::vector<long long> district_population;
    std::vector<long long> district_voting_stations;
    std::vector<long long> good_edges;
    std::vector<long long> total_edges;
    std::vector<double> weights_for_party_gerrymander;

    int nr_parties;
    void initial_districts();
    void setting_up_initial_data();
    void do_flip(int flip_voting_station, int old_district, int new_districting);
public:
    Districting(Map map_, int nr_districts_, std::vector<double> weights_for_party_gerrymander_ = DEFAULT_PARTY_WEIGHTS);
    void generate_svg(const char * name_of_file) const;
    void improve_gerrymandering(int nr_of_tries);
    double score() const;
    void write_in_terminal_expected_seats() const;
    void write_in_terminal_seats() const;
};