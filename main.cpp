#include "metro.h"

int main()
{
    get_mtx();


    red_line::Red_line metro_red;
    violet_line::Violet_line metro_violet;
    light_green_line::Light_green_line metro_light_green;
    green_line::Green_line metro_green_line;
    
    std::thread red_thread(&red_line::Red_line::start, &metro_red);
    std::thread violet_thread(&violet_line::Violet_line::start, &metro_violet);
    std::thread light_green_thread(&light_green_line::Light_green_line::start, &metro_light_green);
    std::thread green_thread(&green_line::Green_line::start, &metro_green_line);

    red_thread.join();
    violet_thread.join();
    light_green_thread.join();
    green_thread.join();

    return 0;
}
