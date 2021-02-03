/*----------------------------------------------------------------------------
  Copyright (c) 2020-2020 SinoGNSS Technologies, Inc.
  All Rights Reserved.
  Designed by ZHANG Mingkai.
  Date in July. 2020.
----------------------------------------------------------------------------*/

#include "ppecom.h"

#include  "logger.h"
extern LOG_CONFIG log_config;
extern int week;

#define PPE_DUG 


/* ussages by EP */

/* calling instructions by EP */
/* 0. 'ppecom.h' should be included in PrecisePosEngine.cpp */

/* 1. the function of 'obsqumcom' is called by 'ePProvideGnssSvMeasurement' 
      in PrecisePosEngine.cpp */
/* 2. the function of 'ephqumcom' is called by 'ePProvideGnssEphemeris'
      in PrecisePosEngine.cpp */
/* 3. the function of 'epposout_gga' is called by 'ePProvidePosition'
      in PrecisePosEngine.cpp */
/* 4. all above functions called by PrecisePosEngine.cpp should be 
      processed from separate thread context */

/* defaults ppe_t */
ppe_t ppes={0};                 /* ppe struct */
smooth_t m_smooth={0};

static const int solq_nmea[]={  /* nmea quality flags to rtklib sol quality */
    /* nmea 0183 v.2.3 quality flags: */
    /*  0=invalid, 1=gps fix (sps), 2=dgps fix, 3=pps fix, 4=rtk, 5=float rtk */
    /*  6=estimated (dead reckoning), 7=manual input, 8=simulation */
    SOLQ_NONE ,SOLQ_SINGLE, SOLQ_DGPS, SOLQ_PPP , SOLQ_FIX,
    SOLQ_FLOAT,SOLQ_DR    , SOLQ_NONE, SOLQ_NONE, SOLQ_NONE
};
typedef struct {        /* time struct */
	time_t time;        /* time (s) expressed by standard time_t */
	double sec;         /* fraction of second under 1 s */
} gtime_t;
static const double gpst0[] = { 1980,1, 6,0,0,0 }; /* gps time reference */
static const double utc0[] = { 1970,1, 1,0,0,0 }; /* utc time reference */

/* time to calendar day/time ---------------------------------------------------
* convert gtime_t struct to calendar day/time
* args   : epUtcTimestampMs t   I   epUtcTimestampMs struct
*          double *ep           O   day/time {year,month,day,hour,min,sec}
* return : none
* notes  : proper in 1970-2037 or 1970-2099 (64bit time_t)
*-----------------------------------------------------------------------------*/
static void qtime2epoch(epUtcTimestampMs tms, double *ep)
{
    const int mday[]={ /* # of days in a month */
        31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,
        31,29,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31
    };
    int days,sec,mon,day;
    double t=tms*1E-3;

    /* leap year if year%4==0 in 1901-2099 */
    days=(int)(t/86400);
    sec=(int)(t-days*86400);
    for (day=days%1461,mon=0;mon<48;mon++) {
        if (day>=mday[mon]) day-=mday[mon]; else break;
    }
    ep[0]=1970+days/1461*4+mon/12; ep[1]=mon%12+1; ep[2]=day+1;
    ep[3]=sec/3600; ep[4]=sec%3600/60; ep[5]=sec%60;
}


/* convert calendar day/time to time -------------------------------------------
* convert calendar day/time to gtime_t struct
* args   : double *ep       I   day/time {year,month,day,hour,min,sec}
* return : gtime_t struct
* notes  : proper in 1970-2037 or 1970-2099 (64bit time_t)
*-----------------------------------------------------------------------------*/
extern gtime_t epoch2time(const double* ep)
{
	const int doy[] = { 1,32,60,91,121,152,182,213,244,274,305,335 };
	gtime_t time = { 0 };
	int days, sec, year = (int)ep[0], mon = (int)ep[1], day = (int)ep[2];

	if (year < 1970 || 2099 < year || mon < 1 || 12 < mon) return time;

	/* leap year if year%4==0 in 1901-2099 */
	days = (year - 1970) * 365 + (year - 1969) / 4 + doy[mon - 1] + day - 2 + (year % 4 == 0 && mon >= 3 ? 1 : 0);
	sec = (int)floor(ep[5]);
	time.time = (time_t)days * 86400 + (int)ep[3] * 3600 + (int)ep[4] * 60 + sec;
	time.sec = ep[5] - sec;
	return time;
}

/* time to calendar day/time ---------------------------------------------------
* convert gtime_t struct to calendar day/time
* args   : gtime_t t        I   gtime_t struct
*          double *ep       O   day/time {year,month,day,hour,min,sec}
* return : none
* notes  : proper in 1970-2037 or 1970-2099 (64bit time_t)
*-----------------------------------------------------------------------------*/
static void time2epoch(gtime_t t,double *ep)
{
	const int mday[]={ /* # of days in a month */
		31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,
		31,29,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31
	};
	int days,sec,mon,day;

	/* leap year if year%4==0 in 1901-2099 */
	days=(int)(t.time/86400);
	sec=(int)(t.time-(time_t)days*86400);
	for(day=days%1461,mon=0;mon<48;mon++) {
		if(day>=mday[mon]) day-=mday[mon]; else break;
	}
	ep[0]=1970+days/1461*4+mon/12; ep[1]=mon%12+1; ep[2]=day+1;
	ep[3]=sec/3600; ep[4]=sec%3600/60; ep[5]=sec%60+t.sec;
}

/* gps time to time ------------------------------------------------------------
* convert week and tow in gps time to gtime_t struct
* args   : int    week      I   week number in gps time
*          double sec       I   time of week in gps time (s)
* return : gtime_t struct
*-----------------------------------------------------------------------------*/
extern gtime_t gpst2time(int week, double sec)
{
	gtime_t t = epoch2time(gpst0);

	if (sec < -1E9 || 1E9 < sec) sec = 0.0;
	t.time += (time_t)86400 * 7 * week + (int)sec;
	t.sec = sec - (int)sec;
	return t;
}
/* time difference -------------------------------------------------------------
* difference between gtime_t structs
* args   : gtime_t t1,t2    I   gtime_t structs
* return : time difference (t1-t2) (s)
*-----------------------------------------------------------------------------*/
extern double timediff(gtime_t t1, gtime_t t2)
{
	return difftime(t1.time, t2.time) + t1.sec - t2.sec;
}
/* inner product ---------------------------------------------------------------
* inner product of vectors
* args   : double *a,*b     I   vector a,b (n x 1)
*          int    n         I   size of vector a,b
* return : a'*b
*-----------------------------------------------------------------------------*/
extern double dot(const double *a, const double *b, int n)
{
    double c=0.0;
    
    while (--n>=0) c+=a[n]*b[n];
    return c;
}

/* transform ecef to geodetic postion ------------------------------------------
* transform ecef position to geodetic position
* args   : double *r        I   ecef position {x,y,z} (m)
*          double *pos      O   geodetic position {lat,lon,h} (rad,m)
* return : none
* notes  : WGS84, ellipsoidal height
*-----------------------------------------------------------------------------*/
extern void ecef2pos(const double *r, double *pos)
{
    double e2=FE_WGS84*(2.0-FE_WGS84),r2=dot(r,r,2),z,zk,v=RE_WGS84,sinp;
    
    for (z=r[2],zk=0.0;fabs(z-zk)>=1E-4;) {
        zk=z;
        sinp=z/sqrt(r2+z*z);
        v=RE_WGS84/sqrt(1.0-e2*sinp*sinp);
        z=r[2]+v*e2*sinp;
    }
    pos[0]=r2>1E-12?atan(z/sqrt(r2)):(r[2]>0.0?PI/2.0:-PI/2.0);
    pos[1]=r2>1E-12?atan2(r[1],r[0]):0.0;
    pos[2]=sqrt(r2+z*z)-v;
}

/* ecef to local coordinate transfromation matrix ------------------------------
* compute ecef to local coordinate transfromation matrix
* args   : double *pos      I   geodetic position {lat,lon} (rad)
*          double *E        O   ecef to local coord transformation matrix (3x3)
* return : none
* notes  : matirix stored by column-major order (fortran convention)
*-----------------------------------------------------------------------------*/
static void xyz2enu(const double *pos,double *E)
{
	double sinp=sin(pos[0]),cosp=cos(pos[0]),sinl=sin(pos[1]),cosl=cos(pos[1]);

	E[0]=-sinl;      E[3]=cosl;       E[6]=0.0;
	E[1]=-sinp*cosl; E[4]=-sinp*sinl; E[7]=cosp;
	E[2]=cosp*cosl;  E[5]=cosp*sinl;  E[8]=sinp;
}

/* multiply matrix -----------------------------------------------------------*/
static void matmul(const char *tr,int n,int k,int m,double alpha,
	const double *A,const double *B,double beta,double *C)
{
	double d;
	int i,j,x,f=tr[0]=='N'?(tr[1]=='N'?1:2):(tr[1]=='N'?3:4);

	for(i=0;i<n;i++) for(j=0;j<k;j++) {
		d=0.0;
		switch(f) {
		case 1: for(x=0;x<m;x++) d+=A[i+x*n]*B[x+j*m]; break;
		case 2: for(x=0;x<m;x++) d+=A[i+x*n]*B[j+x*k]; break;
		case 3: for(x=0;x<m;x++) d+=A[x+i*m]*B[x+j*m]; break;
		case 4: for(x=0;x<m;x++) d+=A[x+i*m]*B[j+x*k]; break;
		}
		if(beta==0.0) C[i+j*n]=alpha*d; else C[i+j*n]=alpha*d+beta*C[i+j*n];
	}
}

/* transform ecef vector to local tangental coordinate -------------------------
* transform ecef vector to local tangental coordinate
* args   : double *pos      I   geodetic position {lat,lon} (rad)
*          double *r        I   vector in ecef coordinate {x,y,z}
*          double *e        O   vector in local tangental coordinate {e,n,u}
* return : none
*-----------------------------------------------------------------------------*/
static void ecef2enu(const double *pos,const double *r,double *e)
{
	double E[9];

	xyz2enu(pos,E);
	matmul("NN",3,1,3,1.0,E,r,0.0,e);
}

/* convert degree to deg-min-sec -----------------------------------------------
* convert degree to degree-minute-second
* args   : double deg       I   degree
*          double *dms      O   degree-minute-second {deg,min,sec}
*          int    ndec      I   number of decimals of second
* return : none
*-----------------------------------------------------------------------------*/
static void deg2dms(double deg, double *dms, int ndec)
{
    double sign=deg<0.0?-1.0:1.0,a=fabs(deg);
    double unit=pow(0.1,ndec);
    dms[0]=floor(a); a=(a-dms[0])*60.0;
    dms[1]=floor(a); a=(a-dms[1])*60.0;
    dms[2]=floor(a/unit+0.5)*unit;
    if (dms[2]>=60.0) {
        dms[2]=0.0;
        dms[1]+=1.0;
        if (dms[1]>=60.0) {
            dms[1]=0.0;
            dms[0]+=1.0;
        }
    }
    dms[0]*=sign;
}

static int rtkres2gga(const RTK_result *rtkres,int week, char *buff)
{
	double h,ep[6],xyz[3],pos[3]={0},dms1[3],dms2[3],dop=1.0;
	float age=0.0;
	int solq,ns;
	char *p=(char *)buff,*q,sum;

	xyz[0]=rtkres->dRov_Pos_x;
	xyz[1]=rtkres->dRov_Pos_y;
	xyz[2]=rtkres->dRov_Pos_z;
	ecef2pos(xyz,pos);
	deg2dms(fabs(pos[0]*R2D),dms1,7);
	deg2dms(fabs(pos[1]*R2D),dms2,7);
	h=pos[2];
	ns = rtkres->bSatNo;
	dop=rtkres->fPDOP;
	gtime_t gtime = gpst2time(week,rtkres->dRTK_Resolution_Time);
	gtime.time -= 18;
	time2epoch(gtime,ep);

	age=rtkres->dRTK_Diff_Time;/* age */

	solq=rtkres->bStatusFlag;
	if( solq == 14 ){
		solq = 4;
	}
	
	p+=sprintf(p,"$GPGGA,%02.0f%02.0f%06.3f,%02.0f%010.7f,%s,%03.0f%010.7f,%s,%d,%02d,%.1f,%.3f,M,%.3f,M,%.1f,",
		ep[3],ep[4],ep[5],dms1[0],dms1[1]+dms1[2]/60.0,pos[0]>=0?"N":"S",
		dms2[0],dms2[1]+dms2[2]/60.0,pos[1]>=0?"E":"W",solq,
		ns,dop,h,pos[2]-h,age);
	for(q=(char *)buff+1,sum=0;*q;q++) sum^=*q; /* check-sum */
	p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
	return p-(char *)buff;
}

BYTE ppeout_gga(const RTK_result *rtkres, int week, char *buffer)
{
      rtkres2gga(rtkres, week, buffer);
      return 0;
}

static int rtkres2rmc(const RTK_result *rtkres,int week, char *buff)
{
	static double dirp=0.0;
	gtime_t time;
	double ep[6],pos[3],enuv[3],dms1[3],dms2[3],xyz[3],vel,dir,amag=0.0,xyzvel[3];
	char *p=(char *)buff,*q,sum;
    const char *emag = "E";

	double KNOT2M =0.514444444;  /* m/knot */

	if(rtkres->bStatusFlag==0) {
		p+=sprintf(p,"$GPRMC,,,,,,,,,,,,");
		for(q=(char *)buff+1,sum=0;*q;q++) sum^=*q;
		p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
		return p-(char *)buff;
	}
	gtime_t gtime = gpst2time(week,rtkres->dRTK_Resolution_Time);
	gtime.time -= 18;
	time2epoch(gtime,ep);

	xyz[0]=rtkres->dRov_Pos_x;
	xyz[1]=rtkres->dRov_Pos_y;
	xyz[2]=rtkres->dRov_Pos_z;
	xyzvel[0]=rtkres->dRov_Vel_x;
	xyzvel[1]=rtkres->dRov_Vel_y;
	xyzvel[2]=rtkres->dRov_Vel_z;
	ecef2pos(xyz,pos);
	deg2dms(fabs(pos[0]*R2D),dms1,7);
	deg2dms(fabs(pos[1]*R2D),dms2,7);

	ecef2enu(pos,xyzvel,enuv);
	vel=sqrt(enuv[0]*enuv[0]+enuv[1]*enuv[1]+enuv[2]*enuv[2]);
	if(vel>=1.0) {
		dir=atan2(enuv[0],enuv[1])*R2D;
		if(dir<0.0) dir+=360.0;
		dirp=dir;
	}
	else {
		dir=dirp;
	}
	deg2dms(fabs(pos[0])*R2D,dms1,7);
	deg2dms(fabs(pos[1])*R2D,dms2,7);
	p+=sprintf(p,"$GPRMC,%02.0f%02.0f%05.2f,A,%02.0f%010.7f,%s,%03.0f%010.7f,%s,%4.2f,%4.2f,%02.0f%02.0f%02d,%.1f,%s,%s",
		ep[3],ep[4],ep[5],dms1[0],dms1[1]+dms1[2]/60.0,pos[0]>=0?"N":"S",
		dms2[0],dms2[1]+dms2[2]/60.0,pos[1]>=0?"E":"W",vel/KNOT2M,dir,
		ep[2],ep[1],(int)ep[0]%100,amag,emag,
		"D");
	for(q=(char *)buff+1,sum=0;*q;q++) sum^=*q; /* check-sum */
	p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
	return p-(char *)buff;
}

BYTE ppeout_rmc(const RTK_result *rtkres, int week, char *buffer)
{
      rtkres2rmc(rtkres, week, buffer);
      return 0;
}

/* specifies gnss system mask value ------------------------------------------*/
static epGnssConstellationTypeMask gnsssys(epGnssConstellationTypeMask gnssSystem)
{
    return gnssSystem&0xFFFF;
}
/* specifies gnss single mask value ------------------------------------------*/
static epGnssSignalTypeMask gnsssig(epGnssSignalTypeMask type)
{
    return type&0xFFFFFFFF;
}
/* specifies gnss system index -----------------------------------------------*/
static BYTE sysid(epGnssConstellationTypeMask gnssSystem)
{
    switch (gnsssys(gnssSystem)){
        case EP_GNSS_CONSTELLATION_UNKNOWN:   return IDNAN;
        case EP_GNSS_CONSTELLATION_GPS:       return IDGPS;
        case EP_GNSS_CONSTELLATION_GLONASS:   return IDGLO;
        case EP_GNSS_CONSTELLATION_SBAS:      return IDSBS;
        case EP_GNSS_CONSTELLATION_QZSS:      return IDQZS;
        case EP_GNSS_CONSTELLATION_BEIDOU:    return IDBDS;
        case EP_GNSS_CONSTELLATION_GALILEO:   return IDGAL;
        default: return IDNAN;
    }
    return IDNAN;
}
/* satellite id transport from qualcomm to sino ------------------------------*/
static uint16_t satidtrs(epGnssConstellationTypeMask gnssSystem, uint16_t sat)
{
    switch (gnsssys(gnssSystem)) {
        case EP_GNSS_CONSTELLATION_UNKNOWN:   return 0;
        case EP_GNSS_CONSTELLATION_GPS:
            if(sat<MINSATGPS||MAXSATGPS<sat)  return 0;
            sat+=MIN_SAT_GPS-MINSATGPS;
            if(MAX_SAT_GPS<sat) return 0;
            return sat;
        case EP_GNSS_CONSTELLATION_GLONASS:
            if(sat<MINSATGLO||MAXSATGLO<sat)  return 0;
            sat+=MIN_SAT_GLO-MINSATGLO;
            if(MAX_SAT_GLO<sat) return 0;
            return sat;
        case EP_GNSS_CONSTELLATION_GALILEO:
            if(sat<MINSATGAL||MAXSATGAL<sat)  return 0;
            sat+=MIN_SAT_GAL-MINSATGAL;
            if(MAX_SAT_GAL<sat) return 0;
            return sat;
        case EP_GNSS_CONSTELLATION_BEIDOU:
            if(sat<MINSATBDS||MAXSATBDS<sat)  return 0;
            sat+=MIN_SAT_BDS-MINSATBDS;
            if(MAX_SAT_BDS<sat) return 0;
            return sat;
        case EP_GNSS_CONSTELLATION_SBAS:      return 0;
        case EP_GNSS_CONSTELLATION_QZSS:      return 0;
        default: return 0;
    }
    return 0;
}
/* specifies gnss signal type ------------------------------------------------*/
static BYTE sigtype(epGnssSignalTypeMask type)
{
    switch (gnsssig(type)){
        case EP_GNSS_SIGNAL_UNKNOWN:          return SIGNA;
        case EP_GNSS_SIGNAL_GPS_L1CA:         return SIGL1;
        //case EP_GNSS_SIGNAL_GPS_L1C:          return SIGL1;
        case EP_GNSS_SIGNAL_GPS_L2C_L:        return SIGL2;
        case EP_GNSS_SIGNAL_GPS_L5_Q:         return SIGL3;
        case EP_GNSS_SIGNAL_GLONASS_G1_CA:    return SIGL1;
        case EP_GNSS_SIGNAL_GLONASS_G2_CA:    return SIGL2;
        case EP_GNSS_SIGNAL_GALILEO_E1_C:     return SIGL1;
        //2020-12-15,fix E5B-->L2,E5A-->L3
        case EP_GNSS_SIGNAL_GALILEO_E5A_Q:    return SIGL3;
        case EP_GNSS_SIGNAL_GALILIEO_E5B_Q:   return SIGL2;
        case EP_GNSS_SIGNAL_BEIDOU_B1_I:      return SIGL1;
        //case EP_GNSS_SIGNAL_BEIDOU_B1_C:      return SIGL1;
        case EP_GNSS_SIGNAL_BEIDOU_B2_I:      return SIGL2;
        //case EP_GNSS_SIGNAL_BEIDOU_B2A_I:     return SIGL2;
        case EP_GNSS_SIGNAL_QZSS_L1CA:        return SIGL1;
        //case EP_GNSS_SIGNAL_QZSS_L1S:         return SIGL1;
        case EP_GNSS_SIGNAL_QZSS_L2C_L:       return SIGL2;
        case EP_GNSS_SIGNAL_QZSS_L5_Q:        return SIGL3;
        case EP_GNSS_SIGNAL_SBAS_L1_CA:       return SIGL1;
        default: return SIGNA;
    }
    return SIGNA;
}
/* satellite number to satellite system ----------------------------------------
* convert satellite number to satellite system
* args   : int    sat       I   satellite number (1-MAXSAT)
*          int    *prn      IO  satellite prn/slot number (NULL: no output)
* return : satellite system (SYS_GPS,SYS_GLO,...)
*-----------------------------------------------------------------------------*/
static int satsys(int sat, int *prn)
{
    int sys=SYS_NONE;
    if (sat<=0||MAXSATS<sat) sat=0;
    else if (sat<=MAX_SAT_GPS) {
        sys=SYS_GPS; sat-=MIN_SAT_GPS-1;
    }
    else if (sat<=MAX_SAT_GLO) {
        sys=SYS_GLO; sat-=MIN_SAT_GLO-1;
    }
    else if (sat<=MAX_SAT_GAL) {
        sys=SYS_GAL; sat-=MIN_SAT_GAL-1;
    }
    else if (sat<=MAX_SAT_BDS) {
        sys=SYS_BDS; sat-=MIN_SAT_BDS-1; 
    }
    else sat=0;
    if (prn) *prn=sat;
    return sys;
}

/* satellite carrier wave length -----------------------------------------------
* get satellite carrier wave lengths
* args   : int    sat       I   satellite number
*          int    frq       I   frequency index (0:L1,1:L2,2:L5/3,...)
*          nav_t  *nav      I   navigation messages
* return : carrier wave length (m) (0.0: error)
*-----------------------------------------------------------------------------*/
static double satwavelen(int sat, int frq)
{
     const double freq_glo[]={FREQ1_GLO,FREQ2_GLO};
     const double dfrq_glo[]={DFRQ1_GLO,DFRQ2_GLO};

     int sys=satsys(sat,NULL);
                 
     if (sys==SYS_GLO) {
        int glo_offset[24] ={//temp
        +1,-4,+5,+6,+1,-4,+5,+6,-2,-7,+0,-1,
        -2,-7,+0,-1,+4,-3,+3,+2,+4,-3,+3,+2};
        if (0<=frq&&frq<=1) { /* L1,L2 */
            return CLIGHT/(freq_glo[frq]+dfrq_glo[frq]*glo_offset[sat-MIN_GLO_PRN]);            
        }
        else if (frq==2) { /* L3 */
            return CLIGHT/FREQ3_GLO;
        }
     }
     else  if (sys==SYS_BDS) {
        if      (frq==0) return CLIGHT/FREQ1_BDS; /* B1 */
        else if (frq==1) return CLIGHT/FREQ2_BDS; /* B2 */
        else if (frq==2) return CLIGHT/FREQ3_BDS; /* B3 */
     }
     else if (sys==SYS_GAL) {
        if      (frq==0) return CLIGHT/FREQ1; /* E1 */
        else if (frq==1) return CLIGHT/FREQ7; /* E5b */
        else if (frq==2) return CLIGHT/FREQ5; /* E5a */
        else if (frq==3) return CLIGHT/FREQ6; /* E6 */
        else if (frq==5) return CLIGHT/FREQ8; /* E5ab */
     }
     else if(sys==SYS_GPS){ /* GPS,QZS */
        if      (frq==0) return CLIGHT/FREQ1; /* L1 */
        else if (frq==1) return CLIGHT/FREQ2; /* L2 */
        else if (frq==2) return CLIGHT/FREQ5; /* L5 */
        else if (frq==3) return CLIGHT/FREQ6; /* L6/LEX */
        else if (frq==6) return CLIGHT/FREQ9; /* S */
     }
     else
     {
        return 0.0;
                                    
     }
     return 0.0;
}

/* specifies gnss system char name -------------------------------------------*/
extern void syschr(BYTE sysid, char *id)
{
    switch (sysid){
        case IDNAN: strcpy(id,"NON"); return;
        case IDGPS: strcpy(id,"GPS"); return;
        case IDGLO: strcpy(id,"GLO"); return;
        case IDSBS: strcpy(id,"SBS"); return;
        case IDQZS: strcpy(id,"QZS"); return;
        case IDBDS: strcpy(id,"BDS"); return;
        case IDGAL: strcpy(id,"GAL"); return;
        default: strcpy(id,"NON"); return;
    }
    strcpy(id,"NON");
}


/* eph value transport from qualcomm to sino (gps) ---------------------------*/
/*
static BYTE ephcomgps(const epGpsEphemerisResponse *ephqum, epGnssConstellationTypeMask sys,
                      ServerEphemeris *ephs)
{
    uint16_t i;
    const epGpsSatelliteEphemerisData *ephqua = ephqum->gpsEphemerisData;
    
    for(i=0;i<ephqum->numOfEphemeris;i++){
        
        ephs[i].blFlag      = 1;
        ephs[i].bHealth     = (unsigned char)ephqua[i].svHealthMask;
        ephs[i].ID          = (unsigned char)satidtrs(sys,ephqua[i].gnssSvId);
        ephs[i].bIonoValid  = 0;
        ephs[i].iodc        = (short)ephqua[i].iodc;
        ephs[i].accuracy    = (short)ephqua[i].urai;
        ephs[i].week        = (unsigned short)(ephqua[i].fullToeSec/604800);
        ephs[i].iode        = (int)ephqua[i].iode;
        ephs[i].toe         = (double)(ephqua[i].fullToeSec%604800);
        ephs[i].toc         = (double)(ephqua[i].fullTocSec%604800);
        ephs[i].af0         = ephqua[i].af0Sec;
        ephs[i].af1         = ephqua[i].af1;
        ephs[i].af2         = ephqua[i].af2;
        ephs[i].Ms0         = ephqua[i].m0Rad;
        ephs[i].deltan      = ephqua[i].deltaNRadPerSec;//����/�� ?
        ephs[i].es          = ephqua[i].eccentricity;
        ephs[i].roota       = ephqua[i].aSqrt;
        ephs[i].omega0      = ephqua[i].omegaORad;
        ephs[i].i0          = ephqua[i].inclinationAngleRad;
        ephs[i].ws          = ephqua[i].omegaRad;
        ephs[i].omegaot     = ephqua[i].omegaDotRadPerSec;
        ephs[i].itoet       = ephqua[i].iDotRadPerSec;
        ephs[i].Cuc         = ephqua[i].cUcRad;
        ephs[i].Cus         = ephqua[i].cUsRad;
        ephs[i].Crc         = ephqua[i].cRcMeter;
        ephs[i].Crs         = ephqua[i].cRsMeter;
        ephs[i].Cic         = ephqua[i].cIcRad;
        ephs[i].Cis         = ephqua[i].cIsRad;
        ephs[i].tgd         = ephqua[i].tgd*CLIGHT;
    }
    return 1;
}
*/
static BYTE ephcvtgps(const epGpsEphemerisResponse *ephqum, epGnssConstellationTypeMask sys,
                      ServerEphemeris *ephs)
{
    uint16_t i;
    const epGpsSatelliteEphemerisData *ephqua = ephqum->gpsEphemerisData;	
    
    int16_t index = -1;
    for(i=0;i<ephqum->numOfEphemeris;i++){
        //int prn = satidtrs(sys,ephqua[i].gnssSvId);
		if(ephqua[i].svHealthMask!=0) continue;
        ++index;
        ephs[index].blFlag      = 1;
        ephs[index].bHealth     = (unsigned char)ephqua[i].svHealthMask;
        ephs[index].ID          = (unsigned char)satidtrs(sys,ephqua[i].gnssSvId);
        ephs[index].bIonoValid  = 0;
        ephs[index].iodc        = (short)ephqua[i].iodc;
        ephs[index].accuracy    = (short)ephqua[i].urai;
        ephs[index].week        = (unsigned short)(ephqua[i].fullToeSec/604800);
        ephs[index].iode        = (int)ephqua[i].iode;
        ephs[index].toe         = (double)(ephqua[i].fullToeSec%604800);
        ephs[index].toc         = (double)(ephqua[i].fullTocSec%604800);
        ephs[index].af0         = ephqua[i].af0Sec;
        ephs[index].af1         = ephqua[i].af1;
        ephs[index].af2         = ephqua[i].af2;
        ephs[index].Ms0         = ephqua[i].m0Rad;
        ephs[index].deltan      = ephqua[i].deltaNRadPerSec;//����/�� ?
        ephs[index].es          = ephqua[i].eccentricity;
        ephs[index].roota       = ephqua[i].aSqrt;
        ephs[index].omega0      = ephqua[i].omegaORad;
        ephs[index].i0          = ephqua[i].inclinationAngleRad;
        ephs[index].ws          = ephqua[i].omegaRad;
        ephs[index].omegaot     = ephqua[i].omegaDotRadPerSec;
        ephs[index].itoet       = ephqua[i].iDotRadPerSec;
        ephs[index].Cuc         = ephqua[i].cUcRad;
        ephs[index].Cus         = ephqua[i].cUsRad;
        ephs[index].Crc         = ephqua[i].cRcMeter;
        ephs[index].Crs         = ephqua[i].cRsMeter;
        ephs[index].Cic         = ephqua[i].cIcRad;
        ephs[index].Cis         = ephqua[i].cIsRad;
//        ephs[index].tgd         = ephqua[i].tgd*CLIGHT;
        ephs[index].tgd         = ephqua[i].tgd;
    }
    if( index != -1){
        week = ephs[index].week;
    }
    //return ephqum->numOfEphemeris;
    return index+1;  
}
/* eph value transport from qualcomm to sino (glonass) -----------------------*/
/*
static BYTE ephcomglo(const epGlonassEphemerisResponse *ephqum, epGnssConstellationTypeMask sys,
                      ServerGloEphemeris *ephs)
{
    uint16_t i,N4z=0,N4r=0;

    const epGlonassSatelliteEphemerisData *ephqua = ephqum->gloEphemerisData;
    
    for(i=0;i<ephqum->numOfEphemeris;i++){

        ephs[i].bSlot       = (unsigned char)satidtrs(sys,ephqua[i].gnssSvId);
        ephs[i].bValid      = 1;
        ephs[i].health      = ephqua[i].bnHealth;
        N4z                 = ephqua[i].fullToeSec/126230400; // 1461*86400 
        N4r                 = ephqua[i].fullToeSec%126230400;
        ephs[i].N4          = (BYTE)(N4z+1);
        ephs[i].NT          = (short int)(N4r/86400+1);
        ephs[i].tb          = (int)(N4r%86400);// seconds  (int)(N4r%86400/900);//15 min
        //ephs[i].tog         = ;
        //ephs[i].week        = ;
        ephs[i].rfchnl      = (short int)(ephqua[i].freqNo-1);
        ephs[i].taun        = (double)ephqua[i].tauN;
        ephs[i].deltataun   = (double)ephqua[i].deltaTau;
        ephs[i].En          = (BYTE)ephqua[i].En;
        ephs[i].gamman      = (double)ephqua[i].gamma;
        ephs[i].xpos        = ephqua[i].x;
        ephs[i].ypos        = ephqua[i].y;
        ephs[i].zpos        = ephqua[i].z;
        ephs[i].xvel        = ephqua[i].vx;
        ephs[i].yvel        = ephqua[i].vy;
        ephs[i].zvel        = ephqua[i].vz;
        ephs[i].xacc        = ephqua[i].lsx;
        ephs[i].yacc        = ephqua[i].lsy;
        ephs[i].zacc        = ephqua[i].lsz;
    }
    return 1;
}
*/
static BYTE ephcvtglo(const epGlonassEphemerisResponse *ephqum, epGnssConstellationTypeMask sys,
                      ServerGloEphemeris *ephs)
{
    uint16_t i,N4z=0,N4r=0;

    const epGlonassSatelliteEphemerisData *ephqua = ephqum->gloEphemerisData;
    
    int16_t index = -1;
    for(i=0;i<ephqum->numOfEphemeris;i++){
		//int prn = satidtrs(sys,ephqua[i].gnssSvId)-MIN_SAT_GLO;
        ++index; 
        ephs[index].bSlot       = (unsigned char)satidtrs(sys,ephqua[i].gnssSvId);
        ephs[index].bValid      = 1;
        ephs[index].health      = ephqua[i].bnHealth;
        N4z                 = ephqua[i].fullToeSec/126230400; /* 1461*86400 */
        N4r                 = ephqua[i].fullToeSec%126230400;
        ephs[index].N4          = (BYTE)(N4z+1);
        ephs[index].NT          = (short int)(N4r/86400+1);
        ephs[index].tb          = (int)(N4r%86400);// seconds  (int)(N4r%86400/900);//15 min
        //ephs[i].tog         = ;
        //ephs[i].week        = ;
        ephs[index].rfchnl      = (short int)(ephqua[i].freqNo-1);
        ephs[index].taun        = (double)ephqua[i].tauN;
        ephs[index].deltataun   = (double)ephqua[i].deltaTau;
        ephs[index].En          = (BYTE)ephqua[i].En;
        ephs[index].gamman      = (double)ephqua[i].gamma;
        ephs[index].xpos        = ephqua[i].x;
        ephs[index].ypos        = ephqua[i].y;
        ephs[index].zpos        = ephqua[i].z;
        ephs[index].xvel        = ephqua[i].vx;
        ephs[index].yvel        = ephqua[i].vy;
        ephs[index].zvel        = ephqua[i].vz;
        ephs[index].xacc        = ephqua[i].lsx;
        ephs[index].yacc        = ephqua[i].lsy;
        ephs[index].zacc        = ephqua[i].lsz;
    }
    return index+1;  
}

/* eph value transport from qualcomm to sino (galileo) -----------------------*/
/*
static BYTE ephcomgal(const epGalileoEphemerisResponse *ephqum, epGnssConstellationTypeMask sys,
                      ServerEphemeris *ephs)
{
    uint16_t i;
    const epGalileoSatelliteEphemerisData *ephqua = ephqum->galEphemerisData;
    
    for(i=0;i<ephqum->numOfEphemeris;i++){

        ephs[i].blFlag      = 1;
        ephs[i].bHealth     = (unsigned char)ephqua[i].svHealth;
        ephs[i].ID          = (unsigned char)satidtrs(sys,ephqua[i].gnssSvId);
        ephs[i].bIonoValid  = 0;
        ephs[i].iodc        = (short)ephqua[i].iodNav;
        ephs[i].accuracy    = (short)ephqua[i].sisaIndex;
        ephs[i].week        = (unsigned short)(ephqua[i].fullToeSec/604800);
        ephs[i].iode        = (int)ephqua[i].iodNav;
        ephs[i].toe         = (double)(ephqua[i].fullToeSec%604800);
        ephs[i].toc         = (double)(ephqua[i].fullTocSec%604800);
        ephs[i].af0         = ephqua[i].af0Sec;
        ephs[i].af1         = ephqua[i].af1;
        ephs[i].af2         = ephqua[i].af2;
        ephs[i].Ms0         = ephqua[i].m0Rad;
        ephs[i].deltan      = ephqua[i].deltaNRadPerSec;//����/�� ?
        ephs[i].es          = ephqua[i].eccentricity;
        ephs[i].roota       = ephqua[i].aSqrt;
        ephs[i].omega0      = ephqua[i].omegaORad;
        ephs[i].i0          = ephqua[i].inclinationAngleRad;
        ephs[i].ws          = ephqua[i].omegaRad;
        ephs[i].omegaot     = ephqua[i].omegaDotRadPerSec;
        ephs[i].itoet       = ephqua[i].iDotRadPerSec;
        ephs[i].Cuc         = ephqua[i].cUcRad;
        ephs[i].Cus         = ephqua[i].cUsRad;
        ephs[i].Crc         = ephqua[i].cRcMeter;
        ephs[i].Crs         = ephqua[i].cRsMeter;
        ephs[i].Cic         = ephqua[i].cIcRad;
        ephs[i].Cis         = ephqua[i].cIsRad;
        ephs[i].tgd         = ephqua[i].bgdE1E5a*CLIGHT;
        ephs[i].tgd2        = ephqua[i].bgdE1E5b*CLIGHT;
    }
    return 1;
}
*/
static BYTE ephcvtgal(const epGalileoEphemerisResponse *ephqum, epGnssConstellationTypeMask sys,
                      ServerEphemeris *ephs)
{
    uint16_t i;
    const epGalileoSatelliteEphemerisData *ephqua = ephqum->galEphemerisData;
    	
    int16_t index = -1;
    for(i=0;i<ephqum->numOfEphemeris;i++){
		if(ephqua[i].svHealth!=0) continue;
        ++index; 
		//int prn = satidtrs(sys,ephqua[i].gnssSvId);		
        ephs[index].blFlag      = 1;
        ephs[index].bHealth     = (unsigned char)ephqua[i].svHealth;
        ephs[index].ID          = (unsigned char)satidtrs(sys,ephqua[i].gnssSvId);
        ephs[index].bIonoValid  = 0;
        ephs[index].iodc        = (short)ephqua[i].iodNav;
        ephs[index].accuracy    = (short)ephqua[i].sisaIndex;
        ephs[index].week        = (unsigned short)(ephqua[i].fullToeSec/604800)+1024;
        ephs[index].iode        = (int)ephqua[i].iodNav;
        ephs[index].toe         = (double)(ephqua[i].fullToeSec%604800);
        ephs[index].toc         = (double)(ephqua[i].fullTocSec%604800);
        ephs[index].af0         = ephqua[i].af0Sec;
        ephs[index].af1         = ephqua[i].af1;
        ephs[index].af2         = ephqua[i].af2;
        ephs[index].Ms0         = ephqua[i].m0Rad;
        ephs[index].deltan      = ephqua[i].deltaNRadPerSec;//����/�� ?
        ephs[index].es          = ephqua[i].eccentricity;
        ephs[index].roota       = ephqua[i].aSqrt;
        ephs[index].omega0      = ephqua[i].omegaORad;
        ephs[index].i0          = ephqua[i].inclinationAngleRad;
        ephs[index].ws          = ephqua[i].omegaRad;
        ephs[index].omegaot     = ephqua[i].omegaDotRadPerSec;
        ephs[index].itoet       = ephqua[i].iDotRadPerSec;
        ephs[index].Cuc         = ephqua[i].cUcRad;
        ephs[index].Cus         = ephqua[i].cUsRad;
        ephs[index].Crc         = ephqua[i].cRcMeter;
        ephs[index].Crs         = ephqua[i].cRsMeter;
        ephs[index].Cic         = ephqua[i].cIcRad;
        ephs[index].Cis         = ephqua[i].cIsRad;
//        ephs[index].tgd         = ephqua[i].bgdE1E5a*CLIGHT;
        ephs[index].tgd         = ephqua[i].bgdE1E5a;
//        ephs[index].tgd2        = ephqua[i].bgdE1E5b*CLIGHT;
        ephs[index].tgd2        = ephqua[i].bgdE1E5b;
    }
    return index+1;  
}
/* eph value transport from qualcomm to sino (bds) ---------------------------*/
/*
static BYTE ephcombds(const epBdsEphemerisResponse *ephqum, epGnssConstellationTypeMask sys, 
                      ServerEphemeris *ephs)
{
    uint16_t i;
    const epBDSSatelliteEphemerisData *ephqua = ephqum->bdsEphemerisData;
    
    for(i=0;i<ephqum->numOfEphemeris;i++){

        ephs[i].blFlag      = 1;
        ephs[i].bHealth     = (unsigned char)ephqua[i].svHealthMask;
        ephs[i].ID          = (unsigned char)satidtrs(sys,ephqua[i].gnssSvId);
        ephs[i].bIonoValid  = 0;
        ephs[i].iodc        = (short)ephqua[i].aodc;
        ephs[i].accuracy    = (short)ephqua[i].urai;
        ephs[i].week        = (unsigned short)(ephqua[i].fullToeSec/604800);
        ephs[i].iode        = (int)ephqua[i].aode;
        ephs[i].toe         = (double)(ephqua[i].fullToeSec%604800);
        ephs[i].toc         = (double)(ephqua[i].fullTocSec%604800);
        ephs[i].af0         = ephqua[i].af0Sec;
        ephs[i].af1         = ephqua[i].af1;
        ephs[i].af2         = ephqua[i].af2;
        ephs[i].Ms0         = ephqua[i].m0Rad;
        ephs[i].deltan      = ephqua[i].deltaNRadPerSec;//����/�� ?
        ephs[i].es          = ephqua[i].eccentricity;
        ephs[i].roota       = ephqua[i].aSqrt;
        ephs[i].omega0      = ephqua[i].omegaORad;
        ephs[i].i0          = ephqua[i].inclinationAngleRad;
        ephs[i].ws          = ephqua[i].omegaRad;
        ephs[i].omegaot     = ephqua[i].omegaDotRadPerSec;
        ephs[i].itoet       = ephqua[i].iDotRadPerSec;
        ephs[i].Cuc         = ephqua[i].cUcRad;
        ephs[i].Cus         = ephqua[i].cUsRad;
        ephs[i].Crc         = ephqua[i].cRcMeter;
        ephs[i].Crs         = ephqua[i].cRsMeter;
        ephs[i].Cic         = ephqua[i].cIcRad;
        ephs[i].Cis         = ephqua[i].cIsRad;
        ephs[i].tgd         = ephqua[i].tgd1Ns*CLIGHT*1E-9;
    }
    return 1;
}
*/
static BYTE ephcvtbds(const epBdsEphemerisResponse *ephqum, epGnssConstellationTypeMask sys,
                      ServerEphemeris *ephs)
{
    uint16_t i;
    const epBDSSatelliteEphemerisData *ephqua = ephqum->bdsEphemerisData;    
	
    int16_t index = -1;
    for(i=0;i<ephqum->numOfEphemeris;i++){
		if(ephqua[i].svHealthMask!=0) continue;
		//int prn = satidtrs(sys,ephqua[i].gnssSvId);
        ++index; 
        ephs[index].blFlag      = 1;
        ephs[index].bHealth     = (unsigned char)ephqua[i].svHealthMask;
        ephs[index].ID          = (unsigned char)satidtrs(sys,ephqua[i].gnssSvId);
        ephs[index].bIonoValid  = 0;
        ephs[index].iodc        = (short)ephqua[i].aodc;
        ephs[index].accuracy    = (short)ephqua[i].urai;
        ephs[index].week        = (unsigned short)(ephqua[i].fullToeSec/604800)+1356;
        ephs[index].iode        = (int)ephqua[i].aode;
        ephs[index].toe         = (double)(ephqua[i].fullToeSec%604800);
        ephs[index].toc         = (double)(ephqua[i].fullTocSec%604800);
        ephs[index].af0         = ephqua[i].af0Sec;
        ephs[index].af1         = ephqua[i].af1;
        ephs[index].af2         = ephqua[i].af2;
        ephs[index].Ms0         = ephqua[i].m0Rad;
        ephs[index].deltan      = ephqua[i].deltaNRadPerSec;//����/�� ?
        ephs[index].es          = ephqua[i].eccentricity;
        ephs[index].roota       = ephqua[i].aSqrt;
        ephs[index].omega0      = ephqua[i].omegaORad;
        ephs[index].i0          = ephqua[i].inclinationAngleRad;
        ephs[index].ws          = ephqua[i].omegaRad;
        ephs[index].omegaot     = ephqua[i].omegaDotRadPerSec;
        ephs[index].itoet       = ephqua[i].iDotRadPerSec;
        ephs[index].Cuc         = ephqua[i].cUcRad;
        ephs[index].Cus         = ephqua[i].cUsRad;
        ephs[index].Crc         = ephqua[i].cRcMeter;
        ephs[index].Crs         = ephqua[i].cRsMeter;
        ephs[index].Cic         = ephqua[i].cIcRad;
        ephs[index].Cis         = ephqua[i].cIsRad;
//        ephs[index].tgd         = ephqua[i].tgd1Ns*CLIGHT*1E-9;
        ephs[index].tgd         = ephqua[i].tgd1Ns*1E-9;
    }
    return index+1;  
}
/* ephmeries transport from qualcomm to sino ---------------------------------*/
/*
extern BYTE ephqumcom(const epGnssEphemerisReport *ephemerisResport, ephcom_t *ephs)
{
    epGnssConstellationTypeMask sys=gnsssys(ephemerisResport->gnssConstellation);

    switch (sys) {
        case EP_GNSS_CONSTELLATION_UNKNOWN: 
                return 0;
        case EP_GNSS_CONSTELLATION_GPS:
            {
                if(!ephcomgps(&ephemerisResport->ephInfo.gpsEphemeris,
                //          sys, &ephs->ephggb))
                          sys, &ephs->ephInfo.ephggb))
                    return 0;
                ephs->sys = EP_GNSS_CONSTELLATION_GPS;
                break;
            }
        case EP_GNSS_CONSTELLATION_GLONASS:
            {
                if(!ephcomglo(&ephemerisResport->ephInfo.glonassEphemeris, 
                          //sys, &ephs->ephglo))
                          sys, &ephs->ephInfo.ephglo))
                    return 0;
                ephs->sys = EP_GNSS_CONSTELLATION_GLONASS;
                break;
            }
        case EP_GNSS_CONSTELLATION_GALILEO:
            {
                if(!ephcomgal(&ephemerisResport->ephInfo.galileoEphemeris, 
                          //sys, &ephs->ephggb))
                          sys, &ephs->ephInfo.ephggb))
                    return 0;
                ephs->sys = EP_GNSS_CONSTELLATION_GALILEO;
                break;
            }
        case EP_GNSS_CONSTELLATION_BEIDOU:
            {
                if(!ephcombds(&ephemerisResport->ephInfo.bdsEphemeris, 
                          //sys, &ephs->ephggb))
                          sys, &ephs->ephInfo.ephggb))
                    return 0;
                ephs->sys = EP_GNSS_CONSTELLATION_BEIDOU;
                break;
            }
        default:return 0;
    }
    return 1;
}
*/
extern BYTE ppeephcvt(const epGnssEphemerisReport *ephemerisResport, ephcom_t *ephs)
{
    epGnssConstellationTypeMask sys=gnsssys(ephemerisResport->gnssConstellation);

    /* save ephemeries to sino */
    switch (sys) {
        case EP_GNSS_CONSTELLATION_UNKNOWN: 
                return -1;
        case EP_GNSS_CONSTELLATION_GPS:
            {
                ephs->sys = EP_GNSS_CONSTELLATION_GPS;
			    return ephcvtgps(&ephemerisResport->ephInfo.gpsEphemeris,
                          sys, ephs->ephInfo.ephggb);
            }
			break;
        case EP_GNSS_CONSTELLATION_GLONASS:
            {
                ephs->sys = EP_GNSS_CONSTELLATION_GLONASS;
                /*
			    return ephcvtglo(&ephemerisResport->ephInfo.glonassEphemeris,
                          sys, ephs->ephInfo.ephglo);
                */
                break;
            }
        case EP_GNSS_CONSTELLATION_GALILEO:
            {
                ephs->sys = EP_GNSS_CONSTELLATION_GALILEO;
			    return ephcvtgal(&ephemerisResport->ephInfo.galileoEphemeris,
                          sys, ephs->ephInfo.ephggb);
            }
        case EP_GNSS_CONSTELLATION_BEIDOU:
            {
                ephs->sys = EP_GNSS_CONSTELLATION_BEIDOU;
			    return ephcvtbds(&ephemerisResport->ephInfo.bdsEphemeris,
                          sys, ephs->ephInfo.ephggb);
            }
        default:return -1;
    }
    return -1;
}


/* obs wangeopch initial -----------------------------------------------------*/
static BYTE init_wobs(WangEpoch *obss){
    
    int i;

    /* obss reset */
    obss->dGPSOffset       =0.0;
    obss->dCompassOffset   =0.0;
    obss->dGlonassOffset   =0.0;
    obss->dGalileoOffset   =0.0;
    obss->wWeek            =0;
    obss->t                =0.0;

    for (i=0;i<MAXSAT;i++) {

        obss->pbMode1[i]=0;
        obss->pbMode2[i]=0;
        obss->pbMode5[i]=0;
        obss->pbID[i]=0;

        obss->pdR1[i]=0.0;
        obss->pdP2[i]=0.0;
        obss->pdR5[i]=0.0;
        obss->pdL1[i]=0.0;
        obss->pdL2[i]=0.0;
        obss->pdL5[i]=0.0;
        obss->pdD1[i]=0.0;
        obss->pdD2[i]=0.0;
        obss->pdD5[i]=0.0;

        obss->pbSN1[i]=0;
        obss->pbSN2[i]=0;
        obss->pbSN5[i]=0;
        obss->pdwSlip1[i]=0;
        obss->pdwSlip2[i]=0;
        obss->pdwSlip5[i]=0;
        obss->pwAzimuth[i]=0;
        obss->pbElevation[i]=0;
    }

    return 1;
}
/* obs phase positive bias ---------------------------------------------------*/
static BYTE phasecor(WangEpoch *obss, ppe_t *ppe){
    
    int i,sat;

    /* phase value positive bias */
    for (i=0;i<obss->n;i++) {

        sat=obss->pbID[i]-1;

        /* get phase bias */
        if(ppe->phaseb[sat][0]==0&&obss->pdR1[i]!=0&&obss->pdL1[i]!=0)
            ppe->phaseb[sat][0]=obss->pdR1[i]/ppe->lam[sat][0]-obss->pdL1[i];

        if(ppe->phaseb[sat][1]==0&&obss->pdP2[i]!=0&&obss->pdL2[i]!=0)
            ppe->phaseb[sat][1]=obss->pdP2[i]/ppe->lam[sat][1]-obss->pdL2[i];

        if(ppe->phaseb[sat][2]==0&&obss->pdR5[i]!=0&&obss->pdL5[i]!=0)
            ppe->phaseb[sat][2]=obss->pdR5[i]/ppe->lam[sat][2]-obss->pdL5[i];

        /* set phase */
        if(obss->pdL1[i]!=0)
            obss->pdL1[i]+=ppe->phaseb[sat][0];

        if(obss->pdL2[i]!=0)
            obss->pdL2[i]+=ppe->phaseb[sat][1];

        if(obss->pdL5[i]!=0)
            obss->pdL5[i]+=ppe->phaseb[sat][2];
    }

    return 1;
}
/* obs value transport from qualcomm to sino ---------------------------------*/
/**/
static BYTE obscom(const epGnssSVMeasurementStructType *svobs, BYTE sid, WangEpoch *obss)
{
    //if (!(svobs->healthStatus&1)) return 0;
    
    //svobs->lossOfLock;

    switch (sigtype(svobs->gnssSignal)){
        case SIGNA: return 0;
        case SIGL1:
            //if(sysid(svobs->gnssSystem)==IDGLO) 
            //    obss->wFreq    =svobs->gloFrequency;
            //else 
            //    obss->wFreq    =SIGL1;
            obss->pdR1[sid]    =svobs->pseudorange;
            obss->pdL1[sid]    =svobs->carrierPhase;
            obss->pdD1[sid]    =(double)svobs->dopplerShift;
            obss->pbSN1[sid]   =(BYTE)svobs->cN0dbHz;
            obss->pdwSlip1[sid]=(DWORD)svobs->lossOfLock.lossOfContinousLock;
            return 1;
        case SIGL2:
            return 1;
            //if(sysid(svobs->gnssSystem)==IDGLO) 
            //    obss->wFreq    =svobs->gloFrequency;
            //else 
            //    obss->wFreq    =SIGL2;
            obss->pdP2[sid]    =svobs->pseudorange;
            obss->pdL2[sid]    =svobs->carrierPhase;
            obss->pdD2[sid]    =(double)svobs->dopplerShift;
            obss->pbSN2[sid]   =(BYTE)svobs->cN0dbHz;
            obss->pdwSlip2[sid]=(DWORD)svobs->lossOfLock.lossOfContinousLock;
            return 1;
        case SIGL3:
            return 1;
            //if(sysid(svobs->gnssSystem)==IDGLO) 
            //    obss->wFreq    =svobs->gloFrequency;
            //else 
            //    obss->wFreq    =SIGL3;
            obss->pdR5[sid]    =svobs->pseudorange;
            obss->pdL5[sid]    =svobs->carrierPhase;
            obss->pdD5[sid]    =(double)svobs->dopplerShift;
            obss->pbSN5[sid]   =(BYTE)svobs->cN0dbHz;
            obss->pdwSlip5[sid]=(DWORD)svobs->lossOfLock.lossOfContinousLock;
            return 1;
        default:return 0;
    }
    return 0;
}
/**/

static BYTE obscvt(const epGnssSVMeasurementStructType *svobs, BYTE sid, WangEpoch *obss)
{
    //int M = 1000;
    int M = log_config._rtk_smooth_cnt;
    int PRN=obss->pbID[sid];
    switch (sigtype(svobs->gnssSignal)){
        case SIGNA: return 0;
        case SIGL1:
        {
            if (m_smooth.lli[obss->pbID[sid]]==svobs->cycleSlipCount&&fabs(m_smooth.dlast_L1[obss->pbID[sid]])>NZ)
            {
                double lastR=m_smooth.dsmooth_R[PRN];
                double lastL=m_smooth.dlast_L1[PRN];
                double lamada=satwavelen(PRN,0);                
                UINT32 smoothcount=m_smooth.smoothcount[PRN];
                if(smoothcount<M) M=smoothcount+1;              
                obss->pdR1[sid]    = 1./M*svobs->pseudorange+(M-1.)/M*(lastR+(svobs->carrierPhase-lastL)*lamada);             

                if (fabs(obss->pdR1[sid]-svobs->pseudorange)>20)
                {
                    obss->pdR1[sid]    =svobs->pseudorange;
                    m_smooth.smoothcount[PRN]=0;
                }
                /*
                if(M<300){                                                                                                                                            
                    LOG_INFO("PPE, PRN:%d, sid:%d, smoothcount:%d, M:%d, lastR:%.8f, lastL:%.8f, lamada:%.8f\n", PRN, sid, smoothcount, M, lastR, lastL, lamada);
                }
                */
            }
            else
            {
                //LOG_INFO("PPE, PRN:%d, sid:%d, pseudorange:%.4f\n", PRN, sid, svobs->pseudorange);
                obss->pdR1[sid]    = svobs->pseudorange;
                m_smooth.smoothcount[PRN]=0;
            }
            m_smooth.dsmooth_R[PRN] =obss->pdR1[sid];
            m_smooth.dlast_L1[PRN]  =svobs->carrierPhase;
            m_smooth.lli[PRN]       =svobs->cycleSlipCount;
            m_smooth.smoothcount[PRN]++;

            obss->pdL1[sid]    =svobs->carrierPhase;
            obss->pdD1[sid]    =(double)svobs->dopplerShift;
            obss->pbSN1[sid]   =(BYTE)svobs->cN0dbHz;
            obss->pdwSlip1[sid]=(DWORD)svobs->lossOfLock.lossOfContinousLock;

            //LOG_INFO("PPE, PRN:%d, sid:%d, pdR1:%.6f, pdL1:%.6f, pdD1:%.6f, pbSN1:%d, pdwSlip1:%d\n", PRN, sid, obss->pdR1[sid], obss->pdL1[sid], obss->pdD1[sid], obss->pbSN1[sid], obss->pdwSlip1[sid]);
            return 1;
        }
        case SIGL2:
        {
            obss->pdP2[sid]    =svobs->pseudorange;
            obss->pdL2[sid]    =svobs->carrierPhase;
            obss->pdD2[sid]    =(double)svobs->dopplerShift;
            obss->pbSN2[sid]   =(BYTE)svobs->cN0dbHz;
            obss->pdwSlip2[sid]=(DWORD)svobs->lossOfLock.lossOfContinousLock;
            return 1;
        }
        case SIGL3:
        {
            obss->pdR5[sid]    =svobs->pseudorange;
            obss->pdL5[sid]    =svobs->carrierPhase;
            obss->pdD5[sid]    =(double)svobs->dopplerShift;
            obss->pbSN5[sid]   =(BYTE)svobs->cN0dbHz;
            obss->pdwSlip5[sid]=(DWORD)svobs->lossOfLock.lossOfContinousLock;

            return 1;
        }
        default:return 0;
    }
    return 0;
}

/* obs satellite number search from array ------------------------------------*/
static void searchsat(BYTE *sats, const BYTE sat, int *j){

    int i;
    
    for(i=0;i<MAXSAT;i++) {
        if(sats[i]==sat) {
            *j=i;
            break;
        }
        else if(sats[i]==0){
            sats[i]=sat;
            *j=i;
            break;
        }
    }
}

/* obs phase&code offset by doppler ------------------------------------------*/
/**
static BYTE mesoffset(WangEpoch *obss){
   int i;
   double offset=0.0;

   // time offset 
   offset=obss->dGPSOffset;
   obss->dGPSOffset=0.0;
                       
   // phase&code mesaurement value cor 
   for (i=0;i<obss->n;i++) {
   // set phase 
        if(obss->pdL1[i]!=0&&obss->pdD1[i]!=0)
        obss->pdL1[i]-=(offset*CLIGHT-offset*obss->pdD1[i])/satwavelen(obss->pbID[i],0);

        if(obss->pdL2[i]!=0&&obss->pdD2[i]!=0)
            obss->pdL2[i]-=(offset*CLIGHT-offset*obss->pdD2[i])/satwavelen(obss->pbID[i],1);

        if(obss->pdL5[i]!=0&&obss->pdD5[i]!=0)
            obss->pdL5[i]-=(offset*CLIGHT-offset*obss->pdD5[i])/satwavelen(obss->pbID[i],2);

        // set code 
        if(obss->pdR1[i]!=0&&obss->pdD1[i]!=0)
            obss->pdR1[i]-=offset*CLIGHT-offset*obss->pdD1[i];

        if(obss->pdP2[i]!=0&&obss->pdD2[i]!=0)
            obss->pdP2[i]-=offset*CLIGHT-offset*obss->pdD2[i];

        if(obss->pdR5[i]!=0&&obss->pdD5[i]!=0)
            obss->pdR5[i]-=offset*CLIGHT-offset*obss->pdD5[i];
                                                                            
    }

    return 1;
}
*/
static BYTE mesoffset(WangEpoch *obss){
     
    int i,sys;
    double offset,GPSOffset,GlonassOffset,CompassOffset,GalileoOffset;

    /* time offset */
    GPSOffset=obss->dGPSOffset;
    GlonassOffset=obss->dGlonassOffset;
    CompassOffset=obss->dCompassOffset;
    GalileoOffset=obss->dGalileoOffset; 
    obss->dGPSOffset=0.0;
    obss->dCompassOffset=0.0;
    obss->dGlonassOffset=0.0;
    obss->dGalileoOffset=0.0;    

    /* phase&code mesaurement value cor */
    for (i=0;i<obss->n;i++) {
        sys=satsys(obss->pbID[i],NULL);
        switch(sys)
        {
            case SYS_GPS:
                offset=GPSOffset;
                break;
            case SYS_BDS:
                offset=CompassOffset;
                break;
            case SYS_GLO:
                offset=GlonassOffset;
                break;
            case SYS_GAL:
                offset=GalileoOffset;
                break;
            default:
                continue;//����
                //break;
        }

        /* set phase */
        if(obss->pdL1[i]!=0&&obss->pdD1[i]!=0)
            obss->pdL1[i]-=(offset*CLIGHT-offset*obss->pdD1[i])/satwavelen(obss->pbID[i],0);

        if(obss->pdL2[i]!=0&&obss->pdD2[i]!=0)
            obss->pdL2[i]-=(offset*CLIGHT-offset*obss->pdD2[i])/satwavelen(obss->pbID[i],1);

        if(obss->pdL5[i]!=0&&obss->pdD5[i]!=0)
            obss->pdL5[i]-=(offset*CLIGHT-offset*obss->pdD5[i])/satwavelen(obss->pbID[i],2);

        /* set code */
        if(obss->pdR1[i]!=0&&obss->pdD1[i]!=0)
            obss->pdR1[i]-=offset*CLIGHT-offset*obss->pdD1[i];

        if(obss->pdP2[i]!=0&&obss->pdD2[i]!=0)
            obss->pdP2[i]-=offset*CLIGHT-offset*obss->pdD2[i];

        if(obss->pdR5[i]!=0&&obss->pdD5[i]!=0)
            obss->pdR5[i]-=offset*CLIGHT-offset*obss->pdD5[i];
                                                                                                                    
    }

    return 1;
}

/* observation transport from qualcomm to sino -------------------------------*/
extern BYTE obsqumcom(const epGnssMeasurementReport *msrReport, ppe_t *ppe)
{
    int i,j;
    BYTE sats[MAXSAT]={0},sat=0;
    WangEpoch* obss=&ppe->wobs;

    /* reset obss */
    if(!init_wobs(obss)) return 0;

    /* obs of gnss cp */
    //obss->dGPSOffset       = (double)msrReport->clockDrift/CLIGHT;
    obss->dGPSOffset      = (double)msrReport->gpsSystemTime.systemClkTimeBias*1E-3;
    obss->dCompassOffset  = (double)msrReport->bdsSystemTime.systemClkTimeBias*1E-3;
    obss->dGlonassOffset  = (double)msrReport->gloSystemTime.gloClkTimeBias*1E-3;
    obss->dGalileoOffset  = (double)msrReport->galSystemTime.systemClkTimeBias*1E-3;
    obss->wWeek           = (int)msrReport->gpsSystemTime.systemWeek;
    obss->t               = (double)msrReport->gpsSystemTime.systemMsec*1.0E-3;
    
    for (i=0;i<msrReport->numMeas;i++) {
        
        if(!(sat=(BYTE)satidtrs(msrReport->svMeasurement[i].gnssSystem,
            msrReport->svMeasurement[i].gnssSvId))) continue;

        searchsat(sats,sat,&j);

        if(!obss->pbID[j]) obss->pbID[j]=sat;

        /* save each epoch to sino */
        if(log_config._rtk_psr_type){
            if(!obscvt(msrReport->svMeasurement+i, j, obss)) continue;
        }else{
            if(!obscom(msrReport->svMeasurement+i, j, obss)) continue;
        }
    }
    for(i=0;i<MAXSAT;i++) if(!sats[i]) break;
    obss->n=i;

    /* obs phase positive by pseudorange */
    //if(!phasecor(obss,ppe)) return 0;
 /* obs phase&code offset by doppler */
    if(!mesoffset(obss)) return 0;
        return i;
}

/* rtk pvt report validity ---------------------------------------------------*/
static BYTE pvtvalmask(epPVTReportValidity *valbit){
    /* validity bits */
    valbit->isUtcTimestampMsValid |= 1;
    valbit->isLatitudeValid |= 1;
    valbit->isLongitudeValid |= 1;
    valbit->isAltitudeWrtEllipsoidValid |= 1;
    valbit->isAltitudeWrtMeanSeaLevelValid |= 1;

    valbit->isNorthStdDeviationValid |= 1;
    valbit->isEastStdDeviationValid |= 1;
    valbit->isAltitudeStdDeviationValid |= 1;

    valbit->isHorizontalSpeedValid |= 1;
    valbit->isHorizontalSpeedUncValid |= 1;
    valbit->isNorthVelocityValid |= 1;

    valbit->isEastVelocityValid |= 1;
    valbit->isUpVelocityValid |= 1;
    valbit->isNorthVelocityStdDeviationValid |= 1;

    valbit->isEastVelocityStdDeviationValid |= 1;
    valbit->isUpVelocityStdDeviationValid |= 1;
    valbit->isHeadingDegValid |= 1;

    valbit->isHeadingUncDegValid |= 1;
    valbit->isMagneticDeviationDegValid |= 1;
    valbit->isHorizontalEllipticalUncSemiMajorAxisValid |= 1;

    valbit->isHorizontalEllipticalUncSemiMinorAxisValid |= 1;
    valbit->isHorizontalEllipticalUncAZValid |= 1;
    valbit->isClockbiasValid |= 1;

    valbit->isClockBiasStdDeviationValid |= 1;
    valbit->isClockDriftValid |= 1;
    valbit->isClockDriftStdDeviationValid |= 1;

    valbit->isPdopValid |= 1;
    valbit->isHdopValid |= 1;
    valbit->isVdopValid |= 1;
    valbit->isGdopValid |= 1;
    valbit->isTdopValid |= 1;

    valbit->isLongAccelValid |= 1;
    valbit->isLatAccelValid |= 1;
    valbit->isVertAccelValid |= 1;
    valbit->isYawRateValid |= 1;

    valbit->isPitchRadValid |= 1;
    valbit->isReferenceStationValid |= 1;
    valbit->isLongAccelUncValid |= 1;
    valbit->isLatAccelUncValid |= 1;

    valbit->isVertAccelUncValid |= 1;
    valbit->isYawRateUncValid |= 1;
    valbit->isPitchRadUncValid |= 1;
    valbit->isPpsLocaltimeStampValid |= 1;

    valbit->isHeadingRateDegValid |= 1;
    valbit->isFullCovMatrixValid |= 1;

    return 1;
}

/* ppe provide rtk resualt epPVTReport ---------------------------------------*/
/*
extern BYTE ppeout_rtk(const RTK_result *rtkres, epPVTReport *posrep)
{
    double xyz[3]={0.0},blh[3]={0.0},vxyz[3]={0.0},enu[3]={0.0};

    // time 
    posrep->epGnssSystemTime.gnssSystemTimeSrc=EP_GNSS_CONSTELLATION_GPS;
    posrep->epGnssSystemTime.u.gpsSystemTime.validityMask.isSystemWeekValid|=1;
    posrep->epGnssSystemTime.u.gpsSystemTime.validityMask.isSystemWeekMsecValid|=1;
    posrep->epGnssSystemTime.u.gpsSystemTime.systemWeek=1;
    posrep->epGnssSystemTime.u.gpsSystemTime.systemMsec=1;

    posrep->utcTimestampMs = (int)(rtkres->dRTK_Resolution_Time*1E3);
    posrep->validityMask.isUtcTimestampMsValid |= 1;

    //pvt val mask 
    pvtvalmask(&posrep->validityMask);

    // pos 
    switch (rtkres->bStatusFlag) {
        case 0: posrep->posFlags =EP_POSITION_FLAG_RTK_FIXED;  break;
        case 1: posrep->posFlags =EP_POSITION_FLAG_GNSS_USED;  break;
        case 2: posrep->posFlags =EP_POSITION_FLAG_DGNSS_CORR; break;
        case 4: posrep->posFlags =EP_POSITION_FLAG_RTK_FIXED;  break;
        case 5: posrep->posFlags =EP_POSITION_FLAG_RTK_CORR;   break;
        default: posrep->posFlags=0; break;
                                                                                                                                           
    }
    posrep->statusOfFix=EP_VALID_FIX;

    xyz[0]=rtkres->dRov_Pos_x;
    xyz[1]=rtkres->dRov_Pos_y;
    xyz[2]=rtkres->dRov_Pos_z;
    ecef2pos(xyz,blh);
    posrep->latitudeDeg  = blh[0]*R2D;
    posrep->longitudeDeg = blh[1]*R2D;
    posrep->altitudeWrtEllipsoid = (float)blh[2];
    posrep->validityMask.isLatitudeValid |=1;
    posrep->validityMask.isLongitudeValid|=1;
    posrep->validityMask.isAltitudeWrtEllipsoidValid|=1;

    // vel 
    vxyz[0]=rtkres->dRov_Vel_x;
    vxyz[1]=rtkres->dRov_Vel_y;
    vxyz[2]=rtkres->dRov_Vel_z;
    ecef2enu(blh,vxyz,enu);
    posrep->eastVelocity  =(float)enu[0];
    posrep->northVelocity =(float)enu[1];
    posrep->upVelocity    =(float)enu[2];
    
    // std 
    posrep->northStdDeviation=rtkres->fSigmaH;
    posrep->eastStdDeviation =rtkres->fSigmaH;
    posrep->altitudeStdDeviation=rtkres->fSigmaV;

    // dop 
    posrep->pDop=rtkres->fPDOP;
    posrep->hDop=(float)rtkres->dHDOP;
                                                                                                                                     
    // cov 
    posrep->upperTriangleFullCovMatrix[0]=rtkres->fQpx;
    posrep->upperTriangleFullCovMatrix[1]=rtkres->fQpxy;
    posrep->upperTriangleFullCovMatrix[2]=rtkres->fQpxz;
    posrep->upperTriangleFullCovMatrix[3]=rtkres->fQpy;
    posrep->upperTriangleFullCovMatrix[4]=rtkres->fQpyz;
    posrep->upperTriangleFullCovMatrix[5]=rtkres->fQpz;

    posrep->upperTriangleFullCovMatrix[6]=rtkres->fQvx;
    posrep->upperTriangleFullCovMatrix[7]=rtkres->fQvxy;
    posrep->upperTriangleFullCovMatrix[8]=rtkres->fQvxz;
    posrep->upperTriangleFullCovMatrix[9]=rtkres->fQvy;
    posrep->upperTriangleFullCovMatrix[10]=rtkres->fQvyz;
    posrep->upperTriangleFullCovMatrix[11]=rtkres->fQvz;
    // time diff 
    //posrep->clockbiasMeter=((float)rtkres->iDeltaT_GPS)*CLIGHT*1.0E-10;

    return 1;
}
*/
/**
   Following table provides mapping of Solution type and
   corresponding Position Flags.

   |--------------------------------------------------|
   |            |      EP_POSITION_FLAG_              |
   |            |-------------------------------------|
   |            |GNSS_ |DGNSS_ | PPP_  | RTK_ | RTK_  |
   |            |USED  |CORR   | CORR  | CORR | FIXED |
   |--------------------------------------------------|
   | Standalone |   1  |   0   |   0   |  0   |   0   |
   |--------------------------------------------------|
   | DGNSS      |   1  |   1   |   0   |  0   |   0   |
   |--------------------------------------------------|
   | PPP        |   1  |   1   |   1   |  0   |   0   |
   |--------------------------------------------------|
   | RTK-Float  |   1  |   1   |   0   |  1   |   0   |
   |--------------------------------------------------|
   | RTK-Fix    |   1  |   1   |   0   |  1   |   1   |
   |--------------------------------------------------|

*/
/* ppe provide rtk resualt epPVTReport ---------------------------------------*/

extern BYTE ppeout_rtk(const RTK_result *rtkres,int week,epPVTReport *posrep)
{
    double xyz[3]={0.0},blh[3]={0.0},vxyz[3]={0.0},venu[3]={0.0};
    /* time */
    posrep->epGnssSystemTime.gnssSystemTimeSrc=EP_GNSS_CONSTELLATION_GPS;
    posrep->epGnssSystemTime.u.gpsSystemTime.validityMask.isSystemWeekValid|=1;
    posrep->epGnssSystemTime.u.gpsSystemTime.validityMask.isSystemWeekMsecValid|=1;
    posrep->epGnssSystemTime.u.gpsSystemTime.systemWeek=week;
    posrep->epGnssSystemTime.u.gpsSystemTime.systemMsec= (uint32_t)(rtkres->dRTK_Resolution_Time*1E3);

	gtime_t gtime = gpst2time(week, rtkres->dRTK_Resolution_Time);
	
//    posrep->utcTimestampMs= (timediff(epoch2time(gpst0),epoch2time(utc0))+gtime.time+ gtime.sec)*1E3;
    posrep->utcTimestampMs= (gtime.time+ gtime.sec)*1E3;
    posrep->validityMask.isUtcTimestampMsValid |= 1;
	
    /* pos */
	if (rtkres->bStatusFlag>0)posrep->posFlags |= EP_POSITION_FLAG_GNSS_USED;
	if (rtkres->bStatusFlag>1)posrep->posFlags |= EP_POSITION_FLAG_DGNSS_CORR;
	if (rtkres->bStatusFlag>=4)posrep->posFlags |= EP_POSITION_FLAG_RTK_CORR;
	if (rtkres->bStatusFlag==4)posrep->posFlags |= EP_POSITION_FLAG_RTK_FIXED;

	if (rtkres->bStatusFlag > 0)posrep->statusOfFix=EP_VALID_FIX;
	else posrep->statusOfFix = EP_NO_FIX;
	
    posrep->pvtSource = EP_POSITION_SOURCE_PPE;

    xyz[0]=rtkres->dRov_Pos_x;
    xyz[1]=rtkres->dRov_Pos_y;
    xyz[2]=rtkres->dRov_Pos_z;
    ecef2pos(xyz,blh);
    posrep->latitudeDeg  = blh[0]*R2D;
    posrep->longitudeDeg = blh[1]*R2D;
    posrep->altitudeWrtEllipsoid = (float)blh[2];
    posrep->validityMask.isLatitudeValid |=1;
    posrep->validityMask.isLongitudeValid|=1;
    posrep->validityMask.isAltitudeWrtEllipsoidValid|=1;

    /* vel */
    vxyz[0]=rtkres->dRov_Vel_x;
    vxyz[1]=rtkres->dRov_Vel_y;
    vxyz[2]=rtkres->dRov_Vel_z;
    ecef2enu(blh,vxyz,venu);
    posrep->eastVelocity  =(float)venu[0];
    posrep->northVelocity =(float)venu[1];
    posrep->upVelocity    =(float)venu[2];
	posrep->validityMask.isEastVelocityValid |= 1;
	posrep->validityMask.isUpVelocityValid |= 1;
	posrep->validityMask.isNorthVelocityStdDeviationValid |= 1;
    posrep->horizontalSpeed = sqrt(venu[0]*venu[0] + venu[1]*venu[1]);
    posrep->horizontalSpeedUnc = sqrt(rtkres->fQvx*rtkres->fQvx + rtkres->fQvy*rtkres->fQvy + rtkres->fQvz*rtkres->fQvz);
    /* std */
    posrep->northStdDeviation=rtkres->fSigmaH; //TBD!
    posrep->eastStdDeviation =rtkres->fSigmaH; //TBD!
    posrep->altitudeStdDeviation=rtkres->fSigmaV;

    /* dop */
    posrep->pDop=rtkres->fPDOP;
    posrep->hDop=(float)rtkres->dHDOP;
	posrep->validityMask.isPdopValid |= 1;
	posrep->validityMask.isHdopValid |= 1;
    
    /* cov */
    posrep->upperTriangleFullCovMatrix[0]=rtkres->fQpx;
    posrep->upperTriangleFullCovMatrix[1]=rtkres->fQpxy;
    posrep->upperTriangleFullCovMatrix[2]=rtkres->fQpxz;
    posrep->upperTriangleFullCovMatrix[3]=rtkres->fQpy;
    posrep->upperTriangleFullCovMatrix[4]=rtkres->fQpyz;
    posrep->upperTriangleFullCovMatrix[5]=rtkres->fQpz;

    posrep->upperTriangleFullCovMatrix[6]=rtkres->fQvx;
    posrep->upperTriangleFullCovMatrix[7]=rtkres->fQvxy;
    posrep->upperTriangleFullCovMatrix[8]=rtkres->fQvxz;
    posrep->upperTriangleFullCovMatrix[9]=rtkres->fQvy;
    posrep->upperTriangleFullCovMatrix[10]=rtkres->fQvyz;
    posrep->upperTriangleFullCovMatrix[11]=rtkres->fQvz;
	posrep->validityMask.isFullCovMatrixValid |= 1;

    /* time diff */
    //posrep->clockbiasMeter=((float)rtkres->iDeltaT_GPS)*CLIGHT*1.0E-10;

    return 1;
}

/* ep provide position (nema 0183) in the form of nmea GGA sentence ----------*/
//extern BYTE epposout_gga(const epPVTReport *posrep, ppe_t *ppe)
extern BYTE epposout_gga(const epPVTReport *posrep, char *gpggabuf)
{
    double h,ep[6],pos[3]={0},dms1[3],dms2[3],dop=1.0;
    float age=0.0;
    int solq,ns;
    char *p=gpggabuf,*q,sum;

    if (posrep->statusOfFix==EP_NO_FIX) {
        p+=sprintf(p,"$GPGGA,,,,,,,,,,,,,,");
        for (q=(char *)gpggabuf+1,sum=0;*q;q++) sum^=*q;
        p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
        return p-(char *)gpggabuf;
    }
    for (solq=0;solq<8;solq++) if (solq_nmea[solq]==posrep->statusOfFix) break;
    if (solq>=8) solq=0;

    /* time & blh stamp value */
    if (posrep->validityMask.isUtcTimestampMsValid&&
        posrep->validityMask.isLatitudeValid && 
        posrep->validityMask.isLongitudeValid &&
        posrep->validityMask.isAltitudeWrtEllipsoidValid&&
        posrep->validityMask.isAltitudeWrtMeanSeaLevelValid) {

        qtime2epoch(posrep->utcTimestampMs, ep);/* !!! if add lep secs ? */

        pos[0]=posrep->latitudeDeg;
        pos[1]=posrep->longitudeDeg;
        pos[2]=(double)posrep->altitudeWrtEllipsoid;
        h=(double)posrep->altitudeWrtMeanSeaLevel;
    }
    else {
        return 1;
    }
    deg2dms(fabs(pos[0]),dms1,7);
    deg2dms(fabs(pos[1]),dms2,7);

    /* hdop */
    if (posrep->validityMask.isHdopValid){
        dop=(double)posrep->hDop;
    }
    /* ns */
    ns=(int)posrep->numOfMeasReceived;
   
    /* age */
    age=0.0;//TBD

    p+=sprintf(p,"$GPGGA,%02.0f%02.0f%06.3f,%02.0f%010.7f,%s,%03.0f%010.7f,%s,%d,%02d,%.1f,%.3f,M,%.3f,M,%.1f,",
               ep[3],ep[4],ep[5],dms1[0],dms1[1]+dms1[2]/60.0,pos[0]>=0?"N":"S",
               dms2[0],dms2[1]+dms2[2]/60.0,pos[1]>=0?"E":"W",solq,
               ns,dop,h,pos[2]-h,age);
    for (q=(char *)gpggabuf+1,sum=0;*q;q++) sum^=*q; /* check-sum */
    p+=sprintf(p,"*%02X%c%c",sum,0x0D,0x0A);
    return p-(char *)gpggabuf;
}


/* initial ppe struct --------------------------------------------------------*/
extern BYTE init_ppe(ppe_t *ppe)
{
    int i,j;

    /* initial wavelength of all satellite */
    for (i=0;i<MAXSATS;i++) for (j=0;j<NFREQ;j++) {
        ppe->lam[i][j]=satwavelen(i+1,j);
    }

    init_wobs(&ppe->wobs);

    memset(&m_smooth, 0x00, sizeof(smooth_t));
    return 1;
}


/* debug functions */
#ifdef PPE_DUG 

/* EP ------------------------------------------------------------------------*/
/* print ephemeries parematers of EP every system ----------------------------*/
//static void snprintfGPSEphemeris(char* buf,const int buflen, const epGpsEphemerisResponse &ephemResp)
//{
//    int pos = 0;
//    int writenLen = 0;
//    for(int i = 0;  i < ephemResp.numOfEphemeris; ++i){
//        const epGpsSatelliteEphemerisData& ephem = ephemResp.gpsEphemerisData[i];
//        writenLen = snprintf( buf, buflen, "prn| WN |Sv_accuracy|    iodt    |  iode | iodc  |  toc  |  toe  |      f2    |      f1    |       f0     |\n");
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%3d|%4d|%11d|%12.4f|%7d|%7d|%7d|%7d|%12.6f|%12.6f|%12.6f|\n", \
//           ephem.gnssSvId, ephem.fullToeSec/604800, ephem.urai, ephem.iDotRadPerSec, ephem.iode, ephem.iodc,\
//           ephem.fullToeSec%604800, ephem.fullTocSec%604800, ephem.af2, ephem.af1, ephem.af0Sec);
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "   delt_n   |     M0     |      e     |     i0     |     crs    |     cuc    |     cus    |    crc     |    cic     |    cis     |\n");
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|\n",\
//           ephem.deltaNRadPerSec, ephem.m0Rad, ephem.eccentricity, ephem.inclinationAngleRad, ephem.cRsMeter, ephem.cUcRad, \
//           ephem.cUsRad, ephem.cRcMeter, ephem.cIcRad, ephem.cIsRad);
//        pos += writenLen; 
//
//	    writenLen = snprintf(&buf[pos], buflen - pos, "   omega    |  Omegadot  |   sqrtA    |    tgd     | Sv_hlth |\n");//" flag | fit|\n");
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%12.6f|%12.6f|%12.6f|%12.6f|%9d|%6d|%4d|\n\n", \
//                         ephem.omegaRad, ephem.omegaDotRadPerSec, ephem.aSqrt, ephem.tgd*CLIGHT, ephem.svHealthMask);//, ephem.blFlag, ephem.fit);
//        pos += writenLen; 
//    }
//}
//
//static void snprintfGLOEphemeris(char* buf,const int buflen, const epGlonassEphemerisResponse &ephemResp)
//{
//    int pos = 0;
//    int writenLen = 0;
//    for(int i = 0;  i < ephemResp.numOfEphemeris; ++i){
//        const epGlonassSatelliteEphemerisData & ephem = ephemResp.gloEphemerisData[i];
//        uint16_t N4z=0,N4r=0;
//        N4z                 = ephem.fullToeSec/126230400; /* 1461*86400 */
//        N4r                 = ephem.fullToeSec%126230400;
//	    writenLen = snprintf( buf, buflen, "prn| fre_num | p1 |   Bn  |  p2  |  tb  |      xn    |     xn_1    |     xn_2   |\n");
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%3d|%9d|%4d|%7d|%7d|%7d|%12.6f|%12.6f|%12.6f|\n", \
//                         ephem.gnssSvId, ephem.freqNo-1,0 /*ephem.P1*/, \
//                         0/*ephem.Bn*/,0/* ephem.P2*/, N4r%86400/*ephem.tb*/, ephem.x, ephem.vx, ephem.lsx);
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "     yn     |    yn_1    |    yn_2    |     zn     |    zn_1    |    zn_2    |  p3  |    garmn   |  tao_n     |\n");
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%6d|%12.6f|%12.6f|\n",\
//                         ephem.y, ephem.vy, ephem.lsy, ephem.z, ephem.vz, ephem.lsz, 0/*ephem.P3*/,\
//                         ephem.gamma, ephem.tauN);
//        pos += writenLen; 
//	//writenLen = snprintf(&buf[pos], buflen - pos, "  delt_taon | En |M_p4|MF_t|MN_t|M_M|AOAD| NA |    tao_c   |MN4|  tao_gps   |M_l5|\n");
//	    writenLen = snprintf(&buf[pos], buflen - pos, "  delt_taon | En |MF_t|MN_t|M_M|MN4|Health|\n");
//        pos += writenLen; 
//	//writenLen = snprintf(&buf[pos], buflen - pos, "%12.5e|%4d|%4d|%4d|%4d|%3d|%4d|%4d|%12.5e|%3d|%12.5e|%3d|\n\n", 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%12.6f|%4d|%4d|%4d|%3d|%3d|%4d|\n", \
//                         ephem.deltaTau, ephem.En, 0/*ephem.FT*/, N4r/86400+1/*ephem.NT*/, 0/*ephem.M*/, \
//                         /*ephem.N4*/N4z+1, ephem.bnHealth);
//        pos += writenLen; 
//    }
//}
//
//static void snprintfBDEphemeris(char* buf,const int buflen, const epBdsEphemerisResponse &ephemResp)
//{
//    int pos = 0;
//    int writenLen = 0;
//    for(int i = 0;  i < ephemResp.numOfEphemeris; ++i){
//        //���õ�һ����,toe,toc 
//        const epBDSSatelliteEphemerisData& ephem = ephemResp.bdsEphemerisData[i];
//        writenLen = snprintf( buf, buflen, "prn| WN |Sv_accuracy|    iodt    |  iode | iodc  |  toc  |  toe  |      f2    |      f1    |       f0     |\n");
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%3d|%4d|%11d|%12.4f|%7d|%7d|%7d|%7d|%12.6f|%12.6f|%12.6f|\n", \
//           ephem.gnssSvId, ephem.fullToeSec/604800/*ephem.week*/, ephem.urai/*accuracy*/, ephem.iDotRadPerSec/*itoet*/,\
//           ephem.aode/*iode*/, ephem.aodc/*iodc*/,\
//           ephem.fullToeSec%604800/*toc*/, ephem.fullToeSec%604800/*toe*/, ephem.af2, ephem.af1, ephem.af0Sec);
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "   delt_n   |     M0     |      e     |     i0     |     crs    |     cuc    |     cus    |    crc     |    cic     |    cis     |\n");
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|\n",\
//           ephem.deltaNRadPerSec/*deltan*/, ephem.m0Rad/*Ms0*/, ephem.eccentricity/*es*/, ephem.inclinationAngleRad/*i0*/,\
//           ephem.cRsMeter/*Crs*/, ephem.cUcRad/*Cuc*/, ephem.cUsRad/*Cus*/, ephem.cRcMeter/*Crc*/, ephem.cIcRad/*Cic*/, ephem.cIsRad/*Cis*/);
//        pos += writenLen; 
//
//	    writenLen = snprintf(&buf[pos], buflen - pos, "   omega    |  Omegadot  |   sqrtA    |    tgd     | Sv_hlth |\n");//" flag | fit|\n");
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%12.6f|%12.6f|%12.6f|%12.6f|%9d|%6d|%4d|\n\n", \
//                         ephem.omegaRad/*ws*/, ephem.omegaDotRadPerSec/*omegaot*/, ephem.aSqrt/*roota*/,\
//                         ephem.tgd1Ns*CLIGHT*1E-9/*tgd*/, ephem.svHealthMask/*bHealth*/);//, ephem.blFlag, ephem.fit);
//        pos += writenLen; 
//    }
//}
//
//static void snprintfGALEphemeris(char* buf,const int buflen, const epGalileoEphemerisResponse &ephemResp)
//{
//    int pos = 0;
//    int writenLen = 0;
//    for(int i = 0;  i < ephemResp.numOfEphemeris; ++i){
//        //iode,iodc���õ�һ����,toe,toc also
//        const epGalileoSatelliteEphemerisData& ephem = ephemResp.galEphemerisData[i];
//        writenLen = snprintf( buf, buflen, "prn| WN |Sv_accuracy|    iodt    |  iode | iodc  |  toc  |  toe  |      f2    |      f1    |       f0     |\n");
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%3d|%4d|%11d|%12.4f|%7d|%7d|%7d|%7d|%12.6f|%12.6f|%12.6f|\n", \
//           ephem.gnssSvId, ephem.fullToeSec/604800/*ephem.week*/, ephem.sisaIndex/*accuracy*/, ephem.iDotRadPerSec/*itoet*/,\
//           ephem.iodNav/*iode*/, ephem.iodNav/*iodc*/,\
//           ephem.fullToeSec%604800/*toc*/, ephem.fullToeSec%604800/*toe*/, ephem.af2, ephem.af1, ephem.af0Sec);
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "   delt_n   |     M0     |      e     |     i0     |     crs    |     cuc    |     cus    |    crc     |    cic     |    cis     |\n");
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|%12.6f|\n",\
//           ephem.deltaNRadPerSec/*deltan*/, ephem.m0Rad/*Ms0*/, ephem.eccentricity/*es*/, ephem.inclinationAngleRad/*i0*/,\
//           ephem.cRsMeter/*Crs*/, ephem.cUcRad/*Cuc*/, ephem.cUsRad/*Cus*/, ephem.cRcMeter/*Crc*/, ephem.cIcRad/*Cic*/, ephem.cIsRad/*Cis*/);
//        pos += writenLen; 
//
//	    writenLen = snprintf(&buf[pos], buflen - pos, "   omega    |  Omegadot  |   sqrtA    |    tgd     | Sv_hlth |\n");//" flag | fit|\n");
//        pos += writenLen; 
//	    writenLen = snprintf(&buf[pos], buflen - pos, "%12.6f|%12.6f|%12.6f|%12.6f|%9d|%6d|%4d|\n\n", \
//                         ephem.omegaRad/*ws*/, ephem.omegaDotRadPerSec/*omegaot*/, ephem.aSqrt/*roota*/,\
//                         ephem.bgdE1E5a*CLIGHT/*tgd*/, ephem.svHealth/*bHealth*/);//, ephem.blFlag, ephem.fit);
//        pos += writenLen; 
//    }
//}
//extern uint32_t ephprintep(char* buf,const int buflen, const epGnssEphemerisReport *ephemerisResport)
//{
//    //int pos = 0;
//    //int writenLen = 0;
//    //if (ephemerisResport->gnssConstellation&EP_GNSS_CONSTELLATION_GPS){
//    //    writenLen =snprintf(&buf[pos], buflen - pos, "systemType  : %s\n", "QL GPS");
//    //    pos += writenLen; 
//    //    snprintfGPSEphemeris(&buf[pos], buflen - pos, ephemerisResport->ephInfo.gpsEphemeris);
//    //}else if (ephemerisResport->gnssConstellation&EP_GNSS_CONSTELLATION_GLONASS){
//    //    writenLen =snprintf(&buf[pos], buflen - pos, "systemType  : %s\n", "QL GLONASS");
//    //    pos += writenLen; 
//    //    snprintfGLOEphemeris(&buf[pos], buflen - pos, ephemerisResport->ephInfo.glonassEphemeris);
//    //}else if (ephemerisResport->gnssConstellation&EP_GNSS_CONSTELLATION_BEIDOU){
//    //    writenLen =snprintf(&buf[pos], buflen - pos, "systemType  : %s\n", "QL BEIDOU");
//    //    pos += writenLen; 
//    //    snprintfBDEphemeris(&buf[pos], buflen - pos, ephemerisResport->ephInfo.bdsEphemeris);
//    //}else if (ephemerisResport->gnssConstellation&EP_GNSS_CONSTELLATION_GALILEO){
//    //    writenLen =snprintf(&buf[pos], buflen - pos, "systemType  : %s\n", "QL GALILEO");
//    //    pos += writenLen; 
//    //    snprintfGALEphemeris(&buf[pos], buflen - pos, ephemerisResport->ephInfo.galileoEphemeris);
//    //}
//    return 1;
//}

/* print ephemeries parematers  of EP ----------------------------------------*/
extern uint32_t ephprintep(char *buffer, const epGnssEphemerisReport* ephrep) {

    char *p=buffer;
    const epGnssSystemTimeStructType *gpst=&ephrep->gpsSystemTime;
    
    /* print title */
    printf("\n");
    p+=sprintf(p,"gnssConstellation          : %d", ephrep->gnssConstellation);
    p+=sprintf(p,"gpsSystemTime->validityMask: %u", gpst->validityMask);

    /* print string to stdout */
    printf("%-*s",(int)(p-buffer),buffer);
    
    //TBD...

    return 1;
}

/* print observations parematers of EP ---------------------------------------*/
extern uint32_t obsprintep(char *buffer, BYTE sysidn, const epGnssMeasurementReport *obsrep) {

    int i;
    char *p=buffer,syschar[4];
    const epGnssSystemTimeStructType gnsst0={0};
    const epGnssGloTimeStructType glot0={0};

    const epGnssSystemTimeStructType *gnsst=&gnsst0;
    const epGnssGloTimeStructType *glot=&glot0;
    const epGnssSVMeasurementStructType *obss=&obsrep->svMeasurement[0];

    /* system time parematers */
    if(sysidn==IDGLO) glot=&obsrep->gloSystemTime;
    switch(sysidn){
        case IDGPS: gnsst=&obsrep->gpsSystemTime;
        case IDGAL: gnsst=&obsrep->galSystemTime;
        case IDBDS: gnsst=&obsrep->bdsSystemTime;
        default:break;
    }

    /* print all satellite list */
    p+=sprintf(p,"\n$START ALL PRINT QUALCOMM   \n");
    p+=sprintf(p,"satellite list qualcomm    :\n");
    for (i=0;i<obsrep->numMeas;i++) {
        //if(sysidn!=sysid(obss[i].gnssSystem)) continue;
        p+=sprintf(p," %4d", obss[i].gnssSvId);
    }
    p+=sprintf(p,"\n");


    /* print title and time */
    syschr(sysidn, syschar);
    p+=sprintf(p,"clockDrift                 : %f\n", obsrep->clockDrift);
    p+=sprintf(p,"clockDriftStdDeviation     : %f\n", obsrep->clockDriftStdDeviation);
    p+=sprintf(p,"validityMask               : %d\n", obsrep->validityMask);
    p+=sprintf(p,"leapSec                    : %d\n", obsrep->leapSec);
    p+=sprintf(p,"numMeas                    : %d\n", obsrep->numMeas);
    p+=sprintf(p,"gnssMeasReportType         : %d\n", obsrep->gnssMeasReportType);
    p+=sprintf(p,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
    p+=sprintf(p,"$SYS: %s\n", syschar);
    p+=sprintf(p,"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");

    p+=sprintf(p,"start time struct of current system -----|:\n");
    if(sysidn!=IDGLO){
        p+=sprintf(p,"SystemTime->validityMask         : %d\n", gnsst->validityMask);
        p+=sprintf(p,"SystemTime->systemWeek           : %d\n", gnsst->systemWeek);
        p+=sprintf(p,"SystemTime->systemMsec           : %d\n", gnsst->systemMsec);
        p+=sprintf(p,"SystemTime->systemClkTB          : %f\n", gnsst->systemClkTimeBias);
        p+=sprintf(p,"SystemTime->systemClkTBu         : %f\n", gnsst->systemClkTimeUncMs);
        p+=sprintf(p,"SystemTime->refFCount            : %d\n", gnsst->refFCount);
        p+=sprintf(p,"SystemTime->numClockResets       : %d\n", gnsst->numClockResets);
        p+=sprintf(p,"SystemTime->intraSystemTimeBias1 : %10.8f\n", gnsst->intraSystemTimeBias1);
        p+=sprintf(p,"SystemTime->intraSystemTimeBias1u: %10.8f\n", gnsst->intraSystemTimeBias1Unc);
    }
    else if(sysidn==IDGLO){
        p+=sprintf(p,"SystemTime->gloDays              : %d\n", glot->gloDays);
        p+=sprintf(p,"SystemTime->validityMask         : %d\n", glot->validityMask);
        p+=sprintf(p,"SystemTime->gloMsec              : %d\n", glot->gloMsec);
        p+=sprintf(p,"SystemTime->gloClkTimeBias       : %f\n", glot->gloClkTimeBias);
        p+=sprintf(p,"SystemTime->gloClkTimeUncMs      : %f\n", glot->gloClkTimeUncMs);
        p+=sprintf(p,"SystemTime->refFCount            : %d\n", glot->refFCount);
        p+=sprintf(p,"SystemTime->numClockResets       : %d\n", glot->numClockResets);
        p+=sprintf(p,"SystemTime->gloFourYear          : %d\n", glot->gloFourYear);
    }
    p+=sprintf(p,"end time struct of current system -------|:\n");


    /* print measurement each satellite (line space: 233)*/
    p+=sprintf(p,"------------------------------------------------\
----------------------------------------\
---------------------------------------------------------------\
----------------------------------------------------------------\
--------------------------------\n");
    p+=sprintf(p,"gnssSignal  gnssSystem  gnssSvId  gloFrequency  \
cN0dbHz        svMs  svSubMs  svTimeUncMs  \
dopplerShift  dopplerShiftUnc  validityMask  healthStatus  \
lossOfLock_lc  cycleSlipCount  pseudorange_uc     carrierPhase  \
carrierPhaseUnc      pseudorange\n");
    p+=sprintf(p,"------------------------------------------------\
-----------------------------------------\
---------------------------------------------------------------\
----------------------------------------------------------------\
--------------------------------\n");

    for (i=0;i<obsrep->numMeas;i++) {

        if(sysidn!=sysid(obss[i].gnssSystem)) continue;

        p+=sprintf(p,"%10d  %10d  %8d  %12d  \
%7.3f  %10d  %7.3f  %11.3f  \
%12.3f  %15.3f  %12d  %12d  \
%13d  %14d  %14.3f  %15.3f  \
%15.3f  %15.3f\n",
                   obss[i].gnssSignal,  obss[i].gnssSystem,     obss[i].gnssSvId,    obss[i].gloFrequency,
                   obss[i].cN0dbHz,     obss[i].svMs,           obss[i].svSubMs,     obss[i].svTimeUncMs,
                   obss[i].dopplerShift,obss[i].dopplerShiftUnc,obss[i].validityMask,obss[i].healthStatus,
                   obss[i].lossOfLock.lossOfContinousLock,  obss[i].cycleSlipCount, obss[i].pseudorange_uncertainty, obss[i].carrierPhase,
                   obss[i].carrierPhaseUnc,obss[i].pseudorange);
    }
    p+=sprintf(p,"\n$END ALL PRINT QUALCOMM   \n");

    return (int)(p-buffer);
}


/* PPE -----------------------------------------------------------------------*/
/* print observations parematers of PPE --------------------------------------*/
extern uint32_t obsprintppe(char* buffer, const WangEpoch* obss) {

    int i;
    char *p=buffer;

    /* print title and time */
    p+=sprintf(p,"\n$START ALL PRINT SINO   \n");
    p+=sprintf(p,"Obs Number                 : %d\n", obss->n);
    p+=sprintf(p,"wWeek                      : %d\n", obss->wWeek);
    p+=sprintf(p,"second                     : %f\n", obss->t);


    /* print measurement each satellite (line space: 233)*/
    p+=sprintf(p,"---------------------------------\
------------------------------------------\
------------------------------------------\
------------------------------------------\
---------------------\
----------------------------\n");
    p+=sprintf(p,"pbID  pbMode1  pbMode2  pbMode5  \
        pdR1          pdP2          pdR5  \
        pdL1          pdL2          pdL5  \
        pdD1          pdD2          pdD5  \
pbSN1  pbSN2  pbSN5  \
pdwSlip1  pdwSlip2  pdwSlip5\n");
    p+=sprintf(p,"---------------------------------\
------------------------------------------\
------------------------------------------\
------------------------------------------\
---------------------\
----------------------------\n");
    for (i=0;i<obss->n;i++){

        p+=sprintf(p, "%4d  %7d  %7d  %7d  \
%12.3f  %12.3f  %12.3f  \
%12.3f  %12.3f  %12.3f  \
%12.3f  %12.3f  %12.3f  \
%5d  %5d  %5d  \
%8d  %8d  %8d  \n",
            obss->pbID[i], obss->pbMode1[i], obss->pbMode2[i], obss->pbMode5[i], 
            obss->pdR1[i], obss->pdP2[i], obss->pdR5[i],
            obss->pdL1[i], obss->pdL2[i], obss->pdL5[i],
            obss->pdD1[i], obss->pdD2[i], obss->pdD5[i], 
            obss->pbSN1[i], obss->pbSN2[i], obss->pbSN5[i],
            obss->pdwSlip1[i], obss->pdwSlip2[i], obss->pdwSlip5[i]);
    }
    p+=sprintf(p,"\n$END ALL PRINT SINO \n");

    return (int)(p-buffer);
}

#endif
