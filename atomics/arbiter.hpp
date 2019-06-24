/**
* Kyle Bjornson
* ARSLab - Carleton University
*/

#ifndef DISCO_ARBITER_HPP
#define DISCO_ARBITER_HPP

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

struct sensor_data {
    char sensor_name[17];
    float temperature;
    float humidity;
};

using namespace cadmium;
using namespace std;

//Port definition
struct arbiter_defs {
    struct lcd_update_out : public out_port<struct lcd_update> { };
    struct sensor_in : public in_port<struct sensor_data> { };
};

template<typename TIME>
class Arbiter {
    using defs=arbiter_defs; // putting definitions in context

private:

    //Create update line for temperature value
    void update_temperature(float temperature) {

        lcd_update_line update_line;

        update_line.line_index = 3;
        update_line.alignment = CENTER_MODE;

        if (isnan(temperature)) {
            sprintf(update_line.characters, "UNKNOWN");
        } else {
            sprintf(update_line.characters, "%.2f C", temperature);
        }
        state.output.lines.push_front(update_line);
    }

    //Create update line for humidity value
    void update_humidity(float humidity) {

        lcd_update_line update_line;

        update_line.line_index = 7;
        update_line.alignment = CENTER_MODE;

        if (isnan(humidity)) {
            sprintf(update_line.characters, "UNKNOWN");
        } else {
            sprintf(update_line.characters, "%.2f %%", humidity);
        }
        state.output.lines.push_front(update_line);
    }

    //Create update line for static text
    void populate_static_lines(void) {

        lcd_update_line update_line;

        update_line.line_index = 1;
        update_line.alignment = CENTER_MODE;
        sprintf(update_line.characters, "---Temperature---");
        state.output.lines.push_front(update_line);

        update_line.line_index = 5;
        update_line.alignment = CENTER_MODE;
        sprintf(update_line.characters, "----Humidity----");
        state.output.lines.push_front(update_line);
    }

    /*
    * Change LCD colour based on temperature value:
    * (Cold) Blue->Green->Red (Hot)
    * Grey indicates sensor failure
    */
    void update_lcd_colour(float temperature) {
        if (isnan(temperature)) {
            state.output.lcd_colour = LCD_COLOR_GRAY;
        } else if (temperature <= 18) {
            state.output.lcd_colour = LCD_COLOR_DARKBLUE;
        } else if (temperature <= 22) {
            state.output.lcd_colour = LCD_COLOR_LIGHTBLUE;
        } else if (temperature <= 25) {
            state.output.lcd_colour = LCD_COLOR_GREEN;
        } else if (temperature <= 28) {
            state.output.lcd_colour = LCD_COLOR_ORANGE;
        } else {
            state.output.lcd_colour = LCD_COLOR_DARKRED;
        }
    }

    //Create update line for title of sensor (Given by switch model)
    void update_sensor_name(const char *sensor_name) {

        lcd_update_line update_line;

        update_line.line_index = 10;
        update_line.alignment = CENTER_MODE;
        sprintf(update_line.characters, "%s", sensor_name);

        state.output.lines.push_front(update_line);
    }


public:

    // default constructor
    Arbiter() noexcept{
        state.propagating = false;
    }

    // state definition
    struct state_type{
        bool propagating;
        lcd_update output;
    };
    state_type state;

    // ports definition
    using input_ports=std::tuple<typename defs::sensor_in>;
    using output_ports=std::tuple<typename defs::lcd_update_out>;

    // internal transition
    void internal_transition() {
        state.propagating = false;
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {

        const std::vector<struct sensor_data> data = get_messages<typename defs::sensor_in>(mbs);

        if (data.size() == 1) {

            state.output.lines.clear();
            state.output.text_colour = LCD_COLOR_WHITE;

            //Prepare values for LCD
            update_temperature(data.front().temperature);
            update_lcd_colour(data.front().temperature);
            update_humidity(data.front().humidity);

            update_sensor_name(data.front().sensor_name);

            populate_static_lines();

            state.propagating = true;
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

        get_messages<typename defs::lcd_update_out>(bags).push_back(state.output);

        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        if(state.propagating)
        return TIME::zero();
        else
        return TIME::infinity();
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename Arbiter<TIME>::state_type& i) {
        os << i.output;
        return os;
    }
};


#endif // DISCO_ARBITER_HPP
