#include <DCCEXProtocol.h>

#include <cstdint>
#include <iostream>
#include <string>

namespace {

class LoopbackStream : public Stream {
public:
  int available() override { return static_cast<int>(_input.size()); }

  int read() override {
    if (_input.empty()) {
      return -1;
    }
    const int value = static_cast<unsigned char>(_input.front());
    _input.erase(0, 1);
    return value;
  }

  size_t write(uint8_t value) override {
    _output.push_back(static_cast<char>(value));
    return 1;
  }

  void enqueue(const char *message) { _input += message; }
  const std::string &output() const { return _output; }

private:
  std::string _input;
  std::string _output;
};

} // namespace

int main() {
  DCCEXProtocol protocol;
  LoopbackStream connection;
  protocol.connect(&connection);

  protocol.requestServerVersion();
  if (connection.output() != "<s>") {
    std::cerr << "unexpected request: " << connection.output() << '\n';
    return 1;
  }

  connection.enqueue("<iDCCEX V-5.4.1 / NATIVE / NONE / 0>");
  protocol.check();
  if (!protocol.receivedVersion() || protocol.getMajorVersion() != 5 || protocol.getMinorVersion() != 4 ||
      protocol.getPatchVersion() != 1) {
    std::cerr << "version response was not parsed\n";
    return 1;
  }

  std::cout << "DCCEXProtocol native example completed\n";
  return 0;
}
