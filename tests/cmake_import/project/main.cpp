#include <databento/record.hpp>
#include <iostream>

int main(int argc, char** argv) {
  databento::RecordHeader header{};
  std::cout << header.Size() << '\n';

  return 0;
}
