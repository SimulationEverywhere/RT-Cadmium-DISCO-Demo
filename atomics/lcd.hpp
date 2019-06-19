/**
* Kyle Bjornson
* ARSLab - Carleton University
*/

#ifndef DISCO_LCD_HPP
#define DISCO_LCD_HPP

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


#ifdef RT_ARM_MBED
  #include "../mbed.h"
  #include "../drivers/LCD_DISCO_F429ZI/LCD_DISCO_F429ZI.h"

  struct lcd_update_line {
      uint8_t line_index;
      char characters[17];
      Text_AlignModeTypdef alignment;
  };

  struct lcd_update{
      std::list<lcd_update_line> lines;
      uint32_t lcd_colour;
      uint32_t text_colour;
  };

  using namespace cadmium;
  using namespace std;

  //Port definition
  struct LCD_defs{
    struct in : public in_port<struct lcd_update> {};
  };

  template<typename TIME>
  class LCD {
  using defs=LCD_defs; // putting definitions in context
  public:
    LCD_DISCO_F429ZI lcd;

    // default c onstructor
    LCD() noexcept{
        BSP_LCD_SetFont(&Font20);

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

      lcd.Clear(state.output.lcd_colour);
      lcd.SetBackColor(state.output.lcd_colour);
      lcd.SetTextColor(state.output.text_colour);

      for (lcd_update_line line : state.output.lines) {
          lcd.DisplayStringAt(0, LINE(line.line_index), (uint8_t*) line.characters, line.alignment);
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

  const char* LCD_FILE = "./outputs/LCD_out.txt";

  //Port definition
  struct LCD_defs{
      struct in : public in_port<std::string> {};
  };

  template<typename TIME>
  class LCD : public oestream_output<std::string, TIME, LCD_defs>{
    public:
      LCD() : oestream_output<std::string, TIME, LCD_defs>(LCD_FILE) {}
  };
#endif //RT_ARM_MBED
#endif // DISCO_LCD_HPP
