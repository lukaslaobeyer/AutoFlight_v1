#ifndef ASMODULES_OPENCV_H_
#define ASMODULES_OPENCV_H_

#include <cmath> // Needed, see http://boost.2283326.n4.nabble.com/Boost-Python-Compile-Error-s-GCC-via-MinGW-w64-td3165793.html#a3166760
                 // and http://bugs.python.org/issue11566

#include <boost/python.hpp>
#include <string>
#include <drone.h>
#include <drones/fpvdrone.h>
#include "../../imgprocui/imagevisualizer.h"
#include "../iscriptsimulationui.h"

class ImgProc
{
	public:
		ImgProc(std::shared_ptr<FPVDrone> drone = NULL, ImageVisualizer *iv = NULL, bool simulationMode = false, IScriptSimulationUI *simUI = NULL);

		PyObject *getLatestFrame();
		unsigned long getFrameAge();
		void showFrame(PyObject *frame);

		bool abortFlag = false;
	private:
		bool initNumPy();

		std::shared_ptr<FPVDrone> d = NULL;
		bool sim = true;
		IScriptSimulationUI *ssui;
		ImageVisualizer *_iv = NULL;
};

#endif
