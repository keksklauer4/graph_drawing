#ifndef __GD_JSON_HANDLING_HPP__
#define __GD_JSON_HANDLING_HPP__

#include <common/instance.hpp>
#include <memory>

namespace gd
{

  instance_t parseInstanceFromFile(const std::string& filename);

}


#endif
