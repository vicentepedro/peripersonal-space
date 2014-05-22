#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <iomanip>

#include "ultimateTrackerThread.h"

IncomingEvent eventFromBottle(const Bottle &b)
{
    IncomingEvent ie;
    ie.fromBottle(b);
    return ie;
};

ultimateTrackerThread::ultimateTrackerThread(int _rate, const string &_name, const string &_robot, int _v) :
                       RateThread(_rate), name(_name), robot(_robot), verbosity(_v)
{
    stateFlag = 0;
    timeNow = yarp::os::Time::now();

    motionCUTBlobs = new BufferedPort<Bottle>;
    motionCUTPos.resize(2,0.0);
}

bool ultimateTrackerThread::threadInit()
{
    motionCUTBlobs -> open(("/"+name+"/mCUT:i").c_str());


    Network::connect("/motionCUT/blobs:o",("/"+name+"/mCUT:i").c_str());
}

void ultimateTrackerThread::run()
{
    switch (stateFlag)
    {
        case 0:
            printMessage(0,"Looking for motion");
            timeNow = yarp::os::Time::now();
            oldMcutPoss.clear();
            stateFlag++;
            break;
        case 1:
            // state #01: check the motionCUT and see if there's something interesting
            if (processMotion())
            {
                printMessage(0,"Initializing tracker");
                initializeTracker();
                stateFlag++;
            }
            break;
        case 2:
            printMessage(2,"Reading from tracker...");
            readFromTracker();

            stateFlag++;
            break;
        default:
            printMessage(0,"ERROR!!! ultimateTrackerThread should never be here!!!\nState: %d",stateFlag);
            Time::delay(1);
            break;
    }
}

bool ultimateTrackerThread::noInput()
{
    if (yarp::os::Time::now() - timeNow > 1.0)
    {
        timeNow = yarp::os::Time::now();
        return true;
    }
    return false;
}

bool ultimateTrackerThread::processMotion()
{
    if (motionCUTBottle = motionCUTBlobs->read(false))
    {
        if (motionCUTBottle!=NULL)
        {
            timeNow = yarp::os::Time::now();
            motionCUTPos.zero();
            Bottle *blob;
            // Let's process the blob with the maximum size
            blob = motionCUTBottle->get(0).asList();
            motionCUTPos(0) = blob->get(0).asInt();
            motionCUTPos(1) = blob->get(1).asInt();
            // If the blob is stable, return true.
            if (stabilityCheck())
                return true;
        }
    }
    else if (noInput())
    {
        oldMcutPoss.clear();
    }
    return false;
}

bool ultimateTrackerThread::stabilityCheck()
{
    oldMcutPoss.push_back(motionCUTPos);

    if (oldMcutPoss.size()>10)
    {
        oldMcutPoss.erase(oldMcutPoss.begin());  //remove first element
        Vector avgPos(2,0.0);

        for (int i = 0; i < oldMcutPoss.size(); i++)
        {
            avgPos(0) += oldMcutPoss[i](0);
            avgPos(1) += oldMcutPoss[i](1);
        }
        avgPos = avgPos/oldMcutPoss.size();

        if (motionCUTPos(0)-avgPos(0)<1.0 && motionCUTPos(1)-avgPos(1)<1.0)
        {
            return true;
        }
    }
    return false;
}


bool ultimateTrackerThread::readFromTracker()
{
    return false;
}

bool ultimateTrackerThread::getPointFromStereo()
{
    return false;
}

bool ultimateTrackerThread::initializeTracker()
{
    Network::connect("/motionCUT/crop:o","/templatePFTracker/template/image:i");
    yarp::os::Time::delay(0.2);
    Network::disconnect("/motionCUT/crop:o","/templatePFTracker/template/image:i");
    return false;
}

int ultimateTrackerThread::printMessage(const int l, const char *f, ...) const
{
    if (verbosity>=l)
    {
        fprintf(stdout,"*** %s: ",name.c_str());

        va_list ap;
        va_start(ap,f);
        int ret=vfprintf(stdout,f,ap);
        va_end(ap);

        return ret;
    }
    else
        return -1;
}

void ultimateTrackerThread::threadRelease()
{
    printMessage(0,"Closing ports..\n");
        closePort(motionCUTBlobs);
        printMessage(1,"    motionCUTBlobs successfully closed!\n");
}

// empty line to make gcc happy
