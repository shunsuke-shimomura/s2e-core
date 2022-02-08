#ifndef __SRPEnvironment_h__
#define __SRPEnvironment_h__
#include <Library/math/Vector.hpp>
#include <Interface/LogOutput/ILoggable.h>
#include <Environment/Local/LocalCelestialInformation.h>

using libra::Vector;

class SRPEnvironment : public ILoggable
{
public:
  bool IsCalcEnabled = true;

  SRPEnvironment(LocalCelestialInformation* local_celes_info);   //Default constructor
  void UpdateAllStates();
  double CalcTruePressure() const;          //Obtaining solar radiation pressure that takes into account eclipse
  double CalcPowerDensity() const;          //Get solar power per unit area considering eclipse [W/m^2]
  double GetPressure() const;               //Get pressure_(for debug)
  double GetSolarConstant() const;          //Get solar constant value [W/m^2]
  double GetShadowFunction() const;         //Get Shadow function
  inline bool GetIsEclipsed() const { return(shadow_function_ >= 1.0 ? false : true); } //Returns true if the shadow function is less than 1

  virtual std::string GetLogHeader() const; //log of header
  virtual std::string GetLogValue() const;  //log of value

private:
  double pressure_;              //solar radiation constant [N/m^2]
  double astronomical_unit_;     //1AU [m]
  double c_;                     //speed of light [m/s]
  double solar_constant_;        //solar constant [W/m^2]
  double r_earth_;               //radius of earth [m]
  double r_sun_;                 //radius of sun [m]
  double shadow_function_ = 1.0; //shadow function

  LocalCelestialInformation* local_celes_info_;

  void CalcShadowFunction(double a, double b, double c, double x, double y);
};

#endif /* SRPEnvironment_h */
