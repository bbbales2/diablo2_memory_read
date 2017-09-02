//build and run with
// clang++ -std=c++11 -m32 main.cpp && sudo ./a.out pid
#include <sys/uio.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cinttypes>
#include <iostream>
//#include <sys/wait.h>
//#include <fcntl.h>
//#include <string>
//#include <sys/ptrace.h>
#include "d2structs.hpp"
#include "d2constants.hpp"
#include "json/src/json.hpp"

using json = nlohmann::json;

//std::string memFilename;
pid_t pid;

template<typename T>
T deref(T *address) {
  T variable;

  /* ptrace version -- works but still need root
  int mem_fd = open(memFilename.c_str(), O_RDONLY);
  ptrace(PTRACE_ATTACH, pid, NULL, NULL);
  waitpid(pid, NULL, 0);
  lseek(mem_fd, (long)address, SEEK_SET);
  read(mem_fd, (void *)&variable, sizeof(T));
  ptrace(PTRACE_DETACH, pid, NULL, NULL);
  close(mem_fd);

  return variable;
  */

  struct iovec local[1];
  struct iovec remote[1];
  ssize_t nread;

  local[0].iov_base = &variable;
  local[0].iov_len = sizeof(T);
  remote[0].iov_base = address;
  remote[0].iov_len = sizeof(T);

  nread = process_vm_readv(pid, local, 1, remote, 1, 0);
  if (nread != sizeof(T)) {
    std::cout << "Remote address read failed: " << std::hex << address << std::dec << std::endl;
    exit(-1);
  }

  return variable;
}

int main(int argc, char **argv) {

  if(argc < 2) {
    std::cout << "Must pass in pid of Diablo client" << std::endl;
    return 0;
  }

  //memFilename = std::string("/proc/") + std::string(argv[1]) + std::string("/mem");
  pid = atoi(argv[1]);

  UnitAny *unitPtr = deref((UnitAny **)0x7A6A70);

  while(1) {
    UnitAny unit = deref(unitPtr);
    Path path = deref(unit.pPath);
    StatList stats = deref(unit.pStats);

    uint32_t xp = 0;
    uint32_t hp = 0;
    uint32_t gold = 0;

    for (int i = 0; i < stats.StatVec.wCount; i++) {
      Stat cur_stat = deref(stats.StatVec.pStats + i);
      switch (cur_stat.wStatIndex) {
      case STAT_EXP:
        xp = cur_stat.dwStatValue;
        break;
      case STAT_GOLD:
        gold = cur_stat.dwStatValue;
        break;
      case STAT_HP:
        hp = cur_stat.dwStatValue;
        break;
      default:
        // Not a stat we're interested in.
        break;
      }
    }

    json j = {
      {"x", path.xPos},
      {"y", path.yPos},
      {"xp", xp},
      {"gold", gold},
      {"hp", hp},
      {"xTarget", path.xTarget},
      {"yTarget", path.yTarget},
      {"lastUnitClicked", path.pTargetUnit != NULL},
      {"lastUnitTypeClicked", path.dwTargetType},
      {"lastUnitIdClicked", path.dwTargetId},
      {"mode", unit.dwMode}
    };

    std::cout << j.dump(4) << std::endl;

    if(argc == 2)
      break;

    usleep(100000);
  }

  return 0;
}
