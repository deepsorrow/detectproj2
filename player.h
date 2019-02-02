#ifndef PLAYER_H
#define PLAYER_H
#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#include "functions.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <iostream>
#include <chrono>
using namespace std;
using namespace cv;
class Player : public QThread
{    Q_OBJECT

 public:
    Player(QObject *parent = 0);
    ~Player();
    bool loadVideo(string filename);
    void Play();
    void Stop();
    bool isStopped() const;
    void _increase_delay();
    void _decrease_delay();
    void set_delay(const int &_delay);
    int get_delay() { return delay; }
    void update_actualtime();
    std::string curr_videofile="";
    clock_t begin = clock();
    datetime requested_time;
    datetime actualtime;
    int framecount = 0;
    void adjust_time();
    bool is_changing = false;
    bool is_it_last_clip = false;

 protected:
     void run();
     void msleep(int ms);

 private:
    clock_t starttime;
    bool stop;
    QMutex mutex;
    QWaitCondition condition;
    int delay = 250;
    Mat RGBframe;
    int frameRate;
    bool was_at_requested_moment = true;
    VideoCapture capture;
    cv::Mat frame;


signals:
    void processedImage(const cv::Mat &frame);
    void nextvideo_requested();
};
#endif // VIDEOPLAYER_H
