/**
 *  \file commonutils.cpp
 *  \brief collection of helper funktions
 */

#include "commonutils.h"
#include <stdio.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#include <io.h>
#include "dirent.h"
#define access _access_s
#define popen _popen
#define pclose _pclose
#else
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/time.h>
#endif


namespace utils
{
/// Emulate printf to const char*
stringf::stringf(const char* format, std::va_list arg_list)
{
    const std::size_t start_len = 1024;

    chars_ = new char[start_len];

    const size_t needed = vsnprintf(this->chars_, start_len, format, arg_list) + 1;
    if(needed <= start_len) {
        return;
    }

    // need more space...
    delete[] this->chars_;

    this->chars_ = new char[needed];
    vsnprintf(this->chars_, needed, format, arg_list);
}

stringf::~stringf()
{
    delete[] this->chars_;
}

const char* stringf::get() const
{
    return chars_;
}

/**
 *  \brief Check File Extis Fast
 *
 *  \param [in] Filename the Filename
 *  \return bool exists
 *
 *  \details Check File Exists via Rigths
 */
bool FileExists(const std::string& Filename)
{
    return access(Filename.c_str(), 0) == 0;
}

std::string str_tolower(std::string data)
{
    std::transform(data.begin(), data.end(), data.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return data;
}

std::string str_toupper(std::string data)
{
    std::transform(data.begin(), data.end(), data.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return data;
}

bool hasEnding(std::string const& fullString, std::string const& ending)
{
    if(fullString.length() >= ending.length()) {
        auto fullStringLow = str_tolower(fullString);
        auto endingLow = str_tolower(ending);
        return 0 == fullStringLow.compare(fullStringLow.length() - endingLow.length(),
                                          endingLow.length(), endingLow);
    } else {
        return false;
    }
}

std::string getFileName(const std::string& pathString)
{
    auto const pos = pathString.find_last_of(DIRCHAR);
    if(pos == std::string::npos) return pathString;
    return pathString.substr(pos + 1, pathString.length() - (pos + 1));
}

std::string exec(const char* cmd, int& resultCode)
{
    char buffer[128];
    std::string result = "";
    auto pipe = popen(cmd, "r");
    if(!pipe) throw std::runtime_error("popen() failed!");
    try {
        while(!feof(pipe)) {
            if(fgets(buffer, 128, pipe) != nullptr) result += buffer;
        }
    } catch(...) {
        resultCode = pclose(pipe);
        throw;
    }
    resultCode = pclose(pipe);
    return result;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    if(from.empty()) return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

std::string GetPrintAbleString(const std::string& text)
{
    auto result = text;
	replaceAll(result, "<", "<3C>");
	replaceAll(result, "<", "<3E>");
    for(unsigned char code = 0; code < 0x20; code++) {
        std::string zeichen("");
        zeichen += (char)code;
        std::string zeichenReplace("<");
        zeichenReplace += uchar2hex(code);
        zeichenReplace += ">";
        replaceAll(result, zeichen, zeichenReplace);
    }
    return result;
}

std::string uchar2hex(unsigned char inchar)
{
    std::ostringstream oss(std::ostringstream::out);
    oss << std::setw(2) << std::setfill('0') << std::hex << (int)(inchar);
    return utils::str_toupper(oss.str());
}

std::string u162hex(uint16_t inchar)
{
    std::ostringstream oss(std::ostringstream::out);
    oss << std::setw(4) << std::setfill('0') << std::hex << (int)(inchar);
    return utils::str_toupper(oss.str());
}

std::ostream& formatDateTime(std::ostream& out, const tm& t, const char* fmt)
{
    const std::time_put<char>& dateWriter = std::use_facet<std::time_put<char>>(out.getloc());
    int n = std::strlen(fmt);
    if(dateWriter.put(out, out, ' ', &t, fmt, fmt + n).failed()) {
        throw std::runtime_error("failure to format date time");
    }
    return out;
}

std::string dateTimeToString()
{
	return dateTimeToString(time(0));
}

std::string dateTimeToString(const time_t& t)
{
    auto result = dateTimeToString(*localtime(&t), "%Y-%m-%d %H:%M:%S");
    int milli = t / 1000;
    result += ",";
    auto milliString = std::to_string(milli);
    while(milliString.size() < 3) {
        milliString = "0" + milliString;
    }
    result += milliString.substr(0, 3);
    return result;
}

std::string dateTimeToString(const tm& t, const char* format)
{
    std::stringstream s;
    formatDateTime(s, t, format);
    return s.str();
}

std::string dateTimeToJsonString(const timeval& t)
{
    auto result = dateTimeToString(*gmtime(&t.tv_sec), "%Y-%m-%dT%H:%M:%S");
    int milli = t.tv_usec / 1000;
    result += ",";
    auto milliString = std::to_string(milli);
    while(milliString.size() < 3) {
        milliString = "0" + milliString;
    }
    result += milliString.substr(0, 3);
    result += "Z";
    return result;
}


tm now()
{
    time_t now = time(0);
    return *localtime(&now);
}

tm utcNow()
{
    time_t now = time(0);
    return *gmtime(&now);
}

#ifdef _WIN32

/**
 * \brief portable version of SCFmemopen for Windows works on top of real temp files
 * \param buffer that holds the file content
 * \param size of the file buffer
 * \param mode mode of the file to open
 * \retval pointer to the file; NULL if something is wrong
 */
FILE* MNFmemopen(const void* buffer, size_t size, const TCHAR* mode)
{
    // Thanks https://doxygen.openinfosecfoundation.org/util-fmemopen_8c_source.html
    // Not Happy with this but an way at the moment
    // Todo Kill Temfile at close

    TCHAR temppath[MAX_PATH - 13];
    if(GetTempPath(MAX_PATH - 13, temppath) == 0) return nullptr;

    TCHAR filename[MAX_PATH + 1];
    if(GetTempFileName(temppath, _T("MN"), 0, filename) == 0) return nullptr;

    FILE* tempStream;
    auto result = _tfopen_s(&tempStream, filename, _T("wb"));
    if(result != 0 || nullptr == tempStream) return nullptr;

    std::cout << "write temp file " << filename << " with size " << size << std::endl;

    auto written = fwrite(buffer, size, 1, tempStream);
    fflush(tempStream);
    fclose(tempStream);

    FILE* stream;
    result = _tfopen_s(&stream, filename, mode);
    if(result != 0 || nullptr == stream) return nullptr;
    return stream;
}

#else
FILE* MNFmemopen(void* buf, size_t size, const char* mode)
{
    return fmemopen(buf, size, mode);
}
#endif

#ifdef _WIN32
#else
std::string GetHomePath()
{
    struct passwd pwd;
    struct passwd* result;
    char* buf;
    int s;
    auto bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if(bufsize == -1) bufsize = 0x4000; // = all zeroes with the 14th bit set (1 << 14)
    buf = new char[bufsize];
    if(buf == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    s = getpwuid_r(getuid(), &pwd, buf, bufsize, &result);
    if(result == NULL) {
        if(s == 0)
            printf("Not found\n");
        else {
            errno = s;
            perror("getpwnam_r");
        }
        exit(EXIT_FAILURE);
    }
    auto homedir = std::string(result->pw_dir);
    delete[] buf;
    return homedir;
}
#endif

} // namespace utils