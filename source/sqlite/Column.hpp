//
//  Column.hpp
//  sqlite
//
//  Created by Vladas Zakrevskis on 8/27/19.
//  Copyright © 2019 VladasZ. All rights reserved.
//


#pragma once

#include <string>

#include "sqlite3.h"

namespace  sql {

    class Column {

        unsigned _index;
        sqlite3_stmt* _stmt;

        std::string _name;

    public:

        Column() = default;
        Column(unsigned index, sqlite3_stmt* stmt, const std::string& name);

        std::string name() const;

        std::string string() const;
        int integer() const;
        float floating() const;

        std::string to_string() const;

    };

}
