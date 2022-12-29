#include <iostream>
// #include "adder.h"
#include <string>

//file
#include <filesystem>
using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
#include <sstream>

#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <cstdlib>
#include <regex>
#include <algorithm>

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING 1;
#include <experimental/filesystem> // or #include <filesystem> for C++17 and up
    
namespace fs = std::experimental::filesystem;

std::string NormalizePath(const std::string& messyPath) {
    std::filesystem::path path(messyPath);
    std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path);
    std::string npath = canonicalPath.make_preferred().string();
    return npath;
}

std::vector<std::string> AppendToVector(std::vector<std::string> v, std::string x) {
    if(!(std::find(v.begin(), v.end(), x) != v.end())) {
        v.push_back(x);
    }
    return v;
}

int CreateFolderIfNotExist(std::string folder) {
    if (!fs::is_directory(folder) || !fs::exists(folder)) { // Check if src folder exists
        std::ostringstream oss;
        oss << "mkdir " + folder; // prevent space in source path
        std::string command = oss.str();
        std::cout << command << std::endl;
        system(command.c_str());
    }
    return 0;
}

int CopyFileAndFolderCommand(std::string file, std::string destanation) {
    CreateFolderIfNotExist(destanation);
    std::cout << destanation << std::endl;
    std::replace(file.begin(), file.end(), '/', '\\');
    std::ostringstream oss;

    std::vector<std::string> result = {};
    std::stringstream  data(file);
    std::string line;
    while(std::getline(data,line,'\\'))
    {
        result.push_back(line);
    }
    std::string folderName = result[result.size()-1].c_str();

    if (fs::is_directory(file)) {
        oss << "xcopy " <<"\""<< file << "\"" << " " << destanation <<  "\\" << folderName << "\\" <<"*" << " /Y /I /S /J"; // prevent space in source path
    } else {
        oss << "xcopy " <<"\""<< file  << "\"" << " " << destanation << " /Y"; // prevent space in source path
    } // is file
    std::string command = oss.str();
    std::cout << command << std::endl;
    system(command.c_str());
    return 0;
}

std::string getPath(std::string s) {
    std::vector<std::string> result = {};
    std::stringstream  data(s);
    std::string line;
    while(std::getline(data,line,' '))
    {
        result.push_back(line);
    }
    s = result[result.size()-1];
    s.erase(remove(s.begin(), s.end(), '\"'),s.end()); // remove double quotes
    s = NormalizePath(s);
    return s;
}

std::vector<std::string> getReference(std::string s) {
    std::vector<std::string> result = {};
    std::ifstream file(s);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line,';')) {
            if (line.find("J:") != std::string::npos) {
                line = getPath(line);
                if (line == "") continue;
                result.push_back(line);
            }  
        }
        file.close();
    }
    return result;
}

int writeSourceFileToTxt(std::string fPath, std::string shotFolder) {
    CreateFolderIfNotExist(shotFolder);
    std::ofstream myfile (shotFolder+"\\filePath.txt", std::ios_base::app);
    if (myfile.is_open())
    {
        myfile << fPath+"\n";
        myfile.close();
    }
    else std::cout << "Unable to open file";
    return 0;
}

int copyFile(std::string fPath, std::string bkpRootFolder) {
    std::vector<std::string> result = {};
    std::stringstream  data(fPath);
    std::string line;
    while(std::getline(data,line,'\\'))
    {
        result.push_back(line);
    }
    std::string seq = result[5];
    std::string shot = result[6];

    std::string seqFolder = bkpRootFolder + "\\" + seq;
    std::string shotFolder = seqFolder + "\\" + shot;
    std::string assFolder = shotFolder + "\\cache\\ass";
    std::string abcFolder = shotFolder + "\\cache\\alembic";
    std::string exrFolder = shotFolder + "\\sourceimages";
    std::string vdbFolder = shotFolder + "\\cache\\vdb";

    std::vector<std::string> sceneList = {};
    std::vector<std::string> assList = {};
    std::vector<std::string> abcList = {};
    std::vector<std::string> exrList = {};
    std::vector<std::string> vdbList = {};

    std::vector<std::string> refResult = getReference(fPath);

    for(std::string i : refResult) {
        std::filesystem::path fPath = i;
        if (fPath.extension().string() == ".ocio") continue;
        if (fPath.extension().string() == ".ass") {
            assList = AppendToVector(assList, fPath.parent_path().string());
        } else if (fPath.extension().string() == ".abc") {
            abcList = AppendToVector(abcList, fPath.parent_path().string());
        } else if (fPath.extension().string() == ".exr") {
            exrList = AppendToVector(exrList, fPath.parent_path().string());
        } else if (fPath.extension().string() == ".vdb") {
            vdbList = AppendToVector(vdbList, fPath.parent_path().string());
        } else {
            sceneList = AppendToVector(sceneList, fPath.string());
        }
        writeSourceFileToTxt(i, shotFolder);
    }

    for(std::string i : sceneList) {
        std::cout << i << std::endl;
        CopyFileAndFolderCommand(i, shotFolder);
    }
    for(std::string i : assList) {
        std::cout << i << std::endl;
        CopyFileAndFolderCommand(i, assFolder);
    }
    for(std::string i : abcList) {
        std::cout << i << std::endl;
        CopyFileAndFolderCommand(i, abcFolder);
    }
    for(std::string i : exrList) {
        std::cout << i << std::endl;
        CopyFileAndFolderCommand(i, exrFolder);
    }
    for(std::string i : vdbList) {
        std::cout << i << std::endl;
        CopyFileAndFolderCommand(i, vdbFolder);
    }

    return 0;
}

int getFileVersion(std::string fPath) {
    int version = 0;
    std::vector<std::string> result = {};
    std::stringstream  data(fPath);
    std::string line;
    while(std::getline(data,line,'_'))
    {
        result.push_back(line); // Note: You may get a couple of blank lines
                                // When multiple underscores are beside each other.
    }
    std::string res = result[result.size()-1].c_str();
    std::regex str_expr ("v([0-9])([0-9])([0-9])");
    if (regex_match (res,str_expr)) {
        res = res.erase(0,1);
        int ver = atoi(res.c_str());
        return ver;
    } else return -1;
}

int main(int argc, const char**argv) {
    std::string proj = "vd2";
    std::string rootPath = "J:\\"+proj+"\\work\\prod\\lig";
    std::string bkpFolder = "\\\\isilon-nl\\archive\\packet\\vd2\\work\\prod\\lig";

    for (const auto& seq : fs::directory_iterator(rootPath)) {
        if (fs::is_directory(seq)) {
            const auto seqStr = seq.path().filename().string();
            if (seqStr == "CharacterTest") continue;
            for (const auto& shot : fs::directory_iterator(seq)) {
                if (fs::is_directory(shot)) {
                    const auto shotStr = shot.path().filename().string();
                    int latestVersion = 0;
                    std::string latestFilePath = "";
                    for (const auto& scene : fs::directory_iterator(shot.path().string()+"\\scenes")) {
                        if (fs::is_regular_file(scene)) {
                            if (scene.path().extension().string() != ".ma") continue;
                            const auto sceneStr = scene.path().filename().string();
                            // std::cout << "seq:  " << seqStr << '\n';
                            // std::cout << "shot:  " << shotStr << '\n';
                            // std::cout << "scene:  " << sceneStr << '\n';
                            std::string fPath = scene.path().filename().string();
                            size_t lastindex = fPath.find_last_of("."); 
                            std::string rawname = fPath.substr(0, lastindex); 

                            int version = getFileVersion(rawname);
                            if (version >= latestVersion) {
                                latestVersion = version;
                                latestFilePath = scene.path().string();
                            }
                        }
                    }
                    
                    if (latestFilePath != "") {
                        copyFile(latestFilePath, bkpFolder);
                    }
                }
            } 
        }
    }
    return 0;
}