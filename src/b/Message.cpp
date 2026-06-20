#include "b/Message.hpp"

Message::Message() {}

Message::Message(const std::string& command,
                 const std::vector<std::string>& params)
    : _command(command), _params(params) {}

Message::Message(const std::string& prefix, const std::string& command,
                 const std::vector<std::string>& params)
    : _prefix(prefix), _command(command), _params(params) {}

Message::~Message() {}

const std::string& Message::getPrefix() const { return _prefix; }

const std::string& Message::getCommand() const { return _command; }

const std::vector<std::string>& Message::getParams() const { return _params; }

size_t Message::getParamCount() const { return _params.size(); }

const std::string& Message::getSingleParam(size_t index) const {
  static const std::string empty;
  if (index >= _params.size()) {
    return empty;
  }
  return _params[index];
}

bool Message::hasParam(size_t index) const { return index < _params.size(); }
