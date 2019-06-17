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
    struct lcd_update_out : public out_port<lcd_update> { };
    struct temperature_in : public in_port<float> { };
    struct humidity_in : public in_port<float> { };
};

template<typename TIME>
class Arbiter {
    using defs=arbiter_defs; // putting definitions in context

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

    using input_ports=std::tuple<typename defs::temperature_in, typename defs::humidity_in>;
    using output_ports=std::tuple<typename defs::lcd_update_out>;

    // internal transition
    void internal_transition() {
        state.propagating = false;
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {

        state.output.lines.clear();
        lcd_update_line update_line;

        state.output.text_colour = LCD_COLOR_WHITE;

        for(const auto &x : get_messages<typename defs::temperature_in>(mbs)){

            if (isnan(x)) {
                state.output.lcd_colour = LCD_COLOR_GRAY;
            } else if (x <= 18) {
                state.output.lcd_colour = LCD_COLOR_DARKBLUE;
            } else if (x <= 22) {
                state.output.lcd_colour = LCD_COLOR_LIGHTBLUE;
            } else if (x <= 25) {
                state.output.lcd_colour = LCD_COLOR_GREEN;
            } else if (x <= 28) {
                state.output.lcd_colour = LCD_COLOR_ORANGE;
            } else {
                state.output.lcd_colour = LCD_COLOR_DARKRED;
            }

            update_line.line_index = 3;
            update_line.alignment = CENTER_MODE;

            if (isnan(x)) {
                sprintf(update_line.characters, "UNKNOWN");
            } else {
                sprintf(update_line.characters, "%.2f C", x);
            }

            state.output.lines.push_front(update_line);


        }

        for(const auto &x : get_messages<typename defs::humidity_in>(mbs)){
            update_line.line_index = 7;
            update_line.alignment = CENTER_MODE;

            if (isnan(x)) {
                sprintf(update_line.characters, "UNKNOWN");
            } else {
                sprintf(update_line.characters, "%.2f %%", x);
            }

            state.output.lines.push_front(update_line);
        }

        update_line.line_index = 1;
        update_line.alignment = CENTER_MODE;
        sprintf(update_line.characters, "---Temperature---");
        state.output.lines.push_front(update_line);

        update_line.line_index = 5;
        update_line.alignment = CENTER_MODE;
        sprintf(update_line.characters, "----Humidity----");
        state.output.lines.push_front(update_line);

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
