//
//  TestSQLModel.hpp
//  mapping
//
//  Created by Vladas Zakrevskis on 21/21/2020.
//  Copyright © 2020 VladasZ. All rights reserved.
//

#pragma once

#include "Mapper.hpp"
#include "JsonMapper.hpp"
#include "SQLiteMapper.hpp"


namespace sql {

	struct Dog {
		int id;
		int age;
		std::string name;
	};

	struct Student {
		int id;
		int age;
		std::string name;
	};

	struct Teacher {
		int id;
		int age;
		std::string name;
		Student pet;
		Student shket;
		Student lepet;
		Dog bobik;
		Dog smogik;
	};

	MAKE_CLASS_INFO(Dog,
		MAKE_ID_PROPERTY(Dog, id),
		MAKE_PROPERTY(Dog, age),
		MAKE_PROPERTY(Dog, name)
	);

	MAKE_CLASS_INFO(Student,
		MAKE_ID_PROPERTY(Student, id),
		MAKE_PROPERTY(Student, age),
		MAKE_PROPERTY(Student, name)
	);

	MAKE_CLASS_INFO(Teacher,
		MAKE_ID_PROPERTY(Teacher, id),
		MAKE_PROPERTY(Teacher, age),
		MAKE_PROPERTY(Teacher, name),
		MAKE_PROPERTY(Teacher, pet),
		MAKE_PROPERTY(Teacher, shket),
		MAKE_PROPERTY(Teacher, lepet),
		MAKE_PROPERTY(Teacher, bobik),
		MAKE_PROPERTY(Teacher, smogik)
	);

	MAKE_MAPPER(mapper,
		InfoOfDog,
		InfoOfStudent,
		InfoOfTeacher);

	static constexpr auto sql_mapper  = sql::SQLiteMapper<mapper>();
	static constexpr auto json_mapper = mapping::JSONMapper<mapper>();

}
