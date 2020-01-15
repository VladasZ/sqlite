
#include "Log.hpp"
#include "Point.hpp"
#include "Database.hpp"
#include "JsonMapper.hpp"
#include "SQLiteMapper.hpp"
#include "TestMappingModels.hpp"

using namespace std;

namespace gm {

MAKE_CLASS_INFO(Point, std::make_tuple(
        MAKE_PROPERTY("x", &Point::x),
        MAKE_PROPERTY("y", &Point::y)
        ));

}

MAKE_MAPPER(mapping::InfoOfTestClass, gm::InfoOfPoint);

constexpr auto json_mapper = mapping::JSONMapper<mapper>();
constexpr auto sql_mapper = mapping::SQLiteMapper<mapper>();


constexpr auto base = sql::Database<sql_mapper>("base");

int main() {


    Log(json_mapper.to_json(gm::Point { 10, 20 }));


    Log(sql_mapper.create_table_command<mapping::TestClass>());

    return 0;
}
