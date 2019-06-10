/**
* Kyle Bjornson
* ARSLab - Carleton University
*/

#ifndef DIGITAL_TEMPERATURE_SH31_HPP
#define DIGITAL_TEMPERATURE_SH31_HPP

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

#ifdef ECADMIUM
//This class will interface with a digital input pin.
#include "../mbed.h"
#include "../drivers/sht31.hpp"

using namespace cadmium;
using namespace std;

//Port definition
struct digitalTemperatureHumidity_defs{
    struct temperature_out : public out_port<float> { };
    struct humidity_out : public out_port<float> { };
};

template<typename TIME>
class DigitalTemperatureHumidity {
    using defs=digitalTemperatureHumidity_defs; // putting definitions in context

public:
    //Parameters to be overwriten when instantiating the atomic model
    drivers::sht31* temp_humid_sensor; //(PB_9, PB_8);
    TIME   pollingRate;

    // default constructor
    DigitalTemperatureHumidity() noexcept {
        MBED_ASSERT(false);
        throw std::logic_error("Input atomic model requires a pin definition");
    }
    DigitalTemperatureHumidity(PinName sda, PinName scl) {
        new (this) DigitalTemperatureHumidity(sda, scl, TIME("00:00:01:000"));
    }
    DigitalTemperatureHumidity(PinName sda, PinName scl, TIME rate) {
        pollingRate = rate;
        temp_humid_sensor = new drivers::sht31(sda, scl);
        state.temperature = temp_humid_sensor->read_temperature();
        state.humidity = temp_humid_sensor->read_humidity();
    }

    // state definition
    struct state_type{
        float temperature;
        float humidity;
    };
    state_type state;

    // ports definition
    using input_ports=std::tuple<>;
    using output_ports=std::tuple<typename defs::temperature_out, typename defs::humidity_out>;

    // internal transition
    void internal_transition() {
        state.temperature = temp_humid_sensor->read_temperature();
        state.humidity = temp_humid_sensor->read_humidity();
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        MBED_ASSERT(false);
        throw std::logic_error("External transition called in a model with no input ports");
    }
    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
    }

    // output function
    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;

        get_messages<typename defs::temperature_out>(bags).push_back(state.temperature);
        get_messages<typename defs::humidity_out>(bags).push_back(state.humidity);

        return bags;
    }

    // time_advance function
    TIME time_advance() const {
        return pollingRate;
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename DigitalTemperatureHumidity<TIME>::state_type& i) {
        os << "Temperature: " ;//<< i.temperature << ", Humidity: " << i.humidity;
        return os;
    }
};
#else
#include <cadmium/io/iestream.hpp>
using namespace cadmium;
using namespace std;

const char* LCD_FILE = "./inputs/temperature_in.txt"

//Port definition
struct digitalTemperatureHumidity_defs{
    struct out : public out_port<bool> { };
};

template<typename TIME>
class DigitalInput : public iestream_input<bool,TIME, digitalTemperatureHumidity_defs>{
public:
    DigitalInput() : iestream_input<bool,TIME, digitalTemperatureHumidity_defs>(file_path) {}
};
#endif // ECADMIUM

#endif // DIGITAL_TEMPERATURE_SH31_HPP
