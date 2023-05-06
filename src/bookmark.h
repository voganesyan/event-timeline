#pragma once
#include <QString>

struct Bookmark
{
    Bookmark(const QString &name, int timestamp, int duration)
        : name(name), timestamp(timestamp), duration(duration) {};

    QString name;
    int timestamp;
    int duration;
};
