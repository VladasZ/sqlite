
#include "Log.hpp"

#include "Database.hpp"

#include "sqlite3.h"

#include "Mappable.hpp"

using namespace mapping;

class TestObject : public Mappable<TestObject> {

public:

    int spes = 1;
    std::string rgles = "orob";

    static auto properties() {
        return std::make_tuple(
                PROPERTY(spes, 5),
                PROPERTY(rgles, "astank")
        );
    }

    static std::string class_name() {
        return "TestObject";
    }

};

int main() {

    sql::Database base("spees");

    TestObject seeee;


    Info(TestObject::parse(seeee.to_json()).to_json());


    auto result = base.get_all<TestObject>();

    return 0;
}

