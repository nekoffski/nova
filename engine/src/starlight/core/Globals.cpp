#include "Globals.hh"

namespace sl {

Globals::Globals(const Config& config) : m_config(config) {}

const Config& Globals::getConfig() const { return m_config; }

}  // namespace sl
