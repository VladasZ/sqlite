
#include "Log.hpp"
#include "Database.hpp"
#include "TestSQLModel.hpp"
#include "ExceptionCatch.hpp"

using namespace cu;
using namespace std;
using namespace sql;

static auto base = Database<sql_mapper>("test_base2.sqlite");

void test() {
    Log << "sp";

    //Student st;
    //st.age = 140;
    //st.name = "Alak";
    //base.insert(st);

    base.dump_all<json_mapper>();

    Student student = base.select_first_where<&Student::age>(100);


    student.name = "sokol";

    base.update(student);

    Log << json_mapper.to_json_string(student);


    base.dump_all<json_mapper>();


}

int main() {

    try {
        test();
    }
    catch (...)
    {
        Log << what();
    }

    

    return 0;
}
