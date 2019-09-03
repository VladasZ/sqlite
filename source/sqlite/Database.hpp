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

#include "Value.hpp"
#include "Result.hpp"
#include "Column.hpp"
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

        template<class T>
        T where(std::function<void(T&)>);

        template<class T>
        std::string dump_all();

    private:
    public:

        void _execute_command(const std::string& command);
        sqlite3_stmt* _compile_command(const std::string& command);
        void _get_rows(const std::string& command, std::function<void(sqlite3_stmt*)> row);

        template<class T>
        std::map<std::string, Column> _columns();

        template<class T>
        unsigned _rows_count();

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

        std::vector<T> result;

        auto columns = _columns<T>();

        _get_rows(T::select_command(), [&](auto stmt) {
            T object;
            T::iterate_properties([&](auto property) {
                if (columns.find(property.name) == columns.end()) {
                    Fatal("Invalid table for: " << T::class_name() << " missing property: " << property.name);
                }
                columns[property.name].set_property(object, property, stmt);
            });
            result.push_back(object);
        });

        return result;
    }

    template<class T>
    T Database::where(std::function<void(T&)> edit) {
        auto object = T::empty();
        edit(object);
        auto field_name = object.edited_field();

        Info(object.template get<mapping::Value>(field_name).to_string());

        return object;
    }

    template<class T>
    std::string Database::dump_all() {
        std::string result = "\n";

        _get_rows(T::select_command(), [&](auto stmt) {

            for (unsigned i = 0; i < sqlite3_data_count(stmt); i++) {

                result += std::string() +
                          sqlite3_column_name(stmt, i) + " : " +
                          reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)) + "\n";
            }
            result += "============================\n";
        });

        return result;
    }

    template<class T>
    std::map<std::string, Column> Database::_columns() {
        auto stmt = _compile_command(T::select_command());

        std::map<std::string, Column> result;

        for (unsigned i = 0; i < sqlite3_column_count(stmt); i++) {
            auto name = sqlite3_column_name(stmt, i);
            result[name] = Column(i, name);
        }

        sqlite3_finalize(stmt);

        return result;
    }

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
    void Database::_register_class() {
        _execute_command(T::create_table_command());
    }

}
