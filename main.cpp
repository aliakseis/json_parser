#include "json_parser.h"

#include <iostream>
#include <cassert>

#include <windows.h>

#pragma execution_character_set( "utf-8" )


void testSmoke()
{
    auto parsed = parseJson(
 R"({
    "data":[
        {
            "type":"articles",
            "id":"1",
            "attributes":{
                "title":"JSON API paints my bikeshed!",
                "body":"The shortest article. Ever.",
                "created":"2015-05-22T14:56:29.000Z",
                "updated":"2015-05-22T14:56:28.000Z"
            },
            "relationships":{
                "author":{
                    "data":{
                        "id":"42",
                        "type":"people"
                    }
                }
            }
        }
    ],
    "extra":[],
    "included":[
        {
            "type":"people",
            "id":"42",
            "attributes":{
                "lastName":"T\u00e4rn\u00e4by",
                "firstName":"Bj\u00f6rn",
                "age":80,
                "gender":"male",
                "info":"\\trouble \u0139q\\\"we\"r\\\ty\\"
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
            if (auto lastName = attributes.find("lastName"); lastName != attributes.end())
            {
                std::string value = std::any_cast<std::string>(lastName->second);
                //std::cout << value << '\n';
                assert(value == u8"T\u00e4rn\u00e4by");
            }
            else
            {
                assert(false);
            }
            if (auto firstName = attributes.find("firstName"); firstName != attributes.end())
            {
                std::string value = std::any_cast<std::string>(firstName->second);
                //std::cout << value << '\n';
                assert(value == u8"Bj\u00f6rn");
            }
            else
            {
                assert(false);
            }
            if (auto age = attributes.find("age"); age != attributes.end())
            {
                int value = static_cast<int>(std::any_cast<double>(age->second));
                //std::cout << value << '\n';
                assert(value == 80);
            }
            else
            {
                assert(false);
            }
            if (auto info = attributes.find("info"); info != attributes.end())
            {
                std::string value = std::any_cast<std::string>(info->second);
                //std::cout << value << '\n';
                assert(value == u8"\\trouble \u0139q\\\"we\"r\\\ty\\");
            }
            else
            {
                assert(false);
            }
        }
        else
        {
            assert(false);
        }
    }
    else
    {
        assert(false);
    }
}

void testParseEmptyObject() {
    auto result = parseJson("{}");
    assert(result.type() == typeid(std::map<std::string, std::any>));
}

void testParseSimpleObject() {
    auto result = parseJson(R"({"name": "John", "age": 30})");
    assert(result.type() == typeid(std::map<std::string, std::any>));
    auto& object = std::any_cast<const std::map<std::string, std::any>&>(result);
    assert(object.at("name").type() == typeid(std::string));
    assert(std::any_cast<std::string>(object.at("name")) == "John");
    assert(object.at("age").type() == typeid(double));
    assert(std::any_cast<double>(object.at("age")) == 30);
}

void testParseArray() {
    auto result = parseJson(R"([1, 2, 3, 4])");
    assert(result.type() == typeid(std::vector<std::any>));
    auto& array = std::any_cast<const std::vector<std::any>&>(result);
    assert(array.size() == 4);
}

void testParseNestedObject() {
    auto result = parseJson(R"({"data": {"id": 1, "value": "test"}})");
    assert(result.type() == typeid(std::map<std::string, std::any>));
    auto& object = std::any_cast<const std::map<std::string, std::any>&>(result);
    assert(object.at("data").type() == typeid(std::map<std::string, std::any>));
}

void testParseInvalidJson() {
    try {
        auto result = parseJson(R"({"name": "John", "age": )");
        assert(false); // 
    }
    catch (const std::runtime_error&) {
        // 
    }
}

int main()
{
    SetConsoleOutputCP(65001);

    try {
        testSmoke();
        testParseEmptyObject();
        testParseSimpleObject();
        testParseArray();
        testParseNestedObject();
        testParseInvalidJson();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what();
        return 1;
    }
}
