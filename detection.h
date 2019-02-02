#ifndef DETECTION_H
#define DETECTION_H
#include "masking.h"
#include "functions.h"
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <QCoreApplication>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QVariant>
#include <ctime>

class TxtDetector{
public:
    TxtDetector() {
        // задаем нужный размер вектору masks, в зависимости от кол-ва областей для анализа
        for(size_t i=0; i<masks_qnty; ++i)
            masks.push_back(Mask());
    }
    auto process_txtframe_to_find_purchases(const std::vector<std::vector<int>> _cnts) -> void;
    auto analyze_from_txt(const std::string &filename, const std::string &date,
                          const std::string &cashiernum, const QString &hostname) -> void;

    auto get_result() -> std::string { return filestr; }
    auto get_vertices_vector() -> std::vector<std::vector<cv::Point>> { return mask_vector; }

    std::vector<entry> alerts, entries_without_customer, dbentries;
private:
    auto make_entry(const datetime &cur_time) -> void;
    auto make_entry_without_customer(const datetime &cur_time,
                                     const datetime &time_elapsed) -> void;
    auto add_vertex(const size_t vertex, cv::Point p) -> void;
    auto find_alerts() -> void;

    int base_h, base_m, base_s;
    int was_detection = false;
    int counter = 0;
    int diff = 1;
    const size_t masks_qnty = 3;
    entry curr_dbentry, prev_entry;
    datetime curr_time, detected_time, detected_time_only_cashier;
    std::vector<Mask> masks;
    std::vector<entry> _all_entries;
    std::vector<std::vector<cv::Point>> mask_vector;
    std::string filestr;
    QString hostname;
    bool only_cashier_previously_detected = false;
    bool only_cashier_was_detected = false;
    bool mask_is_parsing = false;
    bool diff_is_defined = false;

};

#endif // DETECTION_H
