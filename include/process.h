#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <map>
#include "linux_parser.h"
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
public:
  Process(int apid, std::string aname, std::string astate, std::string auser, std::string acommand,
    float acpuutil, std::string aram, long auptime) :
    pid(apid), name(aname), state(astate), user(auser), command(acommand), cpuutil(acpuutil),
    ram(aram), uptime(auptime) { }

  // To store process times read in previous read
  static std::map<std::string, LinuxParser::ProcTimes> LastProcTimes;

  int Pid() const;
  std::string User() const;
  std::string Command() const;
  float CpuUtilization() const;
  std::string Ram() const;
  long int UpTime() const;
  bool operator < (Process const& a) const;
  bool IsRunning() const {
    return state == "R";
  }
  std::string State() const {
    return state;
  }

private:
  int pid;
  std::string name;
  std::string state;
  std::string user;
  std::string command;
  float cpuutil;
  std::string ram;
  long uptime;
};

#endif