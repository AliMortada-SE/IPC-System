#ifndef SCANNER_H_
#define SCANNER_H_

#include <string>
#include <vector>

class SCANNER {
public:
    SCANNER(char pointer, char separator,char cap);

    int find(std::string Find, std::string word);
    int reachTo(std::string value, std::string line);
    std::string get(const std::string& type, std::string line,bool json = false);
    int findClass(std::string ClassName, std::string line);
    std::string getClass(std::string ClassName, std::string line);
    std::string Class(std::string data, std::string className);
    void NewClass(std::string& line, std::string Class);
    std::string removeClass(std::string& data, std::string className);
    std::string editValue(std::string type, std::string newValue, std::string& line);
    std::string replace(char from, char to, std::string data);
    std::vector<std::string> getTags(std::string data);
    std::vector<std::string> getValues(std::string data);
    bool SafeChar(std::string data);
    bool Digits(std::string data);
    std::string eraseDigits(std::string& data);
    std::string eraseChar(std::string& data);
    
    std::string addValue(std::string Tag,std::string Value);
    std::string removeValue(std::string Tag,std::string Value);
    std::string Remove(char selected,std::string data);
    std::string fromFlatJson(std::string data);
    std::string toFlatJson(std::string data);
    char pointer;
    char separator;
    char cap;

    static const std::string TYPE_NOT_FOUND;
    static const std::string CLASS_NOT_FOUND;
};

extern SCANNER scan;
#endif 
