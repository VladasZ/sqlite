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

#include "Column.hpp"
#include "Result.hpp"
#include "SQLiteMappable.hpp"

namespace sql {

    class Database {

        std::string _path;
        sqlite3* _handle = nullptr;

    public:

        Database(const std::string& path);
        ~Database();

    public:

        template<class T>
        std::string insert(const T& object) { static_assert(mapping::is_sqlite_mappable<T>);

            static bool new_type = true;

            if (new_type) {
                new_type = false;
                _register_class<T>();
            }

            return _execute_command(object.insert_command());
        }

        template<class T>
        std::vector<T> get_all() { static_assert(mapping::is_sqlite_mappable<T>);
            std::vector<T> result;
            _get_rows(mapping::SQLiteMappable<T>::select_command(), [&](auto stmt) {
                result.push_back(_parse_row<T>(stmt));
            });
            return result;
        }

        template<class T>
        cu::Result<T> get(const T& object) { static_assert(mapping::is_sqlite_mappable<T>);
            cu::Result<T> result;
            _get_rows(object.select_command(), [&](auto stmt) {
                result = _parse_row<T>(stmt);
            });
            return result;
        }

        template<class T>
        cu::Result<T> where(std::function<void(T&)> edit) { static_assert(mapping::is_sqlite_mappable<T>);
            auto empty = T::empty();
            edit(empty);
            cu::Result<T> result;
            _get_rows(empty.select_where_command(), [&](auto stmt) {
                result = _parse_row<T>(stmt);
            });
            return result;
        }

        template<class T>
        void update(const T& object) { static_assert(mapping::is_sqlite_mappable<T>);
            auto error = _execute_command(object.update_command());
            if (error.empty()) {
                return;
            }
            throw std::runtime_error(
                    "Failed to update object of class " + T::class_name() + "\n" +
                    "Error: " + error + "\n" +
                    "Object data:" + "\n" +
                    object.to_json()
                    );
        }

        template<class T>
        std::string dump_all() { static_assert(mapping::is_sqlite_mappable<T>);
            std::string result = "\n";
            _get_rows(mapping::SQLiteMappable<T>::select_all_command(), [&](auto stmt) {
                for (unsigned i = 0; i < sqlite3_data_count(stmt); i++) {
                    result += std::string() +
                              sqlite3_column_name(stmt, i) + " : " +
                              reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)) + "\n";
                }
                result += "============================\n";
            });
            return result;
        }

    private:
	public:

        std::string _execute_command(const std::string& command);
        sqlite3_stmt* _compile_command(const std::string& command);
        void _get_rows(const std::string& command, std::function<void(sqlite3_stmt*)> row);

        template<class T>
        std::map<std::string, Column>& _columns() { static_assert(mapping::is_sqlite_mappable<T>);
            static std::map<std::string, Column> result;
            static bool retrieved = false;
            if (retrieved) {
                return result;
            }
            auto stmt = _compile_command(mapping::SQLiteMappable<T>::select_all_command());
            for (unsigned i = 0; i < sqlite3_column_count(stmt); i++) {
                auto name = sqlite3_column_name(stmt, i);
                result[name] = Column(i, name);
            }
            sqlite3_finalize(stmt);
            retrieved = true;
            return result;
        }

        template<class T>
        unsigned _rows_count() { static_assert(mapping::is_sqlite_mappable<T>);
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
        void _register_class() { static_assert(mapping::is_sqlite_mappable<T>);
            _execute_command(T::create_table_command());
        }

        template<class T>
        T _parse_row(sqlite3_stmt* stmt) { static_assert(mapping::is_sqlite_mappable<T>);
            T object;
            auto columns = _columns<T>();
            T::iterate_properties([&](auto property) {
                if (columns.find(property.name) == columns.end()) {
                    throw std::runtime_error(
                          "Invalid table for: " + T::class_name() +
                          " missing property: " + property.name);
                }
                columns[property.name].set_property(object, property, stmt);
            });
            return object;
        }

    };

}
