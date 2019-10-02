//
//  Database.hpp
//  sqlite
//
//  Created by Vladas Zakrevskis on 8/27/19.
//  Copyright © 2019 VladasZ. All rights reserved.
//

#include "Column.hpp"

using namespace sql;


static std::string sqlite3_column_string(sqlite3_stmt* stmt, unsigned column) {
    return reinterpret_cast<const char*>(sqlite3_column_text(stmt, column));
}

Column::Column(unsigned index, const std::string& name) : _index(index), _name(name) {

}

std::string Column::name() const {
    return _name;
}

std::string Column::string() const {
    return sqlite3_column_string(_stmt, _index);
}

int Column::integer() const {
    return sqlite3_column_int(_stmt, _index);
}

float Column::floating() const {
    return static_cast<float>(sqlite3_column_double(_stmt, _index));
}

std::string Column::to_string() const {
    return _name + " " + std::to_string(_index);
}
