#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>

using namespace std;

const string CMAKELISTS = "CMakeLists.txt";

string SUBLIMEPROJECT(const string& projectname)
{
    string s = ".sublime-project";
    return s;
}


const char* menuopt_cmakelists = "";
const char* menuopt_help = "--help";
const char* menuopt_version = "--version";
const char* menuopt_sublcpp = "--sublcpp";

const string description = "You are expected to inform the source files to build the project\n"
"i.e.: ./cmakeinit main.cpp main.h";

const std::map<string, string> menu = {
                                      {menuopt_cmakelists, description},
                                      {      menuopt_help, "Show help menu"},
                                      {   menuopt_version, "Show version"},
                                      {   menuopt_sublcpp, "Generates .sublime-project for C++"},
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

string timestamp(const char* format="%Y%m%d%H%M%S", const size_t size = 14)
{
    auto t = time(nullptr);
    
    string s(size, ' ');

    std::strftime(s.data(), size, format, localtime(&t));

    return s;
}

void cmakelists_create(const Project& p)
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

void replace(string& str, const string& substr_src, const string& substr_dst)
{
    size_t index = 0;
    while (true) {
         index = str.find(substr_src, index);
         if (index == std::string::npos) 
            break;
    
         str.replace(index, substr_src.size(), substr_dst);
    
         index += substr_dst.size();
    }
}

void sublcpp_create(const string& projectname)
{
    string subl_project = R"(
{
    "folders":
    [
        {
            "path": ".",
        }
    ],
    "settings":
    {
        "translate_tabs_to_spaces": true,
    },
    "build_systems":
    [
        {
            "name": "THEPROJECTNAME Build",
            "target": "terminus_exec",

            "cancel": "terminus_cancel_build", 

            "shell_cmd": "g++ -std=c++17 \"${file}\" -o \"${file_path}/THEPROJECTNAME\"",
            "file_regex": "^(..[^:]*):([0-9]+):?([0-9]+)?:? (.*)$",
            "working_dir": "${file_path}",
            "selector": "source.c++",

            "variants":
            [
                {
                    "name": "Run",
                    "shell_cmd": "g++ -std=c++17 \"${file}\" -o \"${file_path}/THEPROJECTNAME\" && \"${file_path}/THEPROJECTNAME\""
                }
            ]
        }
    ],
    "created": "TIMESTAMPCREATED",
}
)";

    replace(subl_project, "THEPROJECTNAME", projectname);

    replace(subl_project, "TIMESTAMPCREATED", timestamp());

    ofstream file(projectname+".sublime-project");
    file << subl_project << endl;
}

vector<string> split(const string& line, const string& delimiter)
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

    string time_stamp = timestamp();

    pair<string, string> v{time_stamp, commit};

    return v;
}

string parse_args(int args, char** argv)
{
    string opt = "undefined";

    if (args == 2 && string(argv[1]) == menuopt_help)
    {
        return menuopt_help;
    }
    else if (args == 2 && string(argv[1]) == menuopt_version)
    {
        return menuopt_version;
    }
    else if (args == 2 && string(argv[1]) == menuopt_sublcpp)
    {
        return menuopt_sublcpp;
    }
    else if (args == 2 && string(argv[1]).find("-") == 0)
    {
        cout << "unknown arg(s):" << endl;
        for (int i = 1; i < args; ++i)
            cout << argv[i] << endl;
    }
    else
    {
        return menuopt_cmakelists;
    }

    return opt;
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
            cout << k << ": " << v << endl;
    }
    else if (opt == menuopt_version)
    {
        auto v = program_version();
        cout << "https://github.com/kcfnmi/cmakeinit" << " " << v.second << " build on " << v.first << endl;
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
        cout << CMAKELISTS << " created with cmake version: " << v.first << "." << v.second << endl;
    }
    else if (opt == menuopt_sublcpp)
    {
        filesystem::path cwd = std::filesystem::current_path();
        string project = cwd.stem();

        if (filesystem::exists(project+".sublime-project"))
        {
            cout << cwd.append(project+".sublime-project") << " already exist" << endl;
            return 0;
        }

        sublcpp_create(projectname);
        cout << project+".sublime-project" << " created" << endl;
    }

    return 0;
}