//
// Created by yz on 18-9-15.
//

#ifndef LIBPUMP_PUMPCONTAINER_H
#define LIBPUMP_PUMPCONTAINER_H

#include <boost/noncopyable.hpp>

#include "pumpdef.h"

namespace PUMP {

class Pump;
typedef nsp_boost::shared_ptr<Pump> PtrPump;
/**
 * @class WatcherContainer []
 * @brief Watcher 对象的容器抽象对象
 *
 * 用户必须派生 WatcherContainer 对象实现接口, 并实现存储 Watcher 的对象.
 * Watcher 存储逻辑结构可以使任意的
 *
 * > WARNING 现阶段不支持Watcher动态创建
 */
PUMP_ABSTRACT
class PumpContainer
  : virtual public nsp_boost::noncopyable {
public:
  PumpContainer() {}
  
  virtual ~PumpContainer() {}
  
  /**
   * @fn virtual size_t init() = 0
   * @brief 初始化容器对象
   * @return 成功构造的容器个数
   */
  virtual size_t init() = 0;
  
  /**
   * @fn virtual PtrWatcher begin() = 0
   * @brief 获取容器中第一个Watcher指针
   * @return　Watcher指针
   */
  virtual PtrPump begin() = 0;
  
  /**
   * @fn virtual PtrWatcher end() = 0
   * @brief 获取容器中最后一个Watcher指针
   * @return　Watcher指针
   */
  virtual PtrPump end() = 0;
  
  /**
   * @fn virtual PtrWatcher next() = 0
   * @brief 获取容器中当前Watcher指针的下一个
   * @return　Watcher指针
   */
  virtual PtrPump next() = 0;
  
  /**
   * @fn virtual bool empty() = 0
   * @brief 判断容器是否为空
   * @return　数量
   */
  virtual bool empty() = 0;
  
  /**
   * @fn virtual size_t count() = 0
   * @brief Watcher 对象的数量
   * @return　数量
   */
  virtual size_t count() = 0;
};

/**
 * @typedef typedef nsp_boost::shared_ptr<PumpContainer> PtrPumpContainer
 * @brief PtrPumpContainer 内存托管对象
 */
typedef nsp_boost::shared_ptr<PumpContainer> PtrPumpContainer;


}

#endif //LIBPUMP_PUMPCONTAINER_H
