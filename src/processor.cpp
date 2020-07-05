#include "processor.h"

using std::string;
using std::to_string;
using std::vector;
using namespace LinuxParser;


// Read cpus' current states and return count of processors
int Processor::ReadCurrentStates() {
  const vector<string> ProcessorsStrings = CpuUtilization();
  CurrentStates.clear();
  for (const auto &Line : ProcessorsStrings) {
    std::istringstream LineStream(Line);
    string Key;
    StateTimeMap States;
    LineStream >>
      Key >>
      States[kUser_] >>
      States[kNice_] >>
      States[kSystem_] >>
      States[kIdle_] >>
      States[kIOwait_] >>
      States[kIRQ_] >>
      States[kSoftIRQ_] >>
      States[kSteal_] >>
      States[kGuest_] >>
      States[kGuestNice_];
    CurrentStates.push_back(States);
  }
  return CurrentStates.size();
}


// Calculate cpu utilazation for each processor
float Processor::Utilization(const size_t CPUIdx) {
  if (CPUIdx >= CurrentStates.size()) {
    return 0.0;
  }
  const auto &Cur_States = CurrentStates[CPUIdx];
  const auto Last_States = CPUIdx >= LastStates.size() ? StateTimeMap{} : LastStates[CPUIdx];
  float CurActiveUnits = 0;
  float LastActiveUnits = 0;
  float CurTotalUnits = 0;
  float LastTotalUnits = 0;
  for (auto S : Cur_States) {
    if (S.first != kIdle_) {
      CurActiveUnits += S.second;
    }
    CurTotalUnits += S.second;
  }
  for (auto S : Last_States) {
    if (S.first != kIdle_) {
      LastActiveUnits += S.second;
    }
    LastTotalUnits += S.second;
  }
  const float ActiveDelta = CurActiveUnits - LastActiveUnits;
  const float TotalDelta = CurTotalUnits - LastTotalUnits;
  if (CPUIdx == CurrentStates.size() - 1) {
    LastStates = CurrentStates;
  }
  return (ActiveDelta > 0 && TotalDelta > 0) ? ActiveDelta / TotalDelta : 0;
}