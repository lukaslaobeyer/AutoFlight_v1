#ifndef QSTATUSLISTENER_H
#define QSTATUSLISTENER_H

#include <types.h>
#include <memory>

class QStatusListener
{
	public:
        	virtual ~QStatusListener() {}
    public:
        	virtual void statusUpdateAvailable(int status) = 0; // Must be implemented as slot
};

#endif
