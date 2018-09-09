/**
 * @file Service.h
 * @brief Service 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.09.09
 */

#ifndef LIBPUMP_SERVICE_H
#define LIBPUMP_SERVICE_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "../pumpdef.h"

namespace nsp_boost = boost;

namespace PUMP {

/**
 * @class NetService
 * @brief 与 IoFd 绑定的服务对象
 */
PUMP_ABSTRACT
class Service
  : public nsp_boost::noncopyable {
public:
  Service() {}
  
  virtual ~Service() {}
};

typedef nsp_boost::shared_ptr<Service> PtrService;

}

#endif //LIBPUMP_SERVICE_H
