#pragma once
#include <string>
#include "sqlite3.h"


class SqlStatement {
	friend class DatabaseManager;
	sqlite3_stmt* stmt_;
	sqlite3* db_;
	std::string text;
	int paramcount_;
	bool eof_;	
protected:
	explicit SqlStatement(sqlite3* db, sqlite3_stmt* stmt);

public:
	~SqlStatement();
	void checkEof();

	int Execute();
	bool Eof() const;
	bool NextRow();
	int BindBlob(int pos, const unsigned char* buffer, int size);
	int BindInt(int pos, int value);
	std::string GetColumnText(int i) const;
	int GetColumnInt(int i);
	int GetColumnBlob(int column, const void*& data);
};
