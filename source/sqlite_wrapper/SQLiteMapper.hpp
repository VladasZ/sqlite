//
//  SQLiteMapper.hpp
//  mapping
//
//  Created by Vladas Zakrevskis on 15/01/2020.
//  Copyright © 2020 VladasZ. All rights reserved.
//

#pragma once

#include "SQLiteCpp/Database.h"

#include "Mapper.hpp"

#define SQLITE_MAPPER_LOG_COMMANDS

namespace mapping {

    class is_sqlite_mapper_cheker_base { };

    template <auto& _mapper>
    class SQLiteMapper : is_sqlite_mapper_cheker_base {

        using Mapper = cu::remove_all_t<decltype(_mapper)>;

        static_assert(is_mapper_v<Mapper>);

    public:

        static constexpr auto mapper = _mapper;

        template <class Class>
        static std::string create_table_command() {
            static_assert(_exists<Class>());
            std::string command = "CREATE TABLE IF NOT EXISTS ";
            mapper.template get_class_info<Class>([&](const auto& class_info) {
                command += class_info.name;
                command += " (\n";
                class_info.iterate_properties([&](auto property) {
                    using Property = decltype(property);
                    if constexpr (!Property::is_id) {
                        command += property.name() + " " + property.database_type_name();
                        if (property.is_unique) {
                            command += " UNIQUE";
                        }
                        command += ",\n";
                    }
                });
            });
            command.pop_back();
            command.pop_back();
            command += "\n);";
#ifdef SQLITE_MAPPER_LOG_COMMANDS
            Log(command);
#endif
            return command;
        }

        static std::vector<std::string> create_all_tables_commands() {
            std::vector<std::string> result;
            mapper.template iterate_classes([&] (auto class_info) {
                using ClassInfo = decltype(class_info);
                using Class = typename ClassInfo::Class;
                result.push_back(create_table_command<Class>());
            });
            return result;
        }

        template <class T>
        static std::string insert_command(const T& obj) {
            static_assert(_exists<T>());
            std::string columns;
            std::string values;
            std::string class_name;
            mapper.template get_class_info<T>([&](auto class_info) {
                class_name = class_info.name;
                class_info.iterate_properties([&](auto property) {
                    if constexpr (!property.is_id) {
                        columns += property.name() + ", ";
                        values  += property.database_value(obj) + ",";
                    }
                });
            });
            columns.pop_back();
            columns.pop_back();
            values.pop_back();

            auto command = std::string() +
                           "INSERT INTO " + class_name + " (" + columns + ")\n" +
                           "VALUES(" + values + ");";

#ifdef SQLITE_MAPPER_LOG_COMMANDS
            Log(command);
#endif

            return command;
        }

        template <
                auto pointer,
                class Pointer = cu::remove_all_t<decltype(pointer)>,
                class Class = typename cu::pointer_to_member_class<Pointer>::type,
                class Value = typename cu::pointer_to_member_value<Pointer>::type>
        static std::string select_where_command(Value value) {
            static_assert(cu::is_pointer_to_member_v<Pointer>);
            static auto class_name    = std::string(mapper.template get_class_name<Class>());
            static auto property_name = std::string(mapper.template get_property_name<pointer>());

            std::string value_string;

            using Info = cu::TypeInfo<Value>;

            if constexpr (Info::is_string || Info::is_c_string) {
                value_string = std::string() + "\'" + value + "\'";
            }
            else {
                value_string = std::to_string(value);
            }

            auto command = std::string() +
                           "SELECT rowid, * FROM " + class_name +
                           " WHERE " + property_name + " = " + value_string + ";";

#ifdef SQLITE_MAPPER_LOG_COMMANDS
            Log(command);
#endif

            return command;
        }

        template <class Class>
        static std::string select_last_command() {
            static auto class_name = std::string(mapper.template get_class_name<Class>());
            static auto command = std::string() +
                    "SELECT rowid, * FROM " + class_name + " ORDER BY rowid DESC LIMIT 1;";

#ifdef SQLITE_MAPPER_LOG_COMMANDS
            Log(command);
#endif
            return command;
        }

        template <class T>
        static T extract(SQLite::Statement& statement) {
            T result = mapper.template create_empty<T>();
            int index = 0;
            mapper.template iterate_properties<T>([&](auto property) {
                using Property = decltype(property);
                using Info = typename Property::Info;
                auto& ref = property.get_reference(result);
                if constexpr (Info::is_string) {
                    ref = statement.getColumn(index).getString();
                }
                else if constexpr (Info::is_float) {
                    ref = statement.getColumn(index).getDouble();
                }
                else {
                    ref = statement.getColumn(index);
                }
                index++;
            });
            return result;
        }

        template <class T>
        static std::string select_all_command() {
            auto command = std::string() + "SELECT rowid, * FROM " + std::string(mapper.template get_class_name<T>());
#ifdef SQLITE_MAPPER_LOG_COMMANDS
            Log(command);
#endif
            return command;
        }

    private:

        template <class Class>
        static constexpr bool _exists() {
            return mapper.template exists<Class>();
        }

    };

    template <class T> constexpr bool is_sqlite_mapper_v = std::is_base_of_v<is_sqlite_mapper_cheker_base, cu::remove_all_t<T>>;

}
