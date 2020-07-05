#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <map>
#include <vector>
#include "linux_parser.h"

class Processor {
public:
  int ReadCurrentStates();
  float Utilization(const size_t CPUIdx);

private:
  typedef std::map<LinuxParser::CPUStates, long> StateTimeMap;
  // vector of states for each processor
  std::vector<StateTimeMap> CurrentStates;
  // To store states read in previous read
  std::vector<StateTimeMap> LastStates;
};

#endif