#pragma once
#include <utility>
#include <vector>

int Orientation(std::pair <long long, long long> a, std::pair <long long, long long> b, std::pair <long long, long long> c);
bool ConvexHull(std::vector <std::pair <long long, long long>> points, std::vector <std::pair <long long, long long>> & convex_hull_points, std::vector <std::pair <long long, long long>> & interior_points, bool non_line_non_point, int needed_interior_points);
