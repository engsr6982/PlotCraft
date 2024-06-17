#pragma once
#include "fmt/format.h"
#include "plotcraft/Macro.h"
#include <iomanip>
#include <sstream>
#include <string>


using string = std::string;

namespace plo::utils {

struct Date {
private:
    std::tm mTm;

    PLAPI std::tm _now() {
        std::time_t t = std::time(nullptr);
        std::tm     now;
        localtime_s(&now, &t);
        return now;
    }

public:
    PLAPI int  getYear(int year = 0) const { return mTm.tm_year + 1900 + year; }
    PLAPI bool setYear(int year) { return mTm.tm_year = year - 1900; }
    PLAPI int  getMonth() const { return mTm.tm_mon + 1; }
    PLAPI bool setMonth(int month) { return mTm.tm_mon = month - 1; }
    PLAPI int  getDay() const { return mTm.tm_mday; }
    PLAPI bool setDay(int day) { return mTm.tm_mday = day; }
    PLAPI int  getHour() const { return mTm.tm_hour; }
    PLAPI bool setHour(int hour) { return mTm.tm_hour = hour; }
    PLAPI int  getMinute() const { return mTm.tm_min; }
    PLAPI bool setMinute(int minute) { return mTm.tm_min = minute; }
    PLAPI int  getSecond() const { return mTm.tm_sec; }
    PLAPI bool setSecond(int second) { return mTm.tm_sec = second; }

    PLAPI std::time_t getTime() const {
        std::tm tm = mTm;
        return std::mktime(&tm);
    }

    PLAPI string toString() {
        return fmt::format(
            "{}-{}-{} {}:{}:{}",
            std::to_string(getYear()),
            std::to_string(getMonth()),
            std::to_string(getDay()),
            std::to_string(getHour()),
            std::to_string(getMinute()),
            std::to_string(getSecond())
        );
    }

    // Date constructors
    PLAPI Date(std::tm tm) : mTm(tm) {}
    PLAPI Date() { mTm = _now(); }
    PLAPI Date(int year, int month, int day, int hour = 0, int minute = 0, int second = 0) {
        mTm.tm_year = year - 1900;
        mTm.tm_mon  = month - 1;
        mTm.tm_mday = day;
        mTm.tm_hour = hour;
        mTm.tm_min  = minute;
        mTm.tm_sec  = second;
    }

    // Static methods
    PLAPI static Date now() { return Date{}; }
    PLAPI static Date future(int seconds) {
        std::time_t now    = std::time(nullptr);
        std::time_t future = now + seconds;
        std::tm     tm;
        localtime_s(&tm, &future);
        return Date{tm};
    }
    PLAPI static Date parse(const string& str) {
        // 字符串格式: "YYYY-MM-DD HH:MM:SS"
        std::istringstream iss(str);
        string             datePart, timePart;
        std::getline(iss, datePart, ' ');
        std::getline(iss, timePart);

        std::tm tm = {};
        int     year, month, day, hour, minute, second;

        sscanf_s(datePart.c_str(), "%d-%d-%d", &year, &month, &day);     // 解析日期部分
        sscanf_s(timePart.c_str(), "%d:%d:%d", &hour, &minute, &second); // 解析时间部分

        tm.tm_year = year - 1900;
        tm.tm_mon  = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min  = minute;
        tm.tm_sec  = second;

        return Date(tm);
    }
    PLAPI static Date clone(Date& d) { return Date(d.mTm); }

    // 重载操作符
    PLAPI Date& operator=(const Date& other) {
        if (this != &other) {
            mTm = other.mTm;
        }
        return *this;
    }
    PLAPI bool operator==(const Date& other) const {
        return getYear() == other.getYear() && getMonth() == other.getMonth() && getDay() == other.getDay()
            && getHour() == other.getHour() && getMinute() == other.getMinute() && getSecond() == other.getSecond();
    }
    PLAPI bool operator<(const Date& other) const {
        if (getYear() != other.getYear()) return getYear() < other.getYear();
        if (getMonth() != other.getMonth()) return getMonth() < other.getMonth();
        if (getDay() != other.getDay()) return getDay() < other.getDay();
        if (getHour() != other.getHour()) return getHour() < other.getHour();
        if (getMinute() != other.getMinute()) return getMinute() < other.getMinute();
        return getSecond() < other.getSecond();
    }
    PLAPI bool operator>(const Date& other) const { return other < *this; }
    PLAPI bool operator<=(const Date& other) const { return !(*this > other); }
    PLAPI bool operator>=(const Date& other) const { return !(*this < other); }
    PLAPI bool operator!=(const Date& other) const { return !(*this == other); }
    PLAPI std::time_t operator-(const Date& other) const { return (this->getTime() - other.getTime()); }
};

} // namespace plo::utils