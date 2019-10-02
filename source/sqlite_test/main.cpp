
#include "Log.hpp"

#include "Database.hpp"

#include "sqlite3.h"

#include "JSONMappable.hpp"

using namespace std;
using namespace mapping;
using namespace sql;

class TestObject : public mapping::JSONMappable<TestObject>,
				   public mapping::SQLiteMappable<TestObject> {

public:

    int age = 0;
    std::string name = "";
    std::string last_name = "";

    static auto properties() {
        return std::make_tuple(
			PROPERTY(age),
			PRIMARY_KEY(name),
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

    base.insert(seeee);

    Log(base.dump_all<TestObject>());

    auto cols = base._columns<TestObject>();

    for (auto [name, col] : cols) {
        cout << name << " " << col.to_string() << endl;
    }

    Log(base.dump_all<TestObject>());

  //  Log(TestObject::array_to_json(base.get_all<TestObject>()));

    return 0;
}
