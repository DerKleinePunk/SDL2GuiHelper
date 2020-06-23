#pragma once
#include <vector>
#include <string>
#include <functional>
#include <cstdarg>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include <memory>

#ifdef USESDL
    #include <SDL.h>
#endif


#ifdef _WIN32
#define DIRCHAR '\\'
#include <tchar.h>
#else 
#define DIRCHAR '/'
#endif

namespace utils
{
	class stringf {
		char* chars_;
	public:
		stringf(const char* format, std::va_list arg_list);
		~stringf();
		const char* get() const;
	};

	std::string str_tolower(std::string data);
	std::string str_toupper(std::string data);
	bool FileExists(const std::string &Filename);
	bool hasEnding(std::string const &fullString, std::string const &ending);
	std::string getFileName(std::string const &pathString);
	std::string exec(const char* cmd, int& resultCode);
	FILE *MNFmemopen(const void *buffer, size_t size, const char* mode);
	std::string GetHomePath();
	void replaceAll(std::string& str, const std::string& from, const std::string& to);
	std::string GetPrintAbleString(const std::string& text);
	std::string uchar2hex(unsigned char inchar);
	std::string u162hex(uint16_t inchar);
	std::ostream& formatDateTime(std::ostream& out, const tm& t, const char* fmt);
	std::string dateTimeToString();
	std::string dateTimeToString(const time_t& t);
	std::string dateTimeToString(const tm& t, const char* format);
	std::string dateTimeToJsonString(const timeval& t);
	tm now();
	tm utcNow();
	
	template<typename ... Args>
	std::string string_format( const std::string& format, Args ... args )
	{
		size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
		if( size <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
		std::unique_ptr<char[]> buf( new char[ size ] ); 
		snprintf( buf.get(), size, format.c_str(), args ... );
		return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
	}

	template <typename T>
	struct Callback;

	template <typename Ret, typename... Params>
	struct Callback<Ret(Params...)> {
		template <typename... Args>
		static Ret callback(Args... args) { return func(args...); }
		static std::function<Ret(Params...)> func;
	};

	// Initialize the static member.
	template <typename Ret, typename... Params>
	std::function<Ret(Params...)> Callback<Ret(Params...)>::func;
}
