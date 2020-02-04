//
//  Database.cpp
//  sqlite
//
//  Created by Vladas Zakrevskis on 03/02/20.
//  Copyright © 2020 VladasZ. All rights reserved.
//

#include "Database.hpp"

using namespace sql;

Database::Database(const std::string& name) : name(name), db(name) {

}

int Database::execute(const std::string& command) {
    return db.exec(command);
}
