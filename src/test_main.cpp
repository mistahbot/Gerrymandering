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