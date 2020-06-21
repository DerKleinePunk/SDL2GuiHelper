#pragma once
#include <string>

namespace utils
{
    class Geopos
    {
    private:
        double _latitude;
        double _longitude;

    public:
        Geopos(double latitude, double longitude);
        Geopos();
        ~Geopos();

        double GetLat() const;
        double GetLon() const;
        void SetLat(double value);
        void SetLon(double value);
        double GetDistance(const Geopos &target) const;
        double GetDistance(double latitude, double longitude) const;
        void Set(double latitude, double longitude);
        void FromMGRSstring(const std::string& text);
        std::string ToText();
    };

    
   

}//namespace utils