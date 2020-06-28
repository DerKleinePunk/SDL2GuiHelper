#pragma once
#include <string>

enum class DatabaseFieldTypes : unsigned char {
	VARCHAR,
	INTEGER,
	TIMESTAMP,
	DOUBLE,
	BLOB
};


class TableField
{
	bool key_;
	DatabaseFieldTypes type_;
public:
	std::string name_;
	std::string value;
	std::string adddeftext_;
	bool notNull_;
	bool autoinc_;
	int size_;

	TableField();
	void AddToKey();
	void NoKey();
	bool IsKeyField() const;
	void SetType(DatabaseFieldTypes type);
	std::string GetTypeText() const;
	std::string GetTypeDefText() const;
};
