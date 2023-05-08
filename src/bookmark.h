#pragma once
#include <string>

using namespace std::string_literals;

struct Bookmark
{
    explicit Bookmark(int id, long timestamp, long duration)
        : name("Bookmark "s + std::to_string(id)), timestamp(timestamp), duration(duration) {};

    long end_time() const { return timestamp + duration; }

    std::string name;
    long timestamp;
    long duration;
};
