#pragma once

#include <type_traits>
#include <iostream>

#include "meta.hpp"
#include "property.hpp"
#include "json.hpp"


template<class T>
class Mappable {

    template <class Type>
    static constexpr bool is_mappable = std::is_base_of<Mappable<Type>, Type>::value;

    template <class Type>
    using if_mappable = typename std::enable_if<is_mappable<Type> && !is_vector<Type>::value>::type;

    template <class Type>
    using if_not_mappable = typename std::enable_if<!is_mappable<Type> && !is_vector<Type>::value>::type;

    //Extraction

	template <class Member, class Property>
	static if_mappable<Member> extract(Member& member, const Property& property, const nlohmann::json& json_to_parse) {
		if (json_to_parse.find(property.name) == json_to_parse.end())
		{
			for_each(Member::properties(), [&](auto member_property) {
				member.*member_property.pointer = member_property.default_value;
			});
			return;
		}

		member = Member::parse(json_to_parse[property.name]);
	}

	template <class Member, class Property>
	static if_not_mappable<Member> extract(Member& member, const Property& property, const nlohmann::json& json_to_parse) {
		member = json_to_parse.value<Member>(property.name, property.default_value);
	}

	template <class Member, class Property>
	static if_vector<Member> extract(Member& member, const Property& property, const nlohmann::json& json_to_parse) {
		extract_vector(member, property, json_to_parse);
	}

	template <class VectorType, class Property>
	static if_mappable<VectorType> extract_vector(
	        std::vector<VectorType>& vector,
	        const Property& property,
	        const nlohmann::json& json_to_parse) {
		if (!json_to_parse.is_array())
		{
			vector = property.default_value;
			return;
		}

		vector.clear();

		for (const auto& object_json : json_to_parse)
			vector.emplace_back(VectorType::parse(object_json));
	}

	template <class VectorType, class Property>
	static if_not_mappable<VectorType> extract_vector(
	        std::vector<VectorType>& vector,
	        const Property& property,
	        const nlohmann::json& json_to_parse) {
		vector = json_to_parse.value<std::vector<VectorType>>(property.name, property.default_value);
	}

	//Packing

	template <class Member, class Property>
	static if_not_mappable<Member> pack(const Member& member, const Property& property, nlohmann::json& json_to_pack) {
		json_to_pack[property.name] = member;
	}

	template <class Member, class Property>
	static if_mappable<Member> pack(const Member& member, const Property& property, nlohmann::json& json_to_pack) {
		json_to_pack[property.name] = Member::to_json(member);
	}

	template <class Member, class Property>
	static if_vector<Member> pack(const Member& member, const Property& property, nlohmann::json& json_to_pack) {
		pack_vector(member, property, json_to_pack);
	}

	template <class VectorType, class Property>
	static if_not_mappable<VectorType> pack_vector(
	        const std::vector<VectorType>& vector,
	        const Property& property,
	        nlohmann::json& json_to_pack) {
		json_to_pack[property.name] = nlohmann::json::array();

		for (const auto& object : vector)
			json_to_pack[property.name].emplace_back(object);
	}

	template <class VectorType, class Property>
	static if_mappable<VectorType> pack_vector(
	        const std::vector<VectorType>& vector,
	        const Property& property,
	        nlohmann::json& json_to_pack) {

		json_to_pack[property.name] = nlohmann::json::array();

		for (const auto& object : vector)
			json_to_pack[property.name].emplace_back(VectorType::to_json(object));
	}

public:
	static T parse(const nlohmann::json& json_to_parse) {
		T object;
		for_each(T::properties(), [&](auto property) {
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

		for_each(T::properties(), [&](auto property) {
			pack(object.*property.pointer, property, json);
		});

		return json;
	}

	template <class Object>
	static std::string to_json_string(const Object& object) {
		return to_json(object).dump();
	}

    static void printProperties() {
        for_each(T::properties(), [](auto property) {
            std::cout << property << std::endl;
        });
    }
};
