#include "QApplication"
#include "QSlider"
#include "QAction"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <locale>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    myPlayer = new Player();
    qRegisterMetaType<cv::Mat>("cv::Mat"); // регистрируем cv::Mat(тип переменной из OpenCV) в нашем проекте
    QObject::connect(myPlayer, SIGNAL(processedImage(cv::Mat)), this, SLOT(updatePlayerUI(cv::Mat)));
    QObject::connect(myPlayer, SIGNAL(nextvideo_requested()), this, SLOT(nextvideo()));

    ui->setupUi(this);

    hpos=0; mpos=0; spos=0;

    // отключаем кнопки след.видео и пред.видео
    ui->nextmoment_button->setEnabled(false);
    ui->prevmoment_button->setEnabled(false);

    // связываем сигналы и слоты
    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &MainWindow::show_moment);
    connect(ui->listWidget2, &QListWidget::currentRowChanged, this, &MainWindow::show_moment2);
    connect(ui->playback_slider, &QSlider::sliderReleased, this, &MainWindow::settime);
    connect(ui->playback_slider, &QSlider::sliderMoved, this, &MainWindow::show_actual_time);
}

void MainWindow::updatePlayerUI(cv::Mat frame)
{
    // отображение скорости воспроизведения
    putText(frame, "Playback: " + std::to_string(250/myPlayer->get_delay()) + "x", cv::Point(0, 40),
            cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,195,0),2);

    prevtimecolor = timecolor;
    // если сейчас идет время из любой записи в векторе 'alerts', то время подсвечивается красным цветом
    if(is_it_alert_time(myPlayer->actualtime))
        timecolor = cv::Scalar(0, 0, 215);
    else
        timecolor = cv::Scalar(215, 215, 215);

    // если цвет времени сменился, то скорость воспроизведения ставим на стандартную(1x),
    // и увеличиваем сильно в размерах чтобы было заметно
    if(timecolor != prevtimecolor){
        myPlayer->set_delay(250);
        putText(frame, myPlayer->actualtime.time, cv::Point(0, 300),
                cv::FONT_HERSHEY_SIMPLEX, 4, cv::Scalar(0,0,255), 20);
    }
    else
        putText(frame, myPlayer->actualtime.time, cv::Point(0, 70),
            cv::FONT_HERSHEY_SIMPLEX, 0.7, timecolor, 2);

    if(show_masks)
        draw_masks(frame); // рисуем маску на cv::Mat frame

    //переводим изображение кадра из Cv::Mat(OpenCV'шный) в QImage
    img = QImage(frame.data, frame.cols,frame.rows, QImage::Format_RGB888);

    // ставим ползунок слайдера в нужное положение
    ui->playback_slider->setValue
            (myPlayer->actualtime.h*3600+myPlayer->actualtime.m*60+myPlayer->actualtime.s);

    if (!img.isNull())
    {
        // воспроизведение видео на объекте 'label'
        ui->label->setPixmap(QPixmap::fromImage(img.rgbSwapped()));
    }
}

MainWindow::~MainWindow()
{
    delete myPlayer;
    delete ui;
}

void MainWindow::on_start_button_clicked()
{
    // сменение значка на кнопке при пуске/паузе видео
    if(ui->start_button->text() == "||"){
        ui->start_button->setText(tr(">"));
        myPlayer->Stop();
    }
    else{
        myPlayer->Play();
        ui->start_button->setText(tr("||"));
    }
}

void MainWindow::on_slowdown_button_clicked()
{
    // замедление воспроизведения
    myPlayer->_increase_delay();
}

void MainWindow::on_nextmoment_button_clicked()
{
    // по нажатию по кнопке выбирается следующее событие (ниже в списке) текущего
    if(ui->listWidget->currentRow() != static_cast<int>(d.back().alerts.size()-1))
        ui->listWidget->setCurrentRow(ui->listWidget->currentRow()+1);
}

void MainWindow::on_prevmoment_button_clicked()
{
    // по нажатию по кнопке выбирается предыдущее событие (выше в списке) текущего
    if(ui->listWidget->currentRow() != 0)
        ui->listWidget->setCurrentRow(ui->listWidget->currentRow()-1);
}

void MainWindow::on_speedup_button_clicked()
{
    myPlayer->_decrease_delay();
}

// функция для перехода на нужное событие, выбранное в первом списке ('listWidget')
void MainWindow::show_moment(){
    clock_t end = clock();
    double elapsed_secs = double(end - myPlayer->begin) / CLOCKS_PER_SEC;

    std::cout << elapsed_secs << std::endl;
    if(elapsed_secs > 0.7){
        delete myPlayer;
        initplayer();

        myPlayer->is_changing = false;
        std::string selected_item = ui->listWidget->currentItem()->text().toUtf8().constData();
        selected_item = selected_item.substr(0, 8);
        selectedtime = datetime(selected_item);

        currentvideo = functions::_find_videofile_by_time(datetime(selected_item), videolist, hpos, mpos, spos);
        std::cout << "found_curr_video: " << currentvideo << "\n";

        // hiwatch сохраняет в .mp4, у него на других местах находятся час,минута,секунда начала видео
        if(currentvideo.substr(currentvideo.length()-3, 3) == "mp4"){
            hpos = 27; mpos = 25; spos = 23;
        }
        else{
            hpos = 15; mpos = 13; spos = 11;
        }

        update_currtime();
        myPlayer->requested_time = selectedtime;
        myPlayer->loadVideo(currentvideo);
        myPlayer->Play();
        ui->start_button->setText(tr("||"));

    }
}

// функция для перехода на нужное событие, выбранное во втором списке ('listWidget2')
void MainWindow::show_moment2(){
    clock_t end = clock();
    double elapsed_secs = double(end - myPlayer->begin) / CLOCKS_PER_SEC;

    std::cout << elapsed_secs << std::endl;
    // слишком быстро нельзя перепрыгивать между событиями - крашится
    if(elapsed_secs > 0.7){ // 0.7 было найдено эмпирическим путем
        delete myPlayer;
        initplayer();

        myPlayer->is_changing = false;
        std::string selected_item = ui->listWidget2->currentItem()->text().toUtf8().constData();
        selected_item = selected_item.substr(0, 8); // выбранное событие
        selectedtime = datetime(selected_item);

        currentvideo = functions::_find_videofile_by_time(datetime(selected_item), videolist, hpos, mpos, spos);
        update_currtime();
        myPlayer->requested_time = selectedtime;
        myPlayer->loadVideo(currentvideo);
        myPlayer->Play();
        ui->start_button->setText(tr("||"));
    }

    if(d.back().entries_without_customer.size() < 2)
        ui->listWidget2->setCurrentRow(2);
}

//переход к следующему видеофайлу
void MainWindow::nextvideo(){
    for(size_t i = 0; i < videolist.size(); ++i)
        if(videolist[i] == currentvideo){
            if(i+1 != videolist.size()){
                currentvideo = videolist[i+1];
                int _prev_delay = myPlayer->get_delay();
                delete myPlayer;
                initplayer();
                myPlayer->set_delay(_prev_delay);
                myPlayer->Play();
                update_currtime();
                std::cout << currentvideo << " should be next" << std::endl;
                break;
            }
        }
}

void MainWindow::initplayer(){
    myPlayer = new Player();
    QObject::connect(myPlayer, SIGNAL(processedImage(cv::Mat)), this, SLOT(updatePlayerUI(cv::Mat)));
    QObject::connect(myPlayer, SIGNAL(nextvideo_requested()), this, SLOT(nextvideo()));

    if(currentvideo!=""){
        myPlayer->loadVideo(currentvideo);
    }
}

void MainWindow::update_currtime(){
    std::cout << "currentvideo = " << currentvideo << "!!\n";
    int h = std::stoi(currentvideo.substr(currentvideo.length()-hpos,2));
    int m = std::stoi(currentvideo.substr(currentvideo.length()-mpos,2));
    int s = std::stoi(currentvideo.substr(currentvideo.length()-spos,2));
    myPlayer->actualtime = datetime(h,m,s);
    std::cout << "actualtime = " << myPlayer->actualtime.time << std::endl;
}

// задаем минимум и максимум для слайдера. минимум - видеофайл с минимальной датой/временем,
// максимум - видеофайл с максимальной датой/временем
void MainWindow::init_slider(){
    int h = std::stoi(videolist[0].substr(videolist[0].length()-hpos,2));
    int m = std::stoi(videolist[0].substr(videolist[0].length()-mpos,2));
    int s = std::stoi(videolist[0].substr(videolist[0].length()-spos,2));
    int min = h*3600+m*60+s + 1;
    ui->playback_slider->setMinimum(min);
    std::cout << "min: " << min << " = " << datetime(0,0,min).time << std::endl;

    size_t max_index = videolist.size()-1;
    VideoCapture lastfile(videolist[max_index]);
    h = std::stoi(videolist[max_index].substr(videolist[max_index].length()-hpos,2));
    m = std::stoi(videolist[max_index].substr(videolist[max_index].length()-mpos,2));
    s = std::stoi(videolist[max_index].substr(videolist[max_index].length()-spos,2));
    int max = h*3600 + m*60 + s + static_cast<int>(lastfile.get(CAP_PROP_FRAME_COUNT)/4) - 10;
    ui->playback_slider->setMaximum(max);
    std::cout << "max: " << max << " = " << datetime(0,0,max).time << std::endl;
}

//смотрим, где пользователь бросил ползунок на слайдере, и переходим на этот момент
void MainWindow::settime(){

    if(currentvideo!=""){
        datetime new_time = datetime(0, 0, ui->playback_slider->value());
        std::cout << "requested_time: " << new_time.time << std::endl;
        delete myPlayer;
        initplayer();

        currentvideo = functions::_find_videofile_by_time(new_time, videolist, hpos, mpos, spos);
        if(currentvideo == videolist[videolist.size()-1])
            myPlayer->is_it_last_clip = true;
        update_currtime();
        myPlayer->requested_time = new_time;

        myPlayer->loadVideo(currentvideo);
        myPlayer->Play();
        ui->start_button->setText(tr("||"));
    }
}

// чтобы при перемещении ползунка(когда пользователь его ещё не отпустил) показывалось время
void MainWindow::show_actual_time(){
    myPlayer->actualtime = datetime(0,0,ui->playback_slider->value());
    myPlayer->is_changing = true;
}

// рисуем маски красным цветом
auto MainWindow::draw_masks(cv::Mat &img) -> void{
    for(auto &mask : maskverticiesvector){
        for(size_t i=1; i<mask.size(); ++i){
            cv::line(img, mask[i-1], mask[i], cv::Scalar(0,0,255), 1);
        }
        cv::line(img, mask[mask.size()-1], mask[0], cv::Scalar(0,0,255), 1);
    }
}

auto MainWindow::populate_verticies() -> void{
    for(size_t i = 0; i < masks_qnty; ++i)
        maskverticiesvector.push_back(std::vector<cv::Point>());
}

auto MainWindow::add_vertex(const size_t vertex, cv::Point p) -> void{
    maskverticiesvector[vertex].push_back(p);
}

void MainWindow::on_show_masks_triggered()
{
    show_masks = !show_masks;
    std::cout << "showmasks changed!" << show_masks << std::endl;
}

void MainWindow::change_showmaks_option(){

}

// если запись alert включает (.has) в себя dt, то...
bool MainWindow::is_it_alert_time(datetime &dt) const{
    for(auto &alert : d.back().alerts)
        if(alert.has(dt))
            return true;
    return false;
}

// если кликнули по "Выбрать файл для анализа"
void MainWindow::on_changefile_action_triggered()
{
    file_to_analize = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "C:/Users/1987wrk/Documents/build-detectproj_2"
                          "-Desktop_Qt_5_11_2_MinGW_32bit-Release/inifiles",
                                                   tr("Text files (*.ini)")).toUtf8().constData();
    if(file_to_analize != ""){
        size_t pos = file_to_analize.find("inifiles");

        auto servernamelength = file_to_analize.find('_') - file_to_analize.find_last_of('/') - 2;
        auto offset = servernamelength - 1;
        std::string servername = file_to_analize.substr(pos+9, 2+offset);

        if(servername != "a7" && servername != "A7") // т.к. у А7 офисный адрес
            hostname = QString::fromStdString("192.168."+servername.substr(1, servernamelength)+".22");
        else
            hostname = QString::fromStdString("217.138.143.4");

        cashiernum = file_to_analize.substr(pos+21+offset, 1);
    }

}

// если кликнули по "Указать путь к видеофайлам"
void MainWindow::on_change_videopath_action_triggered()
{
    base_path = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                  "C:/inifiles",
                                                  QFileDialog::ShowDirsOnly
                                                  | QFileDialog::DontResolveSymlinks).toUtf8().constData();
    base_path += "/";
    std::cout << "selected path: " << base_path << "\n";
    videolist = functions::get_vector_of_files(base_path);
}

// если кликнули по "Начать анализ"
void MainWindow::on_begin_analyze_triggered()
{
    std::cout << "begin analyze triggered...\n";
    // если .txt файл выбран и папка с видеозаписями указана
    if(base_path != "" && file_to_analize != ""){
        myPlayer->Stop();
        delete myPlayer;

        //определяем, через сколько символов будут час, минута, секунда в имени файла
        if(videolist[0].substr(videolist[0].length()-3, 3) == "mp4"){
            hpos = 27; mpos = 25; spos = 23;
        }
        else{
            hpos = 15; mpos = 13; spos = 11;
        }
        std::cout << "videolist[0] = " << videolist[0] << "\n\nhpos = " << hpos << "; mpos= " << mpos << "; spos = " << spos << "\n\n";

        initplayer();
        init_slider();

        ui->listWidget->clear();
        ui->listWidget2->clear();

        d.push_back(TxtDetector());

        std::cout << "Analyze begin..." << std::endl;

        //определяем, через сколько символов будут год, месяц, день в имени файла
        size_t ypos, mpos, dpos;
        if(videolist[0].substr(videolist[0].length()-3,3) == "mp4"){
            // пример: 20181201090000_20181201090856_1.mp4
            ypos = 35; mpos = 35-4; dpos = 35-6;
        }
        else{
            // пример: 20181202_090453_6436.avi
            ypos = 24; mpos = 24-4; dpos = 24-6;
        }

        //вырезам год, месяц, день из имени файла
        std::string year = videolist[0].substr(videolist[0].length()-ypos,4);
        std::string month = videolist[0].substr(videolist[0].length()-mpos,2);
        std::string day = videolist[0].substr(videolist[0].length()-dpos,2);
        std::string date = year+"-"+month+"-"+day;
        std::cout << "the date is " << date << ", cashiernum = " << cashiernum << "\n";
        d.back().analyze_from_txt(file_to_analize, date, cashiernum, hostname);
        maskverticiesvector = d.back().get_vertices_vector();

        // уведомляем, какие чеки были найдены
        for(auto &_entry_ : d.back().dbentries)
            if(!_entry_.found)
                std::cout << _entry_.timestart << " - " << _entry_.endtime
                          << " NOT FOUND!!" << std::endl;
            else
                std::cout << _entry_.timestart << " - " << _entry_.endtime
                          << " OK! " << std::endl;


        // добавляем события(где движение во всех 3ех областях были, но чека
        // в базе данных за этот период не было) в listWidget(1ый список справа в плеере)
        // для просмотра
        for(entry &alert : d.back().alerts){
            QString qstring = QString::fromStdString(alert.timestart+
                                                     " — "+alert.endtime+"// "
                                                     +std::to_string(alert.elapsed.get_in_seconds()));
            ui->listWidget->addItem(qstring);
        }

        // добавляем чеки(те, которые в базе данных есть, но обнаружено на видео не было)
        for(entry &_db_entry : d.back().dbentries){
            if(!_db_entry.found){
                _db_entry.elapsed = datetime(_db_entry.endtime) - datetime(_db_entry.timestart);
                QString qstring = QString::fromStdString(_db_entry.timestart+
                                                         " — "+_db_entry.endtime+"// "
                                                         +std::to_string
                                                         (_db_entry.elapsed.get_in_seconds()));
                ui->listWidget2->addItem(qstring);
            }
        }

        //сделали кнопки активными
        ui->nextmoment_button->setEnabled(true);
        ui->prevmoment_button->setEnabled(true);
    }
}
