#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <cmath>
#include <experimental/filesystem>

#include "linux_parser.h"
#include "process.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
namespace fs = std::experimental::filesystem;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os;
  string vesrion;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> vesrion >> vesrion;
  }
  return vesrion;
}


vector<int> LinuxParser::Pids() {
  vector<int> pids;
  for (auto &p : fs::directory_iterator("/proc")) {
    if (fs::is_directory(p.path())) {
      std::string filename = p.path().filename();
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}


float LinuxParser::MemoryUtilization() {
  float Mem;
  float MemTotal = 0;
  float MemAvailable = 0;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    string line;
    while (std::getline(stream, line) && (MemTotal == 0 || MemAvailable == 0)) {
      string Key;
      std::istringstream linestream(line);
      linestream >> Key >> Mem;
      if (Key == "MemTotal:") {
        MemTotal = Mem;
      }
      else if (Key == "MemAvailable:") {
        MemAvailable = Mem;
      }
    }
  }
  return (MemTotal > 0 && MemAvailable > 0) ? ((MemTotal - MemAvailable) / MemTotal) : 0;
}


long LinuxParser::UpTime() {
  float Total = 0;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> Total;
  }
  return std::ceil(Total);
}


long LinuxParser::Jiffies() {
  long jiffies = 0;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    string Line;
    string Key;
    std::getline(stream, Line);
    std::istringstream LineStream(Line);
    std::map<CPUStates, long> States;
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
    if (Key == "cpu") {
      for (auto S : States) {
        jiffies += S.second;
      }
    }
  }
  return jiffies;
}


long LinuxParser::ActiveJiffies(int pid) {
  long jiffies = 0;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    string Line;
    string Value;
    std::getline(stream, Line);
    std::istringstream linestream(Line);
    int n = 0;
    while (linestream >> Value && n++ < 15) {
      if (n == 14 || n == 15) {
        jiffies += std::atoi(Value.c_str());
      }
    }
  }
  return jiffies;
}


long LinuxParser::ActiveJiffies() {
  return Jiffies() - IdleJiffies();
}


long LinuxParser::IdleJiffies() {
  long jiffies = 0;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    string Line;
    string Key;
    std::getline(stream, Line);
    std::istringstream LineStream(Line);
    std::map<CPUStates, long> States;
    LineStream >>
      Key >>
      States[kUser_] >>
      States[kNice_] >>
      States[kSystem_] >>
      States[kIdle_];
    if (Key == "cpu") {
      jiffies = States[kIdle_];
    }
  }
  return jiffies;
}


// Returns "stat" string for each processor
vector<string> LinuxParser::CpuUtilization() {
  vector<string> ProcessorsStrings;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    string line;
    int i = 0;
    while (std::getline(stream, line)) {
      if (line.substr(0, 3) == "cpu") {
        if (++i > 1) {
          ProcessorsStrings.push_back(line);
        }
      }
      else {
        break;
      }
    }
  }
  return ProcessorsStrings;
}


void LinuxParser::GetProcessInfo(
                                 const int pid, string &name, string &state,
                                 string &user, string &command, float &cpuutil,
                                 string &ram, long &uptime) {
  name = Name(pid);
  state = State(pid);
  user = User(pid);
  command = Command(pid);
  ram = Ram(pid);
  uptime = UpTime(pid);
  const ProcTimes proctimes((ActiveJiffies(pid) * 1.0) / sysconf(_SC_CLK_TCK), uptime);
  const string ID = name + "/" + to_string(pid);
  const float active_delta = proctimes.cputime - Process::LastProcTimes[ID].cputime;
  const float total_delta = proctimes.uptime - Process::LastProcTimes[ID].uptime;
  cpuutil = (active_delta > 0 && total_delta > 0) ? active_delta / total_delta : 0;
  Process::LastProcTimes[ID] = proctimes;
}


int LinuxParser::TotalProcesses() {
  return Pids().size();
}


int LinuxParser::RunningProcesses() {
  int n = 0;
  const auto pids = Pids();
  for (int pid : pids) {
    std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
    if (stream.is_open()) {
      string str;
      std::getline(stream, str);
      std::istringstream linestream(str);
      if ((linestream >> str >> str >> str) && str == "R") {
        n++;
      }
    }
  }
  return n;
}


string LinuxParser::Command(int pid) {
  string cmdline{"'n/a'"};
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, cmdline);
  }
  return cmdline.empty() ? "'n/a'" : cmdline;
}


string LinuxParser::Ram(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (!stream.is_open()) {
    return "'n/a'";
  }
  string Line;
  string Key;
  string Value;
  string ram;
  while (std::getline(stream, Line)) {
    std::istringstream linestream(Line);
    linestream >> Key >> Value;
    if (Key == "VmSize:") {
      const int rami = std::ceil(std::atof(Value.c_str()) / 1024);
      ram = to_string(rami);
      break;
    }
  }
  if (ram.empty()) {
    stream.close();
    stream.open(kProcDirectory + to_string(pid) + kStatFilename);
    if (!stream.is_open()) {
      return "'n/a'";
    }
    int n = 0;
    std::getline(stream, Line);
    std::istringstream linestream(Line);
    while (linestream >> Value && n++ < 23) {
      if (n == 23) {
        const int rami = std::ceil((std::atof(Value.c_str()) / 1024) / 1024);
        ram = to_string(rami);
      }
    }
  }
  return (ram.empty() || ram == "0") ? "'n/a'" : ram;
}


string LinuxParser::Uid(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (!stream.is_open()) {
    return "'n/a'";
  }
  string Line;
  string Key;
  string Value;
  string uid;
  while (std::getline(stream, Line)) {
    std::istringstream linestream(Line);
    linestream >> Key >> Value;
    if (Key == "Uid:") {
      uid = Value;
      break;
    }
  }
  return uid.empty() ? "'n/a'" : uid;
}


string LinuxParser::User(int pid) {
  std::ifstream stream(kPasswordPath);
  if (!stream.is_open()) {
    return "'n/a'";
  }
  const string uid = Uid(pid);
  string npwid[3];
  string Line;
  string user{"'n/a'"};
  while (std::getline(stream, Line)) {
    for (int i{0}; i < 3; i++) {
      const string::size_type p = Line.find(":");
      if (p != Line.npos) {
        npwid[i] = Line.substr(0, p);
        Line.erase(Line.begin(), Line.begin() + p + 1);
      }
    }
    if (npwid[2] == uid) {
      user = npwid[0];
      break;
    }
  }
  return user;
}


string LinuxParser::State(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (!stream.is_open()) {
    return "'?'";
  }
  string Line;
  string Key;
  string Value;
  string state;
  while (std::getline(stream, Line)) {
    std::istringstream linestream(Line);
    linestream >> Key >> Value;
    if (Key == "State:") {
      state = Value;
      break;
    }
  }
  return state.empty() ? "'?'" : state;
}


string LinuxParser::Name(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (!stream.is_open()) {
    return "'n/a'";
  }
  string Line;
  string Key;
  string Value;
  string name;
  while (std::getline(stream, Line)) {
    std::istringstream linestream(Line);
    linestream >> Key >> Value;
    if (Key == "Name:") {
      name = Value;
      break;
    }
  }
  return name.empty() ? "'n/a'" : name;
}


long LinuxParser::UpTime(int pid) {
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (!stream.is_open()) {
    return 0;
  }
  long uptime = 0;
  int n = 0;
  string Line;
  string Value;
  std::getline(stream, Line);
  std::istringstream linestream(Line);
  while (linestream >> Value && n++ < 22) {
    if (n == 22) {
      uptime = UpTime() - std::ceil(std::atof(Value.c_str()) / sysconf(_SC_CLK_TCK));
    }
  }
  return uptime;
}