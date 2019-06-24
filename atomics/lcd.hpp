/**
* Kyle Bjornson
* ARSLab - Carleton University
*/

#ifndef DISCO_LCD_HPP
#define DISCO_LCD_HPP

#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <limits>
#include <math.h>
#include <assert.h>
#include <memory>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <algorithm>
#include <limits>
#include <random>

#ifdef RT_ARM_MBED
    #include "../drivers/LCD_DISCO_F429ZI/LCD_DISCO_F429ZI.h"
#else

typedef enum {
    CENTER_MODE = 0x01, RIGHT_MODE = 0x02, LEFT_MODE = 0x03,
} Text_AlignModeTypdef;

//LCD colours
#define LCD_COLOR_BLUE          0xFF0000FF
#define LCD_COLOR_GREEN         0xFF00FF00
#define LCD_COLOR_RED           0xFFFF0000
#define LCD_COLOR_CYAN          0xFF00FFFF
#define LCD_COLOR_MAGENTA       0xFFFF00FF
#define LCD_COLOR_YELLOW        0xFFFFFF00
#define LCD_COLOR_LIGHTBLUE     0xFF8080FF
#define LCD_COLOR_LIGHTGREEN    0xFF80FF80
#define LCD_COLOR_LIGHTRED      0xFFFF8080
#define LCD_COLOR_LIGHTCYAN     0xFF80FFFF
#define LCD_COLOR_LIGHTMAGENTA  0xFFFF80FF
#define LCD_COLOR_LIGHTYELLOW   0xFFFFFF80
#define LCD_COLOR_DARKBLUE      0xFF000080
#define LCD_COLOR_DARKGREEN     0xFF008000
#define LCD_COLOR_DARKRED       0xFF800000
#define LCD_COLOR_DARKCYAN      0xFF008080
#define LCD_COLOR_DARKMAGENTA   0xFF800080
#define LCD_COLOR_DARKYELLOW    0xFF808000
#define LCD_COLOR_WHITE         0xFFFFFFFF
#define LCD_COLOR_LIGHTGRAY     0xFFD3D3D3
#define LCD_COLOR_GRAY          0xFF808080
#define LCD_COLOR_DARKGRAY      0xFF404040
#define LCD_COLOR_BLACK         0xFF000000
#define LCD_COLOR_BROWN         0xFFA52A2A
#define LCD_COLOR_ORANGE        0xFFFFA500
#define LCD_COLOR_TRANSPARENT   0xFF000000

#endif

struct lcd_update_line {
    uint8_t line_index;
    char characters[17];
    Text_AlignModeTypdef alignment;

    friend std::ostream& operator<<(std::ostream& os, const lcd_update_line& i) {
        os << "Index: " << to_string(i.line_index) << ", Text: " << i.characters << "\n";
        return os;
    }
};

struct lcd_update{
    std::list<lcd_update_line> lines;
    uint32_t lcd_colour;
    uint32_t text_colour;

    friend std::ostream& operator<<(std::ostream& os, const lcd_update& i) {
        os << "LCD Colour: " << to_string(i.lcd_colour) << ", Text Colour: " << to_string(i.text_colour) << "\n---Lines---\n";

        for (lcd_update_line line : i.lines) {
            os << line;
        }

        return os;
    }
};

/******************************************************************************
* REAL-TIME IMPLEMENTATION
*****************************************************************************/
#ifdef RT_ARM_MBED

#include "../mbed.h"

using namespace cadmium;
using namespace std;

//Port definition
struct LCD_defs{
    struct in : public in_port<struct lcd_update> {};
};

template<typename TIME>
class LCD {
    using defs=LCD_defs; // putting definitions in context
public:
    LCD_DISCO_F429ZI lcd;

    // default c onstructor
    LCD() noexcept{
        BSP_LCD_SetFont(&Font20);

    }

    // state definition
    struct state_type{
        lcd_update output;
    };
    state_type state;

    // ports definition
    using input_ports=std::tuple<typename defs::in>;
    using output_ports=std::tuple<>;

    // internal transition
    void internal_transition() {}

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        for(const auto &x : get_messages<typename defs::in>(mbs)){
            state.output = x;
        }

        lcd.Clear(state.output.lcd_colour);
        lcd.SetBackColor(state.output.lcd_colour);
        lcd.SetTextColor(state.output.text_colour);

        for (lcd_update_line line : state.output.lines) {
            lcd.DisplayStringAt(0, LINE(line.line_index), (uint8_t*) line.characters, line.alignment);
        }

    }
    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
    }

    // output function
    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        return std::numeric_limits<TIME>::infinity();
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename LCD<TIME>::state_type& i) {
        os << "Printed: " << "1";//i.output;
        return os;
    }
};


/******************************************************************************
* SIMULATOR IMPLEMENTATION
*****************************************************************************/
#else

#include <cadmium/io/oestream.hpp>

using namespace cadmium;
using namespace std;

//Output file for LCD
const char* LCD_FILE = "./outputs/LCD_out.txt";

//Port definition
struct LCD_defs{
    struct in : public in_port<struct lcd_update> {};
};

template<typename TIME>
class LCD : public oestream_output<struct lcd_update, TIME, LCD_defs>{
public:
    LCD() : oestream_output<struct lcd_update, TIME, LCD_defs>(LCD_FILE) {}
};

#endif //RT_ARM_MBED
#endif // DISCO_LCD_HPP
