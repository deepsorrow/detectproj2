#include "player.h"

Player::Player(QObject *parent)
 : QThread(parent)
{
    stop = true;
}

bool Player::loadVideo(string filename) {
    capture.open(filename);

    adjust_time();

    if (capture.isOpened())
    {
        return true;
    }
    else
        return false;
}

void Player::Play()
{
    if (!isRunning()) {
        if (isStopped()){
            stop = false;
        }
        start(LowPriority);
    }
}

void Player::run()
{
    while(!stop){
        int i = 0;

        // если кадр почему-то не прочитался, считаем их количество. если таких непрерывно 5, то, значит,
        // видеофайл просто закончился, иначе это были битые кадры, пропускаем
        while(!capture.read(frame) && i < 5)
            i++;

        // переход к следующему видео, если такое имеется (!is_it_last_clip)
        if(i == 5 && !is_it_last_clip){
            i = 0;
            std::cout << "threadid: " << this->currentThreadId() << " has requested nextvideo." << std::endl;
            emit nextvideo_requested();
            this->msleep(delay);
        }
        // цикл отправки кадра на отображение
        else{
            if(!is_changing) // если ползунок на слайдере не перемещается, то время нужно обновлять
                update_actualtime();
            cv::resize(frame, frame, Size(frame.cols/2, frame.rows/2)); // уменьшаем размер в 2 раза

            emit processedImage(frame); // испускаем кадр, который будет пойман в mainwindow.cpp
            this->msleep(delay); // задержка перед следующим кадром (этим задается скорость воспроизведения)
        }
    }
}

Player::~Player()
{
    stop = true;
    wait();
}
void Player::Stop()
{
    stop = true;
}
void Player::msleep(int ms){
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
}
bool Player::isStopped() const{
    return this->stop;
}

// ускорение воспроизведения
void Player::_increase_delay(){
    delay = delay * 2;
}

// замедление воспроизведения
void Player::_decrease_delay(){
    delay = max(1, delay / 2);
}

// установить задержку вручную
void Player::set_delay(const int &_delay){
    delay = _delay;
}

void Player::update_actualtime(){
    framecount++;
    if(framecount%4==0)
        actualtime = datetime(actualtime.h, actualtime.m, actualtime.s + 1);
}

// переход к интересующим нас кадрам в видеофрагменте
void Player::adjust_time(){
    datetime diff_time = requested_time - actualtime;
    int difference_in_frames = (diff_time.h*3600+diff_time.m*60+diff_time.s)*4;
    capture.set(CAP_PROP_POS_FRAMES,  difference_in_frames);
    actualtime = requested_time;
    std::cout << "the time was adjusted!" << std::endl;
}
