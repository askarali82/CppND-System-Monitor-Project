#include <curses.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "format.h"
#include "ncurses_display.h"
#include "system.h"

using std::string;
using std::to_string;

// 50 bars uniformly displayed from 0 - 100 %
// 2% is one bar(|)
std::string NCursesDisplay::ProgressBar(float percent) {
  std::string result{"0%"};
  int size{50};
  float bars{percent * size};

  for (int i{0}; i < size; ++i) {
    result += i <= bars ? '|' : ' ';
  }

  string display{to_string(percent * 100).substr(0, 4)};
  if (percent < 0.1 || percent == 1.0)
    display = " " + to_string(percent * 100).substr(0, 3);
  return result + " " + display + "/100%";
}


void NCursesDisplay::DisplaySystem(System& system, WINDOW* window) {
  int row{0};
  mvwprintw(window, ++row, 2, ("OS: " + system.OperatingSystem()).c_str());
  mvwprintw(window, ++row, 2, ("Kernel: " + system.Kernel()).c_str());

  const int nc = system.Cpu().ReadCurrentStates();
  for (int i{0}; i < nc; ++i) {
    wattroff(window, COLOR_PAIR(1));
    const string CPUN = "CPU" + to_string(i + 1) + ": ";
    mvwprintw(window, ++row, 2, CPUN.c_str());
    wattron(window, COLOR_PAIR(1));
    mvwprintw(window, row, 10, "");
    wprintw(window, ProgressBar(system.Cpu().Utilization(i)).c_str());
  }

  wattroff(window, COLOR_PAIR(1));
  mvwprintw(window, ++row, 2, "Memory: ");
  wattron(window, COLOR_PAIR(1));
  mvwprintw(window, row, 10, "");
  wprintw(window, ProgressBar(system.MemoryUtilization()).c_str());
  wattroff(window, COLOR_PAIR(1));
  mvwprintw(window, ++row, 2,
            ("Total Processes: " + to_string(system.TotalProcesses())).c_str());
  mvwprintw(
      window, ++row, 2,
      ("Running Processes: " + to_string(system.RunningProcesses())).c_str());
  mvwprintw(window, ++row, 2,
            ("Up Time: " + Format::ElapsedTime(system.UpTime(), true)).c_str());
  wrefresh(window);
}


void NCursesDisplay::DisplayProcesses(std::vector<Process>& processes, WINDOW* window) {
  int row{0};
  int const pid_column{2};
  int const user_column{9};
  int const cpu_column{20};
  int const ram_column{28};
  int const state_column{37};
  int const time_column{40};
  int const command_column{53};
  wattron(window, COLOR_PAIR(2));
  mvwprintw(window, ++row, pid_column, "PID");
  mvwprintw(window, row, user_column, "USER");
  mvwprintw(window, row, cpu_column, "CPU[%%]");
  mvwprintw(window, row, ram_column, "RAM[MB]");
  mvwprintw(window, row, state_column, "S");
  mvwprintw(window, row, time_column, "TIME+");
  mvwprintw(window, row, command_column, "COMMAND");
  wattroff(window, COLOR_PAIR(2));
  for (size_t i = 0; i < processes.size(); ++i) {
    mvwprintw(window, ++row, pid_column, to_string(processes[i].Pid()).c_str());
    string::size_type max_len = cpu_column - user_column - 2;
    std::string user = processes[i].User();
    if (user.length() > max_len) {
      user = user.substr(0, max_len - 1) + "+";
    }
    mvwprintw(window, row, user_column, user.c_str());
    const string cpu = Format::FloatToStr(processes[i].CpuUtilization() * 100, 1);
    mvwprintw(window, row, cpu_column, cpu.c_str());
    mvwprintw(window, row, ram_column, processes[i].Ram().c_str());
    mvwprintw(window, row, state_column, processes[i].State().c_str());
    mvwprintw(window, row, time_column,
              Format::ElapsedTime(processes[i].UpTime()).c_str());
    max_len = window->_maxx - command_column - 2;
    std::string cmdline = processes[i].Command();
    if (cmdline.length() > max_len) {
      cmdline = cmdline.substr(0, max_len - 1) + "+";
    }
    mvwprintw(window, row, command_column, cmdline.c_str());
    if (window->_cury >= window->_maxy - 1) {
      break;
    }
  }
}


void NCursesDisplay::Display(System& system) {
  initscr();      // start ncurses
  noecho();       // do not print input values
  cbreak();       // terminate ncurses on ctrl + c
  start_color();  // enable color

  const int nc = system.Cpu().ReadCurrentStates();
  int x_max{getmaxx(stdscr)};
  int y_max{getmaxy(stdscr)};
  WINDOW* system_window = newwin(8 + nc, x_max, 0, 0);
  WINDOW* process_window =
      newwin(y_max - (system_window->_maxy + 1), x_max, system_window->_maxy + 1, 0);

  while (1) {
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    box(system_window, 0, 0);
    box(process_window, 0, 0);
    DisplaySystem(system, system_window);
    DisplayProcesses(system.Processes(), process_window);
    system.Processes().clear();
    wrefresh(system_window);
    wrefresh(process_window);
    refresh();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    werase(system_window);
    werase(process_window);
    if (getmaxx(stdscr) != x_max || getmaxy(stdscr) != y_max) {
      delwin(system_window);
      delwin(process_window);
      x_max = getmaxx(stdscr);
      y_max = getmaxy(stdscr);
      system_window = newwin(8 + nc, x_max, 0, 0);
      process_window =
        newwin(y_max - (system_window->_maxy + 1), x_max, system_window->_maxy + 1, 0);
    }
  }
  endwin();
}