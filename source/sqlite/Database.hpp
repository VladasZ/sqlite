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

#include "boost/core/demangle.hpp"

#include "Mappable.hpp"

using namespace boost::core;

namespace sql {

    std::string sqlite3_column_string(sqlite3_stmt*, int column);

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

        template<class Property>
        auto _extract_function(std::enable_if_t<Property::is_string>* = nullptr)  { return sqlite3_column_string; }

        template<class Property>
        auto _extract_function(std::enable_if_t<Property::is_float>* = nullptr)   { return sqlite3_column_double; }

        template<class Property>
        auto _extract_function(std::enable_if_t<Property::is_integer>* = nullptr) { return sqlite3_column_int; }

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

        auto stmt = _compile_command("SELECT * from " + T::class_name() + ";");

        int code = 0;

        std::vector<T> result;

        while ((code = sqlite3_step(stmt)) == SQLITE_ROW) {
            T object;
            unsigned index = 0;
            T::iterate_properties([&](auto property) {
                object.*property.pointer = _extract_function<decltype(property)>()(stmt, index++);
            });
            result.push_back(object);
        }

        if (code != SQLITE_DONE) {
            Error(sqlite3_errmsg(_handle));
        }

        return result;
    }

    template<class T>
    void Database::_register_class() {
        _execute_command(T::create_table_command());
    }

}
