#ifndef FORMAT_H
#define FORMAT_H

#include <string>

namespace Format {
std::string ElapsedTime(long times, bool days = false);
std::string FloatToStr(const float value, const int d);
};                               // namespace Format

#endif