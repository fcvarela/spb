#ifndef __CONSTANTS
#define __CONSTANTS

#include <cmath>

class Constant {
public:
  static const double PC_TO_KM;
  static const double SEC_PER_YEAR;
  static const double DEG_TO_RAD;
  static const double RAD_TO_DEG;
};

const double Constant::PC_TO_KM = 3.08567758129e13;
const double Constant::SEC_PER_YEAR = 365.25 * 86400;
const double Constant::DEG_TO_RAD = M_PI / 180.0;
const double Constant::RAD_TO_DEG = 180.0 / M_PI;

#endif
