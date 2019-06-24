/**
* Kyle Bjornson
* ARSLab - Carleton University
*/
#include <iostream>
#include <chrono>
#include <algorithm>
#include <string>

#define MISSED_DEADLINE_TOLERANCE 1000000

#include <cadmium.h>

#include <NDTime.hpp>
#include <cadmium/io/iestream.hpp>

#include "../atomics/lcd.hpp"
#include "../atomics/digital_temp_humidity.hpp"
#include "../atomics/arbiter.hpp"
#include "../atomics/touch_screen.hpp"
#include "../atomics/switch.hpp"

#include <cadmium/real_time/arm_mbed/io/analogInput.hpp>

#ifdef RT_ARM_MBED
#include "../mbed.h"
#include <cadmium/real_time/arm_mbed/embedded_error.hpp>
#else
//Dummy pins for Temperature Sensors
const char* PC_9;
const char* PA_8;
const char* PF_6;
#endif

using namespace std;

using hclock=chrono::high_resolution_clock;
using TIME = NDTime;


int main(int argc, char ** argv) {

    #ifdef RT_ARM_MBED
    //Logging is done over cout in RT_ARM_MBED
    struct oss_sink_provider{
        static std::ostream& sink(){
            return cout;
        }
    };
    #else
    // all simulation timing and I/O streams are ommited when running embedded
    auto start = hclock::now(); //to measure simulation execution time

    static std::ofstream out_data("disco_output.txt");
    struct oss_sink_provider{
        static std::ostream& sink(){
            return out_data;
        }
    };
    #endif

    /*************** Loggers *******************/
    using info=cadmium::logger::logger<cadmium::logger::logger_info, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
    using debug=cadmium::logger::logger<cadmium::logger::logger_debug, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
    using state=cadmium::logger::logger<cadmium::logger::logger_state, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
    using log_messages=cadmium::logger::logger<cadmium::logger::logger_messages, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
    using routing=cadmium::logger::logger<cadmium::logger::logger_message_routing, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
    using global_time=cadmium::logger::logger<cadmium::logger::logger_global_time, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
    using local_time=cadmium::logger::logger<cadmium::logger::logger_local_time, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
    using log_all=cadmium::logger::multilogger<info, debug, state, log_messages, routing, global_time, local_time>;

    using logger_top=cadmium::logger::multilogger<log_messages, global_time>;

    /*******************************************/
    using AtomicModelPtr=std::shared_ptr<cadmium::dynamic::modeling::model>;
    using CoupledModelPtr=std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>>;

    /********************************************/
    /******* Temperature Sensors *********/
    /********************************************/
    AtomicModelPtr digital_temp_humidity1 = cadmium::dynamic::translate::make_dynamic_atomic_model<DigitalTemperatureHumidity, TIME>("digital_temp_humidity1", PC_9, PA_8);
    AtomicModelPtr analog_temp1 = cadmium::dynamic::translate::make_dynamic_atomic_model<AnalogInput, TIME>("analog_temp1", PF_6, TIME("00:00:01:000"));

    /********************************************/
    /**************** LCD ***********************/
    /********************************************/
    AtomicModelPtr lcd1 = cadmium::dynamic::translate::make_dynamic_atomic_model<LCD, TIME>("lcd1");

    /********************************************/
    /************ Touch Screen ******************/
    /********************************************/
    AtomicModelPtr ts1 = cadmium::dynamic::translate::make_dynamic_atomic_model<TouchScreen, TIME>("ts1");

    /********************************************/
    /**************** Arbiter *******************/
    /********************************************/
    AtomicModelPtr arbiter1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Arbiter, TIME>("arbiter1");

    /********************************************/
    /**************** Switch *******************/
    /********************************************/
    AtomicModelPtr switch1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Switch, TIME>("switch1");

    /************************/
    /*******TOP MODEL********/
    /************************/
    cadmium::dynamic::modeling::Ports iports_TOP = {};
    cadmium::dynamic::modeling::Ports oports_TOP = {};
    cadmium::dynamic::modeling::Models submodels_TOP =  {digital_temp_humidity1, analog_temp1, arbiter1, lcd1, ts1, switch1};
    cadmium::dynamic::modeling::EICs eics_TOP = {};
    cadmium::dynamic::modeling::EOCs eocs_TOP = {};
    cadmium::dynamic::modeling::ICs ics_TOP = {
        cadmium::dynamic::translate::make_IC<digitalTemperatureHumidity_defs::temperature_out, switch_defs::temperature_in_1>("digital_temp_humidity1","switch1"),
        cadmium::dynamic::translate::make_IC<digitalTemperatureHumidity_defs::humidity_out, switch_defs::humidity_in_1>("digital_temp_humidity1","switch1"),
        cadmium::dynamic::translate::make_IC<analogInput_defs::out, switch_defs::temperature_in_2>("analog_temp1","switch1"),

        cadmium::dynamic::translate::make_IC<TS_defs::out, switch_defs::ts_in>("ts1", "switch1"),
        cadmium::dynamic::translate::make_IC<switch_defs::sensor_out, arbiter_defs::sensor_in>("switch1", "arbiter1"),
        cadmium::dynamic::translate::make_IC<arbiter_defs::lcd_update_out, LCD_defs::in>("arbiter1","lcd1"),
    };
    CoupledModelPtr TOP = std::make_shared<cadmium::dynamic::modeling::coupled<TIME>>(
        "TOP",
        submodels_TOP,
        iports_TOP,
        oports_TOP,
        eics_TOP,
        eocs_TOP,
        ics_TOP
    );

    ///****************////
    #ifdef RT_ARM_MBED
    //Logging not possible on DISCO: UART over SWD USB not supported
    cadmium::dynamic::engine::runner<NDTime, cadmium::logger::not_logger> r(TOP, {0});
    r.run_until(TIME::infinity());
    #else
    cadmium::dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});
    r.run_until(TIME("00:01:00:000"));
    return 0;
    #endif
}
