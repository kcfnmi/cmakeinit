#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

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

int main()
{
    cout << "cmakeinit - for those times cmakelists.txt does not exist" << endl;

    filesystem::path cwd = std::filesystem::current_path();

    bool cmakelists_exist = filesystem::exists(CMAKELISTS);

    if (cmakelists_exist)
        cout << cwd.append(CMAKELISTS) << " exist" << endl;
    else
    {
        cout << "Create? Yes(y) or No(n)?" << endl;
        string answer;
        cin >> answer;
        if (answer == "n" || answer == "N") {}
        else
        {
            cmakelists_create({{3, 20}, "TheProject", {"main.cpp"}});
        }
    }

    return 0;
}