//
//  Column.hpp
//  sqlite
//
//  Created by Vladas Zakrevskis on 8/27/19.
//  Copyright © 2019 VladasZ. All rights reserved.
//


#pragma once

#include <string>

#include "sqlite3.h"

#include "Log.hpp"
#include "SQLiteMappable.hpp"

namespace  sql {

    class Column {

        unsigned _index;
        sqlite3_stmt* _stmt = nullptr;

        std::string _name;

    public:

        Column() = default;
        Column(unsigned index, const std::string& name);

        std::string name() const;

        std::string string() const;
        int integer() const;
        float floating() const;

        std::string to_string() const;

        template<class Property, class Object>
        void set_property(Object& object, const Property& property, sqlite3_stmt* stmt);

        template<class Object>
        void set_id(Object& object, sqlite3_stmt* stmt);

    };

    template<class Property, class Object>
    void Column::set_property(Object& object, const Property& property, sqlite3_stmt* stmt) {

        static_assert(mapping::is_sqlite_mappable<Object>);

        _stmt = stmt;

        auto& value = object.*property.pointer;

        if constexpr (Property::is_string) {
            value = string();
        }
        else if (Property::is_integer) {
            value = integer();
        }
        else if (Property::is_float) {
            value = floating();
        }
        else {
            Fatal("Invalid property");
        }
    }

    template<class Object>
    void Column::set_id(Object& object, sqlite3_stmt* stmt) {
        static_assert(mapping::is_sqlite_mappable<Object>);
        _stmt = stmt;
        object.id = integer();
    }
}
