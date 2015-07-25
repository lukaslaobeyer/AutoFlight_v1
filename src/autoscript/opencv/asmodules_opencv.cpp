#include "asmodules_opencv.h"
#include <opencv2/core.hpp>
#include "CVBoostConverter.hpp"

ImgProc::ImgProc(std::shared_ptr<FPVDrone> drone, ImageVisualizer *iv, bool simulationMode, IScriptSimulationUI *simUI)
{
	d = drone;
	sim = simulationMode;
	ssui = simUI;

	_iv = iv;

	initNumPy();
}

PyObject *ImgProc::getLatestFrame()
{
	//TODO: this
	cv::Mat mat = d->getLatestFrame();

	PyObject *ret = bcvt::fromMatToNDArray(mat);

	return ret;
}

unsigned long ImgProc::getFrameAge()
{
	return d->getFrameAge();
}

void ImgProc::showFrame(PyObject *frame)
{
	cv::Mat mat = bcvt::fromNDArrayToMat(frame);
	_iv->showImage(mat);
}

bool ImgProc::initNumPy()
{
    import_array1(false);
    return true;
}