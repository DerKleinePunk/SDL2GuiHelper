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
}//namespace utils