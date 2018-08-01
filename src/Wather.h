//
// Created by yz on 18-7-27.
//

#ifndef LIBPUMP_WATHER_H
#define LIBPUMP_WATHER_H

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace nsp_boost = ::boost;

namespace PUMP{

class Pump;

typedef nsp_boost::weak_ptr<void> PtrArg;

class Wather {
public:
	Wather();
	~Wather();
	void doWatching();
private:
	virtual void preProcess() = 0;
	virtual int dispatch(PtrArg _IN, PtrArg _Out) = 0;
	virtual void postProcess() = 0;
};

}



#endif //LIBPUMP_WATHER_H
