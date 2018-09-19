//
// Created by yz on 18-9-8.
//

#include "WatcherContainer.h"

namespace Pump {

////////////////////////////////////////////////
//                   WatcherList
////////////////////////////////////////////////

WatcherList::WatcherList() {}

WatcherList::~WatcherList() {}

/**
 * @fn WatcherList::size_t init()
 * @brief 初始化容器对象
 * @return 成功构造的容器个数
 */
size_t WatcherList::init() {}

#ifdef _TEST_LEVEL_INFO

template<class T>
void WatcherList::test_createWatcher() {
  nsp_boost::shared_ptr<T> pWatcher = nsp_boost::make_shared<T>();
  m_vecWatcher.push_back(pWatcher);
}

#endif // _TEST_LEVEL_INFO

/**
 * @fn WatcherList::WPtrWatcher begin()
 * @brief 获取容器中第一个Watcher指针
 * @return　Watcher指针
 */
PtrWatcher WatcherList::begin() {
  return *m_vecWatcher.begin();
}

/**
 * @fn WatcherList::WPtrWatcher end()
 * @brief 获取容器中最后一个Watcher指针
 * @return　Watcher指针
 */
PtrWatcher WatcherList::end() {

}

/**
 * @fn WatcherList::WPtrWatcher next()
 * @brief 获取容器中当前Watcher指针的下一个
 * @return　Watcher指针
 */
PtrWatcher WatcherList::next() {}

/**
 * @fn WatcherList::bool empty()
 * @brief 判断容器是否为空
 * @return　数量
 */
bool WatcherList::empty() {}

/**
 * @fn WatcherList::size_t count()
 * @brief PWatcher 对象的数量
 * @return　数量
 */
size_t WatcherList::count() {}

}
