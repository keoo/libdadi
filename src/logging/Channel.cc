/**
 * @file   Channel.cc
 * @author Haïkel Guémar <haikel.guemar@sysfera.com>
 * @brief  defines an abstract base class for channels
 * @section License
 *   |LICENSE|
 *
 */

#include "dadi/Logging/Channel.hh"

namespace dadi {

Channel::Channel() {}

Channel::~Channel() {}

void
Channel::open() {}

void
Channel::close() {}

} /* namespace dadi*/
