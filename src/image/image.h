#pragma once

#include <string>

class Image {
private:
    const std::string path;
public:
    Image(std::string path) : path(std::move(path)) {};
    std::string getPath() {return path;}
};