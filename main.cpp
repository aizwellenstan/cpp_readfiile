#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

std::string getWordByIndex(const std::string& sentence, int index) {
    std::istringstream iss(sentence);
    std::string word;
    for (int i = 0; i <= index; ++i) {
        if (!(iss >> word)) {
            return "";
        }
    }
    return word;
}

std::vector<std::string> readFile(const std::string& filePath) {
    std::vector<std::string> res;

    std::ifstream file(filePath);
    std::string keyword = "createNode ";
    std::string keyword2 = "xgm";
    
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line, ';')) {
            if (line.find(keyword) != std::string::npos && line.find(keyword2) != std::string::npos) {
                std::cout << line << std::endl;
                line = getWordByIndex(line, 1);
                if (std::find(res.begin(), res.end(), line) != res.end()) {
                    continue;
                }
                std::cout << line << std::endl;
                res.push_back(line);
            }
        }
        file.close();
    }
    return res;
}

int main() {
    std::string filePath = "D:/Test.ma";
    std::vector<std::string> res = readFile(filePath);
    for (const std::string& i : res) {
        std::cout << i << std::endl;
    }
    return 0;
}
