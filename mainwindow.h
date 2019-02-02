#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "detection.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <player.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    auto speedup_playback() -> void;
    auto slowdown_playback() -> void;
    void update_currtime();
    auto draw_masks(cv::Mat &img) -> void;
    auto populate_verticies() -> void;
    ~MainWindow();

private slots:
    void updatePlayerUI(cv::Mat frame);
    void on_start_button_clicked();
    void on_slowdown_button_clicked();
    void on_speedup_button_clicked();
    void on_nextmoment_button_clicked();
    void on_prevmoment_button_clicked();
    void on_show_masks_triggered();
    void on_changefile_action_triggered();
    void on_change_videopath_action_triggered();
    void on_begin_analyze_triggered();
    void change_showmaks_option();
    void show_actual_time();
    void settime();
    void show_moment();
    void show_moment2();
    void nextvideo();

private:
    void initplayer();
    void init_slider();
    auto add_vertex(const size_t vertex, cv::Point p) -> void;
    bool is_it_alert_time(datetime &dt) const;
    Ui::MainWindow *ui;
    std::vector<std::string> videolist;
    Player* myPlayer;
    QImage img;
    std::string cashiernum;
    cv::Scalar timecolor, prevtimecolor;
    std::vector<TxtDetector> d;
    int cooldown = 0;
    datetime selectedtime;
    std::string currentvideo = "";
    const size_t masks_qnty = 3;
    int hpos, mpos, spos;
    bool show_masks = false;
    std::string file_to_analize, base_path = "";
    QString hostname;
    std::vector<std::vector<cv::Point>> maskverticiesvector;
    int current_i = 0;
};

#endif // MAINWINDOW_H
