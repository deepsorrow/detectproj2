#include "functions.h"
#include "dirent.h"

auto functions::get_entries(const std::string date,
                            const std::string cashiernum, const QString hostname) -> std::vector<entry>{
    std::vector<entry> entries;
    //QCoreApplication a();
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(hostname);
    db.setPort(3306);
    db.setUserName("*HIDDEN*");
    db.setPassword("*HIDDEN*");
    db.setDatabaseName("*HIDDEN*");

    if(db.open())
        std::cout << "123" << std::endl;
    else
        std::cout << "321" << std::endl;

    QSqlQuery mysqlquery;
    QString query = QString::fromStdString("Select * from contextevents c"
                                           " left join context cn on c.ID_CONTEXT = cn.ID_CONTEXT"
                                           " where date(c.eventtime)='" + date +
                                           "' and cn.KodPers="+cashiernum);
    mysqlquery.exec(query);
    if(mysqlquery.size()>0){
        entry curr_entry;
        while(mysqlquery.next()){
            std::string idcontext = mysqlquery.value("ID_CONTEXT").toString()
                    .toUtf8().constData();
            std::string idevent = mysqlquery.value("ID_EVENT").toString()
                    .toUtf8().constData();
            std::string eventtime = mysqlquery.value("EventTime").toString()
                    .toUtf8().constData();
            eventtime = eventtime.substr(11, 8);

            if(idcontext == curr_entry.id){
                if(idevent == "5"){
                    curr_entry.endtime = eventtime;
                    entries.push_back(curr_entry);
                }
            }
            else{
                curr_entry.id = idcontext;
                curr_entry.timestart = eventtime;
            }
        }
    } else{
        std::cout << "mysqlquery is empty" << std::endl;
    }

    return entries;
}

auto functions::export_data_to_str(const int fnum,
             const int base_h, const int base_m, const int base_s,
             std::vector<std::vector<int>> cntsareas) -> std::string
{
    // номер кадра и начальное время
    std::string s1 = std::to_string(fnum)+";"+std::to_string(base_h)+
                 ":"+std::to_string(base_m)+":"+std::to_string(base_s);

    // контуры в области масок
    std::string s2 = "[";
    for(size_t i=0; i < cntsareas.size(); ++i){
        s2 += "{";
        for(size_t k=0; k < cntsareas[i].size(); ++k)
            s2 += std::to_string(cntsareas[i][k])+";";
        s2 += "}";
    }
    s2 += "]\n";

    return (s1 + s2);
}

auto functions::time_format(const int n) -> std::string{
    std::string n_t = "";
    if(n <= 9)
        n_t = "0"+std::to_string(n);
    else
        n_t = std::to_string(n);
    return n_t;
}

auto functions::to_time(const int counter, const int base_h,
                        const int base_m, const int base_s, const int _fps) -> std::string {
    int cur_time_s = static_cast<int>(counter/_fps)+base_s;
    int cur_time_m = cur_time_s/60+base_m;
    int cur_time_h = cur_time_m/60+base_h;
    cur_time_s = cur_time_s%60;
    cur_time_m = cur_time_m%60;

    return time_format(cur_time_h) + ":" + time_format(cur_time_m) + ":"
            + time_format(cur_time_s);
}

auto functions::get_vector_of_files(const std::string path) -> std::vector<std::string> {
    const char * path_c = path.c_str();
    std::vector<std::string> filenames;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (path_c)) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
          std::string file = ent->d_name;
          if(file.length() > 3)
            filenames.push_back(path+ent->d_name);
          else
            std::cout << file << " was omitted!" << std::endl;
      }
      closedir (dir);
    }
    return filenames;
}

auto functions::parse_txtframe(std::string &framestring, int &j_count, int &_base_h,
                               int &_base_m, int &_base_s,
                               std::vector<std::vector<int>> &_cntsareas) -> void{
    size_t pos = 0;
    std::string token;
    if(framestring.empty())
        return;
    pos = framestring.find(';');
    j_count = std::stoi(framestring.substr(0, pos));
    framestring.erase(0, pos + 1);

    pos = framestring.find(':');
    _base_h = std::stoi(framestring.substr(0, pos));
    framestring.erase(0, pos + 1);

    pos = framestring.find(':');
    _base_m = std::stoi(framestring.substr(0, pos));
    framestring.erase(0, pos + 1);

    pos = framestring.find("[{");
    _base_s = std::stoi(framestring.substr(0, pos));
    framestring.erase(0, pos + 1 + 1);  // erase "[{"

    size_t pos1, pos2;
    for(size_t i=0; i<3; ++i){
        // [{.FIRST.}, {...}, {...}]
        while ((pos1 = framestring.find(';')) != std::string::npos &&
               (pos2 = framestring.find('}')) != std::string::npos &&
               pos1 < pos2)
        {
            int cnt = std::stoi(framestring.substr(0, pos1));
            framestring.erase(0, pos1 + 1); // erase ';'

            _cntsareas[i].push_back(cnt);
        }
        framestring.erase(0, 2); //erase '{'
    }
}

auto functions::parse_mask_point(std::string &framestring, size_t &mask_n, int &x, int &y) -> void{

    size_t pos = 0;
    if(framestring.empty())
        return;
    pos = framestring.find(';');
    mask_n = static_cast<size_t>(std::stoi(framestring.substr(0, pos)));
    framestring.erase(0, pos + 1);

    pos = framestring.find(';');
    x = std::stoi(framestring.substr(0, pos));
    framestring.erase(0, pos + 1);

    y = std::stoi(framestring);
    framestring = "";
}

auto functions::_find_videofile_by_time(datetime dt,
                                        const std::vector<std::string> &videolist,
                                        int hpos, int mpos, int spos) -> std::string{
    size_t videolist_size = videolist.size();

    int h = std::stoi(videolist[0].substr(videolist[0].length()-hpos,2));
    int m = std::stoi(videolist[0].substr(videolist[0].length()-mpos,2));
    int s = std::stoi(videolist[0].substr(videolist[0].length()-spos,2));
    datetime prev_video_dt(h,m,s);

    for(size_t i = 1; i < videolist_size; ++i){
        h = std::stoi(videolist[i].substr(videolist[i].length()-hpos,2));
        m = std::stoi(videolist[i].substr(videolist[i].length()-mpos,2));
        s = std::stoi(videolist[i].substr(videolist[i].length()-spos,2));

        datetime curr_video_dt(h,m,s);
        if((dt < curr_video_dt && dt > prev_video_dt) || dt == prev_video_dt){
            std::cout << videolist[i-1] << std::endl;
            return videolist[i-1];
        }

        prev_video_dt = curr_video_dt;
    }

    return videolist[videolist_size-1];
}

