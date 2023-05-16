#pragma once
#include <QString>

using namespace std::string_literals;

struct Bookmark
{
    explicit Bookmark(int id, long timestamp, long duration)
        : name(QString("Bookmark %1").arg(id)),
          timestamp(timestamp),
          duration(duration) {}

    long end_time() const
    {
        return timestamp + duration;
    }

    QString name;
    long timestamp;
    long duration;
};
