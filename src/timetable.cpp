#include "timetable.h"
#include <cstdint>
#include <nlohmann/json.hpp>
#include "net.h"
#include <fstream>

using nlohmann::json;

#define NLINES 10
#define NCOLS 40

#define DEFAULT_X_OFFSET 10
#define DEFAULT_Y_OFFSET 2

#define DEFAULT_PADDING 4

void timetable_page() {
    // DONT FORGET TO UNCOMMENT
    // json resp_from_api = bakaapi::get_data_from_endpoint("api/3/timetable/actual");
    std::ifstream f("test-data/timetable.json");
    json resp_from_api = json::parse(f);

    // calculate table size

    uint8_t column_number = 0;
    for(uint8_t i = 0; i < resp_from_api["Days"].size(); i++) {
        (resp_from_api["Days"][i]["Atoms"].size() > column_number) ? (column_number = resp_from_api["Days"][i]["Atoms"].size());
    }
    
    

}