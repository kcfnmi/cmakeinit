#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstdlib>

using namespace std;

const string CMAKELISTS = "CMakeLists.txt";

struct Project
{
    pair<int, int> cmake_version;
    string name;
    vector<string> source;
};

struct CMakeProject 
{
    string version;
    string cpp_standard;
    string name;
    string executable;
};

void cmakelists_create(Project p)
{
    CMakeProject project;

    string version = to_string(p.cmake_version.first) +"."+ to_string(p.cmake_version.second);
    project.version = "cmake_minimum_required(VERSION "+ version +")";
    project.cpp_standard = "set(CMAKE_CXX_STANDARD 17)";
    project.name = "project("+ p.name +")";
    string files;
    for (auto& f : p.source)
        files.append(" "+ f);
    project.executable = "add_executable("+ p.name + files +")";

    vector<string> list;
    list.push_back(project.version);
    list.push_back(project.cpp_standard);
    list.push_back(project.name);
    list.push_back(project.executable); 

    ofstream file(CMAKELISTS);
    for (auto& i : list)
        file << i << endl;
}

vector<string> split(const string& line, string delimiter)
{
    vector<string> list;

    size_t last = 0;
    size_t next = 0;
    while ((next = line.find(delimiter, last)) != string::npos)
    {
        list.push_back(line.substr(last, next - last));
        last = next + 1; 
    }
    list.push_back(line.substr(last));

    return list;
}

pair<int, int> cmakeversion_identify()
{
    pair<int, int> version{3, 20};

    string cmakeversion_txt = "_cmakeversion.txt";
    string command = "cmake --version > " + cmakeversion_txt;
    system(command.data());

    ifstream file_cmakeversion(cmakeversion_txt);
    string line;
    while (getline(file_cmakeversion, line))
        if (line.find("version") != string::npos)
        {
            auto list = split(line, " ");
            list = split(list.back(), ".");
            version.first = stoi(list.at(0));
            version.second = stoi(list.at(1));
        }

    filesystem::remove(cmakeversion_txt);

    return version;
}

void show_help()
{
    cout << "You are expected to inform the source files to build the project" << endl;
    cout << "i.e. cmakeinit main.cpp main.h" << endl;

}

int main(int args, char** argv)
{
    cout << "cmakeinit - for those times cmakelists.txt is not there yet" << endl;

    if (args < 2)
    {
        show_help();
        return 0;
    }

    vector<string> files;
    for (int i = 1; i < args; ++i)
        files.push_back(argv[i]);

    filesystem::path cwd = std::filesystem::current_path();

    string project = cwd.stem();

    bool cmakelists_exist = filesystem::exists(CMAKELISTS);

    if (cmakelists_exist)
    {
        cout << cwd.append(CMAKELISTS) << " already exist" << endl;
        return 0;
    }

    auto v = cmakeversion_identify();

    cmakelists_create({v, project, files});

    cout << "CMakeLists.txt created with cmake version: " << v.first << "." << v.second << endl;

    return 0;
}