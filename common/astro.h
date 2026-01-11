namespace Astro {
//common suncalc functions in C
// Astro constants
float toRad = std::numbers::pi / 180.0 ;
float toDeg = 180.0 / std::numbers::pi ;
double daysPerLunarMonth = 29.530588853 ;
double daySs = 60*60*24 ;
double J1970 = 2440588.0 ;
double J2000 = 2451545.0 ;
double astro_e = toRad * 23.4397 ;
double jd; // Julian Day
double d; // Astronomical Day
double moonagejd; // Moon Age based on Julian Day
float utc_offset;  // Offset from UTC to local hours
double sma; // solarMeanAnomaly(d)
double ecl; // eclipticLongitude(sma)
double phi; // User Latitude in radians
double lw; // User Longitude in radians
double ST; // siderealTime

// sun & moon coords (ra,dec) in radians
struct coords_struct {
    double dec;
    double ra;
};
coords_struct sun_coords, moon_coords ;

// sun & moon position (alt,az) in radians
struct position_struct {
    double alt;
    double az;
};
position_struct sun_position, moon_position ;

// sun & moon transit, rise, fall times
struct riseset_struct {
    float transit;
    float rise;
    float set;
};
riseset_struct sun_riseset, moon_riseset ;

// moon phase, illumination
float moon_phase ;
float moon_illumination ;

void init(float my_utc_offset, float my_latitude, float my_longitude) {
    utc_offset = my_utc_offset ;
    phi = ( toRad * my_latitude ) ;
    lw = ( - my_longitude * toRad ) ;  
}
void julian_day(double my_timestamp) {
    double my_jd = ( my_timestamp / daySs ) - 0.5 + J1970 ;
    jd = my_jd ;
}
void day() {
    d = jd - J2000 ;
}
double moon_age_jd () {
    double Agejdfraction = (jd - 2451550.0) / daysPerLunarMonth ;
    Agejdfraction = Agejdfraction - int(Agejdfraction);
    moonagejd = Agejdfraction * daysPerLunarMonth ;
    return moonagejd ;
}
double solarMeanAnomaly(double d) {
    // solarMeanAnomaly(d) { return rad * (357.5291 + 0.98560028 * d); }
    sma =  toRad * (357.5291 + 0.98560028 * d);
    return sma ;
}
double eclipticLongitude(double sma) {
    double C = toRad * (1.9148 * sin(sma) + 0.02 * sin(2 * sma) + 0.0003 * sin(3 * sma)) ; // equation of center
    double P = toRad * 102.9372 ; // perihelion of the Earth
    ecl = sma + C + P + std::numbers::pi;
    return ecl ;
}
coords_struct getCoords (double l, double b) {
    coords_struct temp { 0.0, 0.0 };
    temp.dec = asin(sin(b) * cos(astro_e) + cos(b) * sin(astro_e) * sin(l));
    double ra = atan2(sin(l) * cos(astro_e) - tan(b) * sin(astro_e), cos(l));
    if (ra < 0) ra = ra + std::numbers::pi * 2;
    temp.ra = ra;
    return temp;
}
void siderealTime() {
    // siderealTime(d, lw) { return rad * (280.16 + 360.9856235 * d) - lw); }
    ST = toRad * (280.16 + 360.9856235 * d) - lw;
    ST = remainder(ST,std::numbers::pi * 2) ;
    if (ST < 0) ST += std::numbers::pi * 2 ;
}
position_struct getPosition (double H, double phi, double dec) {
    position_struct temp { 0.0, 0.0 };
    temp.az = atan2( sin(H),( cos(H) * sin(phi) - tan(dec) * cos(phi)) );
    temp.alt = asin(sin(phi) * sin(dec) + cos(phi) * cos(dec) * cos(H));
    return temp;
}
double Refraction(double h) {
    if (h < 0) // the following formula works for positive altitudes only.
        h = 0; // if h = -0.08901179 a div/0 would occur.
    // formula 16.4 of "Astronomical Algorithms" 2nd edition by Jean Meeus (Willmann-Bell, Richmond) 1998.
    // 1.02 / tan(h + 10.26 / (h + 5.10)) h in degrees, result in arc minutes -> converted to rad:
    return 0.0002967 / tan(h + 0.00312536 / (h + 0.08901179));
}
coords_struct getMoonCoords (double d) {
    // geocentric ecliptic coordinates of the moon
    double L = toRad * (218.316 + 13.176396 * d); // ecliptic longitude
    double M = toRad * (134.963 + 13.064993 * d); // mean anomaly
    double F = toRad * (93.272 + 13.229350 * d);  // mean distance
    double l  = L + toRad * 6.289 * sin(M); // longitude
    double b  = toRad * 5.128 * sin(F);     // latitude
    coords_struct temp = getCoords(l, b);
    return temp;
}
double moonphaseangle(double Ds, double RAs, double Dm, double RAm) {
    double SinA = sin(Dm) * sin(Ds);
    double CosA =  cos(Dm) * cos(Ds) * cos(RAm - RAs);
    double phase_angle = acos(SinA + CosA);
    return phase_angle;
}
double moonillumination (double phase_angle) {
    double illumination_fraction = (1 - cos(phase_angle)) / 2;
    double illumination = illumination_fraction * 100;
    return illumination;
}
std::string moonphasetext (int age) {
    if (age < 2) { return "New Moon"; 
    } else if (age < 7) { return "Waxing Crescent";
    } else if (age < 8) { return "First Quarter";
    } else if (age < 13) { return "Waxing Gibbous";
    } else if (age < 16) { return "Full Moon";
    } else if (age < 21) { return "Waning Gibbous";
    } else if (age < 22) { return "Last Quarter";
    } else if (age < 28) { return "Waning Crescent";
    } else { return "New Moon"; }
}
double GMST_deg (double jd) {
    double T=(jd-J2000)/36525.0;
    double gmst_deg = 280.46061837 + 360.98564736629 * (jd - J2000) + 0.000387933*T*T - T*T*T/38710000.0;
    gmst_deg = fmod(gmst_deg,360);
    return gmst_deg;
}
//Corrects values to make them between 0 and 1
float constrain(float v){
	if(v<0){return v+1;}
	if(v>1){return v-1;}
	return v;
}
riseset_struct getRiseSet (float h0, double ra, double dec) {
    // all input parameters in radians
	    //const h0=-0.8333 //For Sun
	    //const h0=-0.5667 //For stars and planets
	    //const h0=0.125   //For Moon
    float cosH = (sin(h0*toRad) - sin(phi) * sin(dec)) / (cos(phi) * cos(dec));
    float H0 = acos(cosH)*toDeg;
    double gmst = GMST_deg(floor(jd)+0.5);
    float transit = (ra * toDeg - lw * toDeg - static_cast<float>(gmst)) / 360.0;
    float rise = transit - (H0 / 360.0);
    float set = transit + (H0 / 360.0);
    riseset_struct temp = {0.0, 0.0, 0.0};
    temp.transit = fmodf(constrain(transit)*24+utc_offset,24);
    temp.rise = fmodf(constrain(rise)*24+utc_offset,24);
    temp.set = fmodf(constrain(set)*24+utc_offset,24);
    return temp;
}

} // end namespace
