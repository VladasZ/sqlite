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

        template<class T>
        std::string dump_all();

    private:

        void _execute_command(const std::string& command);
        sqlite3_stmt* _compile_command(const std::string& command);
        void _get_rows(const std::string& command, std::function<void(sqlite3_stmt*)> row);

        template<class T>
        unsigned _rows_count();

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
    unsigned Database::_rows_count() {
        unsigned count;
        std::string command = "select count(*) from " + T::class_name();

        auto callback = [](auto count, auto, auto argv, auto) {
            *static_cast<int*>(count) = atoi(argv[0]);
            return 0;
        };

        sqlite3_exec(_handle, command.c_str(), callback, &count, nullptr);
        return count;
    }


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

        std::vector<T> result;

        _get_rows(T::select_command(), [&](auto stmt) {
            T object;
            unsigned index = 0;
            T::iterate_properties([&](auto property) {
                Info(sqlite3_column_name(stmt, index));
                object.*property.pointer = _extract_function<decltype(property)>()(stmt, index++);
            });
            result.push_back(object);
        });

        return result;
    }

    template<class T>
    std::string Database::dump_all() {
        std::string result;


        _get_rows(T::select_command(), [&](auto stmt) {

            for (unsigned i = 0; i < sqlite3_data_count(stmt); i++) {

                result += std::string() +
                        sqlite3_column_name(stmt, i) + " : " +
                        sqlite3_column_string(stmt, i) + "\n";
            }
        });

        return result;
    }

    template<class T>
    void Database::_register_class() {
        _execute_command(T::create_table_command());
    }

}
