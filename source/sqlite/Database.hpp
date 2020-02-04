//
//  Database.hpp
//  sqlite
//
//  Created by Vladas Zakrevskis on 15/01/20.
//  Copyright © 2020 VladasZ. All rights reserved.
//

#pragma once

#include "SQLiteMapper.hpp"

namespace sql {

    template <auto& sqlite_mapper>
    class Database {

        using Mapper = cu::remove_all_t<decltype(sqlite_mapper)>;

        static_assert(mapping::is_sqlite_mapper_v<Mapper>);

        const std::string_view path;

    public:

        Database() = default;
        constexpr Database(const std::string_view& path) : path(path) {

        }


    };

}
