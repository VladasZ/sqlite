
#include "Log.hpp"

#include "Database.hpp"

#include "sqlite3.h"

#include "Mappable.hpp"

using namespace std;
using namespace mapping;
using namespace sql;

class TestObject : public Mappable<TestObject> {

public:

    int age = 0;
    std::string name = "";
    std::string last_name = "";

    static auto properties() {
        return std::make_tuple(
                PROPERTY(age),
                PROPERTY(name),
                PROPERTY(last_name)
        );
    }

    static std::string class_name() {
        return "TestObject";
    }

};

int main() {

    sql::Database base("spees2");

    TestObject seeee;

    seeee.age = 100;
    seeee.name = "pikul";
    seeee.last_name = "spesel";

    // base.add(seeee);

    // Info(base.dump_all<TestObject>());

    auto cols = base._columns<TestObject>();

    for (auto [name, col] : cols) {
        cout << name << " " << col.to_string() << endl;
    }

    Info(base.dump_all<TestObject>());

    Info(TestObject::array_to_json(base.get_all<TestObject>()));

    return 0;
}
