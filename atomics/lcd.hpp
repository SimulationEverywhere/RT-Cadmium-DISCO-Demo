/**
* NOTE: THIS CODE IS UNTESTED - THE BOARD USED AT ARSLABS DOES NOT HAVE A DAC
*
* Ben Earle
* ARSLab - Carleton University
*
* Analog Output:
* Model to interface with a analog output pin for Embedded Cadmium.
*
*/

#ifndef BOOST_SIMULATION_PDEVS_ANALOGOUTPUT_HPP
#define BOOST_SIMULATION_PDEVS_ANALOGOUTPUT_HPP

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
  #include "../mbed.h"
  #include "../drivers/LCD_DISCO_F429ZI/LCD_DISCO_F429ZI.h"

  using namespace cadmium;
  using namespace std;

  //Port definition
  struct LCD_defs{
    struct in : public in_port<std::string> {};
  };

  template<typename TIME>
  class LCD {
  using defs=LCD_defs; // putting definitions in context
  public:
    //Parameters to be overwriten when instantiating the atomic model
    LCD_DISCO_F429ZI lcd;
    uint8_t text[40];

    // default c onstructor
    LCD() noexcept{
        BSP_LCD_SetFont(&Font20);
        lcd.Clear(LCD_COLOR_BLUE);
        lcd.SetBackColor(LCD_COLOR_BLUE);
        lcd.SetTextColor(LCD_COLOR_WHITE);
    }

    // state definition
    struct state_type{
      std::string output;
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
      //sprintf((char*)text,  = %.2f", temp_humid_sensor.read_temperature());
      lcd.DisplayStringAt(0, LINE(3), (uint8_t *)&state.output, LEFT_MODE);

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
      os << "Printed: " << i.output;
      return os;
    }
  };
#else
  #include <cadmium/io/oestream.hpp>
  using namespace cadmium;
  using namespace std;

  const char* LCD_FILE = "./outputs/LCD_out.txt"

  //Port definition
  struct LCD_defs{
      struct in : public in_port<std::string> {};
  };

  template<typename TIME>
  class LCD : public oestream_output<std::string, TIME, LCD_defs>{
    public:
      LCD() : oestream_output<std::string, TIME, LCD_defs>(LCD_FILE) {}
  };
#endif //ECADMIUM
#endif // BOOST_SIMULATION_PDEVS_ANALOGOUTPUT_HPP
