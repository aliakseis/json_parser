// json_parser.cpp : Defines the entry point for the console application.
//

#include "json_parser.h"

#include <cctype>
#include <climits>
#include <algorithm>
#include <assert.h>


namespace {

template<typename T> struct Traits {};

template<> struct Traits<std::map<std::string, std::any> >
{
    enum { OPENING = '{', CLOSING = '}' };
};
template<> struct Traits<std::vector<std::any> >
{
    enum { OPENING = '[', CLOSING = ']' };
};

} // namespace


JsonParser::JsonParser(std::istream& input)
    : m_input(input)
{
}

JsonParser::~JsonParser()
{
}


template<typename T>
bool JsonParser::parseInstance(std::any& result)
{
    if (!skip(Traits<T>::OPENING))
    {
        return false;
    }

    T object;
    bool moreMembers = false;
    while (parseMember(object))
    {
        moreMembers = skip(',');
        if (!moreMembers)
        {
            break;
        }
    }
    moreMembers && error("Missing member");
    skip(Traits<T>::CLOSING) || error("Unclosed instance");

    result = object;
    return true;
}

bool JsonParser::parseMember(std::map<std::string, std::any>& object)
{
    std::string key;
    if (!parseString(key))
    {
        return false;
    }
    skip(':') || error("Expecting object separator");
    object[key] = parse();
    return true;
}

bool JsonParser::parseMember(std::vector<std::any>& array)
{
    std::any value = parse(true);
    if (!value.has_value())
    {
        return false;
    }
    array.push_back(value);
    return true;
}

template<typename T>
bool JsonParser::parseString(T& result)
{
    if (!scan('"'))
    {
        return false;
    }

    std::string value;

    bool stop;
    do
    {
        std::string contents;
        std::getline(m_input, contents, '"') || error("Unclosed string");
        const auto begin = contents.rbegin();
        const auto cnt 
            = std::find_if(begin, contents.rend(), [](char ch) { return ch != '\\'; }) - begin;
        stop = !(cnt & 1);
        if (!stop)
            contents[contents.size() - 1] = '"';
        // TODO unescape
        value += contents;
    } while (!stop);

    result = value;
    return true;
}

std::any JsonParser::parse(bool skipError)
{
    std::any result;
    trimSpace();

    parseInstance<std::map<std::string, std::any> >(result) // object
        || parseInstance<std::vector<std::any> >(result) // array
        || parseString(result)
        || parseKeyword(result)
        || parseNumber(result)
        || skipError
        || error("Illegal JSON value");

    trimSpace();
    return result;
}


bool JsonParser::scan(char ch)
{
    if (!m_input)
        return false;

    if (m_input.peek() == ch)
    {
        m_input.get();
        return true;
    }

    return false;
}

bool JsonParser::skip(char ch)
{
    trimSpace();
    if (scan(ch))
    {
        trimSpace();
        return true;
    }
    return false;
}


void JsonParser::trimSpace()
{
    while (m_input && std::isspace(m_input.peek()))
        m_input.get();
}

bool JsonParser::parseNumber(std::any& result)
{
    try
    {
        double value{};
        if (!(m_input >> value))
        {
            m_input.clear();
            return false;
        }
        int intValue;
        if (value >= INT_MIN && value <= INT_MAX
            && ((intValue = static_cast<int>(value)) == value))
        {
            result = intValue;
        }
        else
        {
            result = value;
        }
    }
    catch (std::exception&)
    {
        return false;
    }
    return true;
}

bool JsonParser::parseKeyword(std::any& result)
{
    if (scan('t'))
    {
        if (scan('r') && scan('u') && scan('e'))
        {
            result = true;
            return true;
        }
    }
    else if (scan('f'))
    {
        if (scan('a') && scan('l') && scan('s') && scan('e'))
        {
            result = false;
            return true;
        }
    }
    else if (scan('n'))
    {
        if (scan('u') && scan('l') && scan('l'))
        {
            result.reset();
            return true;
        }
    }
    else
    {
        return false;
    }

    error("Unexpected keyword");
    return false;
}

bool JsonParser::error(const char* description)const
{
    throw std::runtime_error(description);
}
