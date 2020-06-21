#include "Geopos.h"
#include <cmath>

namespace utils
{
    inline double DegToRad(double deg)
    {
        return deg*M_PI/180.0;
    }

    inline double RadToDeg(double rad)
    {
        return rad*180.0/M_PI;
    }

    double GetSphericalDistance(const Geopos& a, const Geopos& b)
    {
        double r = 6371.01; // Average radius of earth in km
        double aLatRad=DegToRad(a.GetLat());
        double bLatRad=DegToRad(b.GetLat());
        double dLat=DegToRad(b.GetLat()-a.GetLat());
        double dLon=DegToRad(b.GetLon()-a.GetLon());

        double sindLonDiv2=sin(dLon/2);

        double aa = sin(dLat/2)*sin(dLat/2)+
            cos(aLatRad)*cos(bLatRad)*
            sindLonDiv2*sindLonDiv2;

        double c = 2*atan2(sqrt(aa),sqrt(1-aa));

        return r*c; //KM Return
    }

    Geopos::Geopos(double latitude, double longitude)
    {
        _latitude = latitude;
        _longitude = longitude;
    }

    Geopos::Geopos() 
    {
        _latitude = 0;
        _longitude = 0;
    }

    Geopos::~Geopos()
    {
    }

    double Geopos::GetLat() const
    {
        return _latitude;
    }

    double Geopos::GetLon() const
    {
        return _longitude;
    }

    void Geopos::SetLat(double value)
    {
        _latitude = value;
    }

    void Geopos::SetLon(double value)
    {
        _longitude = value;
    }

    double Geopos::GetDistance(const Geopos &target) const 
    {
        return GetSphericalDistance(*this, target);
    }

    double Geopos::GetDistance(double latitude, double longitude) const
    {
        return GetSphericalDistance(*this, Geopos(latitude, longitude));
    }

    void Geopos::Set(double latitude, double longitude) 
    {
         _latitude = latitude;
        _longitude = longitude;
    }

    //https://stackoverflow.com/questions/24950614/abnormal-output-when-using-proj4-to-transform-latlong-to-utm
    const std::string array1[] = { "ABCDEFGH", "JKLMNPQR", "STUVWXYZ" };
    const std::string array2[] = { "ABCDEFGHJKLMNPQRSTUV","FGHJKLMNPQRSTUVABCDE" };
    //var i = [1.1,2.0,2.8,3.7,4.6,5.5,6.4,7.3,8.2,9.1,0,0.8,1.7,2.6,3.5,4.4,5.3,6.2,7.0,7.9];
    const double array3[] = {1.1,2.0,2.8,3.7,4.6,5.5,6.4,7.3,8.2,9.1,0,0.8,1.7,2.6,3.5,4.4,5.3,6.2,7.0,7.9};
    //var j = [0,2,2,2,4,4,6,6,8,8,0,0,0,2,2,4,4,6,6,6];
    const double array4[] = {0,2,2,2,4,4,6,6,8,8,0,0,0,2,2,4,4,6,6,6};

    void Geopos::FromMGRSstring(const std::string& text) 
    {
        //33T XP 38463 08751
        auto c = std::atoi(text.substr(0, 2).c_str()); //33
        auto d = text.substr(2, 1)[0]; //T
        auto subZone = text.substr(4, 2); //XP

        auto test = text.substr(13, 5);
        auto lati = std::atoi(text.substr(7, 5).c_str()); //38463
        auto longi = std::atoi(text.substr(13, 5).c_str()); //08751

        auto e = (c*6-183)*M_PI / 180.0;
        //var f = ["ABCDEFGH","JKLMNPQR","STUVWXYZ"][(c-1) % 3].indexOf(b[1][0]) + 1;
        auto f = array1[(c-1) % 3].find_first_of(subZone[0]) + 1;
        //var g = "CDEFGHJKLMNPQRSTUVWXX".indexOf(d);
        auto g = std::string("CDEFGHJKLMNPQRSTUVWX").find_first_of(d);
        //var h = ["ABCDEFGHJKLMNPQRSTUV","FGHJKLMNPQRSTUVABCDE"][(c-1) % 2].indexOf(b[1][1]);
        double h = array2[(c-1) % 2].find_first_of(subZone[1]);
        auto k = array3[g];
        //var l = Number(j[g]) + h / 10;
        auto l = array4[g] + h / 10.0;
        if (l < k) l += 2;
        auto m = f*100000.0 + lati;
        auto n = l*1000000 + longi;
        m -= 500000.0;
        if (d < 'N') n -= 10000000.0;
        m /= 0.9996; n /= 0.9996;
        auto o = n / 6367449.14570093;
        auto p = o + (0.0025188266133249035*sin(2.0*o)) + (0.0000037009491206268*sin(4.0*o)) + (0.0000000074477705265*sin(6.0*o)) + (0.0000000000170359940*sin(8.0*o));
        auto q = tan(p);
        auto r = q*q;
        auto s = r*r;
        auto t = cos(p);
        auto u = 0.006739496819936062*pow(t,2);
        auto v = 40680631590769 / (6356752.314*sqrt(1 + u));
        auto w = v;
        auto x = 1.0 / (w*t); w *= v;
        auto y = q / (2.0*w); w *= v;
        auto z = 1.0 / (6.0*w*t); w *= v;
        auto aa = q / (24.0*w); w *= v;
        auto ab = 1.0 / (120.0*w*t); w *= v;
        auto ac = q / (720.0*w); w *= v;
        auto ad = 1.0 / (5040.0*w*t); w *= v;
        auto ae = q / (40320.0*w);
        auto af = -1.0-u;
        auto ag = -1.0-2*r-u;
        auto ah = 5.0 + 3.0*r + 6.0*u-6.0*r*u-3.0*(u*u)-9.0*r*(u*u);
        auto ai = 5.0 + 28.0*r + 24.0*s + 6.0*u + 8.0*r*u;
        auto aj = -61.0-90.0*r-45.0*s-107.0*u + 162.0*r*u;
        auto ak = -61.0-662.0*r-1320.0*s-720.0*(s*r);
        auto al = 1385.0 + 3633.0*r + 4095.0*s + 1575*(s*r);
        auto lat = p + y*af*(m*m) + aa*ah*pow(m,4) + ac*aj*pow(m,6) + ae*al*pow(m,8);
        auto lng = e + x*m + z*ag*pow(m,3) + ab*ai*pow(m,5) + ad*ak*pow(m,7);
        _latitude = lat*180 / M_PI;
        _longitude = lng*180 / M_PI;
    }

    std::string Geopos::ToText()
    {
        std::string result("");
        result = "Latitude " + std::to_string(_latitude) + " Longitude " + std::to_string(_longitude);
        return result;
    }
}//namespace utils