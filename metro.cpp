#include "metro.h"


#define __STATION(text) text



std::map<std::string, std::pair<std::mutex, std::mutex>> gr_stations_mtx;

void get_mtx(){
    gr_stations_mtx["Bakmil"];
    gr_stations_mtx["Nariman Narimanov"];
    gr_stations_mtx["Ganjlik"];
    gr_stations_mtx["Ulduz"];
    gr_stations_mtx["Koroglu"];
    gr_stations_mtx["Gara Garayev"];
    gr_stations_mtx["Neftchiler"];
    gr_stations_mtx["Xalqlar Dostlugu"];
    gr_stations_mtx["Ahmedli"];
    gr_stations_mtx["Hazi Aslanov"];
}


Virtual_time global_time;

void Virtual_time::update_time(const int &add_time)
{
    std::lock_guard<std::mutex> lock(v_t_mtx);
    minutes += add_time;
    
    while (minutes >= 60) 
    {
        minutes -= 60;
        hours++;
    }
    
    if (hours >= 24)
    {
        hours -= 24; 
    } 
}

std::string Virtual_time::get_time()
{
    std::lock_guard<std::mutex> lock(v_t_mtx);
    std::ostringstream oss;

    oss << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2) << std::setfill('0') << minutes;

    return oss.str();
}

void Virtual_time::sleep(const int &add_time)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(200 * add_time));
    update_time(add_time);
}

void fill_green_and_red_line(std::map<std::string, Station> &stations)
{
    stations["Bakmil"] = {"Bakmil", "Nariman Narimanov", "Depo"};
    stations["Nariman Narimanov"] = {"Nariman Narimanov", "Ganjlik", "Ulduz"};
    stations["Ganjlik"] = {"Ganjlik", "28 May", "Nariman Narimanov"};

    stations["Ulduz"] = {"Ulduz", "Nariman Narimanov", "Koroglu"};
    stations["Koroglu"] = {"Koroglu", "Ulduz", "Gara Garayev"};
    stations["Gara Garayev"] = {"Gara Garayev", "Koroglu", "Neftchiler"};
    stations["Neftchiler"] = {"Neftchiler", "Gara Garayev", "Xalqlar Dostlugu"};
    stations["Xalqlar Dostlugu"] = {"Xalqlar Dostlugu", "Neftchiler", "Ahmedli"};
    stations["Ahmedli"] = {"Ahmedli", "Xalqlar Dostlugu", "Hazi Aslanov"};
    stations["Hazi Aslanov"] = {"Hazi Aslanov", "Ahmedli", ""};
}

void red_line::Red_line::fill_stations()
{
    stations["28 May"] = {"28 May", "Sahil", "Ganjlik"};
    stations["Sahil"] = {"Sahil", "Icherisheher", "28 May"};
    stations["Icherisheher"] = {"Icherisheher", "", "Sahil"};
}

red_line::Red_line::Red_line()
{
    fill_green_and_red_line(stations);
    red_line::Red_line::fill_stations();

    filename.open("output_red_line.txt", std::ios::app);
}

red_line::Red_line::~Red_line()
{
    if(filename.is_open())
    {
        filename.close();
    }
}

bool red_line::Red_line::is_working_time()
{
    return (global_time.hours >= 5 && global_time.hours < 20);
}

void red_line::Red_line::move_train(const size_t &train_id, const std::string &start_station)
{
    std::string current_station = start_station;

    bool going_forward = true;

    while(red_line::Red_line::is_working_time())
    {
        std::mutex &current_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        {
            std::lock_guard<std::mutex> lock(current_mutex);

            std::ostringstream oss;
            if(going_forward)
            {
                oss << "[ " << global_time.get_time() << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].next) << "\n\n";
            }
            else
            {
                oss << "[ " << global_time.get_time() << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].previous) << "\n\n";
            }
            write_to_file(oss.str());

            global_time.sleep(3);
        }


        if(going_forward)
        {
            current_station = stations[current_station].next;

            if(stations[current_station].next.empty()) 
            {
                current_station = "Icherisheher"; 
                going_forward = false;
            }
        }
        else
        {
            current_station = stations[current_station].previous;
            
            if(current_station == "Bakmil" || current_station == "Hazi Aslanov")
            {
                going_forward = true;
            }
        }

        std::mutex &new_station_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(new_station_mutex);
        
        std::ostringstream oss;
        oss << "[ " << global_time.get_time() << " ] Train " << train_id << " in " << __STATION(stations[current_station].name) << " station\n\n";
        write_to_file(oss.str());

        global_time.sleep(1);
    
    }

    std::string prev_station = "";

    while(!(stations[current_station].name == "Nariman Narimanov" && prev_station == "Ganjlik"))
    {
        std::mutex &current_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        {
            std::lock_guard<std::mutex> lock(current_mutex);

            std::ostringstream oss;
            if(going_forward)
            { 
                oss << "[ " << global_time.get_time() << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].next) << "\n\n";
            }
            else
            {
                oss << "[ " << global_time.get_time() << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].previous) << "\n\n";
            }
            write_to_file(oss.str());
        }

        global_time.sleep(1);

        if(going_forward)
        {
            current_station = stations[current_station].next;

            if(stations[current_station].next.empty()) 
            {
                current_station = "Icherisheher"; 
                going_forward = false;
            }
        }
        else
        {
            current_station = stations[current_station].previous;
            
            if(current_station == "Bakmil" || current_station == "Hazi Aslanov")
            {
                going_forward = true;
            }
        }
        
        std::mutex &new_station_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(new_station_mutex);
            
        std::ostringstream oss;
        oss << "[ " << global_time.get_time() << " ] Train " << train_id << " in " << __STATION(stations[current_station].name) << " station\n\n";
        write_to_file(oss.str());

        if(going_forward)
        {
            prev_station = stations[current_station].previous;
        }
        else
        {
            prev_station = stations[current_station].next;
        }
    }

    {
        std::mutex &current_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(current_mutex);

        std::ostringstream oss;

        oss << "[ " << global_time.get_time() << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations["Bakmil"].name) << "\n\n";

        write_to_file(oss.str());
    }

    global_time.sleep(1);

    current_station = stations["Bakmil"].name;

    {
        std::mutex &new_station_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(new_station_mutex);
            
        std::ostringstream oss;
        oss << "[ " << global_time.get_time() << " ] Train " << train_id << " in " << __STATION(stations[current_station].name) << " station\n\n";
        write_to_file(oss.str());
        
    }
    

    {
        std::mutex &last_station_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(last_station_mutex);

        std::ostringstream oss;
        oss << "[ " << global_time.get_time() << " ] Train " << train_id << " has stopped for the night.\n";
        write_to_file(oss.str());
    }

}

void red_line::Red_line::start()
{
    std::vector<std::thread> trains;

    for(size_t i = 1000; i < 1010; i++)
    {
        trains.emplace_back(&Red_line::move_train, this, i + 1, "Bakmil");
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    for(auto &train: trains)
    {
        train.join();
    }
}

void red_line::Red_line::write_to_file(const std::string &message)
{
    std::lock_guard<std::mutex> file_lock(file_mtx);
    if(filename.is_open())
    {
        filename << message << std::flush;
    }
    else
    {
        std::cout << "ERROR in red_line\n";
    }
}

//=================================================================================================

void violet_line::Violet_line::fill_stations()
{
    stations["Xodjasan"] = {"Xodjasan", "Avtovogzal", "Depo"};
    stations["Avtovogzal"] = {"Avtovogzal", "Memar Adjemi", "Xodjasan"};
    stations["Memar Adjemi"] = {"Memar Adjemi", "8 Noyabr", "Avtovogzal"};
    stations["8 Noyabr"] = {"8 Noyabr", "", "Memar Adjemi"};
}

violet_line::Violet_line::Violet_line()
{
    violet_line::Violet_line::fill_stations();

    filename.open("output_violet_line.txt", std::ios::app);
}

violet_line::Violet_line::~Violet_line()
{
    if(filename.is_open())
    {
        filename.close();
    }
}

bool violet_line::Violet_line::is_working_time(const int &hour)
{
    return (hour >= start_hour || hour < end_hour);
}

void violet_line::Violet_line::move_train(const size_t &train_id, const std::string &start_station, int &virtual_hour, int &virtual_minutes)
{
    std::string current_station = start_station;

    bool going_forward = true;

    while(violet_line::Violet_line::is_working_time(virtual_hour))
    {
        std::mutex &current_mutex = going_forward ? stations_mtx[current_station].first : stations_mtx[current_station].second;
        {
            std::lock_guard<std::mutex> lock(current_mutex);

            if(going_forward)
            {
                std::ostringstream oss;
                oss << "[ " << std::setw(2) << std::setfill('0') << virtual_hour << ":" << std::setw(2) << std::setfill('0') << virtual_minutes << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].next) << "\n\n";                
                write_to_file(oss.str());
            }
            else
            {
                std::ostringstream oss;
                oss << "[ " << std::setw(2) << std::setfill('0') << virtual_hour << ":" << std::setw(2) << std::setfill('0') << virtual_minutes << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].previous) << "\n\n";
                write_to_file(oss.str());
            }
        }

        {
            std::lock_guard<std::mutex> time_lock(time_mtx);
            
            virtual_minutes += measure;
            if (virtual_minutes >= 60) 
            {
                virtual_minutes -= 60;
                virtual_hour++;
                if (virtual_hour == 24)
                {
                    virtual_hour = 0;
                }  
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(600));
        }

        if(going_forward)
        {
            current_station = stations[current_station].next;

            if(stations[current_station].next.empty()) 
            {
                current_station = "8 Noyabr"; 
                going_forward = false;
            }
        }
        else
        {
            current_station = stations[current_station].previous;
            
            if(current_station == "Xodjasan")
            {
                going_forward = true;
            }
        }
            std::mutex &new_station_mutex = going_forward ? stations_mtx[current_station].first : stations_mtx[current_station].second;
        
            std::lock_guard<std::mutex> lock(new_station_mutex);
            
            std::ostringstream oss;
            oss << "[ " << std::setw(2) << std::setfill('0') << virtual_hour << ":" << std::setw(2) << std::setfill('0') << virtual_minutes << " ] Train " << train_id << " in " << __STATION(stations[current_station].name) << " station\n\n";
            write_to_file(oss.str());

            {
                std::lock_guard<std::mutex> time_lock(time_mtx);
                
                virtual_minutes += 1;
                if (virtual_minutes >= 60) 
                {
                    virtual_minutes -= 60;
                    virtual_hour++;
                    if (virtual_hour == 24)
                    {
                        virtual_hour = 0;
                    }  
                }
    
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
    }

    while(stations[current_station].name != "Xodjasan")
    {
        std::mutex &current_mutex = going_forward ? stations_mtx[current_station].first : stations_mtx[current_station].second;
        {
            std::lock_guard<std::mutex> lock(current_mutex);

            std::ostringstream oss;
            if(going_forward)
            { 
                oss << "[ " << std::setw(2) << std::setfill('0') << virtual_hour << ":" << std::setw(2) << std::setfill('0') << virtual_minutes << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].next) << "\n\n";                
            }
            else
            {
                oss << "[ " << std::setw(2) << std::setfill('0') << virtual_hour << ":" << std::setw(2) << std::setfill('0') << virtual_minutes << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].previous) << "\n\n";
            }
            write_to_file(oss.str());
        }

        {
            std::lock_guard<std::mutex> time_lock(time_mtx);
            
            virtual_minutes += 2;
            if (virtual_minutes >= 60) 
            {
                virtual_minutes -= 60;
                virtual_hour++;
                if (virtual_hour == 24)
                {
                    virtual_hour = 0;
                }  
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(600));
        }

        if(going_forward)
        {
            current_station = stations[current_station].next;

            if(stations[current_station].next.empty()) 
            {
                current_station = "8 Noyabr"; 
                going_forward = false;
            }
        }
        else
        {
            current_station = stations[current_station].previous;
        }

        std::mutex &new_station_mutex = going_forward ? stations_mtx[current_station].first : stations_mtx[current_station].second;

        std::lock_guard<std::mutex> lock(new_station_mutex);
            
        std::ostringstream oss;
        oss << "[ " << std::setw(2) << std::setfill('0') << virtual_hour << ":" << std::setw(2) << std::setfill('0') << virtual_minutes << " ] Train " << train_id << " in " << __STATION(stations[current_station].name) << " station\n\n";
        write_to_file(oss.str());

        {
            std::lock_guard<std::mutex> time_lock(time_mtx);
            
            virtual_minutes += 1;
            if (virtual_minutes >= 60) 
            {
                virtual_minutes -= 60;
                virtual_hour++;
                if (virtual_hour == 24)
                {
                    virtual_hour = 0;
                }  
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    {
        std::mutex &last_station_mutex = going_forward ? stations_mtx[current_station].first : stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(last_station_mutex);

        std::ostringstream oss;
        oss << "[ " << std::setw(2) << std::setfill('0') << virtual_hour << ":" << std::setw(2) << std::setfill('0') << virtual_minutes << " ] Train " << train_id << " has stopped for the night.\n";
        write_to_file(oss.str());
    }

}

void violet_line::Violet_line::start()
{
    std::vector<std::thread> trains;

    int global_virtual_minutes = 0;
    int global_virtual_hour = start_hour;

    for(size_t i = 1100; i < 1107; i++)
    {
        trains.emplace_back(&Violet_line::move_train, this, i + 1, "Xodjasan", std::ref(global_virtual_hour), std::ref(global_virtual_minutes));
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    for(auto &train: trains)
    {
        train.join();
    }
}

void violet_line::Violet_line::write_to_file(const std::string &message)
{
    std::lock_guard<std::mutex> file_lock(file_mtx);
    if(filename.is_open())
    {
        filename << message << std::flush;
    }
    else
    {
        std::cout << "ERROR in violet_line\n";
    }
}

//============================================================================

void light_green_line::Light_green_line::fill_stations()
{
    stations["Cafar Cabbarli"] = {"Cafar Cabbarli", "Xatai", "Xatai"};
    stations["Xatai"] = {"Xatai", "Cafar Cabbarli", "Cafar Cabbarli"};
}

light_green_line::Light_green_line::Light_green_line()
{
    light_green_line::Light_green_line::fill_stations();

    filename.open("output_light_green_line.txt", std::ios::app);
}

light_green_line::Light_green_line::~Light_green_line()
{
    if(filename.is_open())
    {
        filename.close();
    }
}

bool light_green_line::Light_green_line::is_working_time(const int &hour)
{
    return (hour >= start_hour || hour < end_hour);
}

void light_green_line::Light_green_line::move_train(const size_t &train_id, const std::string &start_station, int &virtual_hour, int &virtual_minutes)
{
    std::string current_station = start_station;

    bool going_forward = true;

    while(light_green_line::Light_green_line::is_working_time(virtual_hour))
    {
        std::mutex &current_mutex = going_forward ? stations_mtx[current_station].first : stations_mtx[current_station].second;
        {
            std::lock_guard<std::mutex> lock(current_mutex);

            if(going_forward)
            {
                std::ostringstream oss;
                oss << "[ " << std::setw(2) << std::setfill('0') << virtual_hour << ":" << std::setw(2) << std::setfill('0') << virtual_minutes << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].next) << "\n\n";                
                write_to_file(oss.str());
            }
            else
            {
                std::ostringstream oss;
                oss << "[ " << std::setw(2) << std::setfill('0') << virtual_hour << ":" << std::setw(2) << std::setfill('0') << virtual_minutes << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].previous) << "\n\n";
                write_to_file(oss.str());
            }
        }

        {
            std::lock_guard<std::mutex> time_lock(time_mtx);
            
            virtual_minutes += measure;
            if (virtual_minutes >= 60) 
            {
                virtual_minutes -= 60;
                virtual_hour++;
                if (virtual_hour == 24)
                {
                    virtual_hour = 0;
                }  
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(600));
        }

        if(going_forward)
        {
            current_station = stations[current_station].next;
        }
        else
        {
            current_station = stations[current_station].previous;
        }

        std::mutex &new_station_mutex = going_forward ? stations_mtx[current_station].first : stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(new_station_mutex);
        
        std::ostringstream oss;
        oss << "[ " << std::setw(2) << std::setfill('0') << virtual_hour << ":" << std::setw(2) << std::setfill('0') << virtual_minutes << " ] Train " << train_id << " in " << __STATION(stations[current_station].name) << " station\n\n";
        write_to_file(oss.str());

        {
            std::lock_guard<std::mutex> time_lock(time_mtx);
            
            virtual_minutes += 1;
            if (virtual_minutes >= 60) 
            {
                virtual_minutes -= 60;
                virtual_hour++;
                if (virtual_hour == 24)
                {
                    virtual_hour = 0;
                }  
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    {
        std::mutex &last_station_mutex = going_forward ? stations_mtx[current_station].first : stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(last_station_mutex);

        std::ostringstream oss;
        oss << "[ " << std::setw(2) << std::setfill('0') << virtual_hour << ":" << std::setw(2) << std::setfill('0') << virtual_minutes << " ] Train " << train_id << " has stopped for the night.\n\n";
        write_to_file(oss.str());
    }
}

void light_green_line::Light_green_line::start()
{
    std::vector<std::thread> trains;

    int global_virtual_minutes = 0;
    int global_virtual_hour = start_hour;

    for(size_t i = 3100; i < 3102; i++)
    {
        trains.emplace_back(&Light_green_line::move_train, this, i + 1, "Xatai", std::ref(global_virtual_hour), std::ref(global_virtual_minutes));
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    for(auto &train: trains)
    {
        train.join();
    }
}

void light_green_line::Light_green_line::write_to_file(const std::string &message)
{
    std::lock_guard<std::mutex> file_lock(file_mtx);
    if(filename.is_open())
    {
        filename << message << std::flush;
    }
    else
    {
        std::cout << "ERROR in light_green_line\n";
    }
}

//================================================================================

void green_line::Green_line::fill_stations()
{
    stations["Darnagul"] = {"Darnagul", "", "Azadliq Prospekti"};
    stations["Azadliq Prospekti"] = {"Azadliq Prospekti", "Darnagul", "Nasimi"};
    stations["Nasimi"] = {"Nasimi", "Azadliq Prospekti", "Memar Adjemi"};
    stations["Memar Adjemi"] = {"Memar Adjemi", "Nasimi", "20 Yanvar"};
    stations["20 Yanvar"] = {"20 Yanvar", "Memar Adjemi", "Inshaatchilar"};
    stations["Inshaatchilar"] = {"Inshaatchilar", "20 Yanvar", "Elmler Akademiyasi"};
    stations["Elmler Akademiyasi"] = {"Elmler Akademiyasi", "Inshaatchilar", "Nizami"};
    stations["Nizami"] = {"Nizami", "Elmler Akademiyasi", "28 May"};

    stations["28 May"] = {"28 May", "Nizami", "Ganjlik"};
}

green_line::Green_line::Green_line()
{
    fill_green_and_red_line(stations);
    green_line::Green_line::fill_stations();

    filename.open("output_green_line.txt", std::ios::app);
}

green_line::Green_line::~Green_line()
{
    if(filename.is_open())
    {
        filename.close();
    }
}

bool green_line::Green_line::is_working_time()
{
    return (global_time.hours >= 5 && global_time.hours < 20);
}

void green_line::Green_line::move_train(const size_t &train_id, const std::string &start_station)
{
    std::string current_station = start_station;

    bool going_forward = true;

    while(green_line::Green_line::is_working_time())
    {
        {
            std::mutex &current_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
            std::lock_guard<std::mutex> lock(current_mutex);

            std::ostringstream oss;
            if(going_forward)
            {
                oss << "[ " << global_time.get_time() << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].next) << "\n\n";                
            }
            else
            {
                oss << "[ " << global_time.get_time() << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].previous) << "\n\n";
            }
            write_to_file(oss.str());

            global_time.sleep(3);
        }


        if(going_forward)
        {
            current_station = stations[current_station].next;

            if(stations[current_station].next.empty()) 
            {
                current_station = "Darnagul"; 
                going_forward = false;
            }
        }
        else
        {
            current_station = stations[current_station].previous;
            
            if(current_station == "Bakmil" || current_station == "Hazi Aslanov")
            {
                going_forward = true;
            }
        }

        std::mutex &new_station_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(new_station_mutex);
        
        std::ostringstream oss;
        oss << "[ " << global_time.get_time() << " ] Train " << train_id << " in " << __STATION(stations[current_station].name) << " station\n\n";
        write_to_file(oss.str());

        global_time.sleep(1);
    
    }

    std::string prev_station = "";

    while(!(stations[current_station].name == "Nariman Narimanov" && prev_station == "Ganjlik"))
    {
        std::mutex &current_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        {
            std::lock_guard<std::mutex> lock(current_mutex);

            std::ostringstream oss;
            if(going_forward)
            { 
                oss << "[ " << global_time.get_time() << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].next) << "\n\n";                
            }
            else
            {
                oss << "[ " << global_time.get_time() << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations[current_station].previous) << "\n\n";
            }
            write_to_file(oss.str());
        }

        global_time.sleep(1);

        if(going_forward)
        {
            current_station = stations[current_station].next;

            if(stations[current_station].next.empty()) 
            {
                current_station = "Darnagul"; 
                going_forward = false;
            }
        }
        else
        {
            current_station = stations[current_station].previous;
            
            if(current_station == "Bakmil" || current_station == "Hazi Aslanov")
            {
                going_forward = true;
            }
        }
        
        std::mutex &new_station_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(new_station_mutex);
            
        std::ostringstream oss;
        oss << "[ " << global_time.get_time() << " ] Train " << train_id << " in " << __STATION(stations[current_station].name) << " station\n\n";
        write_to_file(oss.str());

        if(going_forward)
        {
            prev_station = stations[current_station].previous;
        }
        else
        {
            prev_station = stations[current_station].next;
        }
    }

    {
        std::mutex &new_station_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(new_station_mutex);

        std::ostringstream oss;

        oss << "[ " << global_time.get_time() << " ] Train " << train_id << " moving: " << __STATION(stations[current_station].name) << " ---> " << __STATION(stations["Bakmil"].name) << "\n\n";

        write_to_file(oss.str());
    }

    global_time.sleep(1);

    current_station = stations["Bakmil"].name;

    {
        std::mutex &new_station_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(new_station_mutex);
            
        std::ostringstream oss;
        oss << "[ " << global_time.get_time() << " ] Train " << train_id << " in " << __STATION(stations[current_station].name) << " station\n\n";
        write_to_file(oss.str());
        
    }

    {
        std::mutex &last_station_mutex = going_forward ? gr_stations_mtx[current_station].first : gr_stations_mtx[current_station].second;
        std::lock_guard<std::mutex> lock(last_station_mutex);

        std::ostringstream oss;
        oss << "[ " << global_time.get_time() << " ] Train " << train_id << " has stopped for the night.\n";
        write_to_file(oss.str());
    }

}

void green_line::Green_line::start()
{
    std::vector<std::thread> trains;

    for(size_t i = 4100; i < 4111; i++)
    {
        trains.emplace_back(&Green_line::move_train, this, i + 1, "Bakmil");
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    for(auto &train: trains)
    {
        train.join();
    }
}

void green_line::Green_line::write_to_file(const std::string &message)
{
    std::lock_guard<std::mutex> file_lock(file_mtx);
    if(filename.is_open())
    {
        filename << message << std::flush;
    }
    else
    {
        std::cout << "ERROR in light_green_line\n";
    }
}


