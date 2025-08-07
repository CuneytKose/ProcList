#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <pwd.h>

namespace fs = std::filesystem;

struct ProcessInfo {
    int pid;
    std::string name;
    std::string user;
};

std::string get_username(uid_t uid) {
    struct passwd* pw = getpwuid(uid);
    if (pw) return pw->pw_name;
    return std::to_string(uid);
}

std::vector<ProcessInfo> list_processes() {
    std::vector<ProcessInfo> processes;

    for (const auto& entry : fs::directory_iterator("/proc")) {
        if (!entry.is_directory()) continue;

        std::string dirname = entry.path().filename();
        if (!std::all_of(dirname.begin(), dirname.end(), ::isdigit)) continue;

        int pid = std::stoi(dirname);
        std::string name, user;

        std::ifstream comm_file("/proc/" + dirname + "/comm");
        if (comm_file.is_open()) {
            std::getline(comm_file, name);
            comm_file.close();
        }

        std::ifstream status_file("/proc/" + dirname + "/status");
        if (status_file.is_open()) {
            std::string line;
            while (std::getline(status_file, line)) {
                if (line.starts_with("Uid:")) {
                    uid_t uid = std::stoi(line.substr(5));
                    user = get_username(uid);
                    break;
                }
            }
            status_file.close();
        }

        processes.push_back({pid, name, user});
    }

    return processes;
}

int main() {
    auto procs = list_processes();
    std::cout << "PID\tUSER\tNAME\n";
    for (const auto& p : procs) {
        std::cout << p.pid << '\t' << p.user << '\t' << p.name << '\n';
    }
    return 0;
}
