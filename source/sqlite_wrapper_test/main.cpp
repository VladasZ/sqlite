
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
