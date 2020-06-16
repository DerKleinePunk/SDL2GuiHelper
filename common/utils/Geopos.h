#pragma once

namespace utils
{
    class Geopos
    {
    private:
        double _latitude;
        double _longitude;

    public:
        Geopos(double latitude, double longitude);
        ~Geopos();

        double GetLat() const;
        double GetLon() const;
        double GetDistance(const Geopos &target) const;
        double GetDistance(double latitude, double longitude) const;
        void Set(double latitude, double longitude);
    };

    
   

}//namespace utils