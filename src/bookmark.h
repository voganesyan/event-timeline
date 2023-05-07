#pragma once
#include <string>

using namespace std::string_literals;

struct Bookmark
{
    Bookmark(int id, long timestamp, long duration)
        : name("Bookmark "s + std::to_string(id)), timestamp(timestamp), duration(duration) {};

    std::string name;
    long timestamp;
    long duration;
};
