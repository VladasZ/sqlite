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

std::string sql::sqlite3_column_string(sqlite3_stmt* stmt, int column) {
    return reinterpret_cast<const char*>(sqlite3_column_text(stmt, column));
}

Database::Database(const string& path) : _path(path) {
    if (sqlite3_open(path.c_str(), &_handle)) {
        Fatal(sqlite3_errmsg(_handle))
    }
}

Database::~Database() {
    if (_handle)
        sqlite3_close(_handle);
}

void Database::_execute_command(const std::string& command) {
    char* error;
    if (sqlite3_exec(_handle, command.c_str(), 0, 0, &error)) {
        Error(error);
        sqlite3_free(error);
    }
    else {
        Info("sucess");
        Info(command);
    }
}

sqlite3_stmt* Database::_compile_command(const std::string& command) {

    sqlite3_stmt* result;

    if (sqlite3_prepare_v3(_handle, command.c_str(), -1, 0, &result, nullptr)) {
        Error(sqlite3_errmsg(_handle));
        sqlite3_finalize(result);
        return nullptr;
    }

    return result;
}
