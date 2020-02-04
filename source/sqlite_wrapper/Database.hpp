//
//  Database.hpp
//  sqlite
//
//  Created by Vladas Zakrevskis on 15/01/20.
//  Copyright © 2020 VladasZ. All rights reserved.
//

#pragma once

#include <string>

#include "SQLiteCpp/Database.h"

namespace sql {

    class Database {

        const std::string name;

        SQLite::Database db;

    public:

        Database(const std::string& name);

        int execute(const std::string& command);

    };

}
