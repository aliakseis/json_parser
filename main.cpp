#include "json_parser.h"

#include <iostream>

int main()
{
    try {
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
                "name":"John",
                "age":80,
                "gender":"male",
                "info":"q\\\"we\"r\\\ty\\"
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
    catch (const std::exception& e)
    {
        std::cerr << e.what();
        return 1;
    }
}
