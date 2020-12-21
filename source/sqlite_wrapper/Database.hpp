//
//  Database.hpp
//  sqlite
//
//  Created by Vladas Zakrevskis on 15/01/20.
//  Copyright © 2020 VladasZ. All rights reserved.
//

#pragma once

#include <string>

#include "Result.hpp"
#include "SQLiteMapper.hpp"


namespace sql {

    template <auto& _mapper>
    class Database {

        using SQLMapper = cu::remove_all_t<decltype(_mapper)>;
        using Mapper = typename SQLMapper::Mapper;

        static_assert(is_sqlite_mapper_v<SQLMapper>);

        const std::string name;

    public:

        static constexpr auto mapper = _mapper;

        SQLite::Database db;

    public:

        Database(const std::string& name) : name(name), db(name,  SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
            Log << "Opening database:" << name;
            _create_tables();
        };

        int execute(const std::string& command) { return db.exec(command); }

        template <class T>
        int insert(const T& obj) {
            return execute(mapper.insert_command(obj));
        }

        template <class T>
        std::vector<T> get_all() {
            SQLite::Statement query(db, SQLMapper::select_all_command<T>());
            std::vector<T> result;
            while (query.executeStep()) {
                result.push_back(SQLMapper::extract<T>(query));
            }
            return result;
        }

        template <auto& json_mapper>
        void dump_all() {
            Mapper::classes([&](auto class_info) {
                using ClassInfo = decltype(class_info);
                using Class = typename ClassInfo::Class;
                std::cout << class_info.name << ":" << std::endl;
                auto all = get_all<Class>();
                for (auto obj : all) {
                    std::cout << json_mapper.to_json_string(obj) << std::endl;
                }
                std::cout << std::endl;
            });
        }

        template <class Class, class Result = cu::Result<Class>>
        Result get_last_entry() {
            SQLite::Statement query(db, mapper.template select_last_command<Class>());
            if (query.executeStep()) {
                return mapper.template extract<Class>(query);
            }
            return { };
        }

        template <auto pointer,
                  class Pointer = decltype(pointer),
                  class Class = typename cu::pointer_to_member_class<Pointer>::type,
                  class Result = cu::Result<Class>,
                  class Value = typename cu::pointer_to_member_value<Pointer>::type>
        Result select_first_where(Value value) {
            SQLite::Statement query(db, mapper.template select_where_command<pointer>(value));
            if (query.executeStep()) {
                return mapper.template extract<Class>(query);
            }
            return { };
        }

        template <auto pointer,
                  class Pointer = decltype(pointer),
                  class Class = typename cu::pointer_to_member_class<Pointer>::type,
                  class Value = typename cu::pointer_to_member_value<Pointer>::type>
        auto select_where(Value value) {
            std::vector<Class> result;
            SQLite::Statement query(db, mapper.template select_where_command<pointer>(value));
            while (query.executeStep()) {
                result.push_back(mapper.template extract<Class>(query));
            }
            return result;
        }

        template <auto pointer,
                class Pointer = decltype(pointer),
                class Class = typename cu::pointer_to_member_class<Pointer>::type,
                class Value = typename cu::pointer_to_member_value<Pointer>::type>
        auto delete_where(Value value) {
            execute(mapper.template delete_where_command<pointer>(value));
        }

        template <auto pointer,
                class Pointer = decltype(pointer),
                class Class = typename cu::pointer_to_member_class<Pointer>::type,
                class Value = typename cu::pointer_to_member_value<Pointer>::type>
        auto update_where(Value value, const Class& object) {
            execute(mapper.template update_where_command<pointer>(value, object));
        }

    private:

        void _create_tables() {
            auto commands = mapper.create_all_tables_commands();
            for (auto command : commands) {
                if (execute(command)) {
                    Fatal(std::string() + "Failed to execute command: " + command);
                }
            }
        }

    };

}
