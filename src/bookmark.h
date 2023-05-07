#pragma once
#include <QString>

struct Bookmark
{
    Bookmark(const QString &name, long timestamp, long duration)
        : name(name), timestamp(timestamp), duration(duration) {};

    QString name;
    long timestamp;
    long duration;
};
