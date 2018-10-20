//
// Created by yz on 18-10-21.
//

#ifndef LIBPUMP_POLICY_HPP
#define LIBPUMP_POLICY_HPP

#include <utility>

#include <boost/noncopyable.hpp>

#include "block.hpp"
#include "Logger.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

namespace Pump {
namespace SmartMem {

/**
 * @class Policy
 * @brief 托管内存的管理策略对象
 */
class Policy
  : public nsp_boost::noncopyable
{
public:
  /**
   * @var m_px
   * @brief 内存描述块的地址 \
   *    \a本对象及其派生只允许赋值和释放
   */
  Block *m_px;
};

class PolicyMgr {
protected:
  Policy * m_pp;
};

}
}

#endif //LIBPUMP_POLICY_HPP
