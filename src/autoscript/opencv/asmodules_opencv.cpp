#include "asmodules_opencv.h"
#include <opencv2/core.hpp>
#include "CVBoostConverter.hpp"

ImgProc::ImgProc(std::shared_ptr<FPVDrone> drone, ImageVisualizer *iv, bool simulationMode, IScriptSimulationUI *simUI)
{
	d = drone;
	sim = simulationMode;
	ssui = simUI;

	_iv = iv;

    _detector_mutex.reset(new std::mutex);
    _tag_family.reset(new TagFamily("Tag36h9"));
    _detector.reset(new TagDetector(*_tag_family));

	initNumPy();
}

PyObject *ImgProc::getLatestFrame()
{
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

void ImgProc::startTagDetector()
{
    if(!_worker_running)
    {
        _worker_running = true;

        _worker.reset(new boost::thread(
                [this]()
                {
                    while(_worker_running && !abortFlag)
                    {
                        processTags();
                        boost::this_thread::sleep(boost::posix_time::milliseconds(TAG_PROCESS_INTERVAL));
                    }
                }
        ));
    }
}

void ImgProc::stopTagDetector()
{
    if(_worker_running)
    {
        _worker_running = false;
        _worker->join();
        _worker.reset();

        _worker_running = false;
    }
}

void ImgProc::setTagFamily(std::string family)
{
    bool ok = false;
    for(std::string existing_family : TagFamily::families())
    {
        if(existing_family == family)
        {
            ok = true;
            break;
        }
    }

    if(!ok)
    {
        std::string error = "Specified tag family '" + family + "' does not exist!";
        PyErr_SetString(PyExc_RuntimeError, error.c_str());
        boost::python::throw_error_already_set();
        return;
    }

    _detector_mutex->lock();
    _tag_family.reset(new TagFamily(family));
    _detector.reset(new TagDetector(*_tag_family));
    _detector_mutex->unlock();
}

void ImgProc::setTagROI(int x, int y, int width, int height)
{
    _roi_rect.x = x;
    _roi_rect.y = y;
    _roi_rect.width = width;
    _roi_rect.height = height;
}

boost::python::list ImgProc::getTagDetections()
{
    boost::python::list py_detections;

    if(!_detections.empty())
    {
        for(TagDetection detection : _detections)
        {
            boost::python::list points;
            for(int i = 0; i < 4; i++)
            {
                points.append(boost::python::make_tuple(detection.p[i].x + _roi_rect.x, detection.p[i].y + _roi_rect.y));
            }
            boost::python::tuple params = boost::python::make_tuple(detection.id, detection.good, points);
            py_detections.append(params);
        }
    }
    return py_detections;
}

void ImgProc::processTags()
{
    if(!d->isConnected())
    {
        return;
    }

    cv::Mat latestFrame_orig = d->getLatestFrame();

    if(_roi_rect.width < 0 || _roi_rect.height < 0 || _roi_rect.x <= 0 || _roi_rect.y <= 0)
    {
        _roi_rect.x = 0;
        _roi_rect.y = 0;
        _roi_rect.width = latestFrame_orig.cols;
        _roi_rect.height = latestFrame_orig.rows;
    }

    cv::Mat roi(latestFrame_orig, _roi_rect);
    cv::Mat latestFrame = roi.clone();

    if(latestFrame.cols == 0 || latestFrame.rows == 0)
    {
        return;
    }

    cv::Point2d opticalCenter(0.5*latestFrame.rows, 0.5*latestFrame.cols);
    if(_detector_mutex->try_lock())
    {
        _detector->process(latestFrame, opticalCenter, _detections);
        _detector_mutex->unlock();
    }
}

bool ImgProc::initNumPy()
{
    import_array1(false);
    return true;
}