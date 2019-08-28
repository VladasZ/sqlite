
#include "Log.hpp"

#include "sqlite3.h"

#include "mappable.hpp"

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

};

int main() {

    Info("Spes");

    Logvar(sqlite3_threadsafe());


    TestObject spes;

    Info(TestObject::to_json_string(TestObject::parse_string(TestObject::to_json_string(spes))));

    return 0;
}

