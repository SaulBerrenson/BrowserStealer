/*
    Simple header only parser for INI files.

    Source: https://github.com/virtmax/iniparser



    The MIT License

    Copyright 2019 Maxim Singer

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software
    and associated documentation files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
    NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
    THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <map>
#include <vector>
#include <sstream>
#include <regex>
#include <FileOperations.h>

//changes delete save (not needs), change parseFile because of needs winapi obfuscation.


class IniParser
{
public:
    IniParser() : iniString(""), filePath("")
    {}

	//changed for winapi obfuscation
    bool parseFile(std::string file_path)
    {

        if (!IO::is_file_exists(file_path)) return false;        
    	if(!IO::read_file(file_path, iniString)) return false;

        return parse(iniString);
    }

    bool parse(std::string ini_content)
    {
        std::vector<std::string> lines = tokenize(ini_content, "\n");
        std::string current_section = "";

        const std::regex patternSection("^\\s*\\[\\s*(.*\\S)\\s*\\]\\s*(?:[#;].*)?$");
        const std::regex patternKeyEqualValue("^\\s*(?:\"(.+)\"|(.*[^\\s\"]))\\s*=\\s*(?:\"(.*)\"|([^#;]*\\S))\\s*(?:[#;].*)?$");

        LineEntry lineEntry;
        for(size_t i = 0; i < lines.size(); i++)
        {
            lines[i].erase(0, lines[i].find_first_not_of(" \t\n\r\f\v"));   // trim leading whitespaces

            if(lines[i].size() == 0 || lines[i].at(0) == '#')
                continue;

            std::smatch match;

            lineEntry.type = LineEntry::TYPE::Comment;
            lineEntry.content = lines[i];

            if(std::regex_search(lines[i], match, patternSection))
            {
                current_section = match[1].str();
                lineEntry.type = LineEntry::TYPE::Section;
                lineEntry.content = current_section;
            }
            else if(std::regex_search(lines[i], match, patternKeyEqualValue))
            {
                const std::string &key = match[1].str().empty() ? match[2].str() : match[1].str();
                const std::string &value = match[3].str().empty() ? match[4].str() : match[3].str();
                storage[current_section][key] = value;
                lineEntry.type = LineEntry::TYPE::Key;
                lineEntry.content = key;
                lineEntry.value = value;
            }

            this->lines.push_back(lineEntry);
        }

        return true;
    }

    struct ValueVariant : public std::string
    {
        template<typename T>
        ValueVariant(const T& t) : std::string(std::to_string(t))
        {}

        template<size_t N>
        ValueVariant(const char (&s)[N]) : std::string(s, N)
        {}

        ValueVariant(const char* charstr) : std::string(charstr)
        {}

        ValueVariant(const std::string& str = std::string()) : std::string(str)
        {}

        template<typename T>
        operator T() const
        {
            T t;
            std::stringstream ss;
            return ss << *this && (ss >> t) ? t : T();
        }

        template<typename T>
        bool operator ==(const T& t) const
        {
            return this->compare(ValueVariant(t)) == 0;
        }

        bool operator ==(const char *t) const
        {
            return this->compare(t) == 0;
        }

        bool saved;
    };

    ValueVariant& operator[](const std::string& section_dot_key)
    {
        std::smatch match;
        if(std::regex_match(section_dot_key, match, std::regex("^.+\\..+$")))
        {
            auto tokens = tokenize(section_dot_key, ".");
            return storage[tokens[0]][tokens[1]];
        }
        else if(std::regex_match(section_dot_key, match, std::regex("^.+$")))
        {
            return storage[""][section_dot_key];
        }
        else
            throw std::invalid_argument("IniParser::operator[](const std::string& section_dot_key): invalid node_dot_key format. "
                                        "Should be 'section.key' or 'key' for global keys without a section.");
    }   

private:
    std::vector<std::string> tokenize(const std::string& str, const std::string& chars) const
    {
        std::vector<std::string> tokens;
        size_t i = 0, j = 0;
        for(; i < str.size(); i++)
        {
            for(auto& delim : chars)
            {
                if(str[i] == delim)
                {
                    tokens.push_back(str.substr(j, i-j));
                    j = i+1;
                    break;
                }
            }
        }
        tokens.push_back(str.substr(j, i-j));
        return tokens;
    }

    void setAllValuesToNotSaved()
    {
        for(auto& section : storage)
        {
            for(auto& keyVal : section.second)
            {
                keyVal.second.saved = false;
            }
        }
    }

    struct LineEntry
    {
        enum TYPE {Comment, Section, Key};

        TYPE type;
        std::string content;
        ValueVariant value;
    };

    std::vector<LineEntry> lines;

    std::map<std::string, std::map<std::string, ValueVariant>> storage;
    std::string iniString;

    std::string filePath;
};
