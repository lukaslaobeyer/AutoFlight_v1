#ifndef QNAVDATALISTENER_H
#define QNAVDATALISTENER_H

#include <types.h>
#include <memory>

class QNavdataListener
{
	public:
        	virtual ~QNavdataListener() {}
    public:
        	virtual void navdataAvailable(std::shared_ptr<const drone::navdata> navdata) = 0; // Must be implemented as slot
};

#endif
