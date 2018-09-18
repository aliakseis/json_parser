// json_parser.cpp : Defines the entry point for the console application.
//

#include "json_parser.h"

#include <iostream>
#include <cctype>
#include <climits>
#include <algorithm>
#include <assert.h>


template<> struct JsonParser::Traits<std::map<std::string, std::any> >
{
    enum { OPENING = '{', CLOSING = '}' };
};
template<> struct JsonParser::Traits<std::vector<std::any> >
{
    enum { OPENING = '[', CLOSING = ']' };
};



JsonParser::JsonParser(std::istream& input)
    : m_input(input)
{
}


JsonParser::~JsonParser()
{
}

bool JsonParser::parse(std::any& result)
{
    try
    {
        result = parseValue();
    }
    catch (const char* errorDescription)
    {
        std::cerr << "JSON parse error: " << errorDescription << '\n'; // TODO additional info
        return false;
    }

    // TODO check if the rest is missing
    return true;
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
    if (moreMembers)
    {
        error("Missing member");
    }
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
    object[key] = parseValue();
    return true;
}

bool JsonParser::parseMember(std::vector<std::any>& array)
{
    std::any value = parseValue(true);
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

std::any JsonParser::parseValue(bool skipError)
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
    throw description;
}


int main()
{
    auto parsed = parseJson(R"({
"data": [{
    "type": "articles",
        "id" : "1",
        "attributes" : {
        "title": "JSON API paints my bikeshed!",
            "body" : "The shortest article. Ever.",
            "created" : "2015-05-22T14:56:29.000Z",
            "updated" : "2015-05-22T14:56:28.000Z"
    },
        "relationships": {
            "author": {
                "data": {"id": "42", "type" : "people"}
            }
        }
}],
"extra": [],
"included": [
{
    "type": "people",
        "id" : "42",
        "attributes" : {
        "name": "John",
            "age" : 80,
            "gender" : "male",
            "info" : "q\\\"we\"r\\\ty\\"
    }
}
]
})");

    const auto& document = std::any_cast<const std::map<std::string, std::any>&>(parsed);
    if (auto included = document.find("included"); included != document.end())
    {
        const auto& included0 = std::any_cast<const std::map<std::string, std::any>&>(
            std::any_cast<const std::vector<std::any>&>(included->second)[0]);
        if (auto it = included0.find("attributes"); it != included0.end())
        {
            const auto& attributes = std::any_cast<const std::map<std::string, std::any>&>(it->second);
            if (auto age = attributes.find("age"); age != attributes.end())
            {
                int value = std::any_cast<int>(age->second);
                std::cout << value << '\n';
            }
            if (auto info = attributes.find("info"); info != attributes.end())
            {
                std::string value = std::any_cast<std::string>(info->second);
                std::cout << value << '\n';
            }
        }
    }
    return 0;
}

