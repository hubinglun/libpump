/**
 * @file WatcherContainer.h
 * @brief WatcherContainer 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.09.08
 */

#ifndef LIBPUMP_WATCHERCONTAINER_H
#define LIBPUMP_WATCHERCONTAINER_H

#include "PWatcher.h"

namespace Pump {

/**
 * @class WatcherContainer []
 * @brief PWatcher 对象的容器抽象对象
 *
 * 用户必须派生 WatcherContainer 对象实现接口, 并实现存储 PWatcher 的对象.
 * PWatcher 存储逻辑结构可以使任意的
 *
 * > WARNING 现阶段不支持Watcher动态创建
 */
PUMP_ABSTRACT
class WatcherContainer
  : virtual public nsp_boost::noncopyable {
public:
  WatcherContainer() {}
  
  virtual ~WatcherContainer() {}
  
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
  virtual PtrWatcher begin() = 0;
  
  /**
   * @fn virtual PtrWatcher end() = 0
   * @brief 获取容器中最后一个Watcher指针
   * @return　Watcher指针
   */
  virtual PtrWatcher end() = 0;
  
  /**
   * @fn virtual PtrWatcher next() = 0
   * @brief 获取容器中当前Watcher指针的下一个
   * @return　Watcher指针
   */
  virtual PtrWatcher next() = 0;
  
  /**
   * @fn virtual bool empty() = 0
   * @brief 判断容器是否为空
   * @return　数量
   */
  virtual bool empty() = 0;
  
  /**
   * @fn virtual size_t count() = 0
   * @brief PWatcher 对象的数量
   * @return　数量
   */
  virtual size_t count() = 0;
};

/**
 * @typedef typedef nsp_boost::shared_ptr<WatcherContainer> PtrWatcherContainer
 * @brief WatcherContainer 内存托管对象
 */
typedef nsp_boost::shared_ptr<WatcherContainer> PtrWatcherContainer;

/**
 * @class WatcherList []
 * @brief 以 std::vector<PtrWatcher> 存储Watcher的容器对象
 *
 */
PUMP_IMPLEMENT
class WatcherList
  : public WatcherContainer {
  /* FIXME 缺少初始化 PWatcher 对象的方法*/
public:
  WatcherList();
  
  virtual ~WatcherList();
  
  /**
   * @fn virtual size_t init()
   * @brief 初始化容器对象
   * @return 成功构造的容器个数
   */
  virtual size_t init();

#ifdef _TEST_LEVEL_DEBUG
  
  /**
   * @brief
   * @tparam T
   * @return
   */
  template<class T>
  void test_createWatcher();

#endif // _TEST_LEVEL_DEBUG
  
  /**
   * @fn virtual PtrWatcher begin()
   * @brief 获取容器中第一个Watcher指针
   * @return　Watcher指针
   */
  virtual PtrWatcher begin();
  
  /**
   * @fn virtual PtrWatcher end()
   * @brief 获取容器中最后一个Watcher指针
   * @return　Watcher指针
   */
  virtual PtrWatcher end();
  
  /**
   * @fn virtual PtrWatcher next()
   * @brief 获取容器中当前Watcher指针的下一个
   * @return　Watcher指针
   */
  virtual PtrWatcher next();
  
  /**
   * @fn virtual bool empty()
   * @brief 判断容器是否为空
   * @return　数量
   */
  virtual bool empty();
  
  /**
   * @fn virtual size_t count()
   * @brief PWatcher 对象的数量
   * @return　数量
   */
  virtual size_t count();

private:
  nsp_std::vector<PtrWatcher> m_vecWatcher;
};

}


#endif //LIBPUMP_WATCHERCONTAINER_H
