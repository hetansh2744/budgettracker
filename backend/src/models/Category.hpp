#pragma once
#include <string>

class Category {
private:
    int id;
    std::string name;

public:
    Category(int id, const std::string& name)
        : id(id), name(name) {}

    int getId() const { return id; }
    std::string getName() const { return name; }
};
