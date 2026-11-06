A randomly generated region is created which is partitioned into voting stations. This process takes into account what is the population, how many parties there are, what percentage of the population votes for each party. The electoral system used is first past the post.

It contains three libraries (Geometry, Map, Districting). The Geometry library is a helper library for Map and Districting

The Map library contains the Voting_Station class which is a helper for the Map class. The Map class describes what the region looks like. An object of the Map class needs as input
- number of voting stations
- population
- vote share of each party

The Districting library contains the Districting class, an object from that class needs
- an object from the Map class
- number of districts
- weights for each party (this determines which party is favored by the gerrymander)
A Districting object starts with a districting where there are n - 1 districts formed by one voting station, and the other voting stations are in the other district.

From a Districting object the functions which can be called are
- improve_gerrymandering which takes as input the number of tries the algorithm does (the algorithm tries to change the district of one voting station)
- generate_svg gives an svg with the map and where each district has a color (randomly given and the voting stations from that region have the color of their district)
- score gives the score of the map
- write_in_terminal_expected_seats the expected won seats by each party based on their voting share
- write_in_terminal_seats the seats won by each party using the current districting

An example is src/test_main.cpp
```cpp
#include "region/Map.h"
#include "region/Districting.h"

int main()
{
    Map m(1000, 1'000'000, {15, 25, 30, 30});
    Districting d(m, 20, {0, 1, 0, 0});
    d.write_in_terminal_expected_seats();
    d.write_in_terminal_seats();
    d.generate_svg("start.svg");
    d.improve_gerrymandering(10'000'000);
    d.write_in_terminal_seats();
    d.generate_svg("end.svg");
    
    return 0;
}
```