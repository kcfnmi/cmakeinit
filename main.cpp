#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>

using namespace std;

const string CMAKELISTS = "CMakeLists.txt";

const char* menuopt_cmakelists = "";
const char* menuopt_help = "--help";
const char* menuopt_version = "--version";

const string description = "You are expected to inform the source files to build the project\n"
"i.e. ./cmakeinit main.cpp main.h";

const std::map<string, string> menu = {
                                      {menuopt_cmakelists, description},
                                      {   menuopt_version, "Show version"},
                                      {      menuopt_help, "Show help menu"} 
                                      };

struct Project
{
    pair<int, int> cmake_version;
    string name;
    vector<string> source;
};

struct CMakeProject 
{
    string version;
    string cxx_standard;
    string cxx_standard_req;
    string name;
    string verbose;
    string cxx_flags;
    string executable;
};

void cmakelists_create(Project p)
{
    CMakeProject project;

    string version = to_string(p.cmake_version.first) +"."+ to_string(p.cmake_version.second);
    project.version = "cmake_minimum_required(VERSION "+ version +")";
    project.cxx_standard = "set(CMAKE_CXX_STANDARD 17)";
    project.cxx_standard_req = "set(CMAKE_CXX_STANDARD_REQUIRED ON)";
    project.name = "project("+ p.name +")";

    project.verbose = "set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL \"ON\" FORCE)";

    project.cxx_flags = "set(CMAKE_CXX_FLAGS \"-Wall -Wextra -pedantic -fsanitize=address,undefined\")";

    string files;
    for (auto& f : p.source)
        files.append(" "+ f);
    project.executable = "add_executable("+ p.name + files +")";

    vector<string> list;
    list.push_back(project.version);
    list.push_back(project.cxx_standard);
    list.push_back(project.cxx_standard_req);
    list.push_back(project.name);
    list.push_back(project.verbose); 
    list.push_back(project.cxx_flags); 
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

pair<string, string> program_version()
{
    string commit_txt = "_commit.txt";
    string command = "git rev-parse HEAD > " + commit_txt;
    system(command.data());

    ifstream file(commit_txt);
    string line;
    getline(file, line);
    string commit = line;
    filesystem::remove(commit_txt);

    string timestamp = __TIMESTAMP__;

    pair<string, string> v{timestamp, commit};

    return v;
}

string parse_args(int args, char** argv)
{
    if (args == 2 && string(argv[1]) == menuopt_help)
    {
        return menuopt_help;
    }
    else if (args == 2 && string(argv[1]) == menuopt_version)
    {
        return menuopt_version;
    }
    else
    {
        return menuopt_cmakelists;
    }

    return "undefined";
}

int main(int args, char** argv)
{
    cout << "cmakeinit - for those times cmakelists.txt is not there yet" << endl;

    if (args < 2)
    {
        cout << "Usage: ./cmake --help" << endl;
        return 0;
    }

    auto opt = parse_args(args, argv);

    if (opt == menuopt_help)
    {
        for (auto& [k, v] : menu)
            cout << k << " " << v << endl;
    }
    else if (opt == menuopt_version)
    {
        auto v = program_version();
        cout << "Build version " << v.first << " of " << v.second << endl;
    }
    else if (opt == menuopt_cmakelists)
    {
        filesystem::path cwd = std::filesystem::current_path();
        string project = cwd.stem();

        if (filesystem::exists(CMAKELISTS))
        {
            cout << cwd.append(CMAKELISTS) << " already exist" << endl;
            return 0;
        }

        vector<string> files;
        for (int i = 1; i < args; ++i)
            files.push_back(argv[i]);

        auto v = cmakeversion_identify();
        cmakelists_create({v, project, files});
        cout << "CMakeLists.txt created with cmake version: " << v.first << "." << v.second << endl;
    }

    return 0;
}