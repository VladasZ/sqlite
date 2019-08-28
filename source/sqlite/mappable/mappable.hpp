#pragma once

#include <iostream>

#include "json.hpp"
#include "property.hpp"

#include "IterateTuple.hpp"

template<class T>
class Mappable {

    template <class Type>
    static constexpr bool is_mappable = std::is_base_of<Mappable<Type>, Type>::value;

    //Extraction

	template <class Member, class Property>
	static void extract(Member& member, const Property& property, const nlohmann::json& json) {
		member = json.value<Member>(property.name, property.default_value);
	}

	//Packing

	template <class Member, class Property>
	static void pack(const Member& member, const Property& property, nlohmann::json& json) {
		json[property.name] = member;
	}

public:
	static T parse(const nlohmann::json& json_to_parse) {
		T object;
		cu::iterate_tuple(T::properties(), [&](auto property) {
			extract(object.*property.pointer, property, json_to_parse);
		});
		return object;
	}

    static T parse_string(const std::string& json_string) {
		const nlohmann::json parsed_json = nlohmann::json::parse(json_string, nullptr, false);
        return parse(parsed_json);
    }

	template <class Object>
	static nlohmann::json to_json(const Object& object) {
		nlohmann::json json;

        cu::iterate_tuple(T::properties(), [&](auto property) {
			pack(object.*property.pointer, property, json);
		});

		return json;
	}

	template <class Object>
	static std::string to_json_string(const Object& object) {
		return to_json(object).dump();
	}

    static void printProperties() {
        cu::iterate_tuple(T::properties(), [](auto property) {
            std::cout << property << std::endl;
        });
    }
};
