#include <string>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds, bool days) {
  string DDs;
  if (days) {
    const int DD = seconds / 86400;
    seconds %= 86400;
    DDs = std::to_string(DD) + " days, ";
  }
  const int HH = seconds / 3600;
  seconds %= 3600;
  const int MM = seconds / 60;
  const int SS = seconds % 60;
  const std::string HHs = HH < 10 ? ("0" + std::to_string(HH)) : std::to_string(HH);
  const std::string MMs = MM < 10 ? ("0" + std::to_string(MM)) : std::to_string(MM);
  const std::string SSs = SS < 10 ? ("0" + std::to_string(SS)) : std::to_string(SS);
  return DDs + HHs + ":" + MMs + ":" + SSs;
}


string Format::FloatToStr(const float value, const int d) {
  string str = std::to_string(value);
  size_t p;
  const bool has_sep = (p = str.find(".")) != str.npos || (p = str.find(",")) != str.npos;
  if (has_sep) {
    str = str.substr(0, p + d + 1);
  }
  else if (d > 0) {
    str += ".0";
  }
  return str;
}