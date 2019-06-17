/**
* Kyle Bjornson
* ARSLab - Carleton University
*/

#ifndef DISCO_TS_HPP
#define DISCO_TS_HPP

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
  #include "../drivers/TS_DISCO_F429ZI/TS_DISCO_F429ZI.h"
  #include "../drivers/LCD_DISCO_F429ZI/LCD_DISCO_F429ZI.h"
  #include <cadmium/embedded/embedded_error.hpp>

  struct cartesian_coordinates {
      int x;
      int y;
  };

  using namespace cadmium;
  using namespace std;

  //Port definition
  struct TS_defs{
    struct out : public out_port<struct cartesian_coordinates> {};
  };

  template<typename TIME>
  class TouchScreen {
  using defs=TS_defs; // putting definitions in context
  public:

    TIME   pollingRate;
    TS_DISCO_F429ZI ts;
    TS_StateTypeDef TS_State;

    // default constructor
    TouchScreen() noexcept {
        new (this) TouchScreen(TIME("00:00:00:100"));
    }

    TouchScreen(TIME rate) {
        pollingRate = rate;

        state.coordinates.x = 0;
        state.coordinates.y = 0;

        if (ts.Init(ILI9341_LCD_PIXEL_WIDTH, ILI9341_LCD_PIXEL_HEIGHT) != TS_OK) {
            cadmium::embedded::embedded_error::hard_fault("Touch Screen Init FAIL!");
        }
    }

    // state definition
    struct state_type{
        cartesian_coordinates coordinates;
    };
    state_type state;

    // ports definition
    using input_ports=std::tuple<>;
    using output_ports=std::tuple<typename defs::out>;

    // internal transition
    void internal_transition() {
        ts.GetState(&TS_State);
        state.coordinates.x = TS_State.X;
        state.coordinates.y = TS_State.Y;
    }

    // external transition
    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        cadmium::embedded::embedded_error::hard_fault("External transition called in a model with no input ports");
    }

    // confluence transition
    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
    }

    // output function
    typename make_message_bags<output_ports>::type output() const {
      typename make_message_bags<output_ports>::type bags;

      if (TS_State.TouchDetected) {
          get_messages<typename defs::out>(bags).push_back(state.coordinates);
      }

      return bags;
    }

    // time_advance function
    TIME time_advance() const {
        return pollingRate;
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename TouchScreen<TIME>::state_type& i) {
      os << "Touched: " << "1";//i.output;
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
#endif // DISCO_TS_HPP