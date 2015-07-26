#ifndef ASMODULES_OPENCV_H_
#define ASMODULES_OPENCV_H_

#include <cmath> // Needed, see http://boost.2283326.n4.nabble.com/Boost-Python-Compile-Error-s-GCC-via-MinGW-w64-td3165793.html#a3166760
                 // and http://bugs.python.org/issue11566

#include <boost/python.hpp>
#include <string>
#include <drone.h>
#include <drones/fpvdrone.h>
#include <memory>
#include <mutex>
#include <boost/thread.hpp>
#include <april/TagFamily.h>
#include <april/TagDetector.h>
#include "../../imgprocui/imagevisualizer.h"
#include "../iscriptsimulationui.h"

#define TAG_PROCESS_INTERVAL 25

class ImgProc
{
	public:
		ImgProc(std::shared_ptr<FPVDrone> drone = NULL, ImageVisualizer *iv = NULL, bool simulationMode = false, IScriptSimulationUI *simUI = NULL);

		PyObject *getLatestFrame();
		unsigned long getFrameAge();
		void showFrame(PyObject *frame);

        void startTagDetector();
        void stopTagDetector();
		void setTagFamily(std::string family);
        void setTagROI(int x, int y, int width, int height);
        boost::python::list getTagDetections();

		bool abortFlag = false;
	private:
		bool initNumPy();

        void processTags();

        std::shared_ptr<boost::thread> _worker;
        TagDetectionArray _detections;
        bool _worker_running = false;
        std::shared_ptr<TagFamily> _tag_family;
        std::shared_ptr<TagDetector> _detector;
        std::shared_ptr<std::mutex> _detector_mutex;
        cv::Rect _roi_rect;

		std::shared_ptr<FPVDrone> d = NULL;
		bool sim = true;
		IScriptSimulationUI *ssui;
		ImageVisualizer *_iv = NULL;
};

#endif
