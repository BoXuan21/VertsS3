#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>

// Helper function: toLowerCase
// Converts a string to lowercase for case-insensitive comparison
void toLowerCase(std::string &str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    // std::transform applies ::tolower to each character from start to end of the string
}

// Main function to search for files
// directory: current directory to search in
// filename: target filename to search
// recursive: if true, search subdirectories as well
// caseInsensitive: if true, perform a case-insensitive match
void findFile(const std::string &directory, const std::string &filename, bool recursive, bool caseInsensitive) {
    DIR *dir = opendir(directory.c_str()); // Open the directory
    if (!dir) { // Error handling if directory can't be opened
        std::cerr << "Error: Cannot open directory " << directory << std::endl;
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string entryName = entry->d_name;

        // Skip the current directory (.) and parent directory (..)
        if (entryName == "." || entryName == "..") {
            continue;
        }

        // Construct the full path of the entry
        std::string fullPath = directory + "/" + entryName;
        struct stat pathStat;
        stat(fullPath.c_str(), &pathStat); // Get file metadata

        // Check if entry is a directory and if recursive search is enabled
        if (S_ISDIR(pathStat.st_mode) && recursive) {
            // Recursive call without forking since subdirectory exploration is handled in a single process
            findFile(fullPath, filename, recursive, caseInsensitive);
        }
        else if (S_ISREG(pathStat.st_mode)) { // Check if entry is a regular file
            std::string searchName = filename;
            std::string targetName = entryName;

            // Perform case-insensitive comparison if specified
            if (caseInsensitive) {
                toLowerCase(searchName);
                toLowerCase(targetName);
            }

            // If the file matches the filename being searched for, print the result
            if (searchName == targetName) {
                std::cout << getpid() << ": " << filename << ": " << fullPath << std::endl;
                // Format: <pid>: <filename>: <full path>
            }
        }
    }

    closedir(dir); // Close the directory after processing all entries
}

// Main program to initiate parallel search processes for each filename
// This function forks a new process for each filename to search in parallel
int main(int argc, char *argv[]) {
    // Explanation:
    // 1. This program creates a child process for each filename provided.
    // 2. Each child process performs a file search in the specified directory
    //    (and subdirectories if -R is enabled).
    // 3. The parent waits for all child processes to prevent "zombie" processes.

    bool recursive = false;         // Flag for recursive search
    bool caseInsensitive = false;   // Flag for case-insensitive search
    std::string searchPath;         // Directory to search in
    std::vector<std::string> filenames; // List of filenames to search

    // Parse command-line arguments using getopt
    int opt;
    while ((opt = getopt(argc, argv, "Ri")) != -1) {
        switch (opt) {
            case 'R':
                recursive = true;
                break;
            case 'i':
                caseInsensitive = true;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-R] [-i] <searchpath> <filename1> [filename2] ...\n";
                return 1;
        }
    }

    // Collect search path and filenames from remaining arguments
    if (optind < argc) {
        searchPath = argv[optind++]; // First remaining argument is the search path
        while (optind < argc) {
            filenames.push_back(argv[optind++]); // Remaining arguments are filenames
        }
    } else {
        std::cerr << "Error: Missing required arguments <searchpath> and filenames.\n";
        return 1;
    }

    // Fork child processes to search for each filename
    for (const auto &filename : filenames) {
        pid_t pid = fork();
        if (pid == 0) {  // Child process
            findFile(searchPath, filename, recursive, caseInsensitive);
            exit(0);  // Exit child after finishing
        }
        else if (pid < 0) { // Error handling for fork failure
            std::cerr << "Error: Failed to fork for filename " << filename << std::endl;
            return 1;
        }
    }

    // Parent process waits for all child processes to finish
    while (wait(nullptr) > 0); // Wait for all child processes

    return 0;
}
