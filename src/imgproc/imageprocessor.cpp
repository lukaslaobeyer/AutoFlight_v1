#include "imageprocessor.h"
#include <iostream>
#include <vector>

#include <april/TagFamily.h>
#include <april/TagDetector.h>

using namespace std;

ImageProcessor::ImageProcessor(ImageVisualizer *iv)
{
	_iv = iv;
}

void ImageProcessor::startProcessing()
{
	if(_t_proc == nullptr)
	{
		_update_latest = true;
		_t_proc = new boost::thread(&ImageProcessor::runProcessingLoop, this);
	}
}

void ImageProcessor::runProcessingLoop()
{
	while(_update_latest)
	{
		processLatestFrame();
	}
}

void ImageProcessor::stopProcessing()
{
	if(_t_proc != nullptr)
	{
		_update_latest = false;
		_t_proc->join();

		delete _t_proc;
		_t_proc = nullptr;
	}
}

void ImageProcessor::videoFrameAvailable(cv::Mat frame)
{
	// Only copy the new frame into _latestFrame if no operation is being done currently
	if(_update_latest && _status == READY_FOR_NEXT_FRAME)
	{
		// Perform a deep copy of the latest frame for processing
		_latestFrame = frame.clone();

		_status = READY_FOR_PROCESS;
	}
}

cv::Mat ImageProcessor::getLatestFrame()
{
	return _latestFrame;
}

void ImageProcessor::processLatestFrame()
{
	while(_status != READY_FOR_PROCESS);

	_status = PROCESSING_FRAME;
	// Just trying out some stuff

    static TagFamily family("Tag36h9");
    static TagDetector detector(family);
    static TagDetectionArray detections;

    cv::Point2d opticalCenter(0.5*_latestFrame.rows, 0.5*_latestFrame.cols);
    detector.process(_latestFrame, opticalCenter, detections);

    cv::Mat final = family.superimposeDetections(_latestFrame, detections);

    if(!detections.empty())
    {
        int i = 0;
        for(TagDetection detection : detections)
        {
            cv::putText(final, "ID = " + to_string(detection.id), cv::Point2f(5, 20 + 25*i), cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar(0,255,255,255));
            i++;
        }

        _iv->showImage(final);
    }

	_status = READY_FOR_NEXT_FRAME;
}
