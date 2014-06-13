#include "iCub/periPersonalSpace/utils.h"

yarp::sig::Vector toVector(yarp::sig::Matrix m)
{
    Vector res(m.rows()*m.cols(),0.0);
    
    for (size_t r = 0; r < m.rows(); r++)
    {
        res.setSubvector(r*m.cols(),m.getRow(r));
    }

    return res;
}

void closePort(Contactable *_port)
{
    if (_port)
    {
        _port -> interrupt();
        _port -> close();

        delete _port;
        _port = 0;
    }
}

yarp::sig::Matrix matrixFromBottle(const Bottle b, int in, const int r, const int c)
{
    yarp::sig::Matrix m(r,c);
    m.zero();
    
    for (size_t i = 0; i<r; i++)
    {
        for (size_t j = 0; j<c; j++)
        {
            m(i,j) =  b.get(in).asDouble();
            in++;
        }
    }
    
    return m;
}

yarp::sig::Vector vectorFromBottle(const Bottle b, int in, const int size)
{
    yarp::sig::Vector v(size,0.0);

    for (size_t i = 0; i < size; i++)
    {
        v[i] = b.get(in).asDouble();
        in++;
    }
    return v;
}

void vectorIntoBottle(const yarp::sig::Vector v, Bottle &b)
{
    for (unsigned int i = 0; i < v.size(); i++)
    {
        b.addDouble(v[i]);
    }
}

void matrixIntoBottle(const yarp::sig::Matrix m, Bottle &b)
{
    Vector v = toVector(m);
    
    for (unsigned int i = 0; i < v.size(); i++)
    {
        b.addDouble(v[i]);
    }
}

string int_to_string( const int a )
{
    std::stringstream ss;
    ss << a;
    return ss.str();
}

unsigned int factorial(unsigned int n) 
{
    if (n == 0)
       return 1;
    return n * factorial(n - 1);
}

/****************************************************************/
/* INCOMING EVENT WRAPPER
*****************************************************************/
    IncomingEvent::IncomingEvent()
    {
        Pos.resize(3,0.0);
        Vel.resize(3,0.0);
        Src="";
        Radius=-1.0;
    }

    IncomingEvent::IncomingEvent(const Vector &p, const Vector &v, const double r, const string &s)
    {
        Pos = p;
        Vel = v;
        Src = s;
        Radius = r;
    }

    IncomingEvent::IncomingEvent(const IncomingEvent &e)
    {
        *this = e;
    }

    IncomingEvent::IncomingEvent(const Bottle &b)
    {
        fromBottle(b);
    }

    IncomingEvent & IncomingEvent::operator=(const IncomingEvent &e)
    {
        Pos    = e.Pos;
        Vel    = e.Vel;
        Src    = e.Src;
        Radius = e.Radius;
        return *this;
    }

    Bottle IncomingEvent::toBottle()
    {
        Bottle b;
        b.clear();

        b.addDouble(Pos[0]);
        b.addDouble(Pos[1]);
        b.addDouble(Pos[2]);
        b.addDouble(Vel[0]);
        b.addDouble(Vel[1]);
        b.addDouble(Vel[2]);
        b.addDouble(Radius);
        b.addString(Src);

        return b;
    }

    bool IncomingEvent::fromBottle(const Bottle &b)
    {
        Pos.resize(3,0.0);
        Vel.resize(3,0.0);
        Src="";
        Radius=-1.0;

        Pos[0] = b.get(0).asDouble();
        Pos[1] = b.get(1).asDouble();
        Pos[2] = b.get(2).asDouble();

        Vel[0] = b.get(3).asDouble();
        Vel[1] = b.get(4).asDouble();
        Vel[2] = b.get(5).asDouble();

        Radius = b.get(6).asDouble();
        Src    = b.get(7).asString();

        return true;
    }

    void IncomingEvent::print()
    {
        printf("Pos: %s\t Vel: %s\t Radius %g\t Src %s\n",Pos.toString().c_str(),Vel.toString().c_str(),Radius,Src.c_str());
    }

    string IncomingEvent::toString(int precision) const
    {
        stringstream res;
        res << "Pos: "<< Pos.toString() << "\t Vel: "<< Vel.toString()
            << "\t Radius: "<< Radius << "\t Src: "<< Src;
        return res.str();
    }

/****************************************************************/
/* INCOMING EVENT 4 TAXEL WRAPPER
*****************************************************************/
    IncomingEvent4Taxel::IncomingEvent4Taxel() : IncomingEvent()
    {
        NRM = 0;
        TTC = 0;
    }

    IncomingEvent4Taxel::IncomingEvent4Taxel(const Vector &p, const Vector &v,
                                             const double r, const string &s):
                                            IncomingEvent(p,v,r,s)
    {
        NRM = 0;
        TTC = 0;
    }

    IncomingEvent4Taxel::IncomingEvent4Taxel(const IncomingEvent4Taxel &e)
    {
        *this = e;   
    }

    IncomingEvent4Taxel::IncomingEvent4Taxel(const IncomingEvent &e)
    {
        *this = e;   
    }

    IncomingEvent4Taxel & IncomingEvent4Taxel::operator=(const IncomingEvent4Taxel &e)
    {
        IncomingEvent::operator=(e);
        TTC    = e.TTC;
        NRM    = e.NRM;
        return *this;
    }

    IncomingEvent4Taxel & IncomingEvent4Taxel::operator=(const IncomingEvent &e)
    {
        Pos    = e.Pos;
        Vel    = e.Vel;
        Src    = e.Src;
        Radius = e.Radius;
        TTC    = 0;
        NRM    = 0;
        return *this;
    }

    void IncomingEvent4Taxel::print()
    {
        printf("NRM: %g\t TTC: %g ", NRM*100, TTC);
        IncomingEvent::print();
    }

/****************************************************************/
/* TAXEL WRAPPER
*****************************************************************/
    void Taxel::init()
    {
        ID   = 0;
        Resp = 0;
        Pos.resize(3,0.0);
        WRFPos.resize(3,0.0);
        Norm.resize(3,0.0);
        px.resize(2,0.0);
        RF = eye(4);
        rfAngle = 40*M_PI/180;        
    }

    Taxel::Taxel()
    {
        init();
    }

    Taxel::Taxel(const Vector &p, const Vector &n)
    {
        init();
        Pos  = p;
        Norm = n;
        setRF();
    }

    Taxel::Taxel(const Vector &p, const Vector &n, const int &i)
    {
        init();
        ID   = i;
        Pos  = p;
        Norm = n;
        setRF();
    }

    Taxel & Taxel::operator=(const Taxel &t)
    {
        ID     = t.ID;
        Resp   = t.Resp;
        Pos    = t.Pos;
        WRFPos = t.WRFPos;
        Norm   = t.Norm;
        px     = t.px;
        RF     = t.RF;
        return *this;
    }

    void Taxel::print(int verbosity)
    {
        if (verbosity > 4)
            printf("ID %i \tPos %s \tNorm %s \n\tPosHst \n%s\n\n\tNegHst \n%s\n", ID,
                    Pos.toString().c_str(), Norm.toString().c_str(),
                    pwe2D.getPosHist().toString().c_str(),
                    pwe2D.getNegHist().toString().c_str());
        else 
            printf("ID %i \tPos %s \tNorm %s\n", ID,
                    Pos.toString().c_str(), Norm.toString().c_str());
            // printf("ID %i \tPos %s \tNorm %s \n\tHst %s\n", ID,
            //         Pos.toString().c_str(), Norm.toString().c_str(),
            //         pwe.getHist().toString().c_str());
    }

    string Taxel::toString(int precision)
    {
        stringstream res;
        res << "ID: " << ID << "\tPos: "<< Pos.toString() << "\t Norm: "<< Norm.toString();

        if (precision)
        {
            res << "\n Hst:\n"<< pwe2D.getPosHist().toString();
            res << "\n Hst:\n"<< pwe2D.getNegHist().toString() << endl;
        }
        return res.str();
    }

    void Taxel::setRF()
    {
        if (Norm == zeros(3))
        {
            RF=eye(4);
            return;
        }
        
        // Set the proper orientation for the touching end-effector
        Vector x(3,0.0), z(3,0.0), y(3,0.0);

        z = Norm;
        if (z[0] == 0.0)
        {
            z[0] = 0.00000001;    // Avoid the division by 0
        }
        y[0] = -z[2]/z[0]; y[2] = 1;
        x = -1*(cross(z,y));
        
        // Let's make them unitary vectors:
        x = x / norm(x);
        y = y / norm(y);
        z = z / norm(z);

        RF=eye(4);
        RF.setSubcol(x,0,0);
        RF.setSubcol(y,0,1);
        RF.setSubcol(z,0,2);
        RF.setSubcol(Pos,0,3);
    }

    bool Taxel::addSample(const IncomingEvent4Taxel ie)
    {
        if (!insideRFCheck(ie))
            return false;

        std::vector <double> X;
        X.push_back(ie.NRM);
        X.push_back(ie.TTC);

        return pwe2D.addSample(X);
    }

    bool Taxel::removeSample(const IncomingEvent4Taxel ie)
    {
        if (!insideRFCheck(ie))
            return false;

        std::vector <double> X;
        X.push_back(ie.NRM);
        X.push_back(ie.TTC);

        return pwe2D.removeSample(X);
    }

    bool Taxel::resetParzenWindow()
    {
        pwe2D.resetAllHist();
        return true;
    }

    bool Taxel::computeResponse()
    {
        if (!insideRFCheck(Evnt))
            return false;

        std::vector<double> In;
        In.push_back(Evnt.NRM);
        In.push_back(Evnt.TTC);
        Resp = pwe2D.computeResponse(In);

        return true;
    }

    bool Taxel::insideRFCheck(const IncomingEvent4Taxel ie)
    {
        std::vector<double> binWidth = pwe2D.getBinWidth();
        double binLimit = 2*binWidth[0];

        // the x,y limit of the receptive field at the incoming event's Z
        double RFlimit = ie.Pos(2)/tan(rfAngle);

        // the x,y limit of the receptive field in the first bin
        double RFlimit_cyl = binLimit/tan(rfAngle);

        // printf("binLimit: %g RFlimit_cyl: %g rfAngle: %g \n", binLimit, RFlimit_cyl, rfAngle);
        // printf("ie.Pos\t%s\n", ie.Pos.toString().c_str());
        // printf("Hist:\n%s\n", pwe2D.getHist().toString().c_str());

        if (ie.Pos(0)*ie.Pos(0)+ie.Pos(1)*ie.Pos(1) < RFlimit*RFlimit )
        {
            return true;
        }
        // There are two ifs only to let me debug things
        if ( (abs(ie.Pos(2))<=binLimit) && (ie.Pos(0)*ie.Pos(0)+ie.Pos(1)*ie.Pos(1) < RFlimit_cyl*RFlimit_cyl) )
        {
            return true;
        }

        return false;
    }

/****************************************************************/
/* SKINPART WRAPPER
*****************************************************************/
    skinPart::skinPart()
    {
        name="";
        size= 0;
    }

    skinPart & skinPart::operator=(const skinPart &spw)
    {
        name =spw.name;
        taxel=spw.taxel;
        size =spw.size;
        return *this;
    }

    void skinPart::print(int verbosity)
    {
        printf("**********\n");
        printf("name: %s\t", name.c_str());
        printf("size: %i\t", size);
        printf("taxel's size: %lu\n", taxel.size());
        
        for (size_t i = 0; i < taxel.size(); i++)
            taxel[i].print(verbosity);
        
        if (verbosity>=4)
        {
            printf("\nTaxel ID -> representative ID:\n");

            for (size_t i=0; i<size; i++)
            {
                printf("[ %lu -> %d ]\t",i,Taxel2Repr[i]);
                if (i % 8 == 7)
                {
                    printf("\n");
                }
            }
            printf("\n");
            
            printf("Representative ID -> Taxel IDs:\n");
            for(map<unsigned int, list<unsigned int> >::const_iterator iter_map = Repr2TaxelList.begin(); iter_map != Repr2TaxelList.end(); ++iter_map)
            {
                list<unsigned int> l = iter_map->second;
                printf("%d -> {",iter_map->first);
                for(list<unsigned int>::const_iterator iter_list = l.begin(); iter_list != l.end(); iter_list++)
                {
                    printf("%u, ",*iter_list);
                }
                printf("}\n");
            }    
            printf("\n");
        }
        printf("**********\n");
    }

    string skinPart::toString(int precision)
    {
        stringstream res;
        res << "**********\n" << "Name: " << name << "\tSize: "<< size << endl;
        for (size_t i = 0; i < taxel.size(); i++)
            res << taxel[i].toString(precision);
        res << "**********\n";
        return res.str();
    }

/****************************************************************/
/* EYE WRAPPER
*****************************************************************/
    bool getAlignHN(const ResourceFinder &rf, const string &type,
                    iKinChain *chain, const bool verbose)
    {
        ResourceFinder &_rf=const_cast<ResourceFinder&>(rf);
        if ((chain!=NULL) && _rf.isConfigured())
        {
            string message=_rf.findFile("from").c_str();
            if (!message.empty())
            {
                message+=": aligning matrix for "+type;
                Bottle &parType=_rf.findGroup(type.c_str());
                if (!parType.isNull())
                {
                    if (Bottle *bH=parType.find("HN").asList())
                    {
                        int i=0;
                        int j=0;

                        Matrix HN(4,4); HN=0.0;
                        for (size_t cnt=0; (cnt<bH->size()) && (cnt<HN.rows()*HN.cols()); cnt++)
                        {
                            HN(i,j)=bH->get(cnt).asDouble();
                            if (++j>=HN.cols())
                            {
                                i++;
                                j=0;
                            }
                        }

                        // enforce the homogeneous property
                        HN(3,0)=HN(3,1)=HN(3,2)=0.0;
                        HN(3,3)=1.0;

                        chain->setHN(HN);

                        if (verbose)
                        {
                            fprintf(stdout,"%s found:\n",message.c_str());
                            fprintf(stdout,"%s\n",HN.toString(3,3).c_str());
                        }

                        return true;
                    }
                }
            }
            else
            {
                message=_rf.find("from").asString().c_str();
                message+=": aligning matrix for "+type;
            }
            if (verbose)
                fprintf(stdout,"%s not found!\n",message.c_str());
        }

        return false;
    };

    /************************************************************************/
    bool getCamPrj(const ResourceFinder &rf, const string &type,
                   Matrix **Prj, const bool verbose)
    {
        ResourceFinder &_rf=const_cast<ResourceFinder&>(rf);
        *Prj=NULL;

        if (!_rf.isConfigured())
            return false;
        
        string message=_rf.findFile("from").c_str();
        if (!message.empty())
        {
            message+=": intrinsic parameters for "+type;
            Bottle &parType=_rf.findGroup(type.c_str());
            if (!parType.isNull())
            {
                if (parType.check("fx") && parType.check("fy") &&
                    parType.check("cx") && parType.check("cy"))
                {
                    double fx=parType.find("fx").asDouble();
                    double fy=parType.find("fy").asDouble();
                    double cx=parType.find("cx").asDouble();
                    double cy=parType.find("cy").asDouble();

                    if (verbose)
                    {
                        fprintf(stdout,"%s found:\n",message.c_str());
                        fprintf(stdout,"fx = %g\n",fx);
                        fprintf(stdout,"fy = %g\n",fy);
                        fprintf(stdout,"cx = %g\n",cx);
                        fprintf(stdout,"cy = %g\n",cy);
                    }

                    Matrix K=eye(3,3);
                    Matrix Pi=zeros(3,4);

                    K(0,0)=fx; K(1,1)=fy;
                    K(0,2)=cx; K(1,2)=cy; 
                    
                    Pi(0,0)=Pi(1,1)=Pi(2,2)=1.0; 

                    *Prj=new Matrix;
                    **Prj=K*Pi;

                    return true;
                }
            }
        }
        else
        {
            message=_rf.find("from").asString().c_str();
            message+=": intrinsic parameters for "+type;
        }

        if (verbose)
            fprintf(stdout,"%s not found!\n",message.c_str());

        return false;
    }

    /************************************************************************/
    eyeWrapper::eyeWrapper(string _name, double _hV, const ResourceFinder &_eyeCalibRF) :
               name(_name), headVersion(_hV)
    {
        if (name=="left")
        {
            eye=new iCubEye(headVersion>1.0?"left_v2":"left");
        }
        else if (name=="right")
        {
            eye=new iCubEye(headVersion>1.0?"right_v2":"right");
        }

        // remove constraints on the links
        // we use the chains for logging purpose
        eye->setAllConstraints(false);

        // release links
        eye->releaseLink(0);
        eye->releaseLink(1);
        eye->releaseLink(2);

        // add aligning matrices read from configuration file
        if (name=="left")
        {
            getAlignHN(_eyeCalibRF,"ALIGN_KIN_LEFT",eye->asChain(),true);
        }
        else if (name=="right")
        {
            getAlignHN(_eyeCalibRF,"ALIGN_KIN_RIGHT",eye->asChain(),true);
        }

        bool ret=0;

        // get camera projection matrix
        if (name=="left")
        {
            ret=getCamPrj(_eyeCalibRF,"CAMERA_CALIBRATION_LEFT",&Prj,true);
        }
        else if (name=="right")
        {
            ret=getCamPrj(_eyeCalibRF,"CAMERA_CALIBRATION_RIGHT",&Prj,true);
        }

        if (!ret)
            Prj=NULL;
    }

    eyeWrapper & eyeWrapper::operator= (const eyeWrapper &ew)
    {
        name        = ew.name;
        eye         = ew.eye;
        headVersion = ew.headVersion;
        Prj         = ew.Prj;
        return *this;
    }

// empty line to make gcc happy
