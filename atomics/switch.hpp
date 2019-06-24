/**
* Ben Earle and Kyle Bjornson
* ARSLab - Carleton University
*
* Blinky:
* Simple modle to toggle the LED using DEVS internal transitions.
*/

#ifndef DISCO_SWITCH_HPP
#define DISCO_SWITCH_HPP

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
struct switch_defs {
    struct temperature_in_1 : public in_port<float> { };
    struct humidity_in_1 : public in_port<float> { };
    struct temperature_in_2 : public in_port<float> { };

    struct sensor_out : public out_port<struct sensor_data> { };
    struct ts_in : public in_port<struct cartesian_coordinates> { };
};

template<typename TIME>
class Switch {
    using defs=switch_defs; // putting definitions in context

public:

    // default constructor
    Switch() noexcept{
        state.propagating = false;
        state.sensor_idx = 0;
        sprintf(state.sensor_update[0].sensor_name, "Digital");
        sprintf(state.sensor_update[1].sensor_name, "Analog");
        state.sensor_update[1].humidity = NAN;
    }

    // state definition
    struct state_type{
        bool propagating;
        int sensor_idx;
        sensor_data sensor_update[2];
    };
    state_type state;

    // ports definition
    using input_ports=std::tuple<typename defs::temperature_in_1,
    typename defs::humidity_in_1,
    typename defs::temperature_in_2,
    typename defs::ts_in>;

    using output_ports=std::tuple<typename defs::sensor_out>;

    // internal transition
    void internal_transition() {
        state.propagating = false;
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {

        const auto coordinates_vector = get_messages<typename defs::ts_in>(mbs);
        if (coordinates_vector.size() == 1) {
            //Check if pressing bottom of screen + some debouncing
            if (coordinates_vector.front().y > 200 && e > TIME("00:00:00:100")) {
                state.sensor_idx = (state.sensor_idx == 1) ? 0:1;
                state.propagating = true;
            }

        }

        //Update Temperature from Digital Sensor 1
        for(const auto &x : get_messages<typename defs::temperature_in_1>(mbs)){
            state.sensor_update[0].temperature = x;
            state.propagating = true;
        }

        //Update Humidity from Digital Sensor 1
        for(const auto &x : get_messages<typename defs::humidity_in_1>(mbs)){
            state.sensor_update[0].humidity = x;
            state.propagating = true;
        }

        //Update Temperature from Analog Sensor 2
        for(const auto &x : get_messages<typename defs::temperature_in_2>(mbs)){
            const unsigned int beta = 4275;

            //Input is a float between 0 and 1. Convert to a Temperature in Celsius
            float resistance = 10000.0 * ((1.0/x) - 1);
            state.sensor_update[1].temperature =(1/((log(resistance/10000.0)/beta) + (1.0/298.15)))-273.15;

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

        get_messages<typename defs::sensor_out>(bags).push_back(state.sensor_update[state.sensor_idx]);

        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        if(state.propagating)
        return TIME::zero();
        else
        return TIME::infinity();
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename Switch<TIME>::state_type& i) {
        os << "Sensor Switch Number: " << i.sensor_idx;
        return os;
    }
};


#endif // DISCO_SWITCH_HPP
