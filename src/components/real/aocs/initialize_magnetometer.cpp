/**
 * @file initialize_magnetometer.cpp
 * @brief Initialize functions for magnetometer
 */
#include "initialize_magnetometer.hpp"

#include "../../base/initialize_sensor.hpp"
#include "library/initialize/initialize_file_access.hpp"

MagSensor InitMagSensor(ClockGenerator* clock_gen, int sensor_id, const std::string fname, double compo_step_time, const GeomagneticField* magnet) {
  IniAccess magsensor_conf(fname);
  const char* sensor_name = "MAGNETOMETER_";
  const std::string section_name = sensor_name + std::to_string(static_cast<long long>(sensor_id));
  const char* MSSection = section_name.c_str();

  int prescaler = magsensor_conf.ReadInt(MSSection, "prescaler");
  if (prescaler <= 1) prescaler = 1;

  Quaternion q_b2c;
  magsensor_conf.ReadQuaternion(MSSection, "quaternion_b2c", q_b2c);

  // Sensor
  Sensor<kMagDim> sensor_base = ReadSensorInformation<kMagDim>(fname, compo_step_time * (double)(prescaler), MSSection, "nT");

  MagSensor magsensor(prescaler, clock_gen, sensor_base, sensor_id, q_b2c, magnet);
  return magsensor;
}

MagSensor InitMagSensor(ClockGenerator* clock_gen, PowerPort* power_port, int sensor_id, const std::string fname, double compo_step_time,
                        const GeomagneticField* magnet) {
  IniAccess magsensor_conf(fname);
  const char* sensor_name = "MAGNETOMETER_";
  const std::string section_name = sensor_name + std::to_string(static_cast<long long>(sensor_id));
  const char* MSSection = section_name.c_str();

  int prescaler = magsensor_conf.ReadInt(MSSection, "prescaler");
  if (prescaler <= 1) prescaler = 1;

  Quaternion q_b2c;
  magsensor_conf.ReadQuaternion(MSSection, "quaternion_b2c", q_b2c);

  // Sensor
  Sensor<kMagDim> sensor_base = ReadSensorInformation<kMagDim>(fname, compo_step_time * (double)(prescaler), MSSection, "nT");

  // PowerPort
  power_port->InitializeWithInitializeFile(fname);

  MagSensor magsensor(prescaler, clock_gen, power_port, sensor_base, sensor_id, q_b2c, magnet);
  return magsensor;
}
