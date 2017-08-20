//build and run with
// clang++ -std=c++11 -m32 main.cpp && sudo ./a.out pid
#include <sys/uio.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cinttypes>
#include <iostream>
#include "d2structs.hpp"

pid_t pid;

template<typename T>
T deref(T *address) {
  struct iovec local[1];
  struct iovec remote[1];
  ssize_t nread;

  T variable;

  local[0].iov_base = &variable;
  local[0].iov_len = sizeof(T);
  remote[0].iov_base = address;
  remote[0].iov_len = sizeof(T);

  nread = process_vm_readv(pid, local, 1, remote, 1, 0);
  if (nread != sizeof(T)) {
    std::cout << "Remote address: " << std::hex << address << std::dec << std::endl;
  }
  
  return variable;
}

int main(int argc, char **argv) {

  if(argc != 2) {
    std::cout << "Must pass in pid of Diablo client" << std::endl;
    return 0;
  }
  
  pid = atoi(argv[1]);
  
  UnitAny *unitPtr = deref((UnitAny **)0x7A6A70);

  std::cout << "Player unit struct address: " << std::hex << unitPtr << std::dec << std::endl;

  while(1) {
    UnitAny unit = deref(unitPtr);
    
    std::cout << "Seed: " << unit.dwSeed[0] << ", Y: " << unit.wY << std::endl;

    usleep(100000);
  }

  return 0;
}
