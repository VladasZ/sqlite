//
//  TestSQLModel.hpp
//  mapping
//
//  Created by Vladas Zakrevskis on 21/21/2020.
//  Copyright © 2020 VladasZ. All rights reserved.
//

#pragma once

#include "Mapper.hpp"
#include "SQLiteMapper.hpp"


namespace sql {

	struct Student {
		int age;
		std::string name;
	};

	struct Teacher {
		int id;
		int age;
		std::string name;
		Student pet;
	};

	MAKE_CLASS_INFO(Student,
		MAKE_PROPERTY("age",  &Student::age),
		MAKE_PROPERTY("name", &Student::name)
		);

	MAKE_CLASS_INFO(Teacher,
		MAKE_ID_PROPERTY(&Teacher::id),
		MAKE_PROPERTY("age", &Teacher::age),
		MAKE_PROPERTY("name", &Teacher::name),
		MAKE_PROPERTY("pet", &Teacher::pet)
	);

	MAKE_MAPPER(mapper,
		InfoOfStudent,
		InfoOfTeacher);

	static constexpr auto sql_mapper = sql::SQLiteMapper<mapper>();

}
