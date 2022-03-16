#ifndef OPEN_TUNER_HPP
#define OPEN_TUNER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <Python.h>

#include "search_technique.hpp"
#include "helper.hpp"

namespace atf {

class open_tuner : public atf::search_technique {
  public:
    open_tuner& database( const std::string& path ) {
      _path_to_database = path;
      return *this;
    }


    void initialize(size_t dimensionality) override {
      _dimensionality = dimensionality;
      std::string python_code = R"(
import opentuner
from opentuner.search.manipulator import ConfigurationManipulator
from opentuner.measurement.interface import DefaultMeasurementInterface
from opentuner.api import TuningRunManager
from opentuner.resultsdb.models import Result
from opentuner.search.manipulator import FloatParameter
import argparse

def get_next_desired_result():
    global desired_result
    desired_result = api.get_next_desired_result()
    while desired_result is None:
        desired_result = api.get_next_desired_result()
    return desired_result.configuration.data

def report_result(runtime):
    api.report_result(desired_result, Result(time=runtime))

def finish():
    api.finish()

parser = argparse.ArgumentParser(parents=opentuner.argparsers())
args = parser.parse_args()

manipulator = ConfigurationManipulator()
:::parameters:::
interface = DefaultMeasurementInterface(args=args,
                                        manipulator=manipulator,
                                        project_name='atf_library',
                                        program_name='atf_library',
                                        program_version='0.1')


api = TuningRunManager(interface, args)

)";

      std::stringstream tp_parameter_code;
      /** Set each dimension to 0.0 - 1.0 for coordinate space */
      for( size_t i = 0 ; i < _dimensionality; ++ i )
        tp_parameter_code << "manipulator.add_parameter(FloatParameter('PARAM" << i << "', 0.0, 1.0))\n";

      size_t start_pos = python_code.find(":::parameters:::");
      python_code.replace( start_pos, strlen(":::parameters:::"), tp_parameter_code.str() );

//      Py_SetProgramName( argv[0] ); // optional but recommended

      static bool first_time = true;
      if( first_time )
      {
        Py_Initialize();
        first_time = false;
      }

      std::vector< std::string > opentuner_cmd_line_arguments;

      opentuner_cmd_line_arguments.emplace_back("python_template.py" ); // python program name
      opentuner_cmd_line_arguments.emplace_back("--no-dups"          ); // supresses printing warnings for duplicate requests'
      if ( !_path_to_database.empty() )
      {
          opentuner_cmd_line_arguments.push_back( "--database"      );   // path to OpenTuner database
          opentuner_cmd_line_arguments.push_back( _path_to_database );
      }


        // convert vector of std::string to vector of char*
      std::vector< char* > opentuner_cmd_line_arguments_as_c_str;
      auto str_to_char = []( const std::string& str ){ char* c_str = new char[ str.size() + 1 ];
        std::strcpy( c_str, str.c_str() );
        return c_str;
      };
      std::transform( opentuner_cmd_line_arguments.begin(), opentuner_cmd_line_arguments.end(), std::back_inserter( opentuner_cmd_line_arguments_as_c_str ), str_to_char );

      // set command line arguments
      PySys_SetArgv( static_cast<int>( opentuner_cmd_line_arguments_as_c_str.size() ), opentuner_cmd_line_arguments_as_c_str.data() );

      // let code run by the python interpreter in the "__main__" module
      auto error = PyRun_SimpleString( python_code.c_str() ); if( error == -1 ) std::cout << "error: running python script fails" << std::endl;

      auto p_module             = PyImport_ImportModule( "__main__" );

      _get_next_desired_result = PyObject_GetAttrString(p_module, "get_next_desired_result");
      _report_result           = PyObject_GetAttrString(p_module, "report_result"          );
      _finish                  = PyObject_GetAttrString(p_module, "finish"                 );

    }
    void finalize() override {
      PyObject_CallObject( _finish, NULL );

      Py_XDECREF( _get_next_desired_result );
      Py_XDECREF( _report_result           );
      Py_XDECREF( _finish                  );

      static bool first_time = true;
      if( first_time )
      {
        std::atexit( Py_Finalize );
        first_time = false;
      }

    }
    std::set<atf::coordinates> get_next_coordinates() override {
      // get indices
      PyObject* p_config = PyObject_CallObject(_get_next_desired_result, NULL );

      atf::coordinates indices;
      /** Get the parameter from OpenTuner */
      for( size_t i = 0 ; i < _dimensionality; ++ i )
      {
        PyObject* pValue = PyDict_GetItemString( p_config, (std::string("PARAM") + std::to_string(i)).c_str() );
        indices.push_back( PyFloat_AsDouble( pValue ) );
      }

      Py_DECREF( p_config );
      /** if the point isn't in the coordinate space convert the point in the coordinate-space */
      if (!atf::valid_coordinates(indices))
        atf::clamp_coordinates_capped(indices);

      return { indices };

    }
    void report_costs(const std::map<atf::coordinates, atf::cost_t>& costs) override {
      PyObject* arg = PyTuple_New( 1 );
      PyTuple_SetItem( arg, 0, PyFloat_FromDouble( costs.begin()->second ) );

      PyObject_CallObject( _report_result, arg );

      Py_DECREF(arg);
    }
  private:
    std::string _path_to_database;

    size_t    _dimensionality;
    PyObject* _get_next_desired_result;
    PyObject* _report_result;
    PyObject* _finish;
};

}

#endif //OPEN_TUNER_HPP
