#include "detection.h"

// сделать запись события в '_all_entries'
auto TxtDetector::make_entry(const datetime &cur_time) -> void{
    std::string s_from = detected_time.time;
    std::string s_to = cur_time.time;

    _all_entries.push_back(entry(s_from, s_to, false));
}

auto TxtDetector::analyze_from_txt(const std::string &filename, const std::string &date,
                                   const std::string &cashiernum, const QString &hostname) -> void{
    std::ifstream txtdata;
    std::string framestring;

    txtdata.open(filename); // открываем файл, где записано кол-во движения в числовом формате

    dbentries = functions::get_entries(date, cashiernum, hostname); // получаем записи из базы данных

    size_t i_entry = 0;
    curr_dbentry = dbentries[i_entry];

    // создаем нужный размер вектора 'mask_vector' в зависимости от кол-ва вершин в маске
    for(size_t i = 0; i < masks_qnty; ++i)
        mask_vector.push_back(std::vector<cv::Point>());

    int prev_counter = 0;
    while (std::getline(txtdata, framestring)) {
        // парсим маску из файла
        //(чтобы была возможность её отображать в плеере)

        if(framestring.substr(0,5) == "-----"){
            std::cout << framestring.substr(5,31) << std::endl;
            if(framestring.substr(5,31) == "BEGIN MASKS VERTICES BLOCK-----")
                mask_is_parsing = true;
            else
                mask_is_parsing = false;
            continue;
        }

        if(mask_is_parsing == true){
            size_t mask_n;
            int x,y;
            functions::parse_mask_point(framestring, mask_n, x, y);
            add_vertex(mask_n, cv::Point(x/2,y/2));
            std::cout << mask_n << "," << x << "," << y << std::endl;
            continue;
        }
        std::vector<std::vector<int>> cntsareas(3);

        prev_counter = counter;
        functions::parse_txtframe(framestring, counter, base_h, base_m, base_s,
                                  cntsareas);

        // to determine framestep ( this is needed in case we didn't analyzed every single frame )
        if(!diff_is_defined && prev_counter!=0){
            diff = counter-prev_counter;
            if(diff < 1)
                throw("diff is less than 1!");
            diff_is_defined = true;
            std::cout << "diff = " << diff << "\n";
        }

        process_txtframe_to_find_purchases(cntsareas);

        // чтобы определить, какое событие мы должны ожидать следующим
        while(i_entry < dbentries.size()-1 && curr_time > datetime(dbentries[i_entry].endtime)){
                i_entry++;
                prev_entry = curr_dbentry;
                curr_dbentry = dbentries[i_entry];
        }
    }

    find_alerts();
}

auto TxtDetector::process_txtframe_to_find_purchases(const std::vector<std::vector<int>> _cnts) -> void
{
    curr_time = datetime(functions::to_time(counter, base_h, base_m, base_s));
    bool everymask_was_detected = true;
    for(size_t i = 0; i < masks_qnty; ++i){
        for(size_t j = 0; j < _cnts[i].size(); j++) {
            //монетница (движение около монетницы редки, поэтому там таймер ожидания больше)
            if(i == 2){
                if(_cnts[i][j] < 300)
                    continue;
                masks[i].set_detected(180);
            }
            //зоны покупателя\продавца
            else{
                if(_cnts[i][j] < 450)
                    continue;
                masks[i].set_detected(80);
            }
        }
        // если в какой-то из масок прекращено движение (таймер истёк)
        masks[i].timeout = std::max(0, masks[i].timeout-diff);
        if(!masks[i].timeout)
            everymask_was_detected = false;
    }

    // если в текущий момент идет покупка(открыт чек), то во всех зонах имитируем движение,
    // чтобы избежать разбиение покупки на несколько событий (например, покупатель может отойти)
    if(curr_dbentry.has(curr_time)){
        for(size_t i =0 ; i< masks_qnty; ++i)
            if(i==2)
                masks[i].set_detected(180);
            else
                masks[i].set_detected(80);

    }
    // если во всех зонах было движение, переводим was_detection в true
    if(everymask_was_detected){
        if(!was_detection)
            detected_time = datetime(functions::to_time(counter, base_h, base_m, base_s));
        was_detection=true;
    }


    //если движение было, а сейчас нет
    if(was_detection && !everymask_was_detected){
        // минус 20 секунд, чтобы не считать то время, в течение которого ожидалось движение
            datetime _curr_time_ = curr_time - datetime(0,0,20);
        // подсчет прошедшего времени
            datetime time_elapsed = _curr_time_ - detected_time;
        // здесь указывается порог (15 секунд)
            if(time_elapsed > datetime(0,0,15))
                make_entry(_curr_time_);
            was_detection = false;
    }
}


auto TxtDetector::find_alerts() -> void {
    // берем чеки из базы данных и сверяем их с нами найденными. если нашли, то
    // ставим true, чтобы потом их не добавлять в алерты

    for(auto &_db_entry_ : dbentries)
        for(size_t i = 0; i < _all_entries.size(); ++i)
            if(_all_entries[i].has(_db_entry_.endtime)){
                _all_entries[i].found = true;
                _db_entry_.found = true;
                break;
            }

    // добавляем ненайденные 'потенциальные покупки' в алерты
    for(auto &__entry__ : _all_entries)
        if(!__entry__.found)
            alerts.push_back(__entry__);
}

// добавление вершины для создание 2д-фигуры, которая будет использована как маска
auto TxtDetector::add_vertex(const size_t vertex, cv::Point p) -> void{
    mask_vector[vertex].push_back(p);
}
