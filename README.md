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
- improve_gerrymandering which takes as input the number of tries the Markov Chain Monte Carlo Algorithm does
- generate_svg gives an svg with the map and where each district has a color (randomly given and the voting stations from that region have the color of their district)
- score gives the score of the map
- write_in_terminal_expected_seats the expected won seats by each party based on their voting share
- write_in_terminal_seats the seats won by each party using the current districting
