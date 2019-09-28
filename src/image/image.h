#pragma once

#include <string>

class Image {
private:
    const std::string path;
    std::string filename;
    std::string extension;
public:
    Image(std::string path) : path(std::move(path)) {
        auto lastDotPos = this->path.find_last_of('.');

        auto pathLen = this->path.length();

        if(lastDotPos != std::string::npos && lastDotPos + 1 < pathLen) {
            extension = this->path.substr(lastDotPos + 1);
        } else {
            extension = "";
        }

        auto lastSlashPos = this->path.find_last_of('/');

        if(lastSlashPos != std::string::npos && lastSlashPos + 1 < pathLen) {
            filename = this->path.substr(lastSlashPos + 1);
        } else {
            filename = this->path;
        }
    }

    std::string getPath() { return path; }

    std::string getFilename() {
        return filename;
    }

    std::string getExtension() {
        return extension;
    }
};