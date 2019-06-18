/**
* Ben Earle and Kyle Bjornson
* ARSLab - Carleton University
*
* Blinky:
* Simple modle to toggle the LED using DEVS internal transitions.
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

using namespace cadmium;
using namespace std;

//Port definition
struct arbiter_defs {
    struct lcd_update_out : public out_port<struct lcd_update> { };
    struct temperature_in : public in_port<float> { };
    struct humidity_in : public in_port<float> { };
    struct ts_in : public in_port<struct cartesian_coordinates> { };
};

template<typename TIME>
class Arbiter {
    using defs=arbiter_defs; // putting definitions in context

private:

    void update_temperature(double temperature) {

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

    void update_humidity(double humidity) {

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

    void update_lcd_colour(double temperature) {
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

    using input_ports=std::tuple<typename defs::temperature_in, typename defs::humidity_in, typename defs::ts_in>;
    using output_ports=std::tuple<typename defs::lcd_update_out>;

    // internal transition
    void internal_transition() {
        state.propagating = false;
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {

        const auto temperature_vector = get_messages<typename defs::temperature_in>(mbs);
        const auto humidity_vector = get_messages<typename defs::humidity_in>(mbs);

        if (temperature_vector.size() == 1 && humidity_vector.size() == 1) {

            state.output.lines.clear();
            state.output.text_colour = LCD_COLOR_WHITE;

            update_temperature(temperature_vector.front());
            update_lcd_colour(temperature_vector.front());
            update_humidity(humidity_vector.front());

            populate_static_lines();
        }

        const auto coordinates_vector = get_messages<typename defs::ts_in>(mbs);
        if (coordinates_vector.size() == 1) {
            //struct cartesian_coordinates coordinates = x;
        }

        state.propagating = true;

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
        os << "Propagating? " << (i.propagating ? 1 : 0);
        return os;
    }
};


#endif // DISCO_ARBITER_HPP
