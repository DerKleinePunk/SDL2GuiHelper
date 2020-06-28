#include "Statement.h"
#include "../exception/SqliteException.h"

SqlStatement::SqlStatement(sqlite3* db, sqlite3_stmt* stmt): 
	eof_(false) {
	db_ = db;
	stmt_ = stmt;
	paramcount_ = 0;
}

SqlStatement::~SqlStatement() {
	sqlite3_clear_bindings(stmt_);
	sqlite3_finalize(stmt_);
}

void SqlStatement::checkEof() {
	auto result = sqlite3_step(stmt_);
	if (result == SQLITE_ROW) {
		eof_ = false;
	} else if(result == SQLITE_DONE) { //No Rows
		eof_ = true;
	} else {
		throw SqliteException(sqlite3_errmsg(db_));
	}
}

int SqlStatement::Execute() {
	if(sqlite3_bind_parameter_count(stmt_) != paramcount_) {
		return -1;
	}
	checkEof();
	if(eof_) return 0;
	return 1;
}

bool SqlStatement::Eof() const {
	return eof_;
}

bool SqlStatement::NextRow() {
	checkEof();
	return eof_;
}

int SqlStatement::BindBlob(const int pos, const unsigned char* buffer, const int size) {
	const auto result = sqlite3_bind_blob(stmt_, pos, buffer, size, SQLITE_TRANSIENT);
	if(result != SQLITE_OK) {
		throw SqliteException(sqlite3_errmsg(db_));
	}
	paramcount_++;
	return result;
}

int SqlStatement::BindInt(const int pos, const int value) {
	const auto result = sqlite3_bind_int(stmt_, pos, value);
	if (result != SQLITE_OK) {
		throw SqliteException(sqlite3_errmsg(db_));
	}
	paramcount_++;
	return result;
}

std::string SqlStatement::GetColumnText(const int column) const {
	const auto data = sqlite3_column_blob(stmt_, column);
	return  std::string(static_cast<const char *>(data), sqlite3_column_bytes(stmt_, column));
}

int SqlStatement::GetColumnInt(int column) {
	return sqlite3_column_int(stmt_, column);
}

int SqlStatement::GetColumnBlob(int column, const void*& data) {
	data = sqlite3_column_blob(stmt_, column);
	return sqlite3_column_bytes(stmt_, column);
}
