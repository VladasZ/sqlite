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

namespace mapping {

    class is_sqlite_mapper_cheker_base { };

    template <auto& mapper>
    class SQLiteMapper : is_sqlite_mapper_cheker_base {

        using Mapper = cu::remove_all_t<decltype(mapper)>;

        static_assert(is_mapper_v<Mapper>);

    public:

        template <class Class>
        static std::string create_table_command() {
            static_assert(_exists<Class>());
            std::string command = "CREATE TABLE IF NOT EXISTS ";
            mapper.template get_class_info<Class>([&](const auto& class_info) {
                command += class_info.name;
                command += " (\n";
                class_info.iterate_properties([&](const auto& property) {
                    command += property.name() + " " + property.database_type_name();
                    if (property.is_unique) {
                        command += " UNIQUE";
                    }
                    command += ",\n";
                });
            });
            command.pop_back();
            command.pop_back();
            command += "\n);";
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
                    columns += property.name() + ", ";
                    values  += property.database_value(obj) + ",";
                });
            });
            columns.pop_back();
            columns.pop_back();
            values.pop_back();
            return std::string() +
                   "INSERT INTO " + class_name + " (" + columns + ")\n" +
                   "VALUES(" + values + ");";
        }

        template <
                auto pointer,
                class Pointer = cu::remove_all_t<decltype(pointer)>,
                class Class = typename cu::pointer_to_member_class<Pointer>::type,
                class Value = typename cu::pointer_to_member_value<Pointer>::type>
        static std::string select_where_command(const Value& value) {
            static_assert(cu::is_pointer_to_member_v<Pointer>);
            static std::string class_name = std::string(mapper.template get_class_name<Class>());
            static std::string property_name = [&] {
                std::string name;
                mapper.template iterate_properties<Class>([&](auto property) {
                    using Property = decltype(property);
                    using PropertyPointer = typename Property::Pointer;
                    if constexpr (std::is_same_v<PropertyPointer, Pointer>) {
                        if constexpr (Property::pointer_to_member == pointer) {
                            name = std::string(property.name());
                        }
                    }
                });
                return name;
            }();

            std::string value_string;

            if constexpr (std::is_same_v<std::string, Value>) {
                value_string = std::string() + "\'" + value + "\'";
            }
            else {
                value_string = std::to_string(value);
            }

            return std::string() +
            "SELECT * FROM " + class_name +
            " WHERE " + property_name + " = " + value_string + ";";
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
            });
            return result;
        }

        template <class T>
        static std::string select_all_command() {
            return std::string() + "SELECT * FROM " + std::string(mapper.template get_class_name<T>());
        }

    private:

        template <class Class>
        static constexpr bool _exists() {
            return mapper.template exists<Class>();
        }

    };

    template <class T> constexpr bool is_sqlite_mapper_v = std::is_base_of_v<is_sqlite_mapper_cheker_base, cu::remove_all_t<T>>;

}
