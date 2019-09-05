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

std::string Database::_execute_command(const std::string& command) {
    char* error;
    if (sqlite3_exec(_handle, command.c_str(), 0, 0, &error)) {
        _Error(error);
        sqlite3_free(error);
        return error;
    }
    return "";
}

sqlite3_stmt* Database::_compile_command(const std::string& command) {

    sqlite3_stmt* result;

    if (sqlite3_prepare_v3(_handle, command.c_str(), -1, 0, &result, nullptr)) {
        _Error(sqlite3_errmsg(_handle));
        sqlite3_finalize(result);
        return nullptr;
    }

    return result;
}

void Database::_get_rows(const std::string& command, std::function<void(sqlite3_stmt*)> row) {
    auto stmt = _compile_command(command);

    int code;

    while (true) {
        code = sqlite3_step(stmt);
        if (code != SQLITE_ROW) {
            break;
        }
        row(stmt);
    }

    if (code != SQLITE_DONE) {
        _Error(sqlite3_errmsg(_handle));
    }

    sqlite3_finalize(stmt);
}
