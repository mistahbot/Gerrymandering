#include "region/Districting.h"
#include <stack>
#include <random>
#include <utility>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <set>
#include <iostream>

constexpr double DEFAULT_TAU = 0.05;
constexpr double W_SEATS = 1.0;
constexpr double W_POP = 5.0;
constexpr double W_SHAPE = 5.0;
constexpr double W_MARGIN = 0.05;
constexpr double T_START = 0.1;
constexpr double T_END = 0.002;

void Districting::initial_districts()
{
    // the plan is just to make the first district have size nr_voting_stations - (nr_districts - 1), and every other district have size 1

    // this is for the dfs walk
    std::stack<int> stack_stations;

    // this is the tool which is needed for random numbers
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> r(0, map.get_nr_voting_stations() - 1);

    int node;

    // attributing the a station for each district
    for (int i = 0; i < nr_districts; i++)
    {
        node = r(gen);
        while (which_district[node] != -1)
        {
            node++;
            if (node == map.get_nr_voting_stations())
                node = 0;
        }
        which_district[node] = i;
    }

    for (int i = 0; i < map.get_nr_voting_stations(); i++)
        if (which_district[i] != -1)
        {
            stack_stations.push(i);
            while (stack_stations.size())
            {
                node = stack_stations.top();

                bool has_visitied = false;
                const auto &n = map.get_stations()[node].get_neighbours();
                for (auto j : n)
                    if (which_district[j] == -1)
                    {
                        has_visitied = true;
                        which_district[j] = which_district[i];
                        stack_stations.push(j);
                        break;
                    }

                // if we didn't find any new vertexes from our vertex we need to pop it out of the stack
                if (has_visitied == false)
                    stack_stations.pop();
            }
        }
}

void Districting::setting_up_initial_data()
{
    // setting up the initial data
    party_votes.resize(nr_districts);
    station_votes.resize(which_district.size());
    district_population.resize(nr_districts);
    std::fill(district_population.begin(), district_population.end(), 0);
    district_voting_stations.resize(nr_districts);
    std::fill(district_voting_stations.begin(), district_voting_stations.end(), 0);
    good_edges.resize(nr_districts);
    std::fill(good_edges.begin(), good_edges.end(), 0);
    total_edges.resize(nr_districts);
    std::fill(total_edges.begin(), total_edges.end(), 0);
    for (int i = 0; i < nr_districts; i++)
    {
        party_votes[i].resize(nr_parties);
        std::fill(party_votes[i].begin(), party_votes[i].end(), 0);
    }
    for (int i = 0; i < int(station_votes.size()); i++)
    {
        station_votes[i].resize(nr_parties);
        std::fill(station_votes[i].begin(), station_votes[i].end(), 0);
    }

    const auto &v = map.get_stations();
    for (int i = 0; i < int(v.size()); i++)
    {
        district_voting_stations[which_district[i]] += 1;
        district_population[which_district[i]] += v[i].get_population();
        const auto &p = v[i].get_vote_shares();
        long long surplus_population = v[i].get_population();

        // there can be rounding errors so, i add to the biggest party so that the sum of the votes is equal to the population
        int index_biggest_party = 0;
        for (int j = 0; j < int(p.size()); j++)
        {
            station_votes[i][j] = v[i].get_population() * p[j];
            surplus_population -= station_votes[i][j];

            if (station_votes[i][j] > station_votes[i][index_biggest_party])
                index_biggest_party = j;
        }
        station_votes[i][index_biggest_party] += surplus_population;

        for (int j = 0; j < int(p.size()); j++)
            party_votes[which_district[i]][j] += station_votes[i][j];

        for (auto n : v[i].get_neighbours())
            if (i < n || which_district[i] != which_district[n])
            {
                total_edges[which_district[i]]++;
                if (which_district[i] == which_district[n])
                    good_edges[which_district[i]]++;
            }
    }
}

Districting::Districting(Map map_, int nr_districts_, std::vector<double> weights_for_party_gerrymander_) : map(std::move(map_)), nr_districts(nr_districts_), weights_for_party_gerrymander(weights_for_party_gerrymander_)
{
    if (nr_districts > map.get_nr_voting_stations())
        throw std::runtime_error("can't have more districts than voting stations");
    if (nr_districts <= 0)
        throw std::runtime_error("must have at least one district");

    nr_parties = map.get_stations()[0].get_vote_shares().size();
    
    // making the values in gerrymander weights be in the [0,1] range
    if (int(weights_for_party_gerrymander.size()) != nr_parties)
        throw std::runtime_error("the size of the weights should be equal to the nr of parties");

    double minimum = weights_for_party_gerrymander[0], maximum = weights_for_party_gerrymander[0];
    for (int i = 1; i < int(weights_for_party_gerrymander.size()); i++)
    {
        minimum = std::min(minimum, weights_for_party_gerrymander[i]);
        maximum = std::max(maximum, weights_for_party_gerrymander[i]);
    }

    // then i will just make the weight 1 for each
    if (minimum == maximum)
    {
        for (int i = 0; i < int(weights_for_party_gerrymander.size()); i++)
            weights_for_party_gerrymander[i] = 1;
    }
    else
    {
        for (int i = 0; i < int(weights_for_party_gerrymander.size()); i++)
            weights_for_party_gerrymander[i] = (weights_for_party_gerrymander[i] - minimum) / (maximum - minimum);
    }

    // initiating each voting station to being assigned to none of the districts
    which_district.resize(map.get_nr_voting_stations());
    for (int i = 0; i < int(which_district.size()); i++)
        which_district[i] = -1;

    initial_districts();
    setting_up_initial_data();
}

void Districting::generate_svg(const char *name_of_file) const
{
    // l is the lenght of the name
    int l;
    for (l = 0; name_of_file[l]; l++)
        ;
    // searched on internet, and max on macos systems is 255 characters
    if (l > 255 || l < 5 || (name_of_file[l - 4] != '.' || name_of_file[l - 3] != 's' || name_of_file[l - 2] != 'v' || name_of_file[l - 1] != 'g'))
        throw std::runtime_error("not valid name of file for svg");
    for (int i = 0; i < l - 4; i++)
        if (!(('0' <= name_of_file[i] && name_of_file[i] <= '9') || ('a' <= name_of_file[i] && name_of_file[i] <= 'z') || ('A' <= name_of_file[i] && name_of_file[i] <= 'Z') || name_of_file[i] == '_'))
            throw std::runtime_error("the name of the svg must contain only letters, numbers and underscore");

    // i need unique colors for each of the district, ill just choose them randomly

    std::random_device rd;
    std::mt19937 gen(rd());
    // there are (1 << 24) colors using the (0,255) paradigm for each rgb
    std::uniform_int_distribution<int> r(0, (1 << 24) - 1);
    std::vector<int> color_of_district;
    for (int i = 0; i < nr_districts; i++)
        color_of_district.push_back(r(gen));

    // writing the svg
    std::ofstream out(name_of_file);

    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 1000000 1000000\" width=\"800\" height=\"800\">\n";

    const auto &v = map.get_stations();

    for (int i = 0; i < int(v.size()); i++)
    {
        auto n = v[i].get_polygon();

        out << "<polygon points=\"";
        bool start = true;
        for (auto p : n)
        {
            if (!start)
                out << ' ';
            start = false;
            out << p.x << ',' << GRID_SIZE - p.y;
        }
        out << "\" ";
        out << "fill=\"";

        out << "#";
        out << std::hex << std::setw(6) << std::setfill('0') << color_of_district[which_district[i]] << std::dec;
        out << "\"";

        out << " stroke=\"#fcfcfb\" stroke-width=\"1200\"";
        out << "/>\n";

        out << "<circle " << "cx=\"" << v[i].get_point().x << "\" " << "cy=\"" << GRID_SIZE - v[i].get_point().y << "\" r=\"3000\" fill=\"#f8f6f6\"/>\n";
    }

    out << "</svg>";

    out.close();
}

void Districting::do_flip(int flip_voting_station, int old_district, int new_district)
{
    district_voting_stations[old_district]--;
    district_voting_stations[new_district]++;
    district_population[old_district] -= map.get_stations()[flip_voting_station].get_population();
    district_population[new_district] += map.get_stations()[flip_voting_station].get_population();
    for (int j = 0; j < nr_parties; j++)
    {
        party_votes[old_district][j] -= station_votes[flip_voting_station][j];
        party_votes[new_district][j] += station_votes[flip_voting_station][j];
    }

    which_district[flip_voting_station] = new_district;
    for (auto j : map.get_stations()[flip_voting_station].get_neighbours())
    {
        if (which_district[j] != old_district)
            total_edges[old_district]--;
        if (which_district[j] != new_district)
            total_edges[new_district]++;

        if (which_district[j] == old_district)
            good_edges[old_district]--;
        if (which_district[j] == new_district)
            good_edges[new_district]++;
    }
}

void Districting::improve_gerrymandering(int nr_of_tries)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> r(0, int(which_district.size()) - 1);
    // this is for choosing if we update the districting even if we get a worse score
    std::uniform_real_distribution<double> r_01(0.0, 1.0);
    // this is for contiguity dfs
    std::vector<bool> visited(which_district.size());

    /// doing the improvement runs
    for (int i = 1; i <= nr_of_tries; i++)
    {
        // in each run we need to
        // select a voting station
        // check if deleting this node makes the district non contigious
        // look at the neighboring districts (different from the one the voting station is) and choose one
        // flip the district, if there is none we cant flip (we do this before contiguity to save on compute)
        // calculate the score of the old_districting and new_districting
        // choose one of them to be the current districting

        // choosing a station
        int flip_voting_station = r(gen);

        // checking if there is a possible candidate for a new district
        bool candidate_exists = false;
        for (auto j : map.get_stations()[flip_voting_station].get_neighbours())
            if (which_district[flip_voting_station] != which_district[j])
                candidate_exists = true;
        if (candidate_exists == false)
            continue;

        // checking contiguity
        bool contiguity_check = true;
        // ill choose a random neighbour with the same district and do a dfs from it without walking in the choosen station
        // also if the size of the district is 1 (special case), then the answer is no for contiguity

        if (district_voting_stations[which_district[flip_voting_station]] == 1)
            contiguity_check = false;
        else
        {
            int voting_stations_visited = 0;
            std::stack<int> stack_stations;
            // this is for unmarking from visited after dfs
            std::stack<int> visited_nodes;

            for (auto j : map.get_stations()[flip_voting_station].get_neighbours())
                if (which_district[j] == which_district[flip_voting_station])
                {
                    visited_nodes.push(j);
                    visited[j] = true;
                    voting_stations_visited++;
                    stack_stations.push(j);
                    break;
                }

            while (stack_stations.size())
            {
                int node = stack_stations.top();

                bool has_visitied = false;
                const auto &n = map.get_stations()[node].get_neighbours();
                for (auto j : n)
                    if (which_district[j] == which_district[node] && j != flip_voting_station && visited[j] == false)
                    {
                        has_visitied = true;
                        visited[j] = true;
                        voting_stations_visited++;
                        visited_nodes.push(j);
                        stack_stations.push(j);
                        break;
                    }

                // if we didn't find any new vertexes from our vertex we need to pop it out of the stack
                if (has_visitied == false)
                    stack_stations.pop();
            }

            // unmarking visited nodes
            while (visited_nodes.size())
            {
                visited[visited_nodes.top()] = false;
                visited_nodes.pop();
            }

            if (voting_stations_visited < district_voting_stations[which_district[flip_voting_station]] - 1)
                contiguity_check = false;
        }

        // if the district is not connected if we remove our choosen voting station, then we can't flip it
        if (contiguity_check == false)
            continue;

        // choosing a new district for our station
        std::set<int> potential_new_districts;
        for (auto j : map.get_stations()[flip_voting_station].get_neighbours())
            if (which_district[j] != which_district[flip_voting_station])
                potential_new_districts.insert(which_district[j]);

        std::uniform_int_distribution<int> r_(0, int(potential_new_districts.size()) - 1);
        for (int j = r_(gen); j > 0; j--)
            potential_new_districts.erase(potential_new_districts.begin());
        int old_district = which_district[flip_voting_station];
        int new_district = *potential_new_districts.begin();

        double old_score = score();

        // updating to the new districting
        do_flip(flip_voting_station, old_district, new_district);

        double new_score = score(); // calculate new score

        bool update = false;
        if (new_score > old_score)
            update = true;
        else
        {
            double T = T_START * std::pow(T_END / T_START, double(i) / nr_of_tries);
            if (r_01(gen) <= std::exp((new_score - old_score) / T))
                update = true;
        }

        if (!update)
        {
            // update to the old districting
            do_flip(flip_voting_station, new_district, old_district);
        }
    }
}

double Districting::score() const
{
    // calculating the population score using the mean squared error using relative distance
    double population_score = 0;
    double expected_population = double(map.get_population()) / nr_districts;
    for (int i = 0; i < int(district_population.size()); i++)
    {
        double relative_error = (district_population[i] - expected_population) / expected_population;
        population_score += relative_error * relative_error;
    }
    population_score /= double(district_population.size());

    // calculating the shape score
    // its a heuristic it looks at how many edges it has and how many good (both endpoints in the district) it has
    // and taking the mean for all all the districts
    double shape_score = 0;
    for (int i = 0; i < int(district_population.size()); i++)
        shape_score += double(good_edges[i]) / total_edges[i];
    shape_score /= double(district_population.size());

    // calculating the seats score
    double seats_score = 0;
    for (int i = 0; i < nr_districts; i++)
    {
        // calculating which party wins the district
        int winner = 0;
        for (int j = 1; j < nr_parties; j++)
            if (party_votes[i][j] > party_votes[i][winner])
                winner = j;

        seats_score += weights_for_party_gerrymander[winner];
    }
    seats_score /= double(district_population.size());

    // calculating the margin score
    double margin_score = 0;
    for (int i = 0; i < nr_districts; i++)
    {
        // calculating which party wins the district
        int winner = 0;
        int runner_up = -1;
        for (int j = 1; j < nr_parties; j++)
            if (party_votes[i][j] > party_votes[i][winner])
            {
                runner_up = winner;
                winner = j;
            }
            else if (runner_up == -1 || party_votes[i][j] > party_votes[i][runner_up])
                runner_up = j;

        // the idea is to also take into account the closeness of a race
        double margin = double(party_votes[i][winner] - party_votes[i][runner_up]) / district_population[i];
        double credit = 1 / (1 + std::exp(-margin / DEFAULT_TAU));
        margin_score += credit * weights_for_party_gerrymander[winner] + (1 - credit) * weights_for_party_gerrymander[runner_up];
    }
    margin_score /= double(district_population.size());

    return W_SEATS * seats_score + W_MARGIN * margin_score + W_SHAPE * shape_score - W_POP * population_score;
}

void Districting::write_in_terminal_expected_seats() const
{
    std::cout << "-----EXPECTED SEATS-----\n";
    // basically i want to calculate share of vote times the nr of districts

    // calculating the party share across the whole map
    std::vector<double> share(nr_parties);
    for (const auto &v : map.get_stations())
    {
        const auto &p = v.get_vote_shares();
        for (int j = 0; j < int(p.size()); j++)
            share[j] += p[j] * v.get_population();
    }
    for (int i = 0; i < nr_parties; i++)
        share[i] /= map.get_population();

    for (int i = 0; i < nr_parties; i++)
        std::cout << "FOR PARTY " << i << " is expected to get " << share[i] * nr_districts << '\n';

    std::cout << "-----END OF EXPECTED SEATS-----\n";
}

void Districting::write_in_terminal_seats() const
{
    std::cout << "-----WON SEATS-----\n";

    std::vector<int> seats_won(nr_parties);

    for (int i = 0; i < nr_districts; i++)
    {
        // calculating which party wins the district
        int winner = 0;
        for (int j = 1; j < nr_parties; j++)
            if (party_votes[i][j] > party_votes[i][winner])
                winner = j;

        seats_won[winner] += 1;
    }

    for (int i = 0; i < nr_parties; i ++)
        std::cout << "FOR PARTY " << i << " they got " << seats_won[i] << '\n';

    std::cout << "-----END OF WON SEATS-----\n";
}