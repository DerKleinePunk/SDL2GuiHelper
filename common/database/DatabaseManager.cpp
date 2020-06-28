#ifndef ELPP_DEFAULT_LOGGER
#   define ELPP_DEFAULT_LOGGER "DatabaseManager"
#endif
#ifndef ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID
#   define ELPP_CURR_FILE_PERFORMANCE_LOGGER_ID ELPP_DEFAULT_LOGGER
#endif

#ifdef NVWAMEMCHECK
    //#include "../nvwa/debug_new.h"
#endif

//http://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
//D:\Mine\C++\CppSQLite3_2\Common


#include "DatabaseManager.h"
#include "../exception/SqliteException.h"
#include "TableField.h"
#include "Statement.h"
#include "../exception/NotImplementedException.h"
#include "../utils/commonutils.h"
#include <AppConfig.h>

static int callback(void *pointerFromCaller, int argc, char **argv, char **azColName) {
	for (auto i = 0; i<argc; i++) {
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

std::ostream& operator<<(std::ostream& os, const DatabaseState state)
{
	switch (state)
	{
		case DatabaseState::Startup: os << "Startup";    
			break;
		case DatabaseState::StartupNew: os << "StartupNew";    
			break;
		case DatabaseState::StartupUpdate: os << "StartupUpdate";    
			break;
		case DatabaseState::Shutdown: os << "Shutdown"; 
			break;
		default: 
			os.setstate(std::ios_base::failbit);
	}
	return os;
}


DatabaseManager::DatabaseManager(const std::string& path, const std::string& name): 
	db_(nullptr) {
	logger_ = el::Loggers::getLogger(ELPP_DEFAULT_LOGGER);
	databaseFile_ = path;
	databaseFile_ += name +".db";
}

bool DatabaseManager::ExitsTable(const char* tableName) const {
	auto exits = false;
	sqlite3_stmt* statement;
	if (sqlite3_prepare_v2(db_, "SELECT name FROM sqlite_master WHERE type = \'table\' AND name = ?", -1, &statement, nullptr) != SQLITE_OK) {
		sqlite3_finalize(statement);
		throw SqliteException(sqlite3_errmsg(db_));
	}
	if (sqlite3_bind_text(statement, 1, tableName, -1, nullptr) != SQLITE_OK) {
		sqlite3_finalize(statement);
		throw SqliteException(sqlite3_errmsg(db_));
	}
	if (sqlite3_step(statement) == SQLITE_ROW) {
		exits = true;
	}
	sqlite3_finalize(statement);
	return exits;
}

void DatabaseManager::DoDml(const std::string& statementText) const {
	LOG(DEBUG) << statementText;
	char *zErrMsg = nullptr;
	if (sqlite3_exec(db_, statementText.c_str(), callback, nullptr, &zErrMsg) != SQLITE_OK) {
		auto errorMessage = statementText;
		errorMessage += "\n";
		errorMessage += zErrMsg;
		sqlite3_free(zErrMsg);
		throw SqliteException(errorMessage);
	}
	LOG(DEBUG) << sqlite3_changes(db_) << " Rows affected";
}

void DatabaseManager::CreateTable(const std::string &tableName, const std::vector<TableField> &fields) const {
	std::string statementText = "create table ";
	auto keyText = "CONSTRAINT " + tableName + "_PK PRIMARY KEY (";
	auto addKeyText = false;
	statementText += tableName;
	statementText += " (\n";
	auto field = fields.begin();
	while (field != fields.end()) {
		statementText += field->GetTypeDefText();
		if (field->autoinc_) {
			statementText += " PRIMARY KEY AUTOINCREMENT";
		}
		if (field->notNull_) {
			statementText += " NOT NULL";
		}
		if (field->adddeftext_.size() > 0) {
			statementText += field->adddeftext_;
		}
		if (field->IsKeyField()) {
			if (addKeyText) {
				keyText += ", ";
			}
			keyText += field->name_;
			addKeyText = true;
		}
		if (std::prev(fields.end()) == field) {
			if (addKeyText) {
				keyText += ")";
				statementText += ",\n" + keyText;
			}
			statementText += " )";
		}
		else {
			statementText += " ,\n";
		}
		++field;
	}

	DoDml(statementText);
}

SqlStatement* DatabaseManager::CreateQuery(std::string queryText) const {
	sqlite3_stmt* statement;
	if (sqlite3_prepare_v2(db_, queryText.c_str(), -1, &statement, nullptr) != SQLITE_OK) {
		sqlite3_finalize(statement);
		throw SqliteException(sqlite3_errmsg(db_));
	}

	return new SqlStatement(db_, statement);
}

void DatabaseManager::CreateTableVersion() const {
	std::vector<TableField> fields;

	TableField firstField;
	firstField.name_ = "SUBSYSTEM";
	firstField.SetType(DatabaseFieldTypes::VARCHAR);
	firstField.size_ = 100;
	firstField.AddToKey();

	fields.push_back(firstField);

	TableField secondField;
	secondField.name_ = "VERSION";
	secondField.SetType(DatabaseFieldTypes::VARCHAR);
	secondField.size_ = 15; //00.00.00.0000
	secondField.notNull_ = true;

	fields.push_back(secondField);

	CreateTable("VERSION", fields);

	const auto statementText = "insert into version values ('DATABASE','" + DATABASE_SCHEMA_VERSION + "')";
	DoDml(statementText);

	//DoDml("commit");
}

void DatabaseManager::CheckSchema() const {
	const auto verionReal = GetVersionInfo("DATABASE");
	if (verionReal != DATABASE_SCHEMA_VERSION) {
		for (const auto callback_function : callbackFunctions_) {
			callback_function(DatabaseState::StartupUpdate);
		}
		throw NotImplementedException("Database Schema Update");
	}
}

std::string DatabaseManager::GetVersionInfo(std::string subsystem) const {
	std::string result;
	sqlite3_stmt* statement;
	if (sqlite3_prepare_v2(db_, "SELECT VERSION FROM VERSION WHERE SUBSYSTEM = ?", -1, &statement, nullptr) != SQLITE_OK) {
		sqlite3_finalize(statement);
		throw SqliteException(sqlite3_errmsg(db_));
	}
	if (sqlite3_bind_text(statement, 1, subsystem.c_str(), -1, nullptr) != SQLITE_OK) {
		sqlite3_finalize(statement);
		throw SqliteException(sqlite3_errmsg(db_));
	}
	if (sqlite3_step(statement) == SQLITE_ROW) {
		// Note: using sqlite3_column_blob and not sqlite3_column_text
		// - no need for sqlite3_column_text to add a \0 on the end, as we're getting the bytes length directly
		auto data = sqlite3_column_blob(statement, 0);

		// SQLite docs: "The safest policy is to invoke
        // sqlite3_column_blob() followed by sqlite3_column_bytes()"
		// Note: std::string is ok to pass nullptr as first arg, if length is 0
		result = std::string(static_cast<const char *>(data), sqlite3_column_bytes(statement, 0));
	}
	sqlite3_finalize(statement);
	return result;
}

std::string DatabaseManager::PrepareSQL(const std::string strStmt, ...) {

	va_list args;
	va_start(args, strStmt);
	auto strResult = InternPrepareSQL(strStmt.c_str(), args);
	va_end(args);

	return strResult;
}

long DatabaseManager::GetIdFromStringKey(std::string table, std::string column, std::string value) const {
	auto statementText = PrepareSQL("select id from %s where %s = '%s'", table.c_str(), column.c_str(), value.c_str());

	long result = -1;
	sqlite3_stmt* statement;
	if (sqlite3_prepare_v2(db_, statementText.c_str(), -1, &statement, nullptr) != SQLITE_OK) {
		sqlite3_finalize(statement);
		throw SqliteException(sqlite3_errmsg(db_));
	}

	if (sqlite3_step(statement) == SQLITE_ROW) {
		result = sqlite3_column_int(statement, 0);
	}
	sqlite3_finalize(statement);

	return result;
}

long DatabaseManager::GetCountFromTable(std::string table, std::string column, std::string value, std::string column2, std::string value2) const {
	auto statementText = PrepareSQL("select count(*) from %s where %s = '%s'", table.c_str(), column.c_str(), value.c_str());
	if(column2.size() > 0) {
		statementText = PrepareSQL("select count(*) from %s where %s = '%s' and %s = '%s'", table.c_str(), column.c_str(), value.c_str(), column2.c_str(), value2.c_str());
	}

	long result = -1;
	sqlite3_stmt* statement;
	if (sqlite3_prepare_v2(db_, statementText.c_str(), -1, &statement, nullptr) != SQLITE_OK) {
		sqlite3_finalize(statement);
		throw SqliteException(sqlite3_errmsg(db_));
	}

	if (sqlite3_step(statement) == SQLITE_ROW) {
		result = sqlite3_column_int(statement, 0);
	}
	sqlite3_finalize(statement);

	return result;
}

void DatabaseManager::SaveSmallImageDatabase(std::string fileName) {
	if(GetCountFromTable("IMAGE32X32", "NAME", utils::getFileName(fileName), "", "") == 1) {
		LOG(DEBUG) << "Image always in Database";
		return;
	}

	auto strSQL = PrepareSQL("INSERT INTO IMAGE32X32 (NAME,DATA) VALUES('%s', ?)", utils::getFileName(fileName).c_str());
	auto query = CreateQuery(strSQL);

	FILE* stream = fopen(fileName.c_str(), "rb");
	fseek(stream, 0, SEEK_END);
	auto size = ftell(stream);
	rewind(stream);
	auto buffer = new unsigned char[size];
	fread(buffer, 1, size, stream);
	query->BindBlob(1, buffer, size);
	fclose(stream);
	auto result = query->Execute();
	if ( result == SQLITE_OK) {
		remove(fileName.c_str());
	}
	delete[] buffer;
	delete query;
}
void DatabaseManager::AddStateCallBack(const DatabaseStartupCallbackFunction callback) {
	callbackFunctions_.push_back(callback);
}

void DatabaseManager::Init() {
	const auto result = sqlite3_open_v2(databaseFile_.c_str(), &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
	if (result != SQLITE_OK) {
		throw SqliteException(sqlite3_errmsg(db_));
	}
	CheckDabaseOk();

	LOG(INFO) << "Database open";
}

void DatabaseManager::Deinit() {
	for (const auto callbackFunction : callbackFunctions_) {
		callbackFunction(DatabaseState::Shutdown);
	}
	if (db_) {
		sqlite3_close(db_);
		LOG(INFO) << "Database closed";
		db_ = nullptr;
	}
}

std::string DatabaseManager::InternPrepareSQL(const char* format, va_list args) {
	//Thanks to XBMC Coders
	std::string strFormat = format;
	std::string strResult = "";

	//  %q is the sqlite format string for %s.
	//  Any bad character, like "'", will be replaced with a proper one
	size_t pos = 0;
	while ((pos = strFormat.find("%s", pos)) != std::string::npos)
		strFormat.replace(pos++, 2, "%q");

	//  the %I64 enhancement is not supported by sqlite3_vmprintf
	//  must be %ll instead
	pos = 0;
	while ((pos = strFormat.find("%I64", pos)) != std::string::npos)
		strFormat.replace(pos++, 4, "%ll");

	const auto p = sqlite3_vmprintf(strFormat.c_str(), args);
	if (p)
	{
		strResult = p;
		sqlite3_free(p);
	}

	return strResult;
}

void DatabaseManager::CheckDabaseOk() {
	DoDml("PRAGMA foreign_keys=on;");
	DoDml("PRAGMA cache_size = 100000;");
    DoDml("PRAGMA encoding = \"UTF-8\";");
	if (!ExitsTable("VERSION")) {
		CreateTableVersion();
		for (const auto callbackFunction : callbackFunctions_) {
			callbackFunction(DatabaseState::StartupNew);
		}
	} else {
		CheckSchema();
	}
}
