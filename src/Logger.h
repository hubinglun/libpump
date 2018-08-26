//
// Created by yz on 18-8-26.
//

#ifndef LIBPUMP_LOGGER_H
#define LIBPUMP_LOGGER_H

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <glog/logging.h>

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;
namespace nsp_google = ::google;

namespace PUMP{

class Logger {
public:
  Logger();
  ~Logger();
};

}

#endif //LIBPUMP_LOGGER_H
