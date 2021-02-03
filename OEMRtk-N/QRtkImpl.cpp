extern "C" {
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
}

#include <sstream>
#include <thread>
#include "QRtkImpl.h"
#include "PolarisInterface.h"
#include "RawFormat.h"
#include "logger.h"

#ifdef  _EC100Y_PLATFORM
extern "C"{ void * __dso_handle = 0 ; }
#endif

extern LOG_CONFIG log_config;
extern uint16_t    _constellationMask;
extern uint16_t    _solutionFormat;

using namespace std;

CQRtkImpl::CQRtkImpl(pRtkReport callback)
:m_pRtkReport(callback)
{
    //to link
#ifdef _LINK_RTK
	m_handle = CreateNewRtkEngine();
    if(!m_handle){
        LOG_ERROR("Failed to Create RTK Engine\n"); 
    }
#ifdef  _EC100Y_PLATFORM
//gcc arm metal,c++ class constructor failed
		RtkInitilization(m_handle, 1);
        ql_rtos_mutex_create(&m_mtxBase);
#endif
#endif
	int iDynamicModel = 0;
	int iMaskAngle = 10;
	int iLane = 1;

    memset(&m_rovEpoch, 0x00, EPOCH_SIZE);
    memset(&m_rovLastEpoch, 0x00, EPOCH_SIZE);
    memset(&m_baseEpoch, 0x00, EPOCH_SIZE);
    memset(&m_baseLastEpoch, 0x00, EPOCH_SIZE);

    memset(&m_rovCoord, 0x00, COORD_SIZE);
    memset(&m_baseCoord, 0x00, COORD_SIZE);

#ifndef _EC100Y_PLATFORM
    m_rovRtcmBuffer = new char[QRTK_RTCM_BUFFER_MAX_SIZE] ;
    //m_rovRtcmBuffer = (char*)malloc(QRTK_RTCM_BUFFER_MAX_SIZE) ;
    m_baseRtcmBuffer = new char[QRTK_RTCM_BUFFER_MAX_SIZE] ;
    //m_baseRtcmBuffer = (char*)malloc(QRTK_RTCM_BUFFER_MAX_SIZE) ;
    m_encodeBuffer = new char[QRTK_ENCODE_BUFFER_MAX_SIZE] ;
    //m_encodeBuffer = (char*)malloc(QRTK_ENCODE_BUFFER_MAX_SIZE) ;
#endif 
    memset(m_rovRtcmBuffer, 0x00, QRTK_RTCM_BUFFER_MAX_SIZE);
    memset(m_baseRtcmBuffer, 0x00, QRTK_RTCM_BUFFER_MAX_SIZE);
    memset(m_encodeBuffer, 0x00, QRTK_ENCODE_BUFFER_MAX_SIZE);
    //to link
#ifdef _LINK_RTK
    if(m_handle){
	    SetupRtk(m_handle, 1, iDynamicModel);
	    SetupRtk(m_handle, 3, iMaskAngle);
	    SetupRtk(m_handle, 6, iLane);
	    SetupRtk(m_handle, 11, 18);
    }
#endif
}

CQRtkImpl::~CQRtkImpl(){
#ifdef _LINK_RTK
	if(m_handle){
    //to link
		RemoveRtkEngine(m_handle);
	}
#endif
#ifdef  _EC100Y_PLATFORM
    ql_rtos_mutex_delete(m_mtxBase);
#else
    if(m_rovRtcmBuffer){
        delete[]  m_rovRtcmBuffer;
    }
    if(m_baseRtcmBuffer){
        delete[]  m_baseRtcmBuffer;
    }
    if(m_encodeBuffer){
        delete[] m_encodeBuffer;
    }
#endif 
}

void CQRtkImpl::CheckKeyForQRtk(){
	return;
}

int CQRtkImpl::PushRtcm(int source, const char *Buffer, int iDataSize)
{
    return ParseRtcm(source, Buffer, iDataSize);
}

int CQRtkImpl::PushEpoch(int source, const WangEpoch *pEpoch)
{
    if (SOURCE_TYPE_ROVER == source){
        memcpy(&m_rovEpoch, pEpoch, EPOCH_SIZE);
        //观测量GPS,BD等外部组织好        
        m_bHasRovAllEpoch = true;
        
        if ( PrepareToSolve(source) ){
            return Solve();
        }
    }else{
        memcpy(&m_baseEpoch, pEpoch, EPOCH_SIZE); 
        m_bHasBaseAllEpoch = true;
        
        if ( PrepareToSolve(source) ){
            return 0;
        }
    }
    return -1;
}
/*
void CQRtkImpl::SetCoordinateofGGA(double *xyz)
{
    m_rovCoord.ARP[0]=xyz[0];
    m_rovCoord.ARP[1]=xyz[1];
    m_rovCoord.ARP[2]=xyz[2];
    m_bHasRovCoord=true;
}
*/

/* transform geodetic to ecef position -----------------------------------------
 * transform geodetic position to ecef position
 * args   : double *pos      I   geodetic position {lat,lon,h} (rad,m)
 *          double *r        O   ecef position {x,y,z} (m)
 * return : none
 * notes  : WGS84, ellipsoidal height
 *-----------------------------------------------------------------------------*/
/* convert ddmm.mm in nmea format to deg -------------------------------------*/
static double dmm2deg(double dmm)
{
     return floor(dmm/100.0)+fmod(dmm,100.0)/60.0;
     
}

//void pos2ecef(const double *pos, double *r)
static void pos2ecef(const double *pos, double *r)
{
     double sinp=sin(pos[0]),cosp=cos(pos[0]),sinl=sin(pos[1]),cosl=cos(pos[1]);
     double e2=FE_WGS84*(2.0-FE_WGS84),v=RE_WGS84/sqrt(1.0-e2*sinp*sinp);
             
     r[0]=(v+pos[2])*cosp*cosl;
     r[1]=(v+pos[2])*cosp*sinl;
     r[2]=(v*(1.0-e2)+pos[2])*sinp;
}

/* decode nmea gxgga: fix information ----------------------------------------*/
//static int decode_nmeagga(char **val, int n, sol_t *sol)
//int decode_nmeagga(char **val, int n, double *xyz)
static int decode_nmeagga(char **val, int n, double *xyz)
{
    //gtime_t time;
    double tod=0.0,lat=0.0,lon=0.0,hdop=0.0,alt=0.0,msl=0.0;
    double pos[3]={0};
    //double xyz[3]={0};

    char ns='N',ew='E',ua=' ',um=' ';
    int i,solq=0,nrcv=0;
    for (i=0;i<n;i++) {
        switch (i) {
            case  0:
            {
                tod =atof(val[i]); 
                break; /* time in utc (hhmmss) */
            }
            case  1: 
            {
                if(strlen(val[i]) == 0) {
                    //LOG_DEBUG("latitude empty. \n");
                    return -1;
                }
                lat =atof(val[i]); 
                break;
            } /* latitude (ddmm.mmm) */
            case  2: 
            {
                ns  =*val[i];      
                break; /* N=north,S=south */
            }
            case  3: 
            {
                if(strlen(val[i]) == 0) {
                    //LOG_DEBUG("longtitude empty. \n");
                    return -1;
                }
                lon =atof(val[i]); 
                break;
            } /* longitude (dddmm.mmm) */
            case  4: ew  =*val[i];      break; /* E=east,W=west */
            case  5: solq=atoi(val[i]); break; /* fix quality */
            case  6: nrcv=atoi(val[i]); break; /* # of satellite tracked */
            case  7: hdop=atof(val[i]); break; /* hdop */
            case  8: 
            {
                if(strlen(val[i]) == 0) {
                    //LOG_DEBUG("altitude empty. \n");
                    return -1;
                }
                alt =atof(val[i]); 
                break;
            } /* altitude in msl */
            case  9: ua  =*val[i];      break; /* unit (M) */
            case 10: 
            {
                if(strlen(val[i]) == 0) {
                    //LOG_DEBUG("msl empty. \n");
                    return -1;
                }
                msl =atof(val[i]); 
                break;
            } /* height of geoid */
            case 11: um  =*val[i];      break; /* unit (M) */
        }
    }
    if ((ns!='N'&&ns!='S')||(ew!='E'&&ew!='W')) {
        //LOG_DEBUG("invalid nmea gpgga format\n");
        return -1;
    }

    pos[0]=(ns=='N'?1.0:-1.0)*dmm2deg(lat)*D2R;
    pos[1]=(ew=='E'?1.0:-1.0)*dmm2deg(lon)*D2R;
    pos[2]=alt+msl;
    pos2ecef(pos,xyz);

    return 0;
}

/* decode nmea ---------------------------------------------------------------*/
//static int decode_nmea(char *buff, sol_t *sol)
//int decode_nmea(const char *buff, double *xyz)
static int decode_nmea(const char *buff, double *xyz)
{
    char *q = NULL,*val[MAXFIELD]={0};
    int n=0;
             
    char *p = (char*)(buff);

    /* parse fields */
    /**/
    for (;*p&&n<MAXFIELD;p=q+1) {
        if ((q=strchr(p,','))||(q=strchr(p,'*'))) {
            val[n++]=p; *q='\0';
        }
        else break;
    }

    //LOG_DEBUG("RTK Engine Parsed GGA field:%d\n", n);
    if ((n -1) > 0 && !strcmp(val[0]+3,"GGA")) { /* $xxGGA */
        return decode_nmeagga(val+1,n-1, xyz);
    }
    return 0;
}

//#ifdef  _EC100Y_PLATFORM
typedef struct {        /* time struct */
	time_t time;        /* time (s) expressed by standard time_t */
	double sec;         /* fraction of second under 1 s */
} gtime_t;

static const double gpst0[] ={1980,1,6,0,0,0}; /* gps time reference */
static const double utc0[] ={1970,1,1,0,0,0}; /* utc time reference */
static gtime_t epoch2time(const double* ep)
{
	const int doy[] ={1,32,60,91,121,152,182,213,244,274,305,335};
	gtime_t time ={0};
	int days,sec,year = (int)ep[0],mon = (int)ep[1],day = (int)ep[2];

	if(year < 1970 || 2099 < year || mon < 1 || 12 < mon) return time;

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
static gtime_t gpst2time(int week,double sec)
{
	gtime_t t = epoch2time(gpst0);

	if(sec < -1E9 || 1E9 < sec) sec = 0.0;
	t.time += (time_t)86400 * 7 * week + (int)sec;
	t.sec = sec - (int)sec;
	return t;
}
/* time difference -------------------------------------------------------------
* difference between gtime_t structs
* args   : gtime_t t1,t2    I   gtime_t structs
* return : time difference (t1-t2) (s)
*-----------------------------------------------------------------------------*/
static double timediff(gtime_t t1,gtime_t t2)
{
	return difftime(t1.time,t2.time) + t1.sec - t2.sec;
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
/* inner product ---------------------------------------------------------------
* inner product of vectors
* args   : double *a,*b     I   vector a,b (n x 1)
*          int    n         I   size of vector a,b
* return : a'*b
*-----------------------------------------------------------------------------*/
static double dot(const double *a,const double *b,int n)
{
	double c=0.0;

	while(--n>=0) c+=a[n]*b[n];
	return c;
}

/* transform ecef to geodetic postion ------------------------------------------
* transform ecef position to geodetic position
* args   : double *r        I   ecef position {x,y,z} (m)
*          double *pos      O   geodetic position {lat,lon,h} (rad,m)
* return : none
* notes  : WGS84, ellipsoidal height
*-----------------------------------------------------------------------------*/
static void ecef2pos(const double *r,double *pos)
{
	double e2=FE_WGS84*(2.0-FE_WGS84),r2=dot(r,r,2),z,zk,v=RE_WGS84,sinp;

	for(z=r[2],zk=0.0;fabs(z-zk)>=1E-4;) {
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
static void deg2dms(double deg,double *dms,int ndec)
{
	double sign=deg<0.0?-1.0:1.0,a=fabs(deg);
	double unit=pow(0.1,ndec);
	dms[0]=floor(a); a=(a-dms[0])*60.0;
	dms[1]=floor(a); a=(a-dms[1])*60.0;
	dms[2]=floor(a/unit+0.5)*unit;
	if(dms[2]>=60.0) {
		dms[2]=0.0;
		dms[1]+=1.0;
		if(dms[1]>=60.0) {
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

static int rtkres2rmc(const RTK_result *rtkres,int week, char *buff)
{
	static double dirp=0.0;
	gtime_t time;
	double ep[6],pos[3],enuv[3],dms1[3],dms2[3],xyz[3],vel,dir,amag=0.0,xyzvel[3];
	char *p=(char *)buff,*q,sum,*emag="E";
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
//#endif

int CQRtkImpl::PushGGA(int source, const char* msg, int len)
{
    char buff[256]={0};
    memcpy(buff, msg, len);

    Coordinate          *pCoord = NULL;
	volatile bool		*pbHasCoord = NULL;

    if (SOURCE_TYPE_ROVER == source){
        pCoord = &m_rovCoord;
	    pbHasCoord = &m_bHasRovCoord;
    }else{
        pCoord = &m_baseCoord;
	    pbHasCoord = &m_bHasBaseCoord;
    }
 
    if(decode_nmea(buff, pCoord->ARP) < 0){
        return -1;
    }
    
    *pbHasCoord = true; 
    return 0;

}

int CQRtkImpl::ParseRtcm(int source, const char *Buffer, int iDataSize)
{
	WangEpoch			*pEpoch = nullptr;
    Coordinate          *pCoord = nullptr;
    char                *pRtcmBuffer = nullptr;
	CXRawFormat         *pRawFormat = nullptr;
	bool				*pbHasCoord = nullptr;
    bool                *pbHasAllEpoch = nullptr;

    int                 *pRemainSize = nullptr;  
    
    if (SOURCE_TYPE_ROVER == source){
    	pEpoch = &m_rovEpoch;
        pCoord = &m_rovCoord;
	    
        pbHasCoord = &m_bHasRovCoord;
        pbHasAllEpoch = &m_bHasRovAllEpoch;
        pRawFormat = &m_rovRawFormat;
        pRtcmBuffer = m_rovRtcmBuffer;
        pRemainSize = &m_rovRtcmRemainSize;
    }else{
    	pEpoch = &m_baseEpoch;
        pCoord = &m_baseCoord;
	    
        pbHasCoord = &m_bHasBaseCoord;
        pbHasAllEpoch = &m_bHasBaseAllEpoch;
        pRawFormat = &m_baseRawFormat; 
        pRtcmBuffer = m_baseRtcmBuffer;
        pRemainSize = &m_baseRtcmRemainSize;
    }
	
    int ret = PARSE_FAILURE;
    int iRtcmDataLength = *pRemainSize + iDataSize;
    memcpy(&pRtcmBuffer[*pRemainSize], Buffer, iDataSize);
	int pos = 0;
	while (true)
	{
		pRawFormat->reset();
		int type = 0;
	    for (int i = pos; i < iRtcmDataLength; i++){
			type = pRawFormat->wGetOneCharOfRTCM32(pRtcmBuffer[i]);
			if (type != 0){
                //处理解析完整的RTCM消息
                //LOG_DEBUG("RTK Engine, get RTCM:%d\n", type);
				switch (type){
					case 1019:
					{
                        if(!(_constellationMask&RTK_GNSS_CONSTELLATION_GPS)){
    					    ret = PARSE_SUCCESS;
						    break;
                        }

                        if(SOURCE_TYPE_BASE == source && log_config._rtk_eph_type){
    					    ret = PARSE_SUCCESS;
						    break;
                        }
						memset(&m_ephemGPS, 0x00, EPHEMERIS_SIZE);
						decode_type1019(NULL, pRawFormat->m_buffer, pRawFormat->m_wWholeLength, &m_ephemGPS);

						if (m_ephemGPS.ID < 1 || m_ephemGPS.ID > MAXPRN){
        					ret = PARSE_ERR_INVALID_SATID;
    					}

						m_ephemGPS.m_wIdleTime = 0;
						if (!m_ephemGPS.blFlag){									//无效星历
	    					m_ephemGPS.week = 0xFFFF;
	    					m_ephemGPS.toe = 0;
							ret = PARSE_ERR_INVALID_EPHEM;
                            break;
						}

                        SendEphemerisToQRtk(&m_ephemGPS);
    					ret = PARSE_SUCCESS;
						break;
					}
					case 1042:
					case 62:
					{
                        if(!(_constellationMask&RTK_GNSS_CONSTELLATION_BEIDOU)){
    					    ret = PARSE_SUCCESS;
						    break;
                        }

                        if(SOURCE_TYPE_BASE == source && log_config._rtk_eph_type){
    					    ret = PARSE_SUCCESS;
						    break;
                        }

						memset(&m_ephemGPS, 0x00, EPHEMERIS_SIZE);
						decode_type1042(NULL, pRawFormat->m_buffer, pRawFormat->m_wWholeLength, &m_ephemGPS);
						if (m_ephemGPS.ID < 1 || m_ephemGPS.ID > MAXPRN){
        					ret = PARSE_ERR_INVALID_SATID;
    					}

						m_ephemGPS.m_wIdleTime = 0;
						if (!m_ephemGPS.blFlag){									//无效星历
	    					m_ephemGPS.week = 0xFFFF;
	    					m_ephemGPS.toe = 0;
							ret = PARSE_ERR_INVALID_EPHEM;
                            break;
						}
                        SendEphemerisToQRtk(&m_ephemGPS);
 					    ret = PARSE_SUCCESS;
						break;
					}
					case 1020:	// GLONASS星历
					{
                        if(!(_constellationMask&RTK_GNSS_CONSTELLATION_GLONASS)){
    					    ret = PARSE_SUCCESS;
						    break;
                        }

	                    if(SOURCE_TYPE_BASE == source && log_config._rtk_eph_type){
    					    ret = PARSE_SUCCESS;
						    break;
                        }

					    memset(&m_ephemGLO, 0x00, GLO_EPHEMERIS_SIZE);

						decode_type1020(NULL, pRawFormat->m_buffer, pRawFormat->m_wWholeLength, &m_ephemGLO, m_iLeapSecond);

						int id, index;
						id = m_ephemGLO.bSlot;  // slot.
						index = id - MIN_GLO_PRN;
						if (id < 1 || id > MAXPRN || index < 0 || index >= MAX_SYS_PRN) {
							ret = PARSE_SUCCESS;
							break;
						}

						if (m_ephemGLO.bValid){
							m_ephemGLO.uLeapSecond = m_iLeapSecond;
						}
						else{
							m_ephemGLO.week = -1;
							m_ephemGLO.tog = 0;
							ret = PARSE_ERR_INVALID_EPHEM;
							break;
						}
	                    SendGloEphemToQRtk(&m_ephemGLO);
   					    ret = PARSE_SUCCESS;
						break;
					}
                    case 1046://Galio
                    {
                        if(!(_constellationMask&RTK_GNSS_CONSTELLATION_GALILEO)){
    					    ret = PARSE_SUCCESS;
						    break;
                        }

	                    if(SOURCE_TYPE_BASE == source && log_config._rtk_eph_type){
    					    ret = PARSE_SUCCESS;
						    break;
                        }

						memset(&m_ephemGPS, 0x00, EPHEMERIS_SIZE);
						decode_type1046(NULL, pRawFormat->m_buffer, pRawFormat->m_wWholeLength, &m_ephemGPS);

						if (m_ephemGPS.ID < 1 || m_ephemGPS.ID > MAXPRN){
        					ret = PARSE_ERR_INVALID_SATID;
    					}

						m_ephemGPS.m_wIdleTime = 0;
						if (!m_ephemGPS.blFlag){									//无效星历
	    					m_ephemGPS.week = 0xFFFF;
	    					m_ephemGPS.toe = 0;
							ret = PARSE_ERR_INVALID_EPHEM;
                            break;
						}

                        SendEphemerisToQRtk(&m_ephemGPS);
    					ret = PARSE_SUCCESS;
						break;

                    }
					case 1074://GPS MSM4
					case 1084://GLONASS MSM4
					case 1094://GAL MSM4
					case 1124://BeiDou MSM4
					{
                        //RtcmDecode.c  check if supports conresponding system  
						int res = -1;
						if (type == 1074){
							res = decode_msm4(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GPS, pEpoch, m_iLeapSecond);
    					}else if (type == 1084) {
							res = decode_msm4(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GLO, pEpoch, m_iLeapSecond);
    					}else if (type == 1094) {
							res = decode_msm4(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GAL, pEpoch, m_iLeapSecond);
    					}else if (type == 1124) {
							res = decode_msm4(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, BDS, pEpoch, m_iLeapSecond);
    					}/*else{
        					ret = PARSE_ERR_MISMATCH_TYPE;
                            break;
    					}*/

						if (res == -1) {
							ret =  PARSE_ERR_INVALID_EPHOCH;
							LOG_ERROR("RTK Engine, Fail to deocde_msm4 type:%hu, time:%.3f, satn:%d, buf pos:%d\r\n", type, pEpoch->t,pEpoch->n, pos); 
                            break;
							//t时间不一样，清空，有可能异常情况，
						}else if (res == -2){ //根据电文，重新创建新历元
							LOG_DEBUG("RTK Engine, Reset EPOCH type:%hu, time:%.3f, satn:%d, buf pos:%d\r\n", type, pEpoch->t,pEpoch->n, pos); 
							memset(pEpoch, 0x00, EPOCH_SIZE);
							if (type == 1074){
							    res = decode_msm4(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GPS, pEpoch, m_iLeapSecond);
    						}else if (type == 1084)	{
							    res = decode_msm4(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GLO, pEpoch, m_iLeapSecond);
    						}else if (type == 1094)	{
							    res = decode_msm4(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GAL, pEpoch, m_iLeapSecond);
    						}else if (type == 1124)	{
							    res = decode_msm4(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, BDS, pEpoch, m_iLeapSecond);
    						}
						}
						//结合了-2的情况
						if ( res == 0 ){ //本次历元结束，送RTK解算
                            *pbHasAllEpoch =  true;
							LOG_DEBUG("RTK Engine, Finish RTCM EPOCH type:%hu, time:%.3f, satn:%d, buf pos:%d\r\n", type, pEpoch->t,pEpoch->n, pos); 
							ret = PARSE_SUCCESS;
						}else{//本次历元尚未结束，请继续等待
							LOG_DEBUG("RTK Engine, RTCM EPOCH type:%hu, time:%.3f, satn:%d, buf pos:%d\r\n", type, pEpoch->t, pEpoch->n, pos); 
							ret = PARSE_EPOCH_SYNC_WAIT;
						}
						//更新时间、计算多普勒移动到RTK库，
						break;
					}
    				case 1077://GPS MSM7
    				case 1087://Glonass MSM7
					case 1097://Galio MSM7
					case 1127://BeiDou MSM7
					{
                        //RtcmDecode.c  check if supports conresponding system  
                        memset(&m_msmTemp,0x00,MSM_B_SIZE);
						int res = -1;
						if (type == 1077){
							//res = decode_msm7(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GPS, pEpoch, &m_msmTemp, m_iLeapSecond);
							res = decode_msm7(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GPS, pEpoch, m_iLeapSecond);
    					}else if (type == 1087) {
							res = decode_msm7(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GLO, pEpoch, m_iLeapSecond);
    					}else if (type == 1097) {
							res = decode_msm7(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GAL, pEpoch, m_iLeapSecond);
    					}else if (type == 1127) {
							res = decode_msm7(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, BDS, pEpoch, m_iLeapSecond);
    					}/*else{
        					ret = PARSE_ERR_MISMATCH_TYPE;
                            break;
    					}*/

						if (res == -1) {
							LOG_ERROR("RTK Engine, Fail to deocde_msm4 type:%hu, time:%.3f, satn:%d, buf pos:%d\r\n", type, pEpoch->t,pEpoch->n, pos); 
							ret =  PARSE_ERR_INVALID_EPHOCH;
                            break;
							//t时间不一样，清空，有可能异常情况，
						}else if (res == -2){ //根据电文，重新创建新历元
							LOG_DEBUG("RTK Engine, Reset EPOCH type:%hu, time:%.3f, satn:%d, buf pos:%d\r\n", type, pEpoch->t,pEpoch->n, pos); 
							memset(pEpoch, 0x00, EPOCH_SIZE);
							if (type == 1077){
							    res = decode_msm7(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GPS, pEpoch, m_iLeapSecond);
    					    }else if (type == 1087) {
							    res = decode_msm7(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GLO, pEpoch, m_iLeapSecond);
    						}else if (type == 1097)	{
							    res = decode_msm7(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, GAL, pEpoch, m_iLeapSecond);
    						}else if (type == 1127)	{
							    res = decode_msm7(NULL, pRawFormat->m_buffer, type, pRawFormat->m_wWholeLength, BDS, pEpoch, m_iLeapSecond);
    						}
						}
						//结合了-2的情况
						if ( res == 0 ){ //本次历元结束，送RTK解算
                            *pbHasAllEpoch =  true;
							LOG_DEBUG("RTK Engine, Finish RTCM EPOCH type:%hu, time:%.3f, satn:%d, buf pos:%d\r\n", type, pEpoch->t,pEpoch->n, pos); 
							ret = PARSE_SUCCESS;
						}else{//本次历元尚未结束，请继续等待
                            //*pbHasEpoch = true;
							LOG_DEBUG("RTK Engine, RTCM EPOCH type:%hu, time:%.3f, satn:%d, buf pos:%d\r\n", type, pEpoch->t, pEpoch->n, pos); 
						    ret = PARSE_EPOCH_SYNC_WAIT;
						}
						break;
					}
 				case 1005:
    				{
                        Coordinate   coord = {0};
						int res = decode_type1005(NULL, pRawFormat->m_buffer, pRawFormat->m_wWholeLength, &coord);
    					if( res == -1 ){
                            break;
						}
                        LOG_DEBUG("RTK Engine, RTCM Base Coordinate:x:%.4f,y:%.4f,z:%.4f\n", coord.ARP[0]*0.0001, coord.ARP[1]*0.0001, coord.ARP[2]*0.0001);
                        if(coord.ARP[0] != 0 && coord.ARP[1] != 0 && coord.ARP[2] != 0){
                            memcpy(pCoord, &coord, COORD_SIZE);
                            *pbHasCoord = true;
    					    ret = PARSE_SUCCESS;
                        }else{
                            LOG_ERROR("RTK Engine, Base Coordinate:x:%.4f,y:%.4f,z:%.4f\n", coord.ARP[0]*0.0001, coord.ARP[1]*0.0001, coord.ARP[2]*0.0001);
    						ret = PARSE_FAILURE;
                        }
        				break;
					}
					default:
						break;
				}

                //下次处理开始的位置
				pos = i+1;
		        
                if ((*pbHasAllEpoch) == true){
                    if ( PrepareToSolve(source) ){
                        if (SOURCE_TYPE_ROVER == source){
                            Solve();
                        }
                    }
                }
			}
		}

        //没有解析到完整的RTCM,退出
		if ( type == 0){
            if(pRawFormat->m_iState == 0){ //invalid rtcm raw,clear
                pos = iRtcmDataLength;
            }
			LOG_DEBUG("RTK Engine, Finish Parsing type:%hu,  pos:%d, and  break\r\n", type, pos); 
			break;
		}
	}

    //RTCM不完整，剩余的字节数
	int remainSize = iRtcmDataLength - pos;
	if (remainSize <= 0){
		*pRemainSize = 0;
        memset(pRtcmBuffer, 0x00, QRTK_RTCM_BUFFER_MAX_SIZE);
    }else{
		*pRemainSize = remainSize;
        memmove(pRtcmBuffer, &pRtcmBuffer[pos], remainSize);
        memset(&pRtcmBuffer[remainSize], 0x00, QRTK_RTCM_BUFFER_MAX_SIZE-remainSize);
    }
	return ret;
}

bool CQRtkImpl::PrepareToSolve(int source){
    if(source == SOURCE_TYPE_ROVER){ //移动站
        if (!m_bHasRovCoord){
            m_bHasRovAllEpoch = false;
            return false;
        }
        if(m_bHasRovAllEpoch){ 
            //计算高度角，方位角等
            GetSinglePosition(&m_rovEpoch, &m_rovLastEpoch, &m_rovCoord);
            LOG_DEBUG("RTK Engine, Calc Rover Azimuth Elevation.\n");
            
            //Epoch time比较
            if(m_bHasBaseLastAllEpoch){
                return true;
            }
        }
    }else{//基站一直返回false，移动站观测量推动计算
        if (!m_bHasBaseCoord){
            m_bHasBaseAllEpoch = false;
            return false;
        }

        if(m_bHasBaseAllEpoch){ 
			if (m_nWeekCurForRTCM > 1024){

				if (m_dSecondCurForRTCM - m_baseEpoch.t > 302400.){
					m_baseEpoch.wWeek = m_nWeekCurForRTCM + 1;
				}else if (m_baseEpoch.t - m_dSecondCurForRTCM > 302400.){
					m_baseEpoch.wWeek = m_nWeekCurForRTCM - 1;
				}
				else{
					m_baseEpoch.wWeek = m_nWeekCurForRTCM;

				}
			}

			m_baseEpoch.x = m_baseCoord.ARP[0];
			m_baseEpoch.y = m_baseCoord.ARP[1];
			m_baseEpoch.z = m_baseCoord.ARP[2];
	
            {
#ifdef  _EC100Y_PLATFORM
            ql_rtos_mutex_lock(m_mtxBase, QL_WAIT_FOREVER);
#else
            std::unique_lock<std::mutex> lk(m_mtxBase);
#endif
            memcpy(&m_baseLastEpoch, &m_baseEpoch, EPOCH_SIZE);
            m_bHasBaseLastAllEpoch = true;
#ifdef  _EC100Y_PLATFORM
            ql_rtos_mutex_unlock(m_mtxBase);
#endif
            }

            memset(&m_baseEpoch, 0x00, EPOCH_SIZE);
            m_bHasBaseAllEpoch = false;  //not set false, rtcm buffer overflow 
        }
        return false;
    }
    return false;
}

int CQRtkImpl::Solve(){
    RTK_result  rtkResult={0};
    
    WangEpoch baseEpoch;
    {
#ifdef  _EC100Y_PLATFORM
    ql_rtos_mutex_lock(m_mtxBase, QL_WAIT_FOREVER);
#else
    std::unique_lock<std::mutex> lk(m_mtxBase);
#endif

    memcpy(&baseEpoch, &m_baseLastEpoch, EPOCH_SIZE);

#ifdef  _EC100Y_PLATFORM
    ql_rtos_mutex_unlock(m_mtxBase);
#endif
 
    }

    LOG_DEBUG("RTK Engine, Solve...\n");

    if((log_config._dat_type & RAW_DATA_OUTPUT_DEBUG)){

        memcpy(&m_encodeBuffer[7], &baseEpoch, EPOCH_SIZE);
        m_encodeBuffer[0] = 0xAA;
        m_encodeBuffer[1] = 0x44;
        m_encodeBuffer[2] = 0x13;
        uint16_t epoch_size = EPOCH_SIZE;
        uint16_t epoch_type = 7;
        memcpy(&m_encodeBuffer[3], &epoch_size , sizeof(uint16_t));
        memcpy(&m_encodeBuffer[5], &epoch_type, sizeof(uint16_t));
        write_data(RAW_DATA_OUTPUT_DEBUG, m_encodeBuffer, epoch_size+7 );
        
        memcpy(&m_encodeBuffer[7], &m_rovEpoch, EPOCH_SIZE);
        epoch_type = 8;
        memcpy(&m_encodeBuffer[5], &epoch_type, sizeof(uint16_t));
        write_data(RAW_DATA_OUTPUT_DEBUG, m_encodeBuffer, epoch_size+7 );
    }

    SendEpochToQRtk(&baseEpoch, &m_rovEpoch, &rtkResult, nullptr);
    
    memcpy(&m_rovLastEpoch, &m_rovEpoch, EPOCH_SIZE);
    memset(&m_rovEpoch, 0x00, EPOCH_SIZE);
    m_bHasRovAllEpoch = false;
#ifdef _LINK_RTK_SOLVE
    //if(m_pRtkReport){
    //    m_pRtkReport(&rtkResult);
    //}
    //#ifdef  _EC100Y_PLATFORM
    if(_solutionFormat&RTK_GNSS_SOLUTION_NMEA){
		const RTK_result *pRtkResult = &rtkResult;

    	//snprintf(m_encodeBuffer, QRTK_ENCODE_BUFFER_MAX_SIZE, "Rtk Result,ResolutionTime:%.4f,Diff Time:%.4f,Status:%d,Sats:%d,x:%.4f,y:%.4f,z:%.4f\r\n", \
        //      pRtkResult->dRTK_Resolution_Time, pRtkResult->dRTK_Diff_Time, pRtkResult->bStatusFlag, pRtkResult->bSatNo,\
        //      pRtkResult->dRov_Pos_x, pRtkResult->dRov_Pos_y, pRtkResult->dRov_Pos_z);
		//write_data(RAW_DATA_OUTPUT_SINOINTER, (const char*)m_encodeBuffer, (int)strlen(m_encodeBuffer));
		//write_data(RAW_DATA_OUTPUT_SINODEBUG, (const char*)m_encodeBuffer, (int)strlen(m_encodeBuffer));
        //
        LOG_DEBUG("Rtk Result,ResolutionTime:%.4f,Diff Time:%.4f,Status:%d,Sats:%d,x:%.4f,y:%.4f,z:%.4f\r\n", \
              pRtkResult->dRTK_Resolution_Time, pRtkResult->dRTK_Diff_Time, pRtkResult->bStatusFlag, pRtkResult->bSatNo,\
              pRtkResult->dRov_Pos_x, pRtkResult->dRov_Pos_y, pRtkResult->dRov_Pos_z);

		double dest = sqrt(pow(pRtkResult->dRov_Pos_x - m_rovCoord.ARP[0], 2) + pow(pRtkResult->dRov_Pos_y - m_rovCoord.ARP[1], 2) +	pow(pRtkResult->dRov_Pos_z - m_rovCoord.ARP[2], 2));

		if ( pRtkResult->bStatusFlag >= 2 &&  dest < 30){
			int pos = rtkres2gga(pRtkResult, m_nWeekCurForRTCM, m_encodeBuffer);
			rtkres2rmc(pRtkResult, m_nWeekCurForRTCM, &m_encodeBuffer[pos]);
		}else{
			memset(m_encodeBuffer, 0x00, QRTK_ENCODE_BUFFER_MAX_SIZE);
		}
		m_pRtkReport((void*)m_encodeBuffer);
    }
//#else
    if(_solutionFormat&RTK_GNSS_SOLUTION_SINO){
        m_pRtkReport((void*)&rtkResult);
    }
//#endif


#endif
    return 0;
}

void CQRtkImpl::SendEphemerisToQRtk(ServerEphemeris* pEphemeris){
	unsigned char satID = static_cast<unsigned char> (pEphemeris->ID);
	if ( satID <= MAX_GPS_PRN ){
		m_nWeekCurForRTCM = pEphemeris->week;
		m_dSecondCurForRTCM = pEphemeris->toc;
	}
#ifdef _LINK_RTK
    //to link
    if(m_handle){
	    //bool ret = SendOneEphemerisToRtk(m_handle, pEphemeris);

#ifdef  _EC100Y_PLATFORM
		bool ret = SendOneEphemerisToRtk(m_handle, pEphemeris, NULL);
#else
		bool ret = SendOneEphemerisToRtk(m_handle, pEphemeris);
#endif

        LOG_DEBUG("RTK Engine, update Ephemeris sat id:%d,result:%d\n", (int)satID, (int)ret);
    }
#endif
    if((log_config._dat_type & RAW_DATA_OUTPUT_SINOINTER)){
        Encode_BD2DecodedEphInfor(m_nWeekCurForRTCM , (UINT32)(m_dSecondCurForRTCM * 1000),(BYTE*)m_encodeBuffer, pEphemeris,
                    SizeOfRawHeader + sizeof(ServerEphemeris)-sizeof(double)* 8 + 4);
        int data_length = SizeOfRawHeader + sizeof(ServerEphemeris)-sizeof(double)* 8 + 4;
        write_data(RAW_DATA_OUTPUT_SINOINTER, m_encodeBuffer, data_length);
    }
	return;
}

void CQRtkImpl::CreateMes723Info(ServerGloEphemeris *pGloEph, Mes723Infor *pMes723){
	pMes723->wSloto = pGloEph->bSlot;
	pMes723->wFreqo = pGloEph->rfchnl;
	pMes723->bSatType = pGloEph->M;
	pMes723->bRes1 = 0;
	pMes723->wEphWeek = pGloEph->week;
	pMes723->iEphTime = (UINT32)(pGloEph->tog * 1000);
	pMes723->iTimeOffset = iInt(pGloEph->uLeapSecond);
	pMes723->wDateNum = pGloEph->NT;
	pMes723->bRes2 = 0;
	pMes723->bRes3 = 0;
	pMes723->iIssue = pGloEph->tb / (15 * 60);
	pMes723->iHealth = pGloEph->health;
	pMes723->dPosX = pGloEph->xpos * 1000.;
	pMes723->dPosY = pGloEph->ypos * 1000.;
	pMes723->dPosZ = pGloEph->zpos * 1000.;
	pMes723->dVelX = pGloEph->xvel * 1000.;
	pMes723->dVelY = pGloEph->yvel * 1000.;
	pMes723->dVelZ = pGloEph->zvel * 1000.;
	pMes723->dAccX = pGloEph->xacc * 1000.;
	pMes723->dAccY = pGloEph->yacc * 1000.;
	pMes723->dAccZ = pGloEph->zacc * 1000.;
	pMes723->dTauN = pGloEph->taun;
	pMes723->dDeltaTauN = pGloEph->deltataun;
	pMes723->dGamma = pGloEph->gamman;
	pMes723->iTk = pGloEph->hh * 3600 + pGloEph->mm * 60 + pGloEph->ss;
	pMes723->iP = pGloEph->P;
	pMes723->iFt = pGloEph->FT;
	pMes723->iAge = pGloEph->En;
	pMes723->iFlag = (pGloEph->P1 & 0x03);
	pMes723->iFlag = ((pGloEph->P2 & 0x01) << 2);
	pMes723->iFlag = ((pGloEph->P3 & 0x01) << 3);
	pMes723->iFlag = ((pGloEph->P4 & 0x01) << 4);
	pMes723->iValid = 1;
}

void CQRtkImpl::SendGloEphemToQRtk(ServerGloEphemeris *pEphem){
    int iFreqOffset;
    double  dFreqL1, dFreqL2;
    if(pEphem->bSlot >= MIN_GLO_PRN && pEphem->bSlot <= MAX_GLO_PRN && pEphem->rfchnl <= 20){
        iFreqOffset = pEphem->rfchnl - 7;
        dFreqL1 =  GLO_L1_BASE_FRE + iFreqOffset * GLO_L1_SUB_BAND; 
        dFreqL2 =  GLO_L2_BASE_FRE + iFreqOffset * GLO_L2_SUB_BAND;
        m_theX.SetFreq(pEphem->bSlot, dFreqL1, dFreqL2, 1.E9);
    }
	Mes723Infor mes723 = {0};
    CreateMes723Info(pEphem, &mes723);
#ifdef _LINK_RTK
    //to link
    if(m_handle){
	    SendOneGloEphemToRtk(m_handle, &mes723);
    }
#endif
    if((log_config._dat_type & RAW_DATA_OUTPUT_SINOINTER)){
        EncodeOneDecEphGlo(m_nWeekCurForRTCM , (UINT32)(m_dSecondCurForRTCM * 1000), &mes723, (BYTE*)m_encodeBuffer,
                                                    SizeOfRawHeader + sizeof(Mes723Infor)+4);
        int data_length = SizeOfRawHeader + sizeof(Mes723Infor) + 4;
        write_data(RAW_DATA_OUTPUT_SINOINTER, m_encodeBuffer, data_length);
    }

}

int CQRtkImpl::GetSatPos(const int nSat, const double t, double *xyz, SatAmAltitu *pSatAm){
    int ret = 0;
#ifdef _LINK_RTK
    //to link
    if(m_handle){
        //ret = iGetSatPos(m_handle, nSat, t, xyz[0], xyz[1], xyz[2], pSatAm->pbID, pSatAm->pdDistance, pSatAm->pdAzimuth, pSatAm->pdElevation);
#ifdef  _EC100Y_PLATFORM
        ret = iGetSatPos(m_handle, nSat, t, xyz[0], xyz[1], xyz[2], pSatAm->pbID, pSatAm->pdDistance, pSatAm->pdAzimuth, pSatAm->pdElevation, pSatAm->pdSatClock);
#else
        ret = iGetSatPos(m_handle, nSat, t, xyz[0], xyz[1], xyz[2], pSatAm->pbID, pSatAm->pdDistance, pSatAm->pdAzimuth, pSatAm->pdElevation);
#endif

    }
#endif    
	return ret;
}

void CQRtkImpl::CaculateEpochDoppler(WangEpoch* pEpochA, WangEpoch* pEpochB)
{
	if (pEpochA->n == 0 || pEpochB->n == 0) return;

	int idA, idB;
	for (int i = 0; i < pEpochA->n; i++)
	{
		idA = pEpochA->pbID[i];
		for (int j = 0; j < pEpochB->n; j++)
		{
			idB = pEpochB->pbID[j];
			if (idA == idB)
			{
				int id = idA;

				pEpochA->pdD1[i] = -(pEpochA->pdL1[i] - pEpochB->pdL1[j]);
				pEpochA->pdD2[i] = -(pEpochA->pdL2[i] - pEpochB->pdL2[j]);
				pEpochA->pdD5[i] = -(pEpochA->pdL5[i] - pEpochB->pdL5[j]);

				break;
			}
		}
	}
}

#ifdef  _EC100Y_PLATFORM
/* obs phase&code offset by doppler ------------------------------------------*/
static BYTE mesoffset(WangEpoch *obss){
    int i,id;
    double offset=0.0,dWaveLength;

    /* time offset */
    offset=obss->dGPSOffset;
    obss->dGPSOffset=0.0;

    /* phase&code mesaurement value cor */
    for(i=0;i<obss->n;i++) {
        id= obss->pbID[i];
        int glo_offset[24] ={
            +1,-4,+5,+6,+1,-4,+5,+6,-2,-7,+0,-1,
            -2,-7,+0,-1,+4,-3,+3,+2,+4,-3,+3,+2
        };
        dWaveLength = (LIGHT / GPS_F1);
        /*L1 phase lock loop.*/
        if(MIN_GPS_PRN <= id && id <= MAX_GPS_PRN)
            dWaveLength = (LIGHT / GPS_F1);
        else if(MIN_GLO_PRN <= id && id <= MAX_GLO_PRN)
            dWaveLength = (LIGHT / (GLO_L1_BASE_FRE + glo_offset[id - MIN_GLO_PRN] * GLO_L1_SUB_BAND));
        else if(MIN_BD2_PRN <= id && id <= MAX_BD2_PRN)
            dWaveLength = (LIGHT / BD2_F1);
        else if(MIN_GAL_PRN <= id && id <= MAX_GAL_PRN)
            dWaveLength = (LIGHT / GAL_F1);
        else continue;

        /* set phase */
        if(obss->pdL1[i]!=0&&obss->pdD1[i]!=0)
            obss->pdL1[i]-=(offset*LIGHT-offset*obss->pdD1[i])/dWaveLength;
           /* set code */
        if(obss->pdR1[i]!=0&&obss->pdD1[i]!=0)
            obss->pdR1[i]-=offset*LIGHT-offset*obss->pdD1[i];


		/*L1 phase lock loop.*/
		if(MIN_GPS_PRN <= id && id <= MAX_GPS_PRN)
			dWaveLength = (LIGHT / GPS_F2);
		else if(MIN_GLO_PRN <= id && id <= MAX_GLO_PRN)
			dWaveLength = (LIGHT / (GLO_L2_BASE_FRE + glo_offset[id - MIN_GLO_PRN] * GLO_L2_SUB_BAND));
		else if(MIN_BD2_PRN <= id && id <= MAX_BD2_PRN)
			dWaveLength = (LIGHT / BD2_F2);
		else if(MIN_GAL_PRN <= id && id <= MAX_GAL_PRN)
			dWaveLength = (LIGHT / GAL_F2);
		else continue;

		/* set phase */
		if(obss->pdL2[i]!=0&&obss->pdD2[i]!=0)
			obss->pdL2[i]-=(offset*LIGHT-offset*obss->pdD2[i])/dWaveLength;
		/* set code */
		if(obss->pdP2[i]!=0&&obss->pdD2[i]!=0)
			obss->pdP2[i]-=offset*LIGHT-offset*obss->pdD2[i];
    }
    return 1;
}
#endif


void CQRtkImpl::GetSinglePosition(WangEpoch *pEpoch, WangEpoch *pLastEpoch, Coordinate *pXyz){
	if (m_nWeekCurForRTCM > 1024){

		if (m_dSecondCurForRTCM - pEpoch->t > 302400.){
			pEpoch->wWeek = m_nWeekCurForRTCM + 1;
		}else if (pEpoch->t - m_dSecondCurForRTCM > 302400.){
			pEpoch->wWeek = m_nWeekCurForRTCM - 1;
		}
		else{
			pEpoch->wWeek = m_nWeekCurForRTCM;
		}
	}
    
    //2020-11-24,not calc doppler,doppler data from rover rtcm.
#ifndef  _EC100Y_PLATFORM
	CaculateEpochDoppler(pEpoch, pLastEpoch);
#endif

    //m_satAm = {0};
    memset(&m_satAm, 0x00, SAT_AM_ALTIT_SIZE);
	memcpy(m_satAm.pbID, pEpoch->pbID, sizeof(pEpoch->pbID));
	
	//correct bias of clock
	/**2020-12-08, WangEpoch extend to MAXSAT_EXT,but Epoch.n take over MAXSAT,employee MAXSAT or n*/
	int validSatCount = pEpoch->n;
	if(validSatCount > MAXSAT){
		validSatCount = MAXSAT;
	}

    //int nSatCount = GetSatPos(pEpoch->n, pEpoch->t, pXyz->ARP, &m_satAm);
#ifdef  _EC100Y_PLATFORM
    int nSatCount = GetSatPos(validSatCount, pEpoch->t-0.075, pXyz->ARP, &m_satAm);
#else
    int nSatCount = GetSatPos(validSatCount, pEpoch->t, pXyz->ARP, &m_satAm);
#endif

	LOG_DEBUG("RTK Engine, Epoch n:%d, time:%.3f, X:%.4f, Y:%.4f, Z:%.4f\n", (int)pEpoch->n, pEpoch->t, pXyz->ARP[0], pXyz->ARP[1], pXyz->ARP[2]);

    nSatCount=0;
    double dclock=0;

	for(int i = 0; i < MAXSAT; ++ i){
		if ( m_satAm.pbID[i] == 0) continue;
		if (pEpoch->pbID[i] == m_satAm.pbID[i]){
			pEpoch->pwAzimuth[i] = static_cast<short>(m_satAm.pdAzimuth[i]);
			pEpoch->pbElevation[i] = static_cast<unsigned char>(m_satAm.pdElevation[i]);

#ifdef  _EC100Y_PLATFORM
            if(fabs(pEpoch->pdR1[i])>NZ && fabs(m_satAm.pdDistance[i])>NZ) {
                double ddr1=pEpoch->pdR1[i] - m_satAm.pdDistance[i] + m_satAm.pdSatClock[i];
				m_satAm.pddr[i] = ddr1;
                if(abs(ddr1) < 0.001*LIGHT && abs(ddr1) > 100) {
                    nSatCount++;
                    dclock+=ddr1;
                }
            }
#endif
		}
	}
 
#ifdef  _EC100Y_PLATFORM
    if(nSatCount != 0)
        pEpoch->dGPSOffset = dclock/nSatCount/LIGHT;
    mesoffset(pEpoch);
#endif
   
    pEpoch->x = pXyz->ARP[0];
	pEpoch->y = pXyz->ARP[1];
	pEpoch->z = pXyz->ARP[2];

	return;
}

bool CQRtkImpl::CreateM919TotalInfor(WangEpoch *pBsEpoch, WangEpoch *pRvEpoch, M919TotalInfor& m919TInfo ){
	int iCorrect = 0;

	double dNM1, dNM2, dNM3;

	m919TInfo.wVersion = M919_VERSION;
	WangEpoch* pEpochA = pBsEpoch;
	WangEpoch* pEpochB = pRvEpoch;

	double t = dInt(pEpochB->t * 1000) / 1000.;
	if (t < 0 || t > 604800){
        LOG_ERROR("[ERROR] Fail to create M919,because epoch time[%.4f] invalid.", pEpochB->t);
		return false;
	}

	m919TInfo.wWeek_GPS = pEpochB->wWeek;
	m919TInfo.dSecond_GPS = pEpochB->t; 
	m919TInfo.bFrqFlag = 0xFB;
	m919TInfo.bFrqFlagHigh = 0x07;
	m919TInfo.bReserved0 = 0x01;//Event为1，常值为0
	m919TInfo.bMesSta_GPS = 0;
	m919TInfo.bMesSta_BD2 = 0;
	m919TInfo.bMesSta_GLO = 0;
	m919TInfo.bMesSta_GAL = 0;
	m919TInfo.iTimeOffset_GPS = (int)((pEpochB->t - pEpochA->t) * 1.e7);// 后续可能需剔除钟差
	m919TInfo.iTimeOffset_BD2 = (int)((pEpochB->t - pEpochA->t) * 1.e7);
	m919TInfo.iTimeOffset_GLO = (int)((pEpochB->t - pEpochA->t) * 1.e7);
	m919TInfo.iTimeOffset_GAL = (int)((pEpochB->t - pEpochA->t) * 1.e7);
	if (fabs(pEpochA->t - pEpochB->t) < NZ)
		m919TInfo.bReserved0 = 0x00;

	m919TInfo.iRefPpsDiffValid = 0;
	m919TInfo.iRefPpsDiff = 0;
	m919TInfo.iRovPpsDiffValid = 0;
	m919TInfo.iRovPpsDiff = 0;
	m919TInfo.dRefPvtTimeDiff_GPS = 0.0;
	m919TInfo.dRefPvtTimeDiff_BD2 = 0.0;
	m919TInfo.dRefPvtTimeDiff_GLO = 0.0;
	m919TInfo.dRefPvtTimeDiff_GAL = 0.0;

	m919TInfo.dRovPvtTimeDiff_GPS = 0.0;
	m919TInfo.dRovPvtTimeDiff_BD2 = 0.0;
	m919TInfo.dRovPvtTimeDiff_GLO = 0.0;
	m919TInfo.dRovPvtTimeDiff_GAL = 0.0;

	int i, j, id, iError = 0;

	int idA, idB;


	/** 组成单差观测值*/

	int iAdjust = 1;	//防止观测数据被赋0而加的扰动
	int nSatNum = 0;
	for (i = 0; i < pEpochA->n; i++)
	{
		/**2020-12-05, RTK lib, MAXSAT 25, keep from overflowing, then MAXSAT break*/
		if(nSatNum >= MAXSAT){
			break;
		}

	    iAdjust *= -1;

		idA = pEpochA->pbID[i];
		if (idA <= 0 || idA >= MAXPRN) continue;	// if SVID == 0

		for (j = 0; j < pEpochB->n; j++)
		{
			idB = pEpochB->pbID[j];
			if (idB <= 0 || idB >= MAXPRN) continue;

			/** 若执行if的话可以break了？--陆小路*/
			if (idA == idB)
			{
				id = idA;

				if (pEpochB->pbElevation[j] <= NZ){
				    break; //20190801 debug
				}

				m919TInfo.SatObsInfor[nSatNum].PRN = id;
				m919TInfo.SatObsInfor[nSatNum].ElevationDegree = pEpochB->pbElevation[j];
				m919TInfo.SatObsInfor[nSatNum].AzimuthDegree = pEpochB->pwAzimuth[j];
				if (m919TInfo.SatObsInfor[nSatNum].AzimuthDegree < 0)
					m919TInfo.SatObsInfor[nSatNum].AzimuthDegree += 360;

				dNM1 = dNM2 = dNM3 = 0.0;
				m_theX.GetWaveLength(id, dNM1, dNM2, dNM3); //Todo 需要增加读取配置参数

				if (fabs(pEpochA->pdR1[i]) > NZ && fabs(pEpochA->pdP2[i]) > NZ)
				{
					/** 监测电离层延迟（不考虑群延迟）*/
					/*
 					 m919TInfo.SatObsInfor[i].dwRefPrDiff = (short)(pEpochA->pdR1[i] - pEpochA->pdP2[i]) * REFPRDIFF_FACTOR;
  					 double dDltPhi = -(pEpochA->pdL1[i] * dNM1 - pEpochA->pdL2[i] * dNM2)*1000.;
					*/

					m919TInfo.SatObsInfor[nSatNum].dwRefPrDiff = (short)((pEpochA->pdP2[i] - pEpochA->pdR1[i]) * REFPRDIFF_FACTOR);
					double dDltPhi = -(pEpochA->pdL1[i] * dNM1 - pEpochA->pdL2[i] * dNM2) * 1000;
					double dRoll = dDltPhi / 65535;
					if (dRoll < 0)
						dRoll = (double)(INT64(dRoll) - 1);
					else
						dRoll = (double)(INT64(dRoll));

					dDltPhi -= dRoll * 65535;
					m919TInfo.SatObsInfor[nSatNum].dwDeltaPhi = (UINT16)(dDltPhi);

					/*
 					if (PR && id == 14)
  					{
 						fprintf(PR, "\r\nTime:%15.4f, PRN: %d, RefDiff: %d, DeltaPi: %d",
  						m919TInfo.dSecond_GPS, id, m919TInfo.SatObsInfor[i].dwRefPrDiff, m919TInfo.SatObsInfor[i].dwDeltaPhi);
 					}
					*/
				}

				/**L1*/
				if (fabs(pEpochA->pdR1[i]) > NZ && fabs(pEpochB->pdR1[j]) > NZ
					&& fabs(pEpochA->pdL1[i]) > NZ && fabs(pEpochB->pdL1[j]) > NZ && fabs(dNM1) > NZ)
				{
					m919TInfo.SatObsInfor[nSatNum].Lx[0].Status = 0x01;//Parity known flag

					/**
 					对Rinex文件中没有或未读信噪比的情况，需要赋值，QRTKdll对信噪比应该作了检测
 					不赋值无法解算--陆小路
 					**/
					if (abs(pEpochB->pbSN1[j]) < NZ)
						pEpochB->pbSN1[j] = 45;

					m919TInfo.SatObsInfor[nSatNum].Lx[0].SNR = pEpochB->pbSN1[j];
					m919TInfo.SatObsInfor[nSatNum].Lx[0].RMS = 1;// 128-135
					m919TInfo.SatObsInfor[nSatNum].Lx[0].LostCount = 0;// 144-164,小于1认为失锁
					double dP1RSDCycle = (pEpochA->pdR1[i] - pEpochB->pdR1[j]) / dNM1;
					/**m919TInfo.SatObsInfor[i].Lx[0].PR_SD_or_Rov_PR = dP1RSDCycle * PR_SD_FACTOR;*/
					m919TInfo.SatObsInfor[nSatNum].Lx[0].PR_SD_or_Rov_PR = dP1RSDCycle;

					double dCP_SD = pEpochA->pdL1[i] - pEpochB->pdL1[j];

					int nCycle = NCycle[0][id];// NCycle 需保存 是否超限int
					if (abs(nCycle) < NZ || fabs(dCP_SD - dP1RSDCycle - NCycle[0][id]) > CYCLE_LIMIT)
					{
						NCycle[0][id] = (int)(dCP_SD - dP1RSDCycle);
						nCycle = NCycle[0][id];
					}

					/* 第二条数据开始做检验
 					m919TInfo.SatObsInfor[i].Lx[0].CP_SDMinusPR_SD = (dCP_SD - dP1RSDCycle - nCycle) * CP_SD_FACTOR;
 		            */
					m919TInfo.SatObsInfor[nSatNum].Lx[0].CP_SDMinusPR_SD = (float)(dCP_SD - dP1RSDCycle - nCycle);

					/*if (PR && id == 14)
 					{
 						fprintf(PR, ",dCP_SD: %15.4f, BR1: %15.4f, AR1: %15.4f, BL1: %15.4f, AL1: %15.4f, NM1: % .10f, PR_SD_or_Rov_PR: %15.4f, CP_SDMinusPR_SD: %15.4f",
 						dCP_SD, pEpochB->pdR1[j], pEpochA->pdR1[i], pEpochB->pdL1[j], pEpochA->pdL1[i], dNM1,
 						m919TInfo.SatObsInfor[i].Lx[0].PR_SD_or_Rov_PR, m919TInfo.SatObsInfor[i].Lx[0].CP_SDMinusPR_SD);
 					}
					*/
				}

				/**L2*/
				if (fabs(pEpochA->pdP2[i]) > NZ && fabs(pEpochB->pdP2[j]) > NZ
					&& fabs(pEpochA->pdL2[i]) > NZ && fabs(pEpochB->pdL2[j]) > NZ && fabs(dNM2) > NZ)
				{
                    if(id >= MIN_GPS_PRN && id <= MAX_GPS_PRN){
					    m919TInfo.SatObsInfor[nSatNum].Lx[1].Status = 0x09;//Parity known flag
                    }else{
					    m919TInfo.SatObsInfor[nSatNum].Lx[1].Status = 0x01;//Parity known flag
                    }
					if (abs(pEpochB->pbSN2[j]) < NZ)
						pEpochB->pbSN2[j] = 45;

					m919TInfo.SatObsInfor[nSatNum].Lx[1].SNR = pEpochB->pbSN2[j];
					m919TInfo.SatObsInfor[nSatNum].Lx[1].RMS = 1;// 128-135
					m919TInfo.SatObsInfor[nSatNum].Lx[1].LostCount = 0;// 144-164,小于1认为失锁
					double dP2RSDCycle = (pEpochA->pdP2[i] - pEpochB->pdP2[j]) / dNM2;
					/**m919TInfo.SatObsInfor[i].Lx[1].PR_SD_or_Rov_PR = dP2RSDCycle * PR_SD_FACTOR;*/
					m919TInfo.SatObsInfor[nSatNum].Lx[1].PR_SD_or_Rov_PR = dP2RSDCycle;

					double dCP_SD = pEpochA->pdL2[i] - pEpochB->pdL2[j];

					int nCycle = NCycle[1][id];// NCycle 需保存 是否超限int
					if (abs(nCycle) < NZ || fabs(dCP_SD - dP2RSDCycle - NCycle[1][id]) > CYCLE_LIMIT)
					{
						NCycle[1][id] = (int)(dCP_SD - dP2RSDCycle);
						nCycle = NCycle[1][id];
					}

					/**m919TInfo.SatObsInfor[i].Lx[1].CP_SDMinusPR_SD = (dCP_SD - dP2RSDCycle - nCycle) * CP_SD_FACTOR;*/
					m919TInfo.SatObsInfor[nSatNum].Lx[1].CP_SDMinusPR_SD = (float)(dCP_SD - dP2RSDCycle - nCycle);

					/*if (PR && id == 14)
 					{
 						fprintf(PR, ", BR2: %15.4f, AR2: %15.4f, BL2: %15.4f, AL2: %15.4f, NM2: % .10f, PR_SD_or_Rov_PR: %15.4f, CP_SDMinusPR_SD: %15.4f",
 						pEpochB->pdP2[j], pEpochA->pdP2[i], pEpochB->pdL2[j], pEpochA->pdL2[i],dNM2,
 						m919TInfo.SatObsInfor[i].Lx[1].PR_SD_or_Rov_PR, m919TInfo.SatObsInfor[i].Lx[1].CP_SDMinusPR_SD);
 					}*/
				}

				/** L5.*/
				if (fabs(pEpochA->pdR5[i]) > NZ && fabs(pEpochB->pdR5[j]) > NZ
					&& fabs(pEpochA->pdL5[i]) > NZ && fabs(pEpochB->pdL5[j]) > NZ && fabs(dNM3) > NZ)
				{
					m919TInfo.SatObsInfor[nSatNum].Lx[2].Status = 0x01;//Parity known flag

					if (abs(pEpochB->pbSN5[j]) < NZ)
						pEpochB->pbSN5[j] = 45;

					m919TInfo.SatObsInfor[nSatNum].Lx[2].SNR = pEpochB->pbSN5[j];
					m919TInfo.SatObsInfor[nSatNum].Lx[2].RMS = 1;// 128-135
					m919TInfo.SatObsInfor[nSatNum].Lx[2].LostCount = 0;// 144-164,小于1认为失锁
					double dP3RSDCycle = (pEpochA->pdR5[i] - pEpochB->pdR5[j]) / dNM3;
					/**m919TInfo.SatObsInfor[i].Lx[2].PR_SD_or_Rov_PR = dP3RSDCycle * PR_SD_FACTOR;*/
					m919TInfo.SatObsInfor[nSatNum].Lx[2].PR_SD_or_Rov_PR = dP3RSDCycle;

					double dCP_SD = pEpochA->pdL5[i] - pEpochB->pdL5[j];

					int nCycle = NCycle[2][id];// NCycle 需保存 是否超限int
					if (abs(nCycle) < NZ || fabs(dCP_SD - dP3RSDCycle - NCycle[2][id]) > CYCLE_LIMIT)
					{
						NCycle[2][id] = (int)(dCP_SD - dP3RSDCycle);
						nCycle = NCycle[2][id];
					}
					/**m919TInfo.SatObsInfor[i].Lx[2].CP_SDMinusPR_SD = (dCP_SD - dP3RSDCycle - nCycle) * CP_SD_FACTOR;*/
					m919TInfo.SatObsInfor[nSatNum].Lx[2].CP_SDMinusPR_SD = (float)(dCP_SD - dP3RSDCycle - nCycle);
				}

				nSatNum++;
				break;
			}
		}
	}

	m919TInfo.bSatNo = nSatNum;// 确认

    /**单点定位结果--陆小路*/
	m919TInfo.SppRefInfor.dRov_SPP_Time = pEpochB->t - pEpochB->dGPSOffset;
	//m919TInfo.SppRefInfor.dRov_SPP_Time = pEpochB->t + pEpochB->dGPSOffset;
	m919TInfo.SppRefInfor.dRov_SPP_x = pEpochB->x;
	m919TInfo.SppRefInfor.dRov_SPP_y = pEpochB->y;
	m919TInfo.SppRefInfor.dRov_SPP_z = pEpochB->z;
	m919TInfo.SppRefInfor.dRov_SPP_vx = 0.0;
	m919TInfo.SppRefInfor.dRov_SPP_vy = 0.0;
	m919TInfo.SppRefInfor.dRov_SPP_vz = 0.0;
	m919TInfo.SppRefInfor.iRef_ID = 1;
	m919TInfo.SppRefInfor.dRef_Pos_Time = pEpochA->t;
	m919TInfo.SppRefInfor.dRef_Pos_x = pEpochA->x;
	m919TInfo.SppRefInfor.dRef_Pos_y = pEpochA->y;
	m919TInfo.SppRefInfor.dRef_Pos_z = pEpochA->z;

	m919TInfo.dBd2Time2GpsTime = 0.0;
	m919TInfo.dGloTime2GpsTime = 0.0;
	m919TInfo.dGalTime2GpsTime = 0.0;
	m919TInfo.dDtGps = 0.0;
	m919TInfo.dDtBds = 0.0;
	m919TInfo.dDtGlo = 0.0;
	m919TInfo.dDtGal = 0.0;
	m919TInfo.bRefRcvrTypeId = 4;
	m919TInfo.bReserved1 = 0;
	m919TInfo.bReserved2 = 0;
	m919TInfo.bReserved3 = 0;
	
    return true;
}

/* inner product ---------------------------------------------------------------
 * * inner product of vectors
 * * args   : double *a,*b     I   vector a,b (n x 1)
 * *          int    n         I   size of vector a,b
 * * return : a'*b
 * *-----------------------------------------------------------------------------*/
double dot_1(const double *a, const double *b, int n)
{
     double c=0.0;
         
         while (--n>=0) c+=a[n]*b[n];
             return c;
             
}
/* transform ecef to geodetic postion ------------------------------------------
 * * transform ecef position to geodetic position
 * * args   : double *r        I   ecef position {x,y,z} (m)
 * *          double *pos      O   geodetic position {lat,lon,h} (rad,m)
 * * return : none
 * * notes  : WGS84, ellipsoidal height
 * *-----------------------------------------------------------------------------*/
void ecef2pos_1(const double *r, double *pos)
{
     double e2=FE_WGS84*(2.0-FE_WGS84),r2=dot_1(r,r,2),z,zk,v=RE_WGS84,sinp;
         
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

bool CQRtkImpl::CreateM916TotalInfor(WangEpoch *pBsEpoch, WangEpoch *pRvEpoch, MixEpoch& m916TInfo )
{	
	memset(&m916TInfo,0,sizeof(MixEpoch));
	WangEpoch* pEpochA = pBsEpoch;
	WangEpoch* pEpochB = pRvEpoch;
	m916TInfo.wSize = MIX_EPOCH_SIZE;
	m916TInfo.wMsgID = 916;
	m916TInfo.wFreq = 0x07FB;	

	m916TInfo.bVersion = M916_VERSION;
	if (pEpochB->t<0||pEpochB->t>604800||pEpochB->n<=0)
	{
		return false;
	}

	int t = iInt(pEpochB->t * 1000);
	m916TInfo.wWeek = pEpochB->wWeek;
	m916TInfo.lTimeMS = t;
	m916TInfo.wReceiverStatus = 0x01;
	m916TInfo.wBaseID = 0x00;
	m916TInfo.wRoverID = 0x00;
	m916TInfo.lRovOffset = iInt(pEpochB->dGPSOffset*LIGHT*1000);

	m916TInfo.iGPSLag = dInt((pEpochB->t - pEpochA->t) * 100);
	m916TInfo.iBDSLag = dInt((pEpochB->t - pEpochA->t) * 100);
	m916TInfo.iGloLag = dInt((pEpochB->t - pEpochA->t) * 100);
	m916TInfo.iGalLag = dInt((pEpochB->t - pEpochA->t) * 100);

	m916TInfo.iBaseGpsOffset = iInt((pEpochA->dGPSOffset) * 1.e7);
	m916TInfo.iBaseBdsOffset = iInt((pEpochA->dGPSOffset) * 1.e7);
	m916TInfo.iBaseGloOffset = iInt((pEpochA->dGPSOffset) * 1.e7);
	m916TInfo.iBaseGalOffset = iInt((pEpochA->dGPSOffset) * 1.e7);

	double xyz[3],blh[3];
	m916TInfo.dBaseX =xyz[0]= pEpochA->x;
	m916TInfo.dBaseY =xyz[1]= pEpochA->y;
	m916TInfo.dBaseZ =xyz[2]= pEpochA->z;
	
	ecef2pos_1(xyz,blh);
	m916TInfo.lBaseLat = iInt(blh[0]*1.e9);
	m916TInfo.lBaseLon = iInt(blh[1]*1.e9);
	m916TInfo.lBaseHcm = iInt(blh[2]*100);

	m916TInfo.lRoverX = iInt((pEpochB->x-pEpochA->x)*1000);
	m916TInfo.lRoverY = iInt((pEpochB->y-pEpochA->y)*1000);
	m916TInfo.lRoverZ = iInt((pEpochB->z-pEpochA->z)*1000);

	m916TInfo.lVelX = 0.0;
	m916TInfo.lVelY = 0.0;
	m916TInfo.lVelZ = 0.0;

	int nsat=0;
	BYTE statusL1,statusL2,statusL5;
	BYTE RMSL1,RMSL2,RMSL5;
	double R1;
	for(int i=0;i<MAXSAT;i++)//ROVER
	{
		statusL1 = statusL2 = statusL5 = 0;
		RMSL1= RMSL2= RMSL5 = 0;
		if(pEpochB->pbID[i]>=MIN_GPS_PRN&&pEpochB->pbID[i]<=MAX_BD2_PRN){
			m916TInfo.pbID[nsat] = pEpochB->pbID[i];
			m916TInfo.pbFlag[i] |= 0x02;
			m916TInfo.pbElevation[nsat] = pEpochB->pbElevation[i];
			m916TInfo.piAzimuth[nsat]   = pEpochB->pwAzimuth[i];
			if(m916TInfo.piAzimuth[nsat]>180)   m916TInfo.piAzimuth[nsat] -=360;

			m916TInfo.piIonoDelay[nsat]  = 0.0;
			m916TInfo.piTropDelay[nsat]  = 0.0;
			
			if(pEpochB->pdR1[i]>0) statusL1=1;
			if(pEpochB->pdP2[i]>0) statusL2=1;
			if(pEpochB->pdR5[i]>0) statusL5=1;

			m916TInfo.pbRoverM1[nsat]  = statusL1 <<4 | RMSL1 &0x0F;
			m916TInfo.pbRoverM2[nsat]  = statusL2 <<4 | RMSL2 &0x0F;
			m916TInfo.pbRoverM5[nsat]  = statusL5 <<4 | RMSL5 &0x0F;

			m916TInfo.pbRoverCN1[nsat] = pEpochB->pbSN1[i];
			m916TInfo.pbRoverCN2[nsat] = pEpochB->pbSN2[i];
			m916TInfo.pbRoverCN5[nsat] = pEpochB->pbSN5[i];

			m916TInfo.pbRoverS1[nsat]  = pEpochB->pdwSlip1[i];
			m916TInfo.pbRoverS2[nsat]  = pEpochB->pdwSlip2[i];
			m916TInfo.pbRoverS5[nsat]  = pEpochB->pdwSlip5[i];

			if(statusL1>0) R1 =pEpochB->pdR1[i];
			else if(statusL2>0) R1 =pEpochB->pdP2[i];
			else if(statusL5>0) R1 =pEpochB->pdR5[i];

			m916TInfo.plRoverR1[nsat] = iInt(R1*10);
			if (statusL2) m916TInfo.piRoverR21[nsat] = iInt((pEpochB->pdP2[i] - R1) *10);
			if (statusL5) m916TInfo.piRoverR51[nsat] = iInt((pEpochB->pdR5[i] - R1) *10);

			UINT32 Utemp[2];
			m916TInfo.plRoverL1[nsat]  = CutDouble2Int(pEpochB->pdL1[i]*256,Utemp);
			m916TInfo.plRoverL2[nsat]  = CutDouble2Int(pEpochB->pdL2[i]*256,Utemp);
			m916TInfo.plRoverL5[nsat]  = CutDouble2Int(pEpochB->pdL5[i]*256,Utemp);

			nsat++;
		}
	}

	m916TInfo.nObs = nsat;
	
	for(int j =0;j<nsat;j++){
		for(int i=0;i<MAXSAT;i++)//BASE
		{
			statusL1 = statusL2 = statusL5 = 0;
			RMSL1= RMSL2= RMSL5 = 0;
			if(pEpochA->pbID[i]==m916TInfo.pbID[j]){

				m916TInfo.pbFlag[i] |= 0x01;

				if(pEpochA->pdR1[i]>0) statusL1=1;
				if(pEpochA->pdP2[i]>0) statusL2=1;
				if(pEpochA->pdR5[i]>0) statusL5=1;

				m916TInfo.pbBaseM1[j]  = statusL1 <<4 | RMSL1 &0x0F;
				m916TInfo.pbBaseM2[j]  = statusL2 <<4 | RMSL2 &0x0F;
				m916TInfo.pbBaseM5[j]  = statusL5 <<4 | RMSL5 &0x0F;

				m916TInfo.pbBaseCN1[j] = pEpochA->pbSN1[i];
				m916TInfo.pbBaseCN2[j] = pEpochA->pbSN2[i];
				m916TInfo.pbBaseCN5[j] = pEpochA->pbSN5[i];

				m916TInfo.pbBaseS1[j]  = pEpochA->pdwSlip1[i];
				m916TInfo.pbBaseS2[j]  = pEpochA->pdwSlip2[i];
				m916TInfo.pbBaseS5[j]  = pEpochA->pdwSlip5[i];

				if(statusL1>0) R1 =pEpochA->pdR1[i];
				else if(statusL2>0) R1 =pEpochA->pdP2[i];
				else if(statusL5>0) R1 =pEpochA->pdR5[i];

				m916TInfo.plBaseR1[j] = iInt(R1*10);
				if(statusL2) m916TInfo.piBaseR21[j] = iInt((pEpochA->pdP2[i] - R1) *10);
				if(statusL5) m916TInfo.piBaseR51[j] = iInt((pEpochA->pdR5[i] - R1) *10);
				
				UINT32 Utemp[2];
				m916TInfo.plBaseL1[j]  = CutDouble2Int(pEpochA->pdL1[i]*256,Utemp);
				m916TInfo.plBaseL2[j]  = CutDouble2Int(pEpochA->pdL2[i]*256,Utemp);
				m916TInfo.plBaseL5[j]  = CutDouble2Int(pEpochA->pdL5[i]*256,Utemp);
			}
		}
	}
	return true;
}
unsigned int CQRtkImpl::CutDouble2Int(double dSource,unsigned int *pBuff)
{
	const double kMax = 4294967296.;
	int int_higherbits  = (int)(dSource / kMax);
	unsigned int int_lowerbits;
	if(dSource < 0){
		double dTemp = ((dSource - (int_higherbits - 1) * kMax));
		UINT32 iTemp = CutDouble2UInt(dTemp);
		
        int_lowerbits = iTemp;
		int_higherbits -= 1;
	}
	else{
		if(fabs(double(int_higherbits)) > 0.5){
			int_lowerbits =  CutDouble2UInt((dSource - int_higherbits * kMax));
		}
		else{
			int_lowerbits = CutDouble2UInt(dSource);
		}
	}

	pBuff[0] = (unsigned int)int_higherbits;
	pBuff[0] = int_lowerbits;

	return  int_lowerbits;
}

unsigned int CQRtkImpl::CutDouble2UInt(double dsource)
{
	if (dsource < 0){
		return UINT32(dsource - 0.5);
	}else{
		return UINT32(dsource + 0.5);
	}
}

void CQRtkImpl::SendEpochToQRtk(WangEpoch* pBSEpoch, WangEpoch *pRVEpoch, RTK_result *pRtkRes, char* RtkDebug){
	M919TotalInfor m919TotalInfo = {0};
	MixEpoch  m916TotalInfo = {0};
    bool isCreatedM919 = false;
    bool isCreatedM916 = false;
    if(log_config._rtk_dat_type & RTK_DATA_M919){
	    isCreatedM919 = this->CreateM919TotalInfor(pBSEpoch, pRVEpoch, m919TotalInfo); 
    }

    if(log_config._rtk_dat_type & RTK_DATA_M916){
        isCreatedM916 = this->CreateM916TotalInfor(pBSEpoch, pRVEpoch, m916TotalInfo);
    }
    
    if((log_config._dat_type & RAW_DATA_OUTPUT_SINOINTER)){
        uint16_t data_length = 0;
        if(isCreatedM919){
            EncodeM919_2B(m919TotalInfo, (BYTE*)m_encodeBuffer, (UINT32&)data_length, 919);
            write_data(RAW_DATA_OUTPUT_SINOINTER, m_encodeBuffer, data_length);
        }
        if(isCreatedM916){
            EncodeM916_2B(&m916TotalInfo, (BYTE*)m_encodeBuffer, (UINT32&)data_length, 916);
            write_data(RAW_DATA_OUTPUT_SINOINTER, m_encodeBuffer, data_length);
        }
    }
    //to link
//#ifdef _LINK_RTK
#ifdef _LINK_RTK_SOLVE
    if(m_handle){
        if(isCreatedM919){
		    SendOneEpochToRtk(m_handle, &m919TotalInfo, pRtkRes, RtkDebug);
        }
        if(isCreatedM916){

        }
    }
#endif

	return;
}

void CQRtkImpl::SendCommandToQRtk(int iMode, int iParam){
    //to link
#ifdef _LINK_RTK
    if(m_handle){
	    SetupRtk(m_handle, iMode, iParam);
    }
#endif
	return ;
}

int  CQRtkImpl::GetCommandOfQRtk(int iMode){
    //to link
    int iParam = 0; 
#ifdef _LINK_RTK
    if(m_handle){
        iParam = iGetRtkSetup(m_handle, iMode);
    }
#endif
	return iParam;
}

void CQRtkImpl::SetQRtkViewEnable(const bool _bViewEnable){
#ifdef _LINK_RTK
    if(m_handle){
	    SetViewEnable(m_handle, _bViewEnable);
    }
#endif
	return ;
}

void CQRtkImpl::SetQRtkLogEnable(const bool _bLogEnable){
#ifdef _LINK_RTK
    if(m_handle){
	    SetLogEnable(m_handle, _bLogEnable);
    }
#endif
	return ;
}

void CQRtkImpl::SetQRtkOutputFormat(const int _iOutputFormat){
#ifdef _LINK_RTK
    if(m_handle){
	    SetOutputFormat(m_handle, _iOutputFormat);
    }
#endif
	return ;
}

void CQRtkImpl::SetupQRtk(int iMode, int iParam)
{
#ifdef _LINK_RTK
    if(m_handle){
	    SetupRtk(m_handle, iMode, iParam);
    }
#endif
	return ;
}

int  CQRtkImpl::GetQRtkSetup(const int iMode)
{
#ifdef _LINK_RTK
    if(m_handle){
	    return iGetRtkSetup(m_handle, iMode);
    }
#endif
    return 0;
}

void CQRtkImpl::GetNovHeader(UINT32 iMsgId, UINT32 iMsgLen, UINT32 iVer, RawHeader* pHeader, UINT32 iWeek_, UINT32 iSecond_)
{
	UINT16 Week;
	UINT32 iSecond;

	/**BUG!对同步模式的报文时间有影响；lrp@20160706*/
	Week = iWeek_;// 919时间
	iSecond = iSecond_;

	pHeader->Sync1 = 0xaa;
	pHeader->Sync2 = 0x44;
	pHeader->Sync3 = 0x12;

	pHeader->HeaderLength = SizeOfRawHeader;
	pHeader->MessageID = UINT16(iMsgId);
	pHeader->MessageType = 0x02;
	pHeader->PortAddress = 0x20;

	pHeader->MessageLength = iMsgLen;
	pHeader->Sequence = 0;

	pHeader->IdleTime = 0;

	/**20150920;*/
	pHeader->TimeStatus = 180;	//time is fine set and being steered;

	pHeader->Week = Week;
	pHeader->GPSec = iSecond;// 毫秒单位

	pHeader->ReceiverStatus = 0x00100000;

	pHeader->ReceiverVersion = iVer;
}

BYTE CQRtkImpl::GetM919SatNum(GnssType Gnss, M919TotalInfor& M919Infor)
{
	BYTE bSatNum = 0;

	if (Gnss == ALEX_GPS)
	{
		for (int i = 0; i < M919Infor.bSatNo; i++)
		{
			if (M919Infor.SatObsInfor[i].PRN >= MIN_GPS_PRN && M919Infor.SatObsInfor[i].PRN <= MAX_GPS_PRN)
				bSatNum++;
		}
	}
	else if (Gnss == ALEX_BD2)
	{
		for (int i = 0; i < M919Infor.bSatNo; i++)
		{
			if (M919Infor.SatObsInfor[i].PRN >= MIN_BD2_PRN && M919Infor.SatObsInfor[i].PRN <= MAX_BD2_PRN)
				bSatNum++;
		}
	}
	else if (Gnss == ALEX_GLONASS)
	{
		for (int i = 0; i < M919Infor.bSatNo; i++)
		{
			if (M919Infor.SatObsInfor[i].PRN >= MIN_GLO_PRN && M919Infor.SatObsInfor[i].PRN <= MAX_GLO_PRN)
				bSatNum++;
		}
	}
	else if(Gnss == ALEX_GALILEO)
	{
		for(int i = 0; i < M919Infor.bSatNo; i++)
		{
			if(M919Infor.SatObsInfor[i].PRN >= MIN_GAL_PRN && M919Infor.SatObsInfor[i].PRN <= MAX_GAL_PRN)
				bSatNum++;
		}
	}

	return bSatNum;
}

BYTE CQRtkImpl::GetM919FrqNum(GnssType Gnss, BYTE bGnssFrqFlag)
{
	BYTE bFrqFlag;

	switch (Gnss)
	{
	case ALEX_GPS:
		bFrqFlag = (bGnssFrqFlag & 0x07);
		break;

	case ALEX_BD2:
		bFrqFlag = ((bGnssFrqFlag & 0x38) >> 3);
		break;

	case ALEX_GLONASS:
		bFrqFlag = ((bGnssFrqFlag & 0xC0) >> 6);
		break;

	case ALEX_GALILEO:
		bFrqFlag = (bGnssFrqFlag & 0x07);
		break;

	default:
		break;
	}

	BYTE bFrqNum = (bFrqFlag & 0x01) + ((bFrqFlag & 0x02) >> 1) + ((bFrqFlag & 0x04) >> 2);

	return bFrqNum;
}
void EditDoubleInBuff(double dTarget, BYTE *pBuff, UINT32 iPos)
{
	BYTE *ch_p = 0;
	ch_p = (BYTE *)(&dTarget);

	pBuff[iPos + 0] = *ch_p;
	pBuff[iPos + 1] = *(ch_p + 1);
	pBuff[iPos + 2] = *(ch_p + 2);
	pBuff[iPos + 3] = *(ch_p + 3);
	pBuff[iPos + 4] = *(ch_p + 4);
	pBuff[iPos + 5] = *(ch_p + 5);
	pBuff[iPos + 6] = *(ch_p + 6);
	pBuff[iPos + 7] = *(ch_p + 7);

}

void EditInt16InBuff(short wTarget, BYTE *pBuff, UINT32 iPos)
{
	BYTE *ch_p = 0;
	ch_p = (BYTE *)(&wTarget);

	pBuff[iPos + 0] = *ch_p;
	pBuff[iPos + 1] = *(ch_p + 1);
}

void EditInt32InBuff(INT iTarget, BYTE *pBuff, UINT32 iPos)
{
	BYTE *ch_p = 0;
	ch_p = (BYTE *)(&iTarget);

	pBuff[iPos + 0] = *ch_p;
	pBuff[iPos + 1] = *(ch_p + 1);
	pBuff[iPos + 2] = *(ch_p + 2);
	pBuff[iPos + 3] = *(ch_p + 3);
}

void EditUint32InBuff(UINT32 iTarget, BYTE *pBuff, UINT32 iPos)
{
	BYTE *ch_p = 0;
	ch_p = (BYTE *)(&iTarget);

	pBuff[iPos + 0] = *ch_p;
	pBuff[iPos + 1] = *(ch_p + 1);
	pBuff[iPos + 2] = *(ch_p + 2);
	pBuff[iPos + 3] = *(ch_p + 3);
}

void EditWordInBuff(unsigned short wTarget, BYTE *pBuff, UINT32 iPos)
{
	BYTE *ch_p = 0;
	ch_p = (BYTE *)(&wTarget);

	pBuff[iPos + 0] = *ch_p;
	pBuff[iPos + 1] = *(ch_p + 1);
}

void EditeNovHead(BYTE *pBuff, RawHeader Header)
{
	pBuff[0] = Header.Sync1;
	pBuff[1] = Header.Sync2;
	pBuff[2] = Header.Sync3;

	pBuff[3] = Header.HeaderLength;

	EditWordInBuff(Header.MessageID, pBuff, 4);

	pBuff[6] = Header.MessageType;
	pBuff[7] = Header.PortAddress;

	EditWordInBuff(Header.MessageLength, pBuff, 8);
	EditWordInBuff(Header.Sequence, pBuff, 10);

	pBuff[12] = Header.IdleTime;
	pBuff[13] = Header.TimeStatus;

	EditWordInBuff(Header.Week, pBuff, 14);
	EditUint32InBuff(Header.GPSec, pBuff, 16);
	EditUint32InBuff(Header.ReceiverStatus, pBuff, 20);
	EditWordInBuff(Header.Reserved, pBuff, 24);
	EditWordInBuff(Header.ReceiverVersion, pBuff, 26);
}

UINT32 GetCRC(BYTE *pBuff, UINT32 iSize)
{
	const unsigned long ulCrcTable[256] =
	{
		0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL,
		0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
		0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L, 0x1db71064L, 0x6ab020f2L,
		0xf3b97148L, 0x84be41deL, 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
		0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
		0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
		0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL, 0x35b5a8faL, 0x42b2986cL,
		0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
		0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L,
		0xcfba9599L, 0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
		0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L, 0x01db7106L,
		0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
		0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL,
		0x91646c97L, 0xe6635c01L, 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
		0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
		0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
		0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L,
		0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
		0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL,
		0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
		0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L,
		0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
		0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L, 0xe3630b12L, 0x94643b84L,
		0x0d6d6a3eL, 0x7a6a5aa8L, 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
		0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
		0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
		0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L, 0xd6d6a3e8L, 0xa1d1937eL,
		0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
		0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L,
		0x316e8eefL, 0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
		0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL, 0xb2bd0b28L,
		0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
		0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL, 0x9c0906a9L, 0xeb0e363fL,
		0x72076785L, 0x05005713L, 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
		0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
		0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
		0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L,
		0x616bffd3L, 0x166ccf45L, 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
		0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL,
		0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
		0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L, 0xcdd70693L,
		0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
		0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
	};

	unsigned int crc = 0;

	for (unsigned int n = 0; n < iSize; n++)
	{
		crc = ulCrcTable[(crc ^ pBuff[n]) & 0xff] ^ (crc >> 8);
	}

	return crc;
}

void CQRtkImpl::EncodeM919_2B(M919TotalInfor& M919Infor, BYTE* chbuff_M919_Total, UINT32& iMesLen, UINT32 iMsgId)
{
#if M919_VERSION >= 0x0054
	const UINT32 iLenth_header919_2 = 24;
	const UINT32 kSatCmnInfoLen = 16;					//增加卫星的三维位置改正；
	const UINT32 iMaxSatNo = MaxSatelliteNumber; 			//
	const UINT32 iSpp_Ref_Len = 68;
	const UINT32 iMaxSingleSatObsLen = 56;					//8+16*3; //增加2字节的DeltaPhi，1字节的电离层延迟绝对值，1保留字节；20141127
	const UINT32 iOtherInforLen = 24 + 32 + 12 + 4 + 60;	//增加移动站的单点单点定位结果，每个4字节，共3个，精确到1米; 	20150905
	/**增加4个字节，第一个字节为参考站接收机类型，其他保留字节；		20151118
 * 	增加IMU信息，共60字节；										20170625
 * 	    */
#elif M919_VERSION >= 0x0051
	const UINT32 iLenth_header919_2 = 24;
	const UINT32 kSatCmnInfoLen = 16;				//增加卫星的三维位置改正；
	const UINT32 iMaxSatNo = MaxSatelliteNumber; 		//
	const UINT32 iSpp_Ref_Len = 68;
	const UINT32 iMaxSingleSatObsLen = 56;				//8+16*3; //增加2字节的DeltaPhi，1字节的电离层延迟绝对值，1保留字节；20141127
	const UINT32 iOtherInforLen = 24 + 32 + 12 + 4;		//增加移动站的单点单点定位结果，每个4字节，共3个，精确到1米; 	20150905
	/**增加4个字节，第一个字节为参考站接收机类型，其他保留字节；		20151118*/
#else
	return;
#endif

	const UINT32 iMaxTotalLen = SizeOfRawHeader + iLenth_header919_2 + (iMaxSingleSatObsLen)*iMaxSatNo + iSpp_Ref_Len + iOtherInforLen + 4;

	UINT32 CRC;
	RawHeader Header919;
	/**BYTE chbuff_M919_Total[iMaxTotalLen] = {0};*/
	UINT32 iNumberOfByteOccupied;

	BYTE bGpsFrqNum = GetM919FrqNum(ALEX_GPS, M919Infor.bFrqFlag);
	BYTE bBd2FrqNum = GetM919FrqNum(ALEX_BD2, M919Infor.bFrqFlag);
	BYTE bGloFrqNum = GetM919FrqNum(ALEX_GLONASS, M919Infor.bFrqFlag);
	BYTE bGalFrqNum = GetM919FrqNum(ALEX_GALILEO, M919Infor.bFrqFlagHigh);

	BYTE bGpsSatNum = GetM919SatNum(ALEX_GPS, M919Infor);
	BYTE bBd2SatNum = GetM919SatNum(ALEX_BD2, M919Infor);
	BYTE bGloSatNum = GetM919SatNum(ALEX_GLONASS, M919Infor);
	BYTE bGalSatNum = GetM919SatNum(ALEX_GALILEO, M919Infor);

	UINT32 iRealMesLen = iLenth_header919_2
		+ bGpsSatNum*(kSatCmnInfoLen + 16 * bGpsFrqNum)
		+ bBd2SatNum*(kSatCmnInfoLen + 16 * bBd2FrqNum)
		+ bGloSatNum*(kSatCmnInfoLen + 16 * bGloFrqNum)
		+ bGalSatNum*(kSatCmnInfoLen + 16 * bGalFrqNum)
		+ iSpp_Ref_Len + iOtherInforLen;

	GetNovHeader(iMsgId, iRealMesLen, M919_VERSION, &Header919, M919Infor.wWeek_GPS, (UINT32)(M919Infor.dSecond_GPS * 1000.0));

	EditeNovHead(chbuff_M919_Total, Header919);

	iNumberOfByteOccupied = SizeOfRawHeader;

	chbuff_M919_Total[iNumberOfByteOccupied++] = M919Infor.bSatNo;
	chbuff_M919_Total[iNumberOfByteOccupied++] = M919Infor.bFrqFlag;

	chbuff_M919_Total[iNumberOfByteOccupied++] = M919Infor.bFrqFlagHigh;
	chbuff_M919_Total[iNumberOfByteOccupied++] = M919Infor.bReserved0;

	chbuff_M919_Total[iNumberOfByteOccupied++] = M919Infor.bMesSta_GPS;
	chbuff_M919_Total[iNumberOfByteOccupied++] = M919Infor.bMesSta_BD2;

	chbuff_M919_Total[iNumberOfByteOccupied++] = M919Infor.bMesSta_GLO;
	chbuff_M919_Total[iNumberOfByteOccupied++] = M919Infor.bMesSta_GAL;

	int iTimeOffset_GPS = M919Infor.iTimeOffset_GPS;
	EditInt32InBuff(iTimeOffset_GPS, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 4;

	int iTimeOffset_BD2 = M919Infor.iTimeOffset_BD2;
	EditInt32InBuff(iTimeOffset_BD2, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 4;

	int iTimeOffset_GLO = M919Infor.iTimeOffset_GLO;
	EditInt32InBuff(iTimeOffset_GLO, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 4;

	int iTimeOffset_GAL = M919Infor.iTimeOffset_GAL;
	EditInt32InBuff(iTimeOffset_GAL, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 4;

	if (iNumberOfByteOccupied != SizeOfRawHeader + iLenth_header919_2)
	{
		//TRACE("\r\nWrong header2 length!\r\n");
		return;
	}

	BYTE bGpsFrqValid[3] = { 0 };
	BYTE bBd2FrqValid[3] = { 0 };
	BYTE bGloFrqValid[3] = { 0 };
	BYTE bGalFrqValid[3] = { 0 };

	BYTE bFrqFlag = M919Infor.bFrqFlag;

	bGpsFrqValid[L1] = (bFrqFlag & 0x01);
	bGpsFrqValid[L2] = ((bFrqFlag & 0x02) >> 1);
	bGpsFrqValid[L5] = ((bFrqFlag & 0x04) >> 2);
	bBd2FrqValid[B1] = ((bFrqFlag & 0x08) >> 3);
	bBd2FrqValid[B2] = ((bFrqFlag & 0x10) >> 4);
	bBd2FrqValid[B3] = ((bFrqFlag & 0x20) >> 5);

	bGloFrqValid[G1] = ((bFrqFlag & 0x40) >> 6);
	bGloFrqValid[G2] = ((bFrqFlag & 0x80) >> 7);

	BYTE bFrqFlagHigh = M919Infor.bFrqFlagHigh;
	bGalFrqValid[E1] = ((bFrqFlagHigh & 0x01));
	bGalFrqValid[E2] = ((bFrqFlagHigh & 0x02) >> 1);
	bGalFrqValid[E5A] = ((bFrqFlagHigh & 0x04) >> 2);
	/**GALILEO;*/

	BYTE bEncodeSatNum = 0;//实际编码的卫星的颗数；20130824；
	BYTE bBdsSatNum = 0;
	BYTE pBdsPrn[TOTAL_CHANNELS] = { 0 };
	/**GPS+BD2 observation infor;*/
	for (int i = 0; i < M919Infor.bSatNo; i++)
	{
		BYTE bPrn = M919Infor.SatObsInfor[i].PRN;

		memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&M919Infor.SatObsInfor[i]), 4);
		iNumberOfByteOccupied += 4;

		chbuff_M919_Total[iNumberOfByteOccupied++] = char(M919Infor.SatObsInfor[i].dwDeltaPhi / 10);	//+-128cm
		chbuff_M919_Total[iNumberOfByteOccupied++] = M919Infor.SatObsInfor[i].bSoc;
		EditWordInBuff(M919Infor.SatObsInfor[i].dwRefPrDiff, chbuff_M919_Total, iNumberOfByteOccupied);
		iNumberOfByteOccupied += 2;

		EditWordInBuff(M919Infor.SatObsInfor[i].dwDx, chbuff_M919_Total, iNumberOfByteOccupied);
		iNumberOfByteOccupied += 2;

		EditWordInBuff(M919Infor.SatObsInfor[i].dwDy, chbuff_M919_Total, iNumberOfByteOccupied);
		iNumberOfByteOccupied += 2;

		EditWordInBuff(M919Infor.SatObsInfor[i].dwDz, chbuff_M919_Total, iNumberOfByteOccupied);
		iNumberOfByteOccupied += 2;

		/**Ver:0x0048，增加网络RTK的残差；*/
		EditWordInBuff(M919Infor.SatObsInfor[i].wSIc, chbuff_M919_Total, iNumberOfByteOccupied);
		iNumberOfByteOccupied += 2;

		if (GNSS_PRN[bPrn] == ALEX_GPS)
		{
			bEncodeSatNum++;

			for (BYTE Lx = L1; Lx <= L5; Lx++)
			{
				if (bGpsFrqValid[Lx] == 1)
				{
					memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&M919Infor.SatObsInfor[i].Lx[Lx]), 4);
					iNumberOfByteOccupied += 4;

					memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Lx].PR_SD_or_Rov_PR)), 8);
					iNumberOfByteOccupied += 8;

					if (M919Infor.bMesSta_GPS == RAW_PR_MODE || M919Infor.bMesSta_GPS == CORRECTED_PR_MODE)
					{
						memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Lx].iCp)), 4);
					}
					else
					{
						memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Lx].CP_SDMinusPR_SD)), 4);
					}

					iNumberOfByteOccupied += 4;
				}
			}
		}
		else if (GNSS_PRN[bPrn] == ALEX_BD2)
		{
			bEncodeSatNum++;
			pBdsPrn[bBdsSatNum++] = bPrn;
			for (BYTE Bx = B1; Bx <= B3; Bx++)
			{
				if (bBd2FrqValid[Bx] == 1)
				{
					memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&M919Infor.SatObsInfor[i].Lx[Bx]), 4);
					iNumberOfByteOccupied += 4;

					memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Bx].PR_SD_or_Rov_PR)), 8);
					iNumberOfByteOccupied += 8;

					if (M919Infor.bMesSta_BD2 == RAW_PR_MODE || M919Infor.bMesSta_BD2 == CORRECTED_PR_MODE)
					{
						memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Bx].iCp)), 4);
					}
					else
					{
						memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Bx].CP_SDMinusPR_SD)), 4);
					}

					iNumberOfByteOccupied += 4;
				}
			}
		}
		else if (GNSS_PRN[bPrn] == ALEX_GLONASS)
		{
			bEncodeSatNum++;

			for (BYTE Gx = G1; Gx <= G2; Gx++)
			{
				if (bGloFrqValid[Gx] == 1)
				{
					memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&M919Infor.SatObsInfor[i].Lx[Gx]), 4);
					iNumberOfByteOccupied += 4;

					memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Gx].PR_SD_or_Rov_PR)), 8);
					iNumberOfByteOccupied += 8;

					if (M919Infor.bMesSta_GLO == RAW_PR_MODE || M919Infor.bMesSta_GLO == CORRECTED_PR_MODE)
					{
						memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Gx].iCp)), 4);
					}
					else
					{
						memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Gx].CP_SDMinusPR_SD)), 4);
					}

					iNumberOfByteOccupied += 4;
				}
			}
		}
		else if(GNSS_PRN[bPrn] == ALEX_GALILEO)
		{
			bEncodeSatNum++;
			for(BYTE Ex = E1; Ex <= E5A; Ex++)
			{
				if(bGalFrqValid[Ex] == 1)
				{
					memcpy(chbuff_M919_Total + iNumberOfByteOccupied,(BYTE *)(&M919Infor.SatObsInfor[i].Lx[Ex]),4);
					iNumberOfByteOccupied += 4;

					memcpy(chbuff_M919_Total + iNumberOfByteOccupied,(BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Ex].PR_SD_or_Rov_PR)),8);
					iNumberOfByteOccupied += 8;

					if(M919Infor.bMesSta_GAL == RAW_PR_MODE || M919Infor.bMesSta_GAL == CORRECTED_PR_MODE)
					{
						memcpy(chbuff_M919_Total + iNumberOfByteOccupied,(BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Ex].iCp)),4);
					}
					else
					{
						memcpy(chbuff_M919_Total + iNumberOfByteOccupied,(BYTE *)(&(M919Infor.SatObsInfor[i].Lx[Ex].CP_SDMinusPR_SD)),4);
					}

					iNumberOfByteOccupied += 4;
				}
			}
		}
	}

	/**卫星数更新为实际编码的卫星数，避免获取数据的卫星数与编码的卫星数不同，对解算和解码数据造成影响；20130824；*/
	M919Infor.bSatNo = bEncodeSatNum;
	chbuff_M919_Total[SizeOfRawHeader] = M919Infor.bSatNo;

	/**Spp-infor;*/
	EditDoubleInBuff(M919Infor.SppRefInfor.dRov_SPP_Time, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	EditDoubleInBuff(M919Infor.SppRefInfor.dRov_SPP_vx, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	EditDoubleInBuff(M919Infor.SppRefInfor.dRov_SPP_vy, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	EditDoubleInBuff(M919Infor.SppRefInfor.dRov_SPP_vz, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	/**Ref-infor;*/
	EditUint32InBuff(M919Infor.SppRefInfor.iRef_ID, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 4;

	EditDoubleInBuff(M919Infor.SppRefInfor.dRef_Pos_Time, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	EditDoubleInBuff(M919Infor.SppRefInfor.dRef_Pos_x, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	EditDoubleInBuff(M919Infor.SppRefInfor.dRef_Pos_y, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	EditDoubleInBuff(M919Infor.SppRefInfor.dRef_Pos_z, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	/**增加北斗时间到GPS时间的转换；*/
	EditDoubleInBuff(M919Infor.dBd2Time2GpsTime, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	/**增加GLONASS时间到GPS时间的小数部分；*/
	EditDoubleInBuff(M919Infor.dGloTime2GpsTime, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	/**增加GALILEO时间到GPS时间的小数部分；*/
	EditDoubleInBuff(M919Infor.dGalTime2GpsTime, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	/**增加各个卫星系统的钟差；20141118；*/
	/**增加GPS钟差；*/
	EditDoubleInBuff(M919Infor.dDtGps, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	/**增加BDS钟差；*/
	EditDoubleInBuff(M919Infor.dDtBds, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	/**增加GLO钟差；*/
	EditDoubleInBuff(M919Infor.dDtGlo, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	/**增加GAL钟差；*/
	EditDoubleInBuff(M919Infor.dDtGal, chbuff_M919_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += 8;

	int iRovSppX, iRovSppY, iRovSppZ;

	iRovSppX = iInt(M919Infor.SppRefInfor.dRov_SPP_x);
	iRovSppY = iInt(M919Infor.SppRefInfor.dRov_SPP_y);
	iRovSppZ = iInt(M919Infor.SppRefInfor.dRov_SPP_z);

	memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&iRovSppX), 4);
	iNumberOfByteOccupied += 4;

	memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&iRovSppY), 4);
	iNumberOfByteOccupied += 4;

	memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)(&iRovSppZ), 4);
	iNumberOfByteOccupied += 4;

	/**lrp@20151118；*/
	chbuff_M919_Total[iNumberOfByteOccupied++] = 0;
	iNumberOfByteOccupied += 3;			//3个保留字节；

#if M919_VERSION >= 0x0054
	/**Edit Imu Data;*/
	memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)&M919Infor.iImuDataValid, 4);
	iNumberOfByteOccupied += 4;

	memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)&M919Infor.iImuDataSts, 4);
	iNumberOfByteOccupied += 4;

	memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)&M919Infor.pdGyro, 24);			//3 double;
	iNumberOfByteOccupied += 24;

	memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)&M919Infor.pdAcc, 24);			//3 double;
	iNumberOfByteOccupied += 24;

	memcpy(chbuff_M919_Total + iNumberOfByteOccupied, (BYTE *)&M919Infor.fImuTemp, 4);
	iNumberOfByteOccupied += 4;
#endif

	/**Confirm and Check;*/
	//if (/*ConfirmMessageLenth(iNumberOfByteOccupied, iRealMesLen + SizeOfRawHeader + 4)*/TRUE)
	if (true)
	{
		CRC = GetCRC(chbuff_M919_Total, iRealMesLen + SizeOfRawHeader);
		EditUint32InBuff(CRC, chbuff_M919_Total, iNumberOfByteOccupied);

		iMesLen = iRealMesLen + SizeOfRawHeader + 4;
	}
}

void CQRtkImpl::EncodeM916_2B(MixEpoch *pM,BYTE* pBuf,UINT32& iMesLen,UINT32 iMsgId)
{
	UINT32  CRC,iPos,iObsNum,iMsgLen;

	RawHeader Hd;

	iMsgLen = 0;

	iPos = SizeOfRawHeader;


	EditWordInBuff(pM->wSize,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->wMsgID,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->wReceiverStatus,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->wFreq,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->wBaseID,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->wRoverID,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->wWeek,pBuf,iPos);    iPos += 2;

	pBuf[iPos++] = pM->bVersion;
	pBuf[iPos++] = pM->bImuValid;

	EditInt32InBuff(pM->lTimeMS,pBuf,iPos);    iPos += 4;
	EditInt32InBuff(pM->lRovOffset,pBuf,iPos);    iPos += 4;
	EditWordInBuff(pM->iEventOffset,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->nObs,pBuf,iPos);    iPos += 2;

	EditWordInBuff(pM->iGPSLag,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->iBDSLag,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->iGloLag,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->iGalLag,pBuf,iPos);    iPos += 2;

	EditWordInBuff(pM->iBaseGpsOffset,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->iBaseBdsOffset,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->iBaseGloOffset,pBuf,iPos);    iPos += 2;
	EditWordInBuff(pM->iBaseGalOffset,pBuf,iPos);    iPos += 2;

	EditDoubleInBuff(pM->dBaseX,pBuf,iPos);    iPos += 8;
	EditDoubleInBuff(pM->dBaseY,pBuf,iPos);    iPos += 8;
	EditDoubleInBuff(pM->dBaseZ,pBuf,iPos);    iPos += 8;

	EditInt32InBuff(pM->lBaseLat,pBuf,iPos);    iPos += 4;
	EditInt32InBuff(pM->lBaseLon,pBuf,iPos);    iPos += 4;
	EditInt32InBuff(pM->lBaseHcm,pBuf,iPos);    iPos += 4;

	EditInt32InBuff(pM->lRoverX,pBuf,iPos);    iPos += 4;
	EditInt32InBuff(pM->lRoverY,pBuf,iPos);    iPos += 4;
	EditInt32InBuff(pM->lRoverZ,pBuf,iPos);    iPos += 4;

	EditInt32InBuff(pM->lVelX,pBuf,iPos);    iPos += 4;
	EditInt32InBuff(pM->lVelY,pBuf,iPos);    iPos += 4;
	EditInt32InBuff(pM->lVelZ,pBuf,iPos);    iPos += 4;

	if(pM->bImuValid == 1){
		EditDoubleInBuff(pM->dImuX,pBuf,iPos);    iPos += 8;
		EditDoubleInBuff(pM->dImuY,pBuf,iPos);    iPos += 8;
		EditDoubleInBuff(pM->dImuZ,pBuf,iPos);    iPos += 8;

		EditDoubleInBuff(pM->dImuVelX,pBuf,iPos);    iPos += 8;
		EditDoubleInBuff(pM->dImuVelY,pBuf,iPos);    iPos += 8;
		EditDoubleInBuff(pM->dImuVelZ,pBuf,iPos);    iPos += 8;

		EditDoubleInBuff(pM->dImuAccX,pBuf,iPos);    iPos += 8;
		EditDoubleInBuff(pM->dImuAccY,pBuf,iPos);    iPos += 8;
		EditDoubleInBuff(pM->dImuAccZ,pBuf,iPos);    iPos += 8;

		EditDoubleInBuff(pM->dImuHeader,pBuf,iPos);    iPos += 8;
		EditDoubleInBuff(pM->dImuPitch,pBuf,iPos);    iPos += 8;
		EditDoubleInBuff(pM->dImuRoll,pBuf,iPos);    iPos += 8;

		EditDoubleInBuff(pM->dImuNonFixTime,pBuf,iPos);    iPos += 8;
		EditDoubleInBuff(pM->dImuLastPosError,pBuf,iPos);    iPos += 8;
		EditDoubleInBuff(pM->dImuLastVelError,pBuf,iPos);    iPos += 8;
	}

	iObsNum =  pM->nObs;

	//-----------------------------------------------------------------------------
	BYTE bGpsFrqValid[3] ={0};
	BYTE bBd2FrqValid[3] ={0};
	BYTE bGloFrqValid[3] ={0};
	BYTE bGalFrqValid[3] ={0};

	UINT16 wFrqFlag = pM->wFreq;

	bGpsFrqValid[L1] = (wFrqFlag & 0x0001);
	bGpsFrqValid[L2] = ((wFrqFlag & 0x0002) >> 1);
	bGpsFrqValid[L5] = ((wFrqFlag & 0x0004) >> 2);

	bBd2FrqValid[B1] = ((wFrqFlag & 0x0008) >> 3);
	bBd2FrqValid[B2] = ((wFrqFlag & 0x0010) >> 4);
	bBd2FrqValid[B3] = ((wFrqFlag & 0x0020) >> 5);

	bGloFrqValid[G1] = ((wFrqFlag & 0x0040) >> 6);
	bGloFrqValid[G2] = ((wFrqFlag & 0x0080) >> 7);
	//?G3

	bGalFrqValid[E1] = ((wFrqFlag & 0x0100) >> 8);
	bGalFrqValid[E2] = ((wFrqFlag & 0x0200) >> 9);
	bGalFrqValid[E5A] = ((wFrqFlag & 0x0400) >> 10);

	UINT32 iGnss;
	BYTE bL1Valid,bL2Valid,bL3Valid;
	BYTE bRefValid,bRovValid,bSlvValid;

	for(int i = 0; i < iObsNum; i++){
		iGnss= GNSS_PRN[pM->pbID[i]];

		switch(iGnss){
		case ALEX_GPS:
			bL1Valid =  bGpsFrqValid[L1];
			bL2Valid =  bGpsFrqValid[L2];
			bL3Valid =  bGpsFrqValid[L5];

			break;
		case ALEX_BD2:
			bL1Valid =  bBd2FrqValid[B1];
			bL2Valid =  bBd2FrqValid[B2];
			bL3Valid =  bBd2FrqValid[B3];

			break;
		case ALEX_GLONASS:
			bL1Valid =  bGloFrqValid[G1];
			bL2Valid =  bGloFrqValid[G2];
			bL3Valid =  bGloFrqValid[G3];

			break;
		case ALEX_GALILEO:
			bL1Valid =  bGalFrqValid[E1];
			bL2Valid =  bGalFrqValid[E2];
			bL3Valid =  bGalFrqValid[E5A];

			break;
		default:
			bL1Valid = 0;
			bL2Valid = 0;
			bL3Valid = 0;

			break;
		}

		bRefValid = (pM->pbFlag[i] & 0x01);
		bRovValid = ((pM->pbFlag[i] & 0x02) >> 1);
		bSlvValid = ((pM->pbFlag[i] & 0x04) >> 2);

		pBuf[iPos++] = pM->pbID[i];
		pBuf[iPos++] = pM->pbFlag[i];
		pBuf[iPos++] = pM->pbElevation[i];


		EditInt16InBuff(pM->piAzimuth[i],pBuf,iPos);    iPos += 2;
		EditInt32InBuff(pM->plClock[i],pBuf,iPos);    iPos += 4;
		EditInt16InBuff(pM->piIonoDelay[i],pBuf,iPos);    iPos += 2;
		EditInt16InBuff(pM->piTropDelay[i],pBuf,iPos);    iPos += 2;

		if(bRefValid > 0){
			if(bL1Valid){
				pBuf[iPos++] = pM->pbBaseM1[i];
				pBuf[iPos++] = pM->pbBaseCN1[i];
				pBuf[iPos++] = pM->pbBaseS1[i];

				pBuf[iPos++] = pM->pbSoc[i];
				pBuf[iPos++] = pM->pbSlc[i];

				EditInt32InBuff(pM->plBaseR1[i],pBuf,iPos);    iPos += 4;
				EditInt32InBuff(pM->plBaseL1[i],pBuf,iPos);    iPos += 4;
			}

			if(bL2Valid){
				pBuf[iPos++] = pM->pbBaseM2[i];
				pBuf[iPos++] = pM->pbBaseCN2[i];
				pBuf[iPos++] = pM->pbBaseS2[i];

				EditInt16InBuff(pM->piBaseR21[i],pBuf,iPos);    iPos += 2;
				EditInt32InBuff(pM->plBaseL2[i],pBuf,iPos);    iPos += 4;
			}

			if(bL3Valid){
				pBuf[iPos++] = pM->pbBaseM5[i];
				pBuf[iPos++] = pM->pbBaseCN5[i];
				pBuf[iPos++] = pM->pbBaseS5[i];

				EditInt16InBuff(pM->piBaseR51[i],pBuf,iPos);    iPos += 2;
				EditInt32InBuff(pM->plBaseL5[i],pBuf,iPos);    iPos += 4;
			}
		}

		if(bRovValid > 0){
			if(bL1Valid){
				pBuf[iPos++] = pM->pbRoverM1[i];
				pBuf[iPos++] = pM->pbRoverCN1[i];
				pBuf[iPos++] = pM->pbRoverS1[i];

				EditInt32InBuff(pM->plRoverR1[i],pBuf,iPos);    iPos += 4;
				EditInt32InBuff(pM->plRoverL1[i],pBuf,iPos);    iPos += 4;
			}

			if(bL2Valid){
				pBuf[iPos++] = pM->pbRoverM2[i];
				pBuf[iPos++] = pM->pbRoverCN2[i];
				pBuf[iPos++] = pM->pbRoverS2[i];

				EditInt16InBuff(pM->piRoverR21[i],pBuf,iPos);    iPos += 2;
				EditInt32InBuff(pM->plRoverL2[i],pBuf,iPos);    iPos += 4;
			}

			if(bL3Valid){
				pBuf[iPos++] = pM->pbRoverM5[i];
				pBuf[iPos++] = pM->pbRoverCN5[i];
				pBuf[iPos++] = pM->pbRoverS5[i];

				EditInt16InBuff(pM->piRoverR51[i],pBuf,iPos);    iPos += 2;
				EditInt32InBuff(pM->plRoverL5[i],pBuf,iPos);    iPos += 4;
			}
		}

		if(bSlvValid > 0){
			if(bL1Valid){
				pBuf[iPos++] = pM->pbSlaveM1[i];
				pBuf[iPos++] = pM->pbSlaveCN1[i];
				pBuf[iPos++] = pM->pbSlaveS1[i];

				EditInt32InBuff(pM->plSlaveR1[i],pBuf,iPos);    iPos += 4;
				EditInt32InBuff(pM->plSlaveL1[i],pBuf,iPos);    iPos += 4;
			}

			if(bL2Valid){
				pBuf[iPos++] = pM->pbSlaveM2[i];
				pBuf[iPos++] = pM->pbSlaveCN2[i];
				pBuf[iPos++] = pM->pbSlaveS2[i];

				EditInt16InBuff(pM->piSlaveR21[i],pBuf,iPos);    iPos += 2;
				EditInt32InBuff(pM->plSlaveL2[i],pBuf,iPos);    iPos += 4;
			}

			if(bL3Valid){
				pBuf[iPos++] = pM->pbSlaveM5[i];
				pBuf[iPos++] = pM->pbSlaveCN5[i];
				pBuf[iPos++] = pM->pbSlaveS5[i];

				EditInt16InBuff(pM->piSlaveR51[i],pBuf,iPos);    iPos += 2;
				EditInt32InBuff(pM->plSlaveL5[i],pBuf,iPos);    iPos += 4;
			}
		}
	}

	iMsgLen = iPos - SizeOfRawHeader;

	//GetNovHeader(iMsgId, iMsgLen, M916VERSIONID, &Hd);
	GetNovHeader(iMsgId,iMsgLen,M916_VERSION,&Hd,pM->wWeek,pM->lTimeMS);
	EditeNovHead(pBuf,Hd);

	//if (/*ConfirmMessageLenth(iPos, iMsgLen + SizeOfRawHeader + 4)*/TRUE)
	if(true)
	{
		CRC = GetCRC(pBuf,iMsgLen + SizeOfRawHeader);
		EditUint32InBuff(CRC,pBuf,iPos);
	}

	iMesLen = iMsgLen + SizeOfRawHeader + 4;
	//return iMesLen;
	return;
}
UINT32 CQRtkImpl::EncodeOneDecEphGlo(UINT16 wWeek_GPS, UINT32 iSecond_GPS, Mes723Infor* pEph,
						BYTE* pMesTotal, const UINT32 KTOTALMESLEN)
{
	RawHeader Header;
	UINT32 CRC, iPos;

	iPos = 0;

	GetNovHeader(723, KTOTALMESLEN - SizeOfRawHeader - 4, 1, &Header, wWeek_GPS, iSecond_GPS);

	EditeNovHead(pMesTotal, Header);
	iPos = 28;

	EditInt16InBuff(pEph->wSloto, pMesTotal, iPos);
	iPos += 2;

	EditInt16InBuff(pEph->wFreqo, pMesTotal, iPos);
	iPos += 2;

	pMesTotal[iPos++] = pEph->bSatType;
	pMesTotal[iPos++] = pEph->bRes1;

	EditInt16InBuff(pEph->wEphWeek, pMesTotal, iPos);
	iPos += 2;

	EditUint32InBuff(pEph->iEphTime, pMesTotal, iPos);
	iPos += 4;

	EditUint32InBuff(pEph->iTimeOffset, pMesTotal, iPos);
	iPos += 4;

	EditInt16InBuff(pEph->wDateNum, pMesTotal, iPos);
	iPos += 2;

	pMesTotal[iPos++] = pEph->bRes2;
	pMesTotal[iPos++] = pEph->bRes3;

	EditUint32InBuff(pEph->iIssue, pMesTotal, iPos);
	iPos += 4;

	EditUint32InBuff(pEph->iHealth, pMesTotal, iPos);
	iPos += 4;

	EditDoubleInBuff(pEph->dPosX, pMesTotal, iPos);
	iPos += 8;

	EditDoubleInBuff(pEph->dPosY, pMesTotal, iPos);
	iPos += 8;

	EditDoubleInBuff(pEph->dPosZ, pMesTotal, iPos);
	iPos += 8;

	EditDoubleInBuff(pEph->dVelX, pMesTotal, iPos);
	iPos += 8;

	EditDoubleInBuff(pEph->dVelY, pMesTotal, iPos);
	iPos += 8;

	EditDoubleInBuff(pEph->dVelZ, pMesTotal, iPos);
	iPos += 8;

	EditDoubleInBuff(pEph->dAccX, pMesTotal, iPos);
	iPos += 8;

	EditDoubleInBuff(pEph->dAccY, pMesTotal, iPos);
	iPos += 8;

	EditDoubleInBuff(pEph->dAccZ, pMesTotal, iPos);
	iPos += 8;

	EditDoubleInBuff(pEph->dTauN, pMesTotal, iPos);
	iPos += 8;

	EditDoubleInBuff(pEph->dDeltaTauN, pMesTotal, iPos);
	iPos += 8;

	EditDoubleInBuff(pEph->dGamma, pMesTotal, iPos);
	iPos += 8;

	EditUint32InBuff(pEph->iTk, pMesTotal, iPos);
	iPos += 4;

	EditUint32InBuff(pEph->iP, pMesTotal, iPos);
	iPos += 4;

	EditUint32InBuff(pEph->iFt, pMesTotal, iPos);
	iPos += 4;

	EditUint32InBuff(pEph->iAge, pMesTotal, iPos);
	iPos += 4;

	EditUint32InBuff(pEph->iFlag, pMesTotal, iPos);
	iPos += 4;

	//if (/*ConfirmMessageLenth(iPos, KTOTALMESLEN)*/TRUE)
	if (true)
	{
		CRC = GetCRC(pMesTotal, KTOTALMESLEN - 4);
		EditUint32InBuff(CRC, pMesTotal, KTOTALMESLEN - 4);

		return 1;
	}

	return 0;
}

UINT32 CQRtkImpl::Encode_BD2DecodedEphInfor(UINT16 wWeek_GPS, UINT32 iSecond_GPS, BYTE *M71_Total,
										ServerEphemeris *EphData71, const UINT32 TotalLength_M71)
{
	RawHeader Header71;
	/**BYTE chbuff_header71[SizeOfRawHeader];	28**/

	UINT32 CRC;
	UINT32 iNumberOfByteOccupied = 0;

	GetNovHeader(71, TotalLength_M71 - SizeOfRawHeader - 4, 1, &Header71, wWeek_GPS, iSecond_GPS);

	EditeNovHead(M71_Total, Header71);

	iNumberOfByteOccupied = SizeOfRawHeader;

	EditWordInBuff(EphData71->wSize, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(unsigned short);

	M71_Total[iNumberOfByteOccupied] = EphData71->blFlag;
	iNumberOfByteOccupied++;

	M71_Total[iNumberOfByteOccupied] = EphData71->bHealth;
	iNumberOfByteOccupied++;

	M71_Total[iNumberOfByteOccupied] = EphData71->ID;
	iNumberOfByteOccupied++;

	/**M71_Total[iNumberOfByteOccupied] = EphData71->bReserved;*/
	iNumberOfByteOccupied++;

	EditWordInBuff(EphData71->uMsgID, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(unsigned short);

	EditInt16InBuff(EphData71->m_wIdleTime, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(short);

	EditInt16InBuff(EphData71->iodc, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(short);

	EditInt16InBuff(EphData71->accuracy, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(short);

	EditWordInBuff(EphData71->week, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(unsigned short);

	EditInt32InBuff(EphData71->iode, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(int);

	EditInt32InBuff(EphData71->tow, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(int);

	EditDoubleInBuff(EphData71->toe, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->toc, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->af2, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->af1, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->af0, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->Ms0, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->deltan, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->es, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->roota, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->omega0, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->i0, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->ws, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->omegaot, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->itoet, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->Cuc, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->Cus, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->Crc, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->Crs, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->Cic, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->Cis, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->tgd, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	EditDoubleInBuff(EphData71->tgd2, M71_Total, iNumberOfByteOccupied);
	iNumberOfByteOccupied += sizeof(double);

	/**memcpy((char *)(M71_Total+SizeOfRawHeader), (char *)((char *)EphData71+iValidID*sizeof(ServerEphemeris)), sizeof(ServerEphemeris));*/
	/**iNumberOfByteOccupied = SizeOfRawHeader + sizeof(ServerEphemeris);*/

	//if (/*ConfirmMessageLenth(iNumberOfByteOccupied, TotalLength_M71)*/TRUE)
    if (true)
	{
		CRC = GetCRC(M71_Total, TotalLength_M71 - 4);
		EditUint32InBuff(CRC, M71_Total, TotalLength_M71 - 4);
		return 1;
	}
	else
	{
		return 0;
	}
}

UINT32 CQRtkImpl::GetM919Info(WangEpoch *pBsEpoch, WangEpoch *pRvEpoch)
{
	M919TotalInfor m919TotalInfo = {0};
	if(!this->CreateM919TotalInfor(pBsEpoch,pRvEpoch,m919TotalInfo)) return -1;
	UINT32 data_length = 0;
	EncodeM919_2B(m919TotalInfo,(BYTE*)m_encodeBuffer,(UINT32&)data_length,919);
	
	FILE *fp;
	fp = fopen("M919.bin", "ab+");
	fwrite(m_encodeBuffer,sizeof(char),data_length,fp);
	fflush(fp);
	fclose(fp);
	return 1;
}

UINT32 CQRtkImpl::GetM916Info(WangEpoch *pBsEpoch,WangEpoch *pRvEpoch)
{
	MixEpoch m916TotalInfo ={0};
	if(!this->CreateM916TotalInfor(pBsEpoch,pRvEpoch,m916TotalInfo)) return -1;
	UINT32 data_length = 0;
	EncodeM916_2B(&m916TotalInfo,(BYTE*)m_encodeBuffer,(UINT32&)data_length,916);

	FILE *fp;
	fp = fopen("M916.bin","ab+");
	fwrite(m_encodeBuffer,sizeof(char),data_length,fp);
	fflush(fp);
	fclose(fp);
	return 1;
}
