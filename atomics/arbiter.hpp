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
    struct string_out : public out_port<std::string> { };
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
        std::string output;
    };
    state_type state;
    // ports definition

    using input_ports=std::tuple<typename defs::temperature_in, typename defs::humidity_in>;
    using output_ports=std::tuple<typename defs::string_out>;

    // internal transition
    void internal_transition() {
        state.propagating = false;
        //state.lightOn=!state.lightOn;
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        std::string s("");
        for(const auto &x : get_messages<typename defs::temperature_in>(mbs)){
            s += "T=" + to_string(x);
        }
        for(const auto &x : get_messages<typename defs::temperature_in>(mbs)){
            s += "H=" + to_string(x);
        }
        state.output = s;
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
        get_messages<typename defs::string_out>(bags).push_back(state.output);

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
