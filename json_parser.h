#pragma once

#include <any>

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <istream>
#include <sstream>


class JsonParser
{
public:
    JsonParser(std::istream& input);
    ~JsonParser();

    std::any parse(bool skipError = false);

private:
    bool error(const char* description)const;
    bool scan(char ch);
    bool skip(char ch); // trims white space on both sides; for delimiters only
    void trimSpace();

    template<typename T>
    bool parseInstance(std::any& result);

    bool parseMember(std::map<std::string, std::any>& object);
    bool parseMember(std::vector<std::any>& array);

    template<typename T>
    bool parseString(T& result);
    bool parseNumber(std::any& result);
    bool parseKeyword(std::any& result);

private:
    std::istream& m_input;
}; // class JsonParser


inline std::any parseJson(const std::string& data)
{
    std::istringstream iss(data);
    return JsonParser(iss).parse();
}
