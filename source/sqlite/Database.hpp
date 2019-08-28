//
//  Database.hpp
//  sqlite
//
//  Created by Vladas Zakrevskis on 8/27/19.
//  Copyright © 2019 VladasZ. All rights reserved.
//

#pragma once

#include <string>

#include "sqlite3.h"

#include "Mappable.hpp"

namespace sql {

    class Database {

        std::string _path;
        sqlite3* _handle = nullptr;

    public:

        Database(const std::string& path);
        ~Database();

        void execute_command(const std::string& command);

        template<class Class>
        void register_class();

    };


    template<class Class>
    void Database::register_class() {
        static_assert(mapping::is_mappable<Class>,
                "Registering class must be derived from mapping::Mappable<T> class");

        Logvar(Class::class_name());

    }

}
