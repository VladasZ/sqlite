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

    class is_sqlite_mapper_checker_base {};

    template <auto& _mapper>
    class SQLiteMapper : is_sqlite_mapper_checker_base {

    public:

        using Mapper = cu::remove_all_t<decltype(_mapper)>;

        static_assert(mapping::is_mapper_v<Mapper>);

        static const bool disable_log = false;
        static constexpr auto mapper = _mapper;

        template <class Class>
        static std::string table_properties() {
            std::string command;
            properties<Class>([&](auto prop) {
                using Prop = decltype(prop);
                using Value = typename Prop::Value;
                if constexpr (!Prop::is_id && !Prop::ValueInfo::is_custom_type) {
                    command += prop.name() + " " + database_type_name<Value>();
                    if constexpr (Prop::is_unique) {
                        command += " UNIQUE";
                    }
                    command += ",\n";
                }
            });
            return command;
        }

        template <class Class>
        static std::string create_table_command(const std::string& more_fields = "") {
            static_assert(_exists<Class>());
            std::string command = "CREATE TABLE IF NOT EXISTS ";

            command += class_name<Class>();
            command += " (\n";

            command += table_properties<Class>();
            command += more_fields;

            command.pop_back();
            command.pop_back();
            command += "\n);";
            Log << cu::log::Off(disable_log) << command;
            return command;
        }

        static std::vector<std::string> create_all_tables_commands() {
            std::vector<std::string> result;
            std::vector<std::string> processed_classes;

            Mapper::classes_with_custom_members([&](auto class_info) {
                using Info = decltype(class_info);
                using Class = typename Info::Class;

                Info::mappable_properties([&](auto prop) {
                    using Prop = decltype(prop);
                    using Value = typename Prop::Value;

                    //if (cu::array::contains(processed_classes, cu::class_name<Value>())) {
                    //    return;
                    //}

                    //std::string command;

                    //Info::template properties_of_type<Value>([&](auto prop) {
                    //    command += prop.foreign_key() + " " + database_type_name<int>();
                    //    command += ",\n";
                    //    });

                    //processed_classes.push_back(cu::class_name<Value>());

                    //result.push_back(create_table_command<Value>(command));

                });
            });

            Mapper::classes([&](auto class_info) {
                using ClassInfo = decltype(class_info);
                using Class = typename ClassInfo::Class;
                if (cu::array::contains(processed_classes, cu::class_name<Class>())) {
                    return;
                }
                result.push_back(create_table_command<Class>());
                });

            return result;
        }

        template <class T>
        static std::string insert_command(const T& obj) {
            static_assert(_exists<T>());
            std::string columns;
            std::string values;

            properties<T>([&](auto property) {
                if constexpr (!property.is_id) {
                    columns += property.name() + ", ";
                    values += database_value(property.get_value(obj)) + ",";
                }
            });

            columns.pop_back();
            columns.pop_back();
            values.pop_back();

            std::string command =
                "INSERT INTO " + class_name<T>() + " (" + columns + ")\n" +
                "VALUES(" + values + ");";

            Log << cu::log::Off(disable_log) << command;

            return command;
        }

        template <
            auto pointer,
            class Pointer = decltype(pointer),
            class Class = typename cu::pointer_to_member_class<Pointer>::type,
            class Value = typename cu::pointer_to_member_value<Pointer>::type>
        static std::string select_where_command(Value value) {

            std::string command = 
                "SELECT rowid, * FROM " + class_name<Class>() +
                " WHERE " + prop_name<pointer>() + " = " + database_value(value) + ";";

            Log << cu::log::Off(disable_log) << command;

            return command;
        }

        template <class Class>
        static std::string select_last_command() {
            static std::string command = 
                "SELECT rowid, * FROM " + class_name<Class>() + " ORDER BY rowid DESC LIMIT 1;";
            Log << cu::log::Off(disable_log) << command;
            return command;
        }

        template <
            auto pointer,
            class Pointer = cu::remove_all_t<decltype(pointer)>,
            class Class = typename cu::pointer_to_member_class<Pointer>::type,
            class Value = typename cu::pointer_to_member_value<Pointer>::type>
        static std::string delete_where_command(Value value) {
            std::string command = 
                "DELETE FROM " + class_name<Class>() +
                " WHERE " + prop_name<pointer>() + " = " + database_value(value) + ";";
            Log << cu::log::Off(disable_log) << command;
            return command;
        }

         template<class T>
         static std::string update_command(const T& object) {

            std::string command = "UPDATE " + class_name<T>() + " SET ";

            std::string where;

            properties<T>([&](auto prop) {
                auto value = database_value(prop.get_reference(object));
                if constexpr (prop.is_id) {
                    where = " WHERE " + prop_name(prop) + "=" + value;
                }
                else {
                    command += prop.name() + " = " + value + ", ";
                }
            });

            command.pop_back();
            command.pop_back();

            command += where;

            Log << cu::log::Off(disable_log) << command;

            return command;
        }

        template <class T>
        static T extract(SQLite::Statement& statement) {
            T result = Mapper::template create_empty<T>();
            int index = 0;
            properties<T>([&](auto property) {
                using Property = decltype(property);
                using Info = typename Property::ValueInfo;
                if constexpr (Info::is_string) {
                    Property::set_value(result, statement.getColumn(index).getString());
                }
                else if constexpr (Info::is_float) {
                    Property::set_value(result, statement.getColumn(index).getDouble());
                }
                else if constexpr (Info::is_integer) {
                    Property::set_value(result, statement.getColumn(index).getInt());
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
            std::string command = "SELECT rowid, * FROM " + class_name<T>();
            Log << cu::log::Off(disable_log) << command;
            return command;
        }

    private:

        template <class T>
        static constexpr bool _exists() { return mapper.template exists<T>(); }

        template <class T>
        static constexpr auto info() { return mapper.template info<T>(); }

        template <class T, class A>
        static constexpr void properties(A a) { info<T>().properties(a); }

        template <class T>
        static auto class_name() { return std::string(info<T>().name); }

        template <auto pointer>
        static std::string prop_name() {
            constexpr auto p = mapper.template property<pointer>();
            if constexpr (p.is_id) {
                return "rowid";
            }
            else {
                return std::string(p.name());
            }
        }

        template <class T>
        static std::string prop_name(T prop) {
            if constexpr (T::is_id) {
                return "rowid";
            }
            else {
                return std::string(prop.name());
            }
        }

        template <class T>
        static std::string database_type_name() {
            using Info = cu::FullTypeInfo<T>;
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
                Fatal("Invalid member: " + cu::class_name<T>());
            }
        }

        template <class T>
        static std::string database_value(const T& value) {
            using Info = cu::FullTypeInfo<T>;
            if constexpr (Info::is_string) {
                return "\'" + value + "\'";
            }
            else {
                return std::to_string(value);
            }
        }

    };

    template <class T> constexpr bool is_sqlite_mapper_v = std::is_base_of_v<is_sqlite_mapper_checker_base, cu::remove_all_t<T>>;

}
