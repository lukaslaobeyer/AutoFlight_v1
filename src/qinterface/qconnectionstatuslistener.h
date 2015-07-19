#ifndef QCONNECTIONSTATUSLISTENER_H
#define QCONNECTIONSTATUSLISTENER_H

#include <types.h>
#include <memory>

class QConnectionStatusListener
{
	public:
        	virtual ~QConnectionStatusListener() {}
    public:
        	virtual void connectionEstablished() = 0; // Must be implemented as slot
			virtual void connectionLost() = 0; // Must be implemented as slot
};

#endif
