
#include "Log.hpp"

#include "Database.hpp"

#include "sqlite3.h"

#include "Mappable.hpp"

using namespace mapping;

class TestObject : public Mappable<TestObject> {

public:

    int spes = 1;
    std::string rgles = "rgles";

    static auto properties() {
        return std::make_tuple(
                PROPERTY(spes, TestObject, 5),
                PROPERTY(rgles, TestObject, "astank")
        );
    }

    static std::string class_name() {
        return "TestObject";
    }

};

int main() {

    sql::Database base("spees");

    Logvar(mapping::is_mappable<TestObject>);

    base.register_class<TestObject>();


    TestObject spes;

    Info(TestObject::to_json(TestObject::parse(TestObject::to_json(spes))));

    TestObject::print_properties();

    return 0;
}

