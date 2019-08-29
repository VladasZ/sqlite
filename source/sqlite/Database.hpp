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

    public:

        template<class T>
        void add(const T& object);

        template<class T>
        std::vector<T> get_all();

    private:

        void _execute_command(const std::string& command);
        sqlite3_stmt* _compile_command(const std::string& command);

        template<class T>
        void _register_class();

    };

    template<class T>
    void Database::add(const T& object) {
        static_assert(mapping::is_mappable<T>,
                      "Adding class must be derived from mapping::Mappable<T> class");

        static bool new_type = true;

        if (new_type) {
            new_type = false;
            _register_class<T>();
        }

        _execute_command(object.insert_command());
    }

    template<class T>
    std::vector<T> Database::get_all() {

        std::string command = "SELECT * from " + T::class_name() + ";";

        Info(command);

        return { };
    }

    template<class T>
    void Database::_register_class() {
        _execute_command(T::create_table_command());
    }

}
