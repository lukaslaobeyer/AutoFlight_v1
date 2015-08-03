#ifndef ICONTROLLERINPUTLISTENER_H
#define ICONTROLLERINPUTLISTENER_H

#include "controllerinput.h"

class IControllerInputListener
{
	public:
        	virtual ~IControllerInputListener() {}
        	virtual void controllerInputAvailable(std::shared_ptr<const ControllerInput>) = 0;
};

#endif
