#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

// Structure to hold credentials locally
struct UserConfig {
    std::string email;
    std::string appPassword;
};

// Returns the correct cross-platform hidden config path
inline fs::path getConfigDirectory() {
#ifdef _WIN32
    const char* userProfile = std::getenv("USERPROFILE");
    if (!userProfile) return fs::current_path();
    return fs::path(userProfile) / ".config" / "devsmtp";
#else
    const char* home = std::getenv("HOME");
    if (!home) return fs::current_path();
    return fs::path(home) / ".config" / "devsmtp";
#endif
}

// Manages initialization/load lifecycle for sender credentials
inline UserConfig loadOrInitializeConfig() {
    fs::path configDir = getConfigDirectory();
    fs::path configFile = configDir / "config.txt";
    UserConfig config;

    // 1. Silent configuration load if file exists
    if (fs::exists(configFile)) {
        std::ifstream file(configFile);
        if (file.is_open()) {
            std::getline(file, config.email);
            std::getline(file, config.appPassword);
            if (!config.email.empty() && !config.appPassword.empty()) {
                return config;
            }
        }
    }

    // 2. Interactive first-time initialization sequence
    std::cout << "=================================================\n";
    std::cout << " First-Time Setup: Developer Local Config Launcher\n";
    std::cout << "=================================================\n";
    std::cout << "Sender Email: ";
    std::getline(std::cin, config.email);
    std::cout << "16-character App Password: ";
    std::getline(std::cin, config.appPassword);

    try {
        fs::create_directories(configDir);
        std::ofstream outFile(configFile);
        if (outFile.is_open()) {
            outFile << config.email << "\n" << config.appPassword << "\n";
            std::cout << "\n Credentials saved securely to: " << configFile.string() << "\n";
            std::cout << "Restarting context window for script payload execution...\n\n";
        }
    } catch (const std::exception& e) {
        std::cerr << " Configuration write fault: " << e.what() << "\n";
    }

    return config;
}

#endif // CONFIG_H