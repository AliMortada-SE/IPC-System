#include "Scanner.h"
#include <cctype>
#include <iostream>
const std::string SCANNER::TYPE_NOT_FOUND = "TYPE_NOT_FOUND";
const std::string SCANNER::CLASS_NOT_FOUND = "CLASS_NOT_FOUND";

SCANNER::SCANNER(char pointer, char separator,char cap) : pointer(pointer), separator(separator),cap(cap) {}

int SCANNER::find(std::string Find, std::string word) {
    bool found = false;
    int index = 0;
    int x = 0;
    int size = Find.length();
    std::string get;
    char c;

    if (size > word.size() && Find != word) return -1;
    if (size == word.size() && Find == word) return 0;

    if (size > 1) {
        while (!found && index < word.length()) {
            while (x < size) {
                get += word[index];
                x++;
                index++;
            }
            if (get == Find && word[index] != cap && word[index-size] != cap) {
                found = true;
                index -= size;
                return index;
            }
            else {
                get.clear();
                x = 0;
                index -= size;
                index++;
            }
            if (index + size > word.length()) {
                return -1;
            }
        }
    }
    else if (size == 1) {
        while (!found && index < word.length()) {
            c = word[index];
            if (Find[0] == c) {
                found = true;
                return index;
            }
            else {
                index++;
            }
        }
        if (!found) {
            return -1;
        }
    }
    else if (size <= 0) {
        return -1;
    }
    return -1;
}

int SCANNER::reachTo(std::string value, std::string line) {
    int index = find(value, line);
    while (line[index] != pointer) {
        index++;
    }
    //-||;
    index++;
    while (line[index] != cap) {
        index++;
    }
    return index;
}

std::string SCANNER::get(const std::string& type, std::string line, bool json) {
    if(json) line = Remove ('"',line);
    int index = find(type, line);
    bool start = false;
    bool end = false;
    std::string takeValue;

    if (index != -1) {
        index = reachTo(type, line);
        while (index + 1 < line.length() && line[index+1] != separator) {
            if(line[index-1]==cap) {
                if(!start) start = true;
            }
            if(line[index]==cap) if (start) end = true;
            if(start && !end) {
                takeValue += line[index];
            }
            index++;
        }
    }
    else {
        return TYPE_NOT_FOUND;
    }
    return takeValue;
}

int SCANNER::findClass(std::string ClassName, std::string line) {
    int Tag;
    int x = 0;
    bool found = false;
    std::string get;

    while (!found && x < line.size()) {
        if (line[x] == '#') {
            Tag = x;
            x++;
            while (line[x] != '<') {
                get += line[x]; x++;
            }
            x++;
            if (get == ClassName) {
                return Tag;
            }
        }
        get.clear();
        x++;
    }
    return -1;
}

std::string SCANNER::getClass(std::string ClassName, std::string line) {
    int x = findClass(ClassName, line);
    if (x == -1) return CLASS_NOT_FOUND;

    bool loaded = false;
    std::string get;
    std::string data;

    while (line[x - 1] != '<') x++;
    while (line[x] != '/' && line[x + 1] != '>') {
        data += line[x]; x++;
    }
    return data;
}

std::string SCANNER::Class(std::string data, std::string className) {
    if (className == "" || data == "") return "NULL";
    std::string Tag = '#' + className;
    return Tag + '<' + data + "/>";
}

void SCANNER::NewClass(std::string& line, std::string Class) {
    line += Class;
}

std::string SCANNER::removeClass(std::string& data, std::string className) {
    int x = findClass(className, data);
    while (data[x] != '/' && data[x + 1] != '>') {
        data.erase(x, 1);
    }
    data.erase(x, 2);
    return data;
}

std::string SCANNER::editValue(std::string type, std::string newValue, std::string& line) {
    if(line.size()<=0){
        std::cout<<"Error! "<<TYPE_NOT_FOUND<<"\n";
        return TYPE_NOT_FOUND;
    }
    std::string NewValue;
    int ValueSize = get(type, line).length();
    int index = reachTo(type, line)+1;
    line.erase(index, ValueSize);
    line.insert(index, newValue);
    return line;
}

std::string SCANNER::replace(char from, char to, std::string data) {
    for (int x = 0; x < data.length(); x++) {
        if (data[x] == from) {
            data[x] = to;
        }
    }
    return data;
}

std::vector<std::string> SCANNER::getTags(std::string data) {
    int x = 0;
    std::string line;
    std::vector<std::string> Tags;

    while (x < data.length()) {
        if (data[x] == pointer) {
            Tags.push_back(line);
            line.clear();
            while (data[x] != separator && data.length()) x++;
            x++;
        }
        if (data[x] != ' ') {
            line += data[x];
        }
        x++;
    }
    return Tags;
}

std::vector<std::string> SCANNER::getValues(std::string data) {
    int x = 0;
    std::string line;
    std::vector<std::string> Values;

    while (x < data.length()) {
        if (data[x] == pointer) {
            x++;
            while (data[x] != separator && x < data.length()) {
                if (data[x]!=cap) {
                    line += data[x];
                }
                x++;
            }
            x++;
            Values.push_back(line);
            line.clear();
        }
        x++;
    }
    return Values;
}

std::string SCANNER::addValue(std::string Tag, std::string Value) {
    return  Tag + pointer + cap + Value + cap + separator;
}
std::string SCANNER::removeValue(std::string Tag, std::string Value) {
    return  Tag + pointer + cap + Value + cap + separator;
}
// Name-|Ali|;
bool SCANNER::SafeChar(std::string data) {
    for (int x = 0; x < data.size(); x++) {
        if (data[x] >= 32 && data[x] <= 126) {
            // Valid character
        }
        else {
            return false;
        }
    }
    return true;
}

bool SCANNER::Digits(std::string data) {
        for (int x = 0; x < data.length(); x++) {
            if (!std::isdigit(data[x])) {
                return false;
            }
        }    
    return true;
}

std::string SCANNER::eraseDigits(std::string& data) {
    for (int x = 0; x < data.length(); x++) {
        if (std::isdigit(data[x])) {
            data.erase(x);
        }
    }
    return data;
}

std::string SCANNER::eraseChar(std::string& data) {
    for (int x = 0; x < data.length(); ) {
        if (!std::isdigit(data[x])) {
            data.erase(x, 1);
        }
        else {
            x++;
        }
    }
    return data;
}
std::string SCANNER::fromFlatJson(std::string data){
    std::string Tag;
    std::string Value;
    std::string NewForm;
    int counter = 0;
    while(counter < data.size() -1){
        if(data[counter]=='\"' ){
            counter++;
            while(data[counter]!='\"'){
                Tag+=data[counter];
                counter++;
            }
            while(data[counter]!=':'){
                counter++;
            }
            counter++;
            while(data[counter]!=',' && data[counter]!='}'){
                Value+=data[counter];
                counter++;
            }
            NewForm += addValue(Tag,Value);
        }
        Tag.clear();
        Value.clear();
        counter++;
    }
    return NewForm;

}
std::string SCANNER::toFlatJson(std::string data){
    std::string Json = "{";
    int x = 0;
    std::vector<std::string > Tags;
    std::vector<std::string > Values;
    Tags = getTags(data);
    Values = getValues(data);
    while(x<Tags.size()){
        Json += "\"";
        Json += Tags[x];
        Json += "\":";
        Json += Values[x];
        if(x<Tags.size()-1)Json += ',';
        x++;
    }
    Json += '}';
    return Json;
}
std::string SCANNER::Remove(char selected,std::string data){
    int x = 0;
    while(x<data.size()){
        if(data[x]==selected){
            data.erase(x,1);
        }
        else{
            x++;
        }
    }
    return data;
}
SCANNER scan('-',';','|');