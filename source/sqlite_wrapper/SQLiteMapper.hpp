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

namespace sql {

    template <auto& _mapper>
    class SQLiteMapper {

    public:

        using Mapper = cu::remove_all_t<decltype(_mapper)>;

        static_assert(mapping::is_mapper_v<Mapper>);

        static constexpr auto mapper = _mapper;

        template <class Class>
        static std::string create_table_command() {
            static_assert(_exists<Class>());
            std::string command = "CREATE TABLE IF NOT EXISTS ";

            constexpr auto info = mapper.info<Class>();

            command += info.name;
            command += " (\n";

            info.properties([&](auto property) {
                using Property = decltype(property);
                if constexpr (!Property::is_id) {
                    command += property.name() + " " + database_type_name<Property>();
                    if (property.is_unique) {
                        command += " UNIQUE";
                    }
                    command += ",\n";
                }
            });

            command.pop_back();
            command.pop_back();
            command += "\n);";
#ifdef SQLITE_MAPPER_LOG_COMMANDS
            Log << command;
#endif
            return command;
        }

        static std::vector<std::string> create_all_tables_commands() {
            std::vector<std::string> result;

            mapper.classes_with_custom_members([&](auto class_info) {
                using ClassInfo = decltype(class_info);
                using Class = typename ClassInfo::Class;
            });

            mapper.classes([&] (auto class_info) {
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

            static constexpr auto info = mapper.info<T>();

            class_name = info.name;
            info.properties([&](auto property) {
                if constexpr (!property.is_id) {
                    columns += property.name() + ", ";
                    values += database_value<decltype(property)>(obj) + ",";
                }
            });

            columns.pop_back();
            columns.pop_back();
            values.pop_back();

            auto command = std::string() +
                           "INSERT INTO " + class_name + " (" + columns + ")\n" +
                           "VALUES(" + values + ");";

#ifdef SQLITE_MAPPER_LOG_COMMANDS
            Log << command;
#endif

            return command;
        }

        template <
                auto pointer,
                class Pointer = decltype(pointer),
                class Class = typename cu::pointer_to_member_class<Pointer>::type,
                class Value = typename cu::pointer_to_member_value<Pointer>::type>
        static std::string select_where_command(Value value) {
            static_assert(cu::is_pointer_to_member_v<Pointer>);

            auto class_name    = std::string(mapper.class_name<Class>());
            auto property_name = std::string(mapper.property_name<pointer>());

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
            Log << command;
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

        template <
                auto pointer,
                class Pointer = cu::remove_all_t<decltype(pointer)>,
                class Class = typename cu::pointer_to_member_class<Pointer>::type,
                class Value = typename cu::pointer_to_member_value<Pointer>::type>
        static std::string delete_where_command(Value value) {
            static_assert(cu::is_pointer_to_member_v<Pointer>);

            auto class_name    = std::string(mapper.template get_class_name<Class>());
            auto property_name = std::string(mapper.template get_property_name<pointer>());

            std::string value_string;

            using Info = cu::TypeInfo<Value>;

            if constexpr (Info::is_string || Info::is_c_string) {
                value_string = std::string() + "\'" + value + "\'";
            }
            else {
                value_string = std::to_string(value);
            }

            auto command = std::string() +
                           "DELETE FROM " + class_name +
                           " WHERE " + property_name + " = " + value_string + ";";

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
        static std::string update_where_command(Value value, const Class& object) {
            static_assert(cu::is_pointer_to_member_v<Pointer>);

            auto class_name    = std::string(mapper.template get_class_name<Class>());
            auto property_name = std::string(mapper.template get_property_name<pointer>());

            std::string value_string;

            using Info = cu::TypeInfo<Value>;

            if constexpr (Info::is_string || Info::is_c_string) {
                value_string = std::string() + "\'" + value + "\'";
            }
            else {
                value_string = std::to_string(value);
            }

            auto command = std::string() + "UPDATE " + class_name + " SET ";

            bool at_least_one_change = false;

            mapper.template iterate_properties<Class>([&](auto property) {
                using Property = decltype(property);
                using PropertyValue = typename Property::Value;

                if constexpr (!Property::is_id) {
                    const PropertyValue default_value = { };
                    auto value = Property::get_reference(object);
                    if (value == default_value) return;
                    command += property.name() + " = " + property.database_value(object) + ", ";
                    at_least_one_change = true;
                }
            });

            if (!at_least_one_change) {
                Fatal("Trying to update database with empty object.");
            }

            command.pop_back();
            command.pop_back();

            command += " WHERE " + property_name + "=" + value_string;

#ifdef SQLITE_MAPPER_LOG_COMMANDS
            Log(command);
#endif

            return command;
        }


        template <class T>
        static T extract(SQLite::Statement& statement) {
            T result = mapper.template create_empty<T>();
            int index = 0;
            mapper.template properties<T>([&](auto property) {
                using Property = decltype(property);
                using Info = typename Property::ValueInfo;
                auto& ref = Property::get_reference(result);
                if constexpr (Info::is_string) {
                    ref = statement.getColumn(index).getString();
                }
                else if constexpr (Info::is_float) {
                    ref = statement.getColumn(index).getDouble();
                }
                else if constexpr (Info::is_integer) {
                    ref = statement.getColumn(index).getInt();
                }
                else {
                   // static_assert(false);
                    Log << property;
                    //ref = statement.getColumn(index);
                }
                index++;
            });
            return result;
        }

        template <class T>
        static std::string select_all_command() {
            auto command = std::string() + "SELECT rowid, * FROM " + std::string(Mapper::class_name<T>());
#ifdef SQLITE_MAPPER_LOG_COMMANDS
            Log << command;
#endif
            return command;
        }

    private:

        template <class Class>
        static constexpr bool _exists() {
            return mapper.template exists<Class>();
        }

        template <class Property>
        static std::string database_type_name() {
            using Info = typename Property::ValueInfo;
            if constexpr (Info::is_string) {
                return "TEXT";
            }
            else if constexpr (Info::is_float) {
                return "REAL";
            }
            else if constexpr (Info::is_integer) {
                return "INTEGER";
            }
            else {
               // static_assert(false);
                Fatal("Invalid member: " + Property::static_to_string());
            }
        }

        template <class Property, class Object>
        static std::string database_value(const Object& obj) {
            using Info = typename Property::ValueInfo;
            const auto& value = Property::get_reference(obj);
            if constexpr (Info::is_string) {
                return value;
            }
            else {
                return std::to_string(value);
            }
        }

    };

    template <class  > struct is_sqlite_mapper : std::false_type { };
    template <auto& t> struct is_sqlite_mapper<SQLiteMapper<t>> : std::true_type { };
    template <class T> constexpr bool is_sqlite_mapper_v = is_sqlite_mapper<T>::value;

}
