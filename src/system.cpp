#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <algorithm>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;


Processor& System::Cpu() {
  return cpu_;
}


vector<Process>& System::Processes() {
  if (processes_.empty()) {
    GetProcesses();
  }
  return processes_;
}


std::string System::Kernel() {
  if (KernelVersion.empty()) {
    KernelVersion = LinuxParser::Kernel();
  }
  return KernelVersion;
}


float System::MemoryUtilization() const {
  return LinuxParser::MemoryUtilization();
}


std::string System::OperatingSystem() {
  if (OSName.empty()) {
    OSName = LinuxParser::OperatingSystem();
  }
  return OSName;
}


int System::RunningProcesses() {
  if (processes_.empty()) {
    GetProcesses();
  }
  int n = 0;
  for (const auto &proc : processes_) {
    if (proc.IsRunning()) {
      n++;
    }
  }
  return n;
}


int System::TotalProcesses() {
  if (processes_.empty()) {
    GetProcesses();
  }
  return processes_.size();
}


long System::UpTime() const {
  return LinuxParser::UpTime();
}


// Read processes' data and populate 'processes_' vector,
// update 'Process::LastProcTimes' if processes end,
// return count of processes
void System::GetProcesses() {
  string name;
  string ram;
  string command;
  string user;
  string state;
  float cpuutil;
  long cputime;
  const vector<int> PIDs = LinuxParser::Pids();
  vector<string> UniquePIDs;
  processes_.clear();
  for (int pid : PIDs) {
    LinuxParser::GetProcessInfo(pid, name, state, user, command, cpuutil, ram, cputime);
    Process process(pid, name, state, user, command, cpuutil, ram, cputime);
    processes_.push_back(process);
    UniquePIDs.push_back(name + "/" + std::to_string(pid));
  }
  std::sort(processes_.begin(), processes_.end());
  
  // Remove element of 'Process::LastProcTimes' if process ends
  bool Process_Ended = true;
  while (Process_Ended) {
    Process_Ended = false;
    for (auto it = Process::LastProcTimes.begin(); it != Process::LastProcTimes.end(); ++it) {
      if (std::find(UniquePIDs.begin(), UniquePIDs.end(), it->first) == UniquePIDs.end()) {
        Process_Ended = true;
        Process::LastProcTimes.erase(it);
        break;
      }
    }
  }
}