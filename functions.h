#ifndef FILEMANAGEMENT_H
#define FILEMANAGEMENT_H
#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>
#include <opencv2/opencv.hpp>
#include <QCoreApplication>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <ctime>

static const int fps = 4;

class functions;

// структура datetime для хранения 'временных точек'
struct datetime
{
    datetime(){}
    datetime(const std::string &t){
        h = std::stoi(t.substr(0, 2));
        m = std::stoi(t.substr(3, 2));
        s = std::stoi(t.substr(6, 2));
        time = t;
    }
    datetime(const int &_h, const int &_m, const int &_s){
        int ss = _h*3600+_m*60+_s;
        time = to_time(ss, 0, 0, 0, 1);
        h = std::stoi(time.substr(0, 2));
        m = std::stoi(time.substr(3, 2));
        s = std::stoi(time.substr(6, 2));
    }
    auto time_format(const int n) -> std::string{
        std::string n_t = "";
        if(n <= 9)
            n_t = "0"+std::to_string(n);
        else
            n_t = std::to_string(n);
        return n_t;
    }
    auto to_time(const int counter, const int base_h,
                            const int base_m, const int base_s, const int _fps) -> std::string {
        int cur_time_s = static_cast<int>(counter/_fps)+base_s;
        int cur_time_m = cur_time_s/60+base_m;
        int cur_time_h = cur_time_m/60+base_h;
        cur_time_s = cur_time_s%60;
        cur_time_m = cur_time_m%60;

        return time_format(cur_time_h) + ":" + time_format(cur_time_m) + ":"
                + time_format(cur_time_s);
    }
    auto get_in_seconds() -> int { return (h*3600+m*60+s); }
    std::string time = "";
    int h=0, m=0, s=0;
    bool operator<(const datetime &dt) const{
        int s1 = h*3600+m*60+s;
        int s2 = dt.h*3600+dt.m*60+dt.s;
        if(s1 < s2)
            return true;
        else
            return false;
    }
    bool operator<=(const datetime &dt) const{
        int s1 = h*3600+m*60+s;
        int s2 = dt.h*3600+dt.m*60+dt.s;
        if(s1 <= s2)
            return true;
        else
            return false;
    }
    bool operator>(const datetime &dt) const{
        int s1 = h*3600+m*60+s;
        int s2 = dt.h*3600+dt.m*60+dt.s;
        if(s1 > s2)
            return true;
        else
            return false;
    }
    bool operator>=(const datetime &dt) const{
        int s1 = h*3600+m*60+s;
        int s2 = dt.h*3600+dt.m*60+dt.s;
        if(s1 >= s2)
            return true;
        else
            return false;
    }
    datetime operator-(const datetime &dt) {
        int s1 = h*3600+m*60+s;
        int s2 = dt.h*3600+dt.m*60+dt.s;
        datetime res = datetime(to_time(s1-s2, 0, 0, 0, 1));
        return res;
    }
    datetime operator+(const datetime &dt) {
        int s1 = h*3600+m*60+s;
        int s2 = dt.h*3600+dt.m*60+dt.s;
        datetime res = datetime(to_time(s2+s1, 0, 0, 0, 1));
        return res;
    }
    void operator=(const datetime &dt) {
        h = dt.h;
        m = dt.m;
        s = dt.s;
        time = datetime(h,m,s).time;
    }
    bool operator==(const datetime &dt) {
        int s1 = h*3600+m*60+s;
        int s2 = dt.h*3600+dt.m*60+dt.s;
        if(s1 == s2)
            return true;
        else
            return false;
    }
};

// структура entry, которая используется для хранения события и её сведений
struct entry{
public:
    entry() {}
    entry(const std::string &_id, const std::string &_time1, const std::string &_time2)
        : id(_id),  timestart(_time1), endtime(_time2) {
        elapsed = datetime(_time2) - datetime(_time1);
    }
    entry(const std::string &_time1, const std::string &_time2, const bool &_alert = false)
        : timestart(_time1), endtime(_time2), alert(_alert) {
        elapsed = datetime(_time2) - datetime(_time1);
    }
    std::string id = "";
    std::string timestart = "";
    std::string endtime = "";
    datetime elapsed;
    bool alert = false;
    bool found = false;

    bool has(const datetime &en) const{
        if(datetime(endtime) >= en && datetime(timestart) <= en)
            return true;
        else
            return false;
    }

};

class functions
{
public:
    static auto time_format(const int n) -> std::string;
    static auto to_time(const int counter, const int base_h,
                 const int base_m, const int base_s, const int _fps=fps) -> std::string;
    static auto get_vector_of_files(const std::string path) -> std::vector<std::string>;
    static auto export_data_to_str(const int fnum,
                    const int base_h, const int base_m, const int base_s,
                    std::vector<std::vector<int>> cntsareas) -> std::string;
    static auto parse_txtframe(std::string &framestring, int &j_count, int &_base_h,
                               int &_base_m, int &_base_s,
                               std::vector<std::vector<int>> &_cntsareas) -> void;
    static auto parse_mask_point(std::string &framestring, size_t &mask_n, int &x, int &y) -> void;
    static auto get_entries(const std::string date,
                                const std::string cashiernum, const QString hostname) -> std::vector<entry>;
    static auto _find_videofile_by_time(datetime dt,
                                        const std::vector<std::string> &list,
                                        int hpos, int mpos, int spos) -> std::string;

};
#endif // FILEMANAGEMENT_H
