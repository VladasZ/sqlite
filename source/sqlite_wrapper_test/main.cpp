
#include "Log.hpp"
#include "Database.hpp"
#include "TestSQLModel.hpp"

using namespace std;
using namespace sql;

static auto base = Database<sql_mapper>("test_base.sqlite");

int main() {

    Log << "sp";

    return 0;
}
