/**
* Kyle Bjornson
* ARSLab - Carleton University
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

struct lcd_update{
    char characters[16][17];
    //std::list<lcd_update_line> lines;
    //uint32_t background_colour;
};

//struct lcd_update_line {
    //uint8_t line_index;
    //char* characters;
    //Text_AlignModeTypdef alignment;
//};

#ifdef ECADMIUM
  #include "../mbed.h"
  #include "../drivers/LCD_DISCO_F429ZI/LCD_DISCO_F429ZI.h"

  using namespace cadmium;
  using namespace std;

  //Port definition
  struct LCD_defs{
    struct in : public in_port<lcd_update> {};
  };

  template<typename TIME>
  class LCD {
  using defs=LCD_defs; // putting definitions in context
  public:

    LCD_DISCO_F429ZI lcd;

    // default c onstructor
    LCD() noexcept{
        BSP_LCD_SetFont(&Font20);
        lcd.Clear(LCD_COLOR_BLUE);
        lcd.SetBackColor(LCD_COLOR_BLUE);
        lcd.SetTextColor(LCD_COLOR_WHITE);
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
      //sprintf((char*)text,  = %.2f", temp_humid_sensor.read_temperature()); //(uint8_t *)&state.output
      for (uint8_t i = 0; i < 16; i++) {
          lcd.DisplayStringAt(0, LINE(i), (uint8_t*) state.output.characters[i], LEFT_MODE);
      }
      //lcd.DisplayStringAt(0, LINE(3), (uint8_t*)state.output.c_str(), LEFT_MODE);

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
