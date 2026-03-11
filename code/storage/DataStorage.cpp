#include "DataStorage.h"
#include <fstream>
#include <filesystem>

using namespace std;

namespace storage {
    string DataStorage::pathForDataFile(const string &filename) {
        namespace fs = std::filesystem;

        const vector<string> candidates = {
            "storage/data/" + filename,
            "../storage/data/" + filename,
            "data/" + filename,
            "code/storage/data/" + filename,
            "../code/storage/data/" + filename
        };

        for (const auto &path : candidates) {
            if (fs::exists(path)) {
                return path;
            }
        }

        // Default to repo-root storage/data to keep reads/writes consistent.
        return "storage/data/" + filename;
    }

    vector <string> DataStorage::readAll(const string &filename) {
        vector<string> lines;
        ifstream file(pathForDataFile(filename));
        string line;
        while (getline(file, line)) {
            lines.push_back(line);
        }
        return lines;
    }

    bool DataStorage::writeAll(const string &filename, const vector<string> &lines) {
        ofstream file(pathForDataFile(filename), ios::trunc);
        if (!file.is_open()) {
            return false;
        }
        for (const auto &line : lines) {
            file << line << "\n";
        }
        return true;
    }

    bool DataStorage::appendLine(const string &filename, const string &line) {
        ofstream file(pathForDataFile(filename), ios::app);
        if (!file.is_open()) {
            return false;
        }
        file << line << "\n";
        return true;
    }
}