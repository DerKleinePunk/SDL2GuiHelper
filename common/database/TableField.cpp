#include "TableField.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const DatabaseFieldTypes type)
{
	switch (type)
	{
	case DatabaseFieldTypes::INTEGER: os << "INTEGER"; break;
	case DatabaseFieldTypes::VARCHAR: os << "VARCHAR"; break;
	case DatabaseFieldTypes::TIMESTAMP: os << "TIMESTAMP"; break;
	case DatabaseFieldTypes::DOUBLE: os << "DOUBLE"; break;
	case DatabaseFieldTypes::BLOB: os << "BLOB"; break;
	default: os.setstate(std::ios_base::failbit);
	}
	return os;
}

TableField::TableField():
	key_(false), type_(DatabaseFieldTypes::VARCHAR),
	notNull_(false),
	autoinc_(false),
	size_(-1) {

}

void TableField::AddToKey() {
	key_ = true;
	notNull_ = true;
}

void TableField::NoKey() {
	key_ = false;
	notNull_ = false;
}

bool TableField::IsKeyField() const {
	return  key_;
}

void TableField::SetType(DatabaseFieldTypes type) {
	type_ = type;
}

std::string TableField::GetTypeText() const {
	std::string result("");
	switch (type_)
	{
		case DatabaseFieldTypes::INTEGER: 
			result = "INTEGER"; 
			break;
		case DatabaseFieldTypes::VARCHAR:
			result = "VARCHAR(" + std::to_string(size_) + ")"; 
			break;
		case DatabaseFieldTypes::TIMESTAMP:
			result = "TIMESTAMP";
			break;
		case DatabaseFieldTypes::DOUBLE:
			result = "DOUBLE";
			break;
		case DatabaseFieldTypes::BLOB:
			result = "BLOB";
			break;
		default: 
			throw std::runtime_error{ "FieldType" };;
	}
	return result;
}

std::string TableField::GetTypeDefText() const {
	auto result = name_;
	result += " ";
	result += GetTypeText();
	return result;
}
