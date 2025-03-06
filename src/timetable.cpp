#include "timetable.h"
#include <nlohmann/json.hpp>
#include "net.h"

using nlohmann::json;

#define NLINES 10
#define NCOLS 40

#define DEFAULT_X_OFFSET 10
#define DEFAULT_Y_OFFSET 2

#define DEFAULT_PADDING 4

void timetable_page() {
     // DONT FORGET TO UNCOMMENT
    json resp_from_api = bakaapi::get_data_from_endpoint("api/3/marks");
    // std::ifstream f("test-data/marks3.json");
    // json resp_from_api = json::parse(f);
    // calculate table size

}