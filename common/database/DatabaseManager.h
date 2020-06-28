#pragma once

#include <sqlite3.h>
#include <functional>
#include <vector>
#include "../easylogging/easylogging++.h"

enum class DatabaseState : unsigned char {
	StartupNew,
	StartupUpdate,
	Startup,
	Shutdown
};

typedef std::function<void(DatabaseState state)> DatabaseStartupCallbackFunction;

class ConfigManager;
class TableField;
class SqlStatement;

class DatabaseManager {
	el::Logger* logger_;
	sqlite3* db_;
	std::string databaseFile_;
	static std::string InternPrepareSQL(const char *format, va_list args);
	std::vector<DatabaseStartupCallbackFunction> callbackFunctions_;

	void CreateTableVersion() const;
	void CheckSchema() const;
	void CheckDabaseOk();
public:
	explicit DatabaseManager(const std::string& path, const std::string& name);

	bool ExitsTable(const char* tableName) const;
	void DoDml(const std::string& statementText) const;
	void CreateTable(const std::string &tableName, const std::vector<TableField> &fields) const;
	SqlStatement* CreateQuery(std::string queryText) const;
	std::string GetVersionInfo(std::string subsystem) const;
	static std::string PrepareSQL(const std::string, ...);
	long GetIdFromStringKey(std::string table, std::string column, std::string value) const;
	long GetCountFromTable(std::string table, std::string column, std::string value, std::string column2, std::string value2) const;
	void SaveSmallImageDatabase(std::string fileName);
	void AddStateCallBack(DatabaseStartupCallbackFunction callback);

	void Init();
	void Deinit();
	
};