#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"

using std::string;
using std::to_string;
using std::vector;


std::map<std::string, LinuxParser::ProcTimes> Process::LastProcTimes;

int Process::Pid() const {
  return pid;
}


float Process::CpuUtilization() const {
  return cpuutil;
}


string Process::Command() const {
  return command;
}


string Process::Ram() const {
  return ram;
}


string Process::User() const {
  return user;
}


long int Process::UpTime() const {
  return uptime;
}


bool Process::operator < (Process const& a) const {
  return a.cpuutil < cpuutil;
}