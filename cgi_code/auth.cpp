#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cstring>

// 检查文件是否存在
bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

// 创建空文件
bool createEmptyFile(const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file.close();
        return true;
    }
    return false;
}

// 检查用户名是否已存在
bool isUsernameTaken(const std::string& filename, const std::string& username) {
    std::ifstream file(filename);
    if (!file.is_open()) return false; // 文件不存在，自然没有重复

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // 跳过空行和注释行（可选）

        std::istringstream lineStream(line);
        std::string fileUsername, filePassword;
        if (lineStream >> fileUsername >> filePassword) {
            if (fileUsername == username) {
                file.close();
                return true;
            }
        }
        // 忽略格式错误的行
    }
    file.close();
    return false;
}

// 注册用户
bool registerUser(const std::string& filename, const std::string& username, const std::string& password) {
    std::ofstream file(filename, std::ios::app); // 追加模式
    if (!file.is_open()) {
        // std::cerr << "无法打开文件进行注册: " << filename << std::endl;
        return false;
    }
    file << username << " " << password << "\n"; // 使用空格分隔
    file.close();
    return true;
}

// 登录验证（直接输出 1 或 0）
void login(const std::string& filename, const std::string& username, const std::string& password) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << R"({"status": "405","msg": "login error, file not exist"})" << username << std::endl;
        return;
    }

    std::map<std::string, std::string> users;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; // 跳过空行和注释

        std::istringstream lineStream(line);
        std::string name, pass;
        if (lineStream >> name >> pass) {
            users[name] = pass;
        }
    }
    file.close();

    // 验证
    if (users.find(username) != users.end() && users[username] == password) {
        std::cout << R"({"status": "200","msg": "login success"})" << std::endl;
    } else {
        std::cout << R"({"status": "406","msg": "login error"})" << std::endl;
    }
}

/**
 * 一个简单的注册登录CGI程序
 * 使用文本文件作为数据库
 * TODO: 对文件读写进行加锁
 */
int main(int argc, char *argv[]) {

    std::string username = argv[1];
    std::string password = argv[2];
    const char* authStr = argv[3];

    // perror(username.c_str());
    // perror(password.c_str());
    // perror(authStr);

    const std::string USER_TABLE_FILE = "usertable.txt";

    // 确保文件存在
    if (!fileExists(USER_TABLE_FILE)) {
        if (!createEmptyFile(USER_TABLE_FILE)) {
            std::cout << R"({"status": "401","msg": "cant create user file"})" << std::endl;
            return 1;
        }
    }

    if (std::strcmp(authStr, "1") == 0) {
        // 注册
        if (isUsernameTaken(USER_TABLE_FILE, username)) {
            std::cout << R"({"status": "402","msg": "register failure, user existed"})" << std::endl;
            return 1;
        }

        if (registerUser(USER_TABLE_FILE, username, password)) {
            std::cout << R"({"status": "200","msg": "register success"})" << std::endl;
            return 0;
        } else {
            std::cout << R"({"status": "403","msg": "register failure"})" << std::endl;
        }

    } else if (std::strcmp(authStr, "2") == 0) {
        // 登录
        login(USER_TABLE_FILE, username, password);
        
        return 0; // 登录逻辑输出 1/0，返回 0 表示程序执行成功
    } else {
        std::cout << R"({"status": "404","msg": "invalid authtype"})" << std::endl;
        return 1;
    }

    return 0;
}

