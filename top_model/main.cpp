/**
* By: Ben Earle and Kyle Bjornson
* ARSLab - Carleton University
*
* Analog Input:
* Model to interface with a analog Input pin for Embedded Cadmium.
*/
#include <iostream>
#include <chrono>
#include <algorithm>
#include <string>
#define MISSED_DEADLINE_TOLERANCE 1000000

#include <cadmium/modeling/coupled_model.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_atomic.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/tuple_to_ostream.hpp>
#include <cadmium/logger/common_loggers.hpp>

#include <NDTime.hpp>
#include <cadmium/io/iestream.hpp>


#include <cadmium/embedded/io/interruptInput.hpp>
#include <cadmium/embedded/io/digitalOutput.hpp>


#include "../atomics/lcd.hpp"
#include "../atomics/digital_temp_humidity.hpp"
#include "../atomics/arbiter.hpp"


#include <cadmium/embedded/embedded_error.hpp>

//#include "../drivers/sht31.hpp"
//#include "../drivers/TS_DISCO_F429ZI/TS_DISCO_F429ZI.h"
//#include "../drivers/LCD_DISCO_F429ZI/LCD_DISCO_F429ZI.h"

#ifdef ECADMIUM
  #include "../mbed.h"
#else
  // When simulating the model it will use these files as IO in place of the pins specified.
  const char* BUTTON1 = "./inputs/BUTTON1_In.txt";
  const char* LCD1    = "./outputs/LCD_out.txt";
#endif

using namespace std;

using hclock=chrono::high_resolution_clock;
using TIME = NDTime;


int main(int argc, char ** argv) {

    // LCD_DISCO_F429ZI lcd;
    // TS_DISCO_F429ZI ts;
    //
    // TS_StateTypeDef TS_State;
    // uint16_t x, y;
    // uint8_t text[40];
    // uint8_t status;
    //
    // drivers::sht31 temp_humid_sensor(PB_9, PB_8);
    // wait(1);
    //
    // BSP_LCD_SetFont(&Font20);
    //
    // lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN", CENTER_MODE);
    // lcd.DisplayStringAt(0, LINE(6), (uint8_t *)"DEMO", CENTER_MODE);
    // wait(1);
    //
    // status = ts.Init(lcd.GetXSize(), lcd.GetYSize());
    //
    // if (status != TS_OK)
    // {
    //     lcd.Clear(LCD_COLOR_RED);
    //     lcd.SetBackColor(LCD_COLOR_RED);
    //     lcd.SetTextColor(LCD_COLOR_WHITE);
    //     lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN", CENTER_MODE);
    //     lcd.DisplayStringAt(0, LINE(6), (uint8_t *)"INIT FAIL", CENTER_MODE);
    // }
    // else
    // {
    //     lcd.Clear(LCD_COLOR_GREEN);
    //     lcd.SetBackColor(LCD_COLOR_GREEN);
    //     lcd.SetTextColor(LCD_COLOR_WHITE);
    //     lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN", CENTER_MODE);
    //     lcd.DisplayStringAt(0, LINE(6), (uint8_t *)"INIT OK", CENTER_MODE);
    // }
    //
    // wait(1);
    // lcd.Clear(LCD_COLOR_BLUE);
    // lcd.SetBackColor(LCD_COLOR_BLUE);
    // lcd.SetTextColor(LCD_COLOR_WHITE);
    //
    //
    // while(1)
    // {
    //
    //     ts.GetState(&TS_State);
    //     if (TS_State.TouchDetected)
    //     {
    //         x = TS_State.X;
    //         y = TS_State.Y;
    //         sprintf((char*)text, "x=%d y=%d    ", x, y);
    //         lcd.DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
    //     }
    //
    //     sprintf((char*)text, "Temp = %.2f", temp_humid_sensor.read_temperature());
    //     lcd.DisplayStringAt(0, LINE(3), (uint8_t *)&text, LEFT_MODE);
    //     sprintf((char*)text, "Humidity = %.2f", temp_humid_sensor.read_humidity());
    //     lcd.DisplayStringAt(0, LINE(4), (uint8_t *)&text, LEFT_MODE);
    //     wait_ms(100);
    // }
    //
    // //drivers::sht31 temp_humid_sensor(PB_9, PB_8);
    // //printf("Temperature = %f \n", temp_humid_sensor.read_temperature());
    // //printf("Humidity = %f \n", temp_humid_sensor.read_humidity());
    //
    // while(1);


  #ifdef ECADMIUM
      //Logging is done over cout in ECADMIUM
      struct oss_sink_provider{
        static std::ostream& sink(){
          return cout;
        }
      };
  #else
    // all simulation timing and I/O streams are ommited when running embedded
    auto start = hclock::now(); //to measure simulation execution time

    static std::ofstream out_data("disco_test_output.txt");
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
  /***************** blinky *******************/
  /********************************************/

  AtomicModelPtr digital_temp_humidity1 = cadmium::dynamic::translate::make_dynamic_atomic_model<DigitalTemperatureHumidity, TIME>("digital_temp_humidity1", PB_9, PB_8);

  /********************************************/
  /********** interruptInput1 *******************/
  /********************************************/
  AtomicModelPtr arbiter1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Arbiter, TIME>("arbiter1");

  /********************************************/
  /********* DigitalOutput1 *******************/
  /********************************************/
  AtomicModelPtr lcd1 = cadmium::dynamic::translate::make_dynamic_atomic_model<LCD, TIME>("lcd1");


  /************************/
  /*******TOP MODEL********/
  /************************/
  cadmium::dynamic::modeling::Ports iports_TOP = {};
  cadmium::dynamic::modeling::Ports oports_TOP = {};
  cadmium::dynamic::modeling::Models submodels_TOP =  {digital_temp_humidity1, arbiter1, lcd1};
  cadmium::dynamic::modeling::EICs eics_TOP = {};
  cadmium::dynamic::modeling::EOCs eocs_TOP = {};
  cadmium::dynamic::modeling::ICs ics_TOP = {
    cadmium::dynamic::translate::make_IC<digitalTemperatureHumidity_defs::temperature_out, arbiter_defs::temperature_in>("digital_temp_humidity1","arbiter1"),
    cadmium::dynamic::translate::make_IC<digitalTemperatureHumidity_defs::humidity_out, arbiter_defs::humidity_in>("digital_temp_humidity1","arbiter1"),
    cadmium::dynamic::translate::make_IC<arbiter_defs::string_out, LCD_defs::in>("arbiter1","lcd1"),
    //cadmium::dynamic::translate::make_IC<interruptInput_defs::out, blinky_defs::in>("interruptInput1", "blinky1")
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
  cadmium::dynamic::engine::runner<NDTime, log_all> r(TOP, {0});
  r.run_until(NDTime("00:10:00:000"));
  #ifndef ECADMIUM
    return 0;
  #endif
}
