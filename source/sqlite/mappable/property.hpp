#pragma once

#include <iostream>
#include <map>

#include "meta.hpp"

template<class _ClassType, class _MemberType>
class Property {
public:
	using ClassType = _ClassType;
	using MemberType = _MemberType;
    using Pointer = MemberType ClassType::*;
    const std::string name;
	const std::string class_name = typeid(_ClassType).name();
    const Pointer pointer;
	const MemberType default_value;
    Property(const std::string &name, Pointer pointer, const MemberType& default_value)
    :
    name(name), pointer(pointer), default_value(default_value) { }
};

template<class _ClassType, class MemberType, class _DefaultType>
static const auto make_property(
        const std::string& name,
        MemberType _ClassType::* pointer,
        const _DefaultType& default_value) {
    return Property<_ClassType, MemberType>(name, pointer, default_value);
}

template<class ClassType, class MemberType>
std::ostream& operator<<(std::ostream& os, const Property<ClassType, MemberType>& obj) {
    return os << "Property: " << obj.name << " of: " << typeid(ClassType).name();
}

#define PROPERTY(name, type, default_value) make_property(#name,  &type::name, default_value)
