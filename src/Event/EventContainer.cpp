//
// Created by yz on 18-9-8.
//

#include "EventContainer.h"

namespace Pump {

////////////////////////////////////////////////
//                    EvList
////////////////////////////////////////////////

void EvList::insert(nsp_std::string &strName, PtrEvent pEv) {
  nsp_std::map<nsp_std::string, PtrEvent>::iterator it = m_lEvs.find(strName);
  if (it != m_lEvs.end()) {
    /* 已存在对应关键字元素, 重置元素 */
    it->second.reset();
    it->second = pEv;
    return;
  }
  m_lEvs.insert(nsp_std::make_pair(strName, pEv));
}

void EvList::erase(nsp_std::string &strName) {
  nsp_std::map<nsp_std::string, PtrEvent>::iterator it = m_lEvs.find(strName);
  if (it == m_lEvs.end()) {
    /* 无查找元素 */
    return;
  }
  it->second.reset();
  m_lEvs.erase(it);
}

PtrEvent EvList::at(nsp_std::string &strName) {
  nsp_std::map<nsp_std::string, PtrEvent>::iterator it = m_lEvs.find(strName);
  if (it == m_lEvs.end()) {
    /* 无查找元素 */
    return PtrEvent();
  }
  return it->second;
}

////////////////////////////////////////////////
//                 PrePostList
////////////////////////////////////////////////

PrePostList::PrePostList() {
  m_level_0 = nsp_boost::make_shared<EvList>();
  m_level_1 = nsp_boost::make_shared<CbList>();
}

PrePostList::~PrePostList() {

}

size_t PrePostList::runAll() {
  return 0;
}

}
