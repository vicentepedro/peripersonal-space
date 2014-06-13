#include "kalmanThread.h"

kalmanThread::kalmanThread(int _rate, const string &_name, const string &_robot, int _v, double _nDThr) :
                       RateThread(_rate), name(_name), robot(_robot), verbosity(_v), noDataThres(_nDThr)
{
    setKalmanState(KALMAN_INIT);
    timeNow = yarp::os::Time::now();

    generateMatrices(4);

    for (size_t i = 0; i < 3; i++)
    {
        posVelKalman.push_back(Kalman(kalA,kalH,kalQ,kalR));
    }
}

bool kalmanThread::threadInit()
{
    return true;
}

void kalmanThread::run()
{
    int kS=-1;

    if (getKalmanState(kS))
    {
        if ( kS == KALMAN_NORMAL || kS == KALMAN_NOINPUT || kS == KALMAN_NEWINPUT )
        {
            kalmanPredict();
            
            if ( kS == KALMAN_NEWINPUT )
            {
                kalmanUpdate();                

                for (size_t i = 0; i < 3; i++)
                {
                    if ( posVelKalman[i].get_ValidationGate() > kalThres )
                    {
                        setKalmanState(KALMAN_STOPPED);
                        printMessage(0,"Kalman filter #%i overcomes the validationGate. Stopping.\n",i);
                    }
                }
            }            

            if ( yarp::os::Time::now() - timeNow > noDataThres)
            {
                setKalmanState(KALMAN_STOPPED);
                printMessage(0,"Kalman filter has been stopped for lack of fresh data.\n");
            }

            setKalmanOutput();
        }
    }
}


bool kalmanThread::generateMatrices(const int order)
{
    kalOrder = order;

    kalOut.resize(kalOrder,0.0);
    kalTs     = 0.1;
    kalIn.resize(3,0.0);

    kalA      = eye(kalOrder);
    kalA(0,1) = 0.01;
    kalA(0,2) = 0.0001;
    kalA(1,2) = 0.01;
    kalA(1,3) = 0.0001;
    kalA(2,3) = 0.01;

    kalH      = zeros(1,kalOrder);
    kalH(0,0) = 1;

    kalQ = 0.00001 * eye(kalOrder);
    kalP = 0.00001 * eye(kalOrder);

    // Threshold is set to chi2inv(0.95,1)
    kalThres = 3.8415;

    kalR      = eye(1);
    kalR(0,0) = 0.01;

    return true;
}


bool kalmanThread::kalmanUpdate()
{
    Vector input(3,0.0);

    if (getKalmanInput(input))
    {
        for (size_t i = 0; i < 3; i++)
        {
            posVelKalman[i].correct(Vector(1,input(i)));
        }
    }

    return true;
}

bool kalmanThread::kalmanPredict()
{
    for (size_t i = 0; i < 3; i++)
    {
        posVelKalman[i].predict();
    }

    return true;
}

bool kalmanThread::kalmanInit(const Vector &inVec)
{
    int kS=-1;

    if (getKalmanState(kS) && kS == KALMAN_INIT)
    {
        for (size_t i = 0; i < 3; i++)
        {
            Vector x0(kalOrder,0.0);
            x0(0) = inVec(i);
            posVelKalman[i].init(x0,kalP);
        }
        setKalmanState(KALMAN_NOINPUT);
        timeNow = yarp::os::Time::now();
    }
    else
        return false;

    printMessage(0,"Kalman filter initialized.\n");
    return true;
}

bool kalmanThread::setKalmanOutput()
{
    outMutex.lock();
        for (size_t i = 0; i < 3; i++)
        {
            Vector output = posVelKalman[i].get_y();
            kalOut(i) = output(0);
        }
    outMutex.unlock();
    
    return true;
}

bool kalmanThread::getKalmanOutput(Vector &e)
{
    outMutex.lock();
        e = kalOut;
    outMutex.unlock();
    
    return true;
}

bool kalmanThread::setKalmanInput(const Vector &inVec)
{
    inputMutex.lock();

        int kS=-1;
        if (getKalmanState(kS) && kS!=KALMAN_STOPPED)
        {
            kalIn = inVec;
            setKalmanState(KALMAN_NEWINPUT);
        }
        timeNow = yarp::os::Time::now();

    inputMutex.unlock();

    return true;
}

bool kalmanThread::getKalmanInput(Vector &inVec)
{
    inputMutex.lock();
        int kS=-1;
        bool result=false;
        if (getKalmanState(kS) && kS==KALMAN_NEWINPUT)
        {
            inVec = kalIn;
            setKalmanState(KALMAN_NOINPUT);
            result = true;
        }
    inputMutex.unlock();
    
    return result;
}

bool kalmanThread::setKalmanState(const int s)
{
    stateMutex.lock();
        kalState = s;
    stateMutex.unlock();
    
    return true;
}

bool kalmanThread::getKalmanState(int &s)
{
    stateMutex.lock();
        s = kalState;
    stateMutex.unlock();
    
    return true;
}

int kalmanThread::printMessage(const int l, const char *f, ...) const
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

void kalmanThread::threadRelease()
{

}

// empty line to make gcc happy
