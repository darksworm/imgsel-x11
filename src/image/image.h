#pragma once

#include <string>
#include <Imlib2.h>

class Image {
private:
    std::string path;
    std::string filename;
    std::string extension;
    Imlib_Image image = nullptr;
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

    void setImg(Imlib_Image img) {
        this->image = img;
    }

    Imlib_Image getImg() {
        return this->image;
    }

    std::string getPath() { return path; }

    std::string getFilename() {
        return filename;
    }

    std::string getExtension() {
        return extension;
    }

    bool operator==(const Image& other) {
        return other.image == this->image;
    }
};