//
//  Database.cpp
//  sqlite
//
//  Created by Vladas Zakrevskis on 8/27/19.
//  Copyright © 2019 VladasZ. All rights reserved.
//

#include "Log.hpp"
#include "Database.hpp"

using namespace sql;
using namespace std;

Database::Database(const string& path) : _path(path) {
    if (sqlite3_open(path.c_str(), &_handle)) {
        Fatal(sqlite3_errmsg(_handle))
    }
}

Database::~Database() {
    if (_handle)
        sqlite3_close(_handle);
}

void Database::execute_command(const std::string& command) {
    char* error;
    sqlite3_exec(_handle, command.c_str(), 0, 0, &error);
}
