#include <math.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
//#include <iostream>

#include "QRTKInterface.h"
#include "RtcmDecode.h"

using namespace std;

extern uint16_t    _constellationMask;
unsigned int counter_epoch[MessageTypeNo] = { 0 };

/*
int iInt(const double d)
{
	return d > 0 ? (long)(d + .5) : (long)(d - .5);
}
*/

int rdn(int y, int m, int d) { /* Rata Die day one is 0001-01-01 */
    if (m < 3)
        y--, m += 12;
    return 365*y + y/4 - y/100 + y/400 + (153*m - 457)/5 + d - 306;
}

int iYMDtoSumDay(int _iYMD)
{
	if (_iYMD == 0) return _iYMD;	// Old version.

	/*_iYMD : yymmdd. 十进制*/
	static int iOldYMD = 99999999;
	static int year = 2100, month = 12, day = 31;
	if (iOldYMD != _iYMD){
		/*计算年、月、日*/
		iOldYMD = _iYMD;

		year = _iYMD / 10000;
		if (year < 80){
			year += 2000;
			_iYMD += 20000000L;
		}
		else if (year < 100){
			year += 1900;
			_iYMD += 19000000L;
		}
		else if (year < 1980) assert(false);//ASSERT(FALSE);//compile error

		month = (_iYMD - year * 10000) / 100;
		day = _iYMD - year * 10000 - month * 100;
		/*#endif*/
	}
	
	//compile error
	//CTime timeToday(year, month, day, 0, 0, 0);
	//CTime timeOrg(1980, 1, 6, 0, 0, 0);

	//CTimeSpan TimeSpan = timeToday - timeOrg;

	//timeSpan TimeSpan = timeToday - timeOrg;
	//return (int)(TimeSpan.GetDays());

	/**lost 2 days
    struct tm timeToday={0};
    timeToday.tm_year = year;
    timeToday.tm_mon = month;
    timeToday.tm_mday = day;
        
	struct tm timeOrg={0};
    timeOrg.tm_year = 1980;
    timeOrg.tm_mon = 1;
    timeOrg.tm_mday = 6;
 
    double secs = difftime(mktime(&timeToday),mktime(&timeOrg));
    static const int secsOneDay = 86400;
    int days = secs/secsOneDay;
	*/

	//printf("[DEBUG]iYMDtoSumDay, today,year:%d,month:%d,day:%d,diff 1980-1-6 days:%d\n", year, month, day,days);
	int days = rdn(year, month, day) - rdn(1980, 1, 6);

    return days;
}

void vGetGlonassUTC(int Tb, int M_Nt, int M_N4, int UTC[])
{
	int year, mon, day, hour, min, sec;
	year = mon = day = hour = min = sec = 0;

	year = M_N4 + 1992;
	if (M_Nt > 366)
	{
		year++;
		M_Nt -= 366;
	}
	for (int y = 0; y < 3; y++)
	{
		if (M_Nt > 365)
		{
			year++;
			M_Nt -= 365;
		}
	}

	int nday[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int ndayr[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	int i = 0;
	if (year % 4 == 0)
	{
		for (i = 0; day < M_Nt; i++)
		{
			day += ndayr[i];
		}
		day -= ndayr[i - 1];
	}
	else
	{
		for (i = 0; day < M_Nt; i++)
		{
			day += nday[i];
		}
		day -= nday[i - 1];
	}

	mon = i;
	day = M_Nt - day;
	hour = floor(Tb / (3600.0));
	min = floor((Tb - hour * 3600.0) / 60.0);
	sec = (Tb - hour * 3600.0 - min * 60.0);
	UTC[0] = year;
	UTC[1] = mon;
	UTC[2] = day;
	UTC[3] = hour;
	UTC[4] = min;
	UTC[5] = sec;
}

extern unsigned int getbitu(const unsigned char *buff, int pos, int len)
{
	unsigned int bits=0;
	int i;
	for (i=pos;i<pos+len;i++) bits=(bits<<1)+((buff[i/8]>>(7-i%8))&1u);
	return bits;
}
extern int getbits(const unsigned char *buff, int pos, int len)
{
	unsigned int bits=getbitu(buff,pos,len);
	if (len<=0||32<=len||!(bits&(1u<<(len-1)))) return (int)bits;
	return (int)(bits|(~0u<<len)); /* extend sign */
}

static double getbits_38(const unsigned char *buff, int pos)
{
	return (double)getbits(buff,pos,32)*64.0+getbitu(buff,pos+32,6);
}


static double getbitg(const unsigned char *buff, int pos, int len)
{
	double value = getbitu(buff, pos + 1, len - 1);
	return getbitu(buff, pos, 1) ? -value : value;
}

/* ssr update intervals ------------------------------------------------------*/
static const double ssrudint[16]={1,2,5,10,15,30,60,120,240,300,600,900,1800,3600,7200,10800};
/********************* signal RINEX code *************************************/
static const char *gps_sig[32]={"","L1C","L1P","L1W","","","","L2C","L2P","L2W","","","","","L2S","L2L","L2X","","","","","L5I","L5Q","L5X","","","","","","","",""};
static const char *glo_sig[32]={"","G1C","G1P","","","","","G2C","G2P","","","","","","","","","","","","","","","","","","","","","","",""};
static const char *gal_sig[32]={"","E1C","E1A","E1B","E1X","E1Z","","E6C","E6A","E6B","E6X","E6Z","","E5BI","E5BQ","E5BX","","E5I","E5Q","E5X","","E5AI","E5AQ","E5AX","","","","","","","",""};
static const char *bds_sig[32]={"","B1I","B1Q","B1X","","","","B3I","B3Q","B3X","","","","B2I","B2Q","B2X","","","","","","","","","","","","","","","",""};
static const char *qzss_sig[32] = {"","L1C","","","","","","","L6S","L6L","L6X","","","","L2S","L2L","L2X","","","","","L5I","L5Q","L5X","","","","","","L1S","L1L","L1X"};
static const unsigned int tbl_CRC24Q[]={
	0x000000,0x864CFB,0x8AD50D,0x0C99F6,0x93E6E1,0x15AA1A,0x1933EC,0x9F7F17,
	0xA18139,0x27CDC2,0x2B5434,0xAD18CF,0x3267D8,0xB42B23,0xB8B2D5,0x3EFE2E,
	0xC54E89,0x430272,0x4F9B84,0xC9D77F,0x56A868,0xD0E493,0xDC7D65,0x5A319E,
	0x64CFB0,0xE2834B,0xEE1ABD,0x685646,0xF72951,0x7165AA,0x7DFC5C,0xFBB0A7,
	0x0CD1E9,0x8A9D12,0x8604E4,0x00481F,0x9F3708,0x197BF3,0x15E205,0x93AEFE,
	0xAD50D0,0x2B1C2B,0x2785DD,0xA1C926,0x3EB631,0xB8FACA,0xB4633C,0x322FC7,
	0xC99F60,0x4FD39B,0x434A6D,0xC50696,0x5A7981,0xDC357A,0xD0AC8C,0x56E077,
	0x681E59,0xEE52A2,0xE2CB54,0x6487AF,0xFBF8B8,0x7DB443,0x712DB5,0xF7614E,
	0x19A3D2,0x9FEF29,0x9376DF,0x153A24,0x8A4533,0x0C09C8,0x00903E,0x86DCC5,
	0xB822EB,0x3E6E10,0x32F7E6,0xB4BB1D,0x2BC40A,0xAD88F1,0xA11107,0x275DFC,
	0xDCED5B,0x5AA1A0,0x563856,0xD074AD,0x4F0BBA,0xC94741,0xC5DEB7,0x43924C,
	0x7D6C62,0xFB2099,0xF7B96F,0x71F594,0xEE8A83,0x68C678,0x645F8E,0xE21375,
	0x15723B,0x933EC0,0x9FA736,0x19EBCD,0x8694DA,0x00D821,0x0C41D7,0x8A0D2C,
	0xB4F302,0x32BFF9,0x3E260F,0xB86AF4,0x2715E3,0xA15918,0xADC0EE,0x2B8C15,
	0xD03CB2,0x567049,0x5AE9BF,0xDCA544,0x43DA53,0xC596A8,0xC90F5E,0x4F43A5,
	0x71BD8B,0xF7F170,0xFB6886,0x7D247D,0xE25B6A,0x641791,0x688E67,0xEEC29C,
	0x3347A4,0xB50B5F,0xB992A9,0x3FDE52,0xA0A145,0x26EDBE,0x2A7448,0xAC38B3,
	0x92C69D,0x148A66,0x181390,0x9E5F6B,0x01207C,0x876C87,0x8BF571,0x0DB98A,
	0xF6092D,0x7045D6,0x7CDC20,0xFA90DB,0x65EFCC,0xE3A337,0xEF3AC1,0x69763A,
	0x578814,0xD1C4EF,0xDD5D19,0x5B11E2,0xC46EF5,0x42220E,0x4EBBF8,0xC8F703,
	0x3F964D,0xB9DAB6,0xB54340,0x330FBB,0xAC70AC,0x2A3C57,0x26A5A1,0xA0E95A,
	0x9E1774,0x185B8F,0x14C279,0x928E82,0x0DF195,0x8BBD6E,0x872498,0x016863,
	0xFAD8C4,0x7C943F,0x700DC9,0xF64132,0x693E25,0xEF72DE,0xE3EB28,0x65A7D3,
	0x5B59FD,0xDD1506,0xD18CF0,0x57C00B,0xC8BF1C,0x4EF3E7,0x426A11,0xC426EA,
	0x2AE476,0xACA88D,0xA0317B,0x267D80,0xB90297,0x3F4E6C,0x33D79A,0xB59B61,
	0x8B654F,0x0D29B4,0x01B042,0x87FCB9,0x1883AE,0x9ECF55,0x9256A3,0x141A58,
	0xEFAAFF,0x69E604,0x657FF2,0xE33309,0x7C4C1E,0xFA00E5,0xF69913,0x70D5E8,
	0x4E2BC6,0xC8673D,0xC4FECB,0x42B230,0xDDCD27,0x5B81DC,0x57182A,0xD154D1,
	0x26359F,0xA07964,0xACE092,0x2AAC69,0xB5D37E,0x339F85,0x3F0673,0xB94A88,
	0x87B4A6,0x01F85D,0x0D61AB,0x8B2D50,0x145247,0x921EBC,0x9E874A,0x18CBB1,
	0xE37B16,0x6537ED,0x69AE1B,0xEFE2E0,0x709DF7,0xF6D10C,0xFA48FA,0x7C0401,
	0x42FA2F,0xC4B6D4,0xC82F22,0x4E63D9,0xD11CCE,0x575035,0x5BC9C3,0xDD8538
};

/* crc-24q parity --------------------------------------------------------------
 * * compute crc-24q parity for sbas, rtcm3
 * * args   : unsigned char *buff I data
 * *          int    len    I      data length (bytes)
 * * return : crc-24Q parity
 * * notes  : see reference [2] A.4.3.3 Parity
 * *-----------------------------------------------------------------------------*/

extern unsigned int crc24q(const unsigned char *buff, int len)
{
	unsigned int crc=0;
	int i;

	for (i=0;i<len;i++) crc=((crc<<8)&0xFFFFFF)^tbl_CRC24Q[(crc>>16)^buff[i]];
	return crc;
}

int decode_msm_head(FILE *m_fp, unsigned char *buff, int sys, int *sync, int *iod, msm_h_t *h, int *hsize, double * tow, int nLeaps)
{
	//msm_h_t h0={0};
	double tod; 
	int i=24,j,dow,mask,staid,type,ncell=0;

	type=getbitu(buff,i,12); i+=12;

	//*h=h0;
	*h={0};

	staid     =getbitu(buff,i,12);       i+=12;

	if (sys==GLO)
	{
		dow   =getbitu(buff,i, 3);       i+= 3;
		tod   =getbitu(buff,i,27)*0.001; i+=27;
		*tow = dow * 86400 + tod - 10800 + nLeaps;
		if (*tow > 604799)
			*tow -= 604800;				
	}
	else if (sys==BDS)
	{
		*tow   =getbitu(buff,i,30)*0.001; i+=30;
		*tow+=14.0; /* BDT -> GPST */
		if (*tow > 604799)
			*tow -= 604800;		
	}
	else 
	{
		*tow   =getbitu(buff,i,30)*0.001; i+=30;
		if (*tow > 604799)
			*tow -= 604800;		
	}
	*sync     =getbitu(buff,i, 1);       i+= 1;
	*iod      =getbitu(buff,i, 3);       i+= 3;
	h->time_s =getbitu(buff,i, 7);       i+= 7;
	h->clk_str=getbitu(buff,i, 2);       i+= 2;
	h->clk_ext=getbitu(buff,i, 2);       i+= 2;
	h->smooth =getbitu(buff,i, 1);       i+= 1;
	h->tint_s =getbitu(buff,i, 3);       i+= 3;
	for (j=1;j<=64;j++) 
	{
		mask=getbitu(buff,i,1); i+=1;
		if (mask)
		{
			h->satmask[j-1] = mask;
			h->sats[h->nsat++]=j;
		}
	}
	for (j=1;j<=32;j++) 
	{
		mask=getbitu(buff,i,1); i+=1;
		if (mask) 
		{
			h->sigmask[j-1] = mask;
			h->sigs[h->nsig++]=j;
		}
	}
	for (j=0;j<h->nsat*h->nsig;j++) 
	{
		h->cellmask[j]=getbitu(buff,i,1); i+=1;
		if (h->cellmask[j]) ncell++;
	}
	*hsize=i;/*header length*/
    //printf("RTK Engine, decode_msm_header nsat:%d,nsig:%d,ncell:%d\n", h->nsat, h->nsig, ncell);
	return ncell;
}

/*content of MSM1 */
/*MSM1 satllite data 10*Nsat(DF398)    signal data 15*Ncell(DF400)    */

/*****************************************************************************************************/

static int decode_msm1(FILE *m_fp, unsigned char *buff,unsigned int Meslen, int sys)
{	
	msm_h_t h={0};
	double pr[64],r_m[64],tow;
	int i,j,sync,iod,ncell,prv,rng_m,RfRm[64],PRs[64];
	const char *sig[32];
	/* decode msm header ****************************************/

	if ((ncell=decode_msm_head(m_fp,buff,sys,&sync,&iod,&h,&i,&tow))<0) return -1;

	for (j=0;j<ncell;j++) pr[j]=-1E16;

	/* decode satellite data ***********************************/
	for (j=0;j<h.nsat;j++) 
	{
		rng_m=getbitu(buff,i,10); i+=10;	
		RfRm[j]=rng_m;			
		r_m[j]=rng_m*P2_10*RANGE_MS;		
	}
	/* decode signal data **************************************/
	for (j=0;j<ncell;j++)     //GNSS signal fine Pseudo ranges 
	{ /* pseudo range */
		prv=getbits(buff,i,15); i+=15;
		if (prv!=-16384) 
		{
			PRs[j]=prv;
			pr[j]=prv*P2_24*RANGE_MS;
		}
	}

	for(i=j=0;i<32;i++)
	{
		if(h.sigmask[i]!=0)
		{
			switch (sys)
			{
				case GPS:sig[j]=gps_sig[i];j++;break;
				case GLO:sig[j]=glo_sig[i];j++;break;
				case GAL:sig[j]=gal_sig[i];j++;break;
				case BDS:sig[j]=bds_sig[i];j++;break;
				default: break;
			}
		}
	}
	fprintf(m_fp,"========================================================\n");
	fprintf(m_fp," PRN |RfRm|RfR Mod value | Sig|  FPRs  |   Pseduorange_i\n");
	fprintf(m_fp,"========================================================\n");
	for(i=j=0;i<32;i++)
	{	
		if(h.nsig==0)
			return sync?0:1;
		else
		{
			int temp =h.sats[i/h.nsig];
			if(temp!=0)
			{
				fprintf(m_fp,"%5d|%4d|%14.6f|%4.4s|",h.sats[i/h.nsig],RfRm[i/h.nsig],r_m[i/h.nsig],sig[i%h.nsig]);	

				if(h.cellmask[i]==0)
				{			
					fprintf(m_fp,"\n");
				}
				else
				{	
					pr[j]+=r_m[i/h.nsig];				
					fprintf(m_fp,"%8d|%16.7f|\n",PRs[j],pr[j]);
					j++;
				}

			 }
		}
	}

	fprintf(m_fp,"------------------------------------------------------------\n\n");
	return sync?0:1;
}

/*content of MSM2 */
static int decode_msm2(FILE *m_fp, unsigned char *buff,unsigned int Meslen, int sys)
{	
	msm_h_t h={0};
	double  cp[64],tow;
	int i,j,sync,iod,ncell,rng_m,cpv,lock[64],half[64],RfRm[64],PhsR[64];
	double r_m[64];	
	const char *sig[32];
	/* decode msm header */
	if ((ncell=decode_msm_head(m_fp,buff,sys,&sync,&iod,&h,&i,&tow))<0) return -1;

	for (j=0;j<ncell;j++)  cp[j]=-1E16;

	/* decode satellite data ***************************/
	for (j=0;j<h.nsat;j++) 
	{
		rng_m=getbitu(buff,i,10); i+=10;
		RfRm[j]=rng_m;			
		r_m[j]=rng_m*P2_10*RANGE_MS;
	}
	/* decode signal data *****************************/
	for (j=0;j<ncell;j++)
	{ /* phase range */
		cpv=getbits(buff,i,22); i+=22;
		if (cpv!=-2097152)
		{
			PhsR[j]=cpv;
			cp[j]=cpv*P2_29*RANGE_MS;
		}
	}
	for (j=0;j<ncell;j++) 
	{ 
		lock[j]=getbitu(buff,i,4); i+=4;
	}
	for (j=0;j<ncell;j++) 
	{
		half[j]=getbitu(buff,i,1); i+=1;
	}
	for(i=j=0;i<32;i++)
	{
		if(h.sigmask[i]!=0)
		{
			switch (sys)
			{
				case GPS:sig[j]=gps_sig[i];j++;break;
				case GLO:sig[j]=glo_sig[i];j++;break;
				case GAL:sig[j]=gal_sig[i];j++;break;
				case BDS:sig[j]=bds_sig[i];j++;break;
				default: break;
			}
		}
	}
	fprintf(m_fp,"=======================================================================\n");
	fprintf(m_fp," PRN |RfRm|RfR Mod value | Sig|Fine PhsR|   Phase Range_i|LTIndc|HAIndc\n");
	fprintf(m_fp,"=======================================================================\n");
	for(i=j=0;i<32;i++)
	{	
		if (h.nsig==0)
			return sync?0:1;
		else
		{
			int temp =h.sats[i/h.nsig];
			if(temp!=0)
			{
				fprintf(m_fp,"%5d|%4d|%14.6f|%4.4s|",h.sats[i/h.nsig],RfRm[i/h.nsig],r_m[i/h.nsig],sig[i%h.nsig]);	

				if(h.cellmask[i]==0)
				{			
					fprintf(m_fp,"\n");
				}
				else
				{					
					cp[j]+=r_m[i/h.nsig];
					fprintf(m_fp,"%9d|%16.7f|%6d|%6d\n",PhsR[j],cp[j],lock[j],half[j]);
					j++;
				}

			}

		}

	}

	fprintf(m_fp,"------------------------------------------------------------------------\n\n");
	return sync?0:1;

}

/*content of MSM3 */
static int decode_msm3(FILE *m_fp, unsigned char *buff,unsigned int Meslen, int sys)
{	
	msm_h_t h={0};
	double pr[64],cp[64],r_m[64],tow;
	int i,j,sync,iod,ncell,rng_m,prv,cpv,lock[64],half[64],RfRm[64],PRs[64],PhsR[64];
		
	const char *sig[32];
	/* decode msm header */
	if ((ncell=decode_msm_head(m_fp,buff,sys,&sync,&iod,&h,&i,&tow))<0) return -1;

	for (j=0;j<ncell;j++) pr[j]=cp[j]=-1E16;

	/* decode satellite data */
	for (j=0;j<h.nsat;j++) 
	{
		rng_m=getbitu(buff,i,10); i+=10;
		RfRm[j]=rng_m;			
		r_m[j]=rng_m*P2_10*RANGE_MS;
	}
	/* decode signal data */
	for (j=0;j<ncell;j++) 
	{ /* pseudorange */
		prv=getbits(buff,i,15); i+=15;
		if (prv!=-16384) 
		{
			PRs[j]=prv;
			pr[j]=prv*P2_24*RANGE_MS;
		}
	}
	for (j=0;j<ncell;j++)
	{ /* phaserange */
		cpv=getbits(buff,i,22); i+=22;
		if (cpv!=-2097152)
		{
			PhsR[j]=cpv;
			cp[j]=cpv*P2_29*RANGE_MS;
		}
	}
	for (j=0;j<ncell;j++) 
	{ /* lock time */
		lock[j]=getbitu(buff,i,4); i+=4;
	}
	for (j=0;j<ncell;j++) 
	{ /* half-cycle ambiguity */
		half[j]=getbitu(buff,i,1); i+=1;
	}
	for(i=j=0;i<32;i++)
	{
		if(h.sigmask[i]!=0)
		{
			switch (sys)
			{
			case GPS:sig[j]=gps_sig[i];j++;break;
			case GLO:sig[j]=glo_sig[i];j++;break;
			case GAL:sig[j]=gal_sig[i];j++;break;
			case BDS:sig[j]=bds_sig[i];j++;break;
			default: break;
			}
		}
	}
	fprintf(m_fp,"==============================================================================================================\n");
	fprintf(m_fp," PRN |RfRm|RfR Mod value | Sig|  FPRs  |Fine PhsR|   Pseduorange_i|   Phase Range_i| PR - CrPhsR|LTIndc|HAIndc\n");
	fprintf(m_fp,"==============================================================================================================\n");
	for(i=j=0;i<32;i++)
	{	
		if(h.nsig==0)
			return sync?0:1;
		else
		{
			int temp =h.sats[i/h.nsig];
			if(temp!=0)
			{
				fprintf(m_fp,"%5d|%4d|%14.6f|%4.4s|",h.sats[i/h.nsig],RfRm[i/h.nsig],r_m[i/h.nsig],sig[i%h.nsig]);	

				if(h.cellmask[i]==0)
				{			
					fprintf(m_fp,"\n");
				}
				else
				{	
					pr[j]+=r_m[i/h.nsig];
					cp[j]+=r_m[i/h.nsig];
					fprintf(m_fp,"%8d|%9d|%16.7f|%16.7f|%12.6f|%6d|%6d\n",PRs[j],PhsR[j],pr[j],cp[j],pr[j]-cp[j],lock[j],half[j]);
					j++;
				}

			}

		}		

	}

	fprintf(m_fp,"-----------------------------------------------------------------------------------------------------------\n\n");
	return sync?0:1;

}

/**************************************************************************************************************************/


/*content of MSM4 */
int decode_msm4(FILE *m_fp, unsigned char *buff, unsigned int Mes_type, unsigned int Meslen, int sys, WangEpoch *pEpoch, int nLeaps)
{
	msm_h_t h={0};
	double r[64],pr[64],cp[64],cnr[64],r_m[64],tow;
	int i,j,sync,iod,ncell,rng,rng_m,prv,cpv,lock[64],half[64],RfR[64],RfRm[64],PRs[64],PhsR[64];

	const char *sig[32];

	/* decode msm header */
	if ((ncell=decode_msm_head(m_fp,buff,sys,&sync,&iod,&h,&i,&tow,nLeaps))<0) return -1;

    if(sys == GPS && !(_constellationMask&RTK_GNSS_CONSTELLATION_GPS) ){
        return sync;
    }

    if(sys == GAL && !(_constellationMask&RTK_GNSS_CONSTELLATION_GALILEO) ){
        return sync;
    }

    if(sys == GLO && !(_constellationMask&RTK_GNSS_CONSTELLATION_GLONASS) ){
        return sync;
    }

    if(sys == BDS && !(_constellationMask&RTK_GNSS_CONSTELLATION_BEIDOU) ){
        return sync;
    }


	//auto dInt = [](double t){ t = t*1000; return (t > 0? t+0.5: t=0.5);};
	long long itow = dInt2(tow);
	long long it = dInt2(pEpoch->t);
	//if ((pEpoch->t != 0) && (pEpoch->t != tow))
	if ((it != 0) && (it != itow))
	{
		//ASSERT(FALSE);//compile error
		//assert(false);
		//TRACE("decode_msm4 time error!\n");
		return -2;
	}

	pEpoch->t = tow;

	for (j=0;j<h.nsat;j++) r[j]=0.0;
	for (j=0;j<ncell;j++) pr[j]=cp[j]=-1E16;

	/* decode satellite data */
	for (j=0;j<h.nsat;j++) 
	{ /* range */
		rng  =getbitu(buff,i, 8); i+= 8;
		if (rng!=255)
		{
			RfR[j]=rng;
			r[j]=rng*RANGE_MS;
		}
	}
	for (j=0;j<h.nsat;j++) 
	{
		rng_m=getbitu(buff,i,10); i+=10;
		if (r[j]!=0.0) 
		{
			RfRm[j]=rng_m;			
			r_m[j]=rng_m*P2_10*RANGE_MS;
		}
	}
	/* decode signal data */
	for (j=0;j<ncell;j++) 
	{ /* pseudorange */
		prv=getbits(buff,i,15); i+=15;
		if (prv!=-16384) 
		{
			PRs[j]=prv;
			pr[j]=prv*P2_24*RANGE_MS;
		}
	}
	for (j=0;j<ncell;j++)
	{ /* phaserange */
		cpv=getbits(buff,i,22); i+=22;
		if (cpv!=-2097152)
		{
			PhsR[j]=cpv;
			cp[j]=cpv*P2_29*RANGE_MS;
		}
	}
	for (j=0;j<ncell;j++) 
	{ /* lock time */
		lock[j]=getbitu(buff,i,4); i+=4;
		/*lock[j] = pow(2, lock[j]) * 16;*/
	}
	for (j=0;j<ncell;j++) 
	{ /* half-cycle ambiguity */
		half[j]=getbitu(buff,i,1); i+=1;
	}
	for (j=0;j<ncell;j++) 
	{ /* cnr */
		cnr[j]=getbitu(buff,i,6)*1.0; i+=6;
	}

	for(i=j=0;i<32;i++)
	{
		if(h.sigmask[i]!=0)
		{
			switch (sys)
			{
				case GPS:sig[j]=gps_sig[i];j++;break;
				case GLO:sig[j]=glo_sig[i];j++;break;
				case GAL:sig[j]=gal_sig[i];j++;break;
				case BDS:sig[j]=bds_sig[i];j++;break;
				default: break;
			}
		}
	}
	/*****************************************************************************************************************************************/
	if (m_fp)
	{
		fprintf(m_fp, "===================================================================================================================================\n");
		fprintf(m_fp, " PRN |RfR| RfR Int value|RfRm|RfR Mod value | Sig|  FPRs  |Fine PhsR|   Pseduorange_i|   Phase Range_i| PR - CrPhsR|LTIndc|HAIndc| CNR\n");
		fprintf(m_fp, "===================================================================================================================================\n");
	}

	for (i = j = 0; i < h.nsat * h.nsig; i++)
	{
		if(h.nsig==0) return sync;
		else
		{
			int temp =h.sats[i/h.nsig];
			if(temp!=0)
			{
				if (m_fp)
					fprintf(m_fp,"%5d|%3d|%14.3f|%4d|%14.6f|%4.4s|",h.sats[i/h.nsig],RfR[i/h.nsig],r[i/h.nsig],RfRm[i/h.nsig],r_m[i/h.nsig],sig[i%h.nsig]);	

				if(h.cellmask[i]==0)
				{
					if (m_fp)
						fprintf(m_fp,"\n");
				}
				else
				{
					if (r[i / h.nsig] == 0.0)
					{
						j++;
						continue;
					}

					pr[j]+=(r[i/h.nsig]+r_m[i/h.nsig]);//r[i/h.nsig]+r_m[i/h.nsig]是变化量
					cp[j]+=(r[i/h.nsig]+r_m[i/h.nsig]);
					if (m_fp)
						fprintf(m_fp,"%8d|%9d|%16.7f|%16.7f|%12.6f|%6d|%6d|%4d\n",PRs[j],PhsR[j],pr[j],cp[j],pr[j]-cp[j],lock[j],half[j],int(cnr[j]));

					int iIndex, iType;
					int id = h.sats[i / h.nsig];
					switch (sys)
					{
					case GPS:id = id + MIN_GPS_PRN - 1; break;
					case GLO:id = id + MIN_GLO_PRN - 1; break;
					case GAL:id = id + MIN_GAL_PRN - 1; break;
					case BDS:id = id + MIN_BD2_PRN - 1; break;
					//default: break;
					default: continue;
					}
					if (bFindObsIndex(pEpoch, id, sig[i%h.nsig], iIndex, iType))
					{
						int glo_offset[24] = {
							+1, -4, +5, +6, +1, -4, +5, +6, -2, -7, +0, -1,
							-2, -7, +0, -1, +4, -3, +3, +2, +4, -3, +3, +2
						};
						double dWaveLength;
						int nlock = (int)tow * 32;
						nlock &= 0x001FFFFF;
						switch (iType)
						{
						case 0:
						    dWaveLength = (LIGHT / GPS_F1);
							/*L1 phase lock loop.*/
							if (MIN_GPS_PRN <= id && id <= MAX_GPS_PRN)
								dWaveLength = (LIGHT / GPS_F1);
							else if (MIN_GLO_PRN <= id && id <= MAX_GLO_PRN)
								dWaveLength = (LIGHT / (GLO_L1_BASE_FRE + glo_offset[id - MIN_GLO_PRN] * GLO_L1_SUB_BAND));
							else if (MIN_BD2_PRN <= id && id <= MAX_BD2_PRN)
								dWaveLength = (LIGHT / BD2_F1);
							else if (MIN_GAL_PRN <= id && id <= MAX_GAL_PRN)
								dWaveLength = (LIGHT / GAL_F1);

							pEpoch->pbMode1[iIndex] = 1;
							pEpoch->pdL1[iIndex] = cp[j] / dWaveLength;
							pEpoch->pdR1[iIndex] = pr[j];
							pEpoch->pbSN1[iIndex] = int(cnr[j]);
							pEpoch->pdwSlip1[iIndex] = lock[j];
							pEpoch->pdwSlip1[iIndex] = nlock + id * 32;
							//fprintf(stderr, "L1 phase Index:%d,pdL1:%f,pdR1:%f,pbSN1:%d,pbwSlip1,%d\n", iIndex, pEpoch->pdL1[iIndex],pEpoch->pdR1[iIndex],pEpoch->pbSN1[iIndex],pEpoch->pdwSlip1[iIndex] );
							break;
						case 1:
							/*L2 phase lock loop.*/
							/*if (pEpoch->pdL1[iIndex] == 0.0 || pEpoch->pdR1[iIndex] == 0.0)
 * 							{
 * 								pEpoch->pbID[iIndex] = 0;
 * 								pEpoch->n--;
 * 							}
 * 							else*/
							{
						        dWaveLength = (LIGHT / GPS_F2);
								if (MIN_GPS_PRN <= id && id <= MAX_GPS_PRN)
									dWaveLength = (LIGHT / GPS_F2);
								else if (MIN_GLO_PRN <= id && id <= MAX_GLO_PRN)
									dWaveLength = (LIGHT / (GLO_L2_BASE_FRE + glo_offset[id - MIN_GLO_PRN] * GLO_L2_SUB_BAND));
								else if (MIN_BD2_PRN <= id && id <= MAX_BD2_PRN)
									dWaveLength = (LIGHT / BD2_F2);
							    else if (MIN_GAL_PRN <= id && id <= MAX_GAL_PRN)
								    dWaveLength = (LIGHT / GAL_F2);

								pEpoch->pbMode2[iIndex] = 1;
								pEpoch->pdL2[iIndex] = cp[j] / dWaveLength;
								pEpoch->pdP2[iIndex] = pr[j];
								pEpoch->pbSN2[iIndex] = int(cnr[j]);
								pEpoch->pdwSlip2[iIndex] = lock[j];
								pEpoch->pdwSlip2[iIndex] = nlock + (id)* 32;
								//fprintf(stderr, "L2 phase Index:%d,pdL2:%f,pdP2:%f,pbSN2:%d,pbwSlip2,%d\n", iIndex, pEpoch->pdL2[iIndex],pEpoch->pdP2[iIndex],pEpoch->pbSN2[iIndex],pEpoch->pdwSlip2[iIndex] );
							}
							break;
						case 2:
							/* L5 phase lock loop.*/
							/*if (MIN_GPS_PRN <= id && id <= MAX_GPS_PRN)
 * 								dWaveLength = (LIGHT / GPS_F5);
 * 							else if (MIN_GLO_PRN <= id && id <= MAX_GLO_PRN)
 * 								dWaveLength = (LIGHT / (GLO_L1_BASE_FRE + glo_offset[id - MIN_GLO_PRN] * GLO_L1_SUB_BAND));
 * 							else if (MIN_BD2_PRN <= id && id <= MAX_BD2_PRN)
 * 								dWaveLength = (LIGHT / BD2_F3);
 * 							pEpoch->pbMode5[iIndex] = 1;
 *							pEpoch->pdL5[iIndex] = cp[j] / dWaveLength;
 * 							pEpoch->pdR5[iIndex] = pr[j];
 * 							pEpoch->pbSN5[iIndex] = int(cnr[j]);
 * 							pEpoch->pdwSlip5[iIndex] = lock[j];
 * 							pEpoch->pdwSlip5[iIndex] = nlock + (id) * 32;*/
							break;
						default:
							break;
						}
					}
					j++;
				}

			}
		}
	}

	if (m_fp)
		fprintf(m_fp,"--------------------------------------------------------------------------------------------------------------------------------------\n\n");


	return sync;

}

/*content of MSM5 */

 int decode_msm5(FILE *m_fp, unsigned char *buff, unsigned int Mes_type, unsigned int Meslen, int sys)
{	
	msm_h_t h={0};
	double r[64],rr[64],pr[64],cp[64],rrf[64],cnr[64],r_m[64],tow;
	int i,j,type,sync,iod,ncell,rng,rng_m,rate,prv,cpv,rrv,lock[64];
	int ex[64],half[64],RfR[64],RfRm[64],PRs[64],PhsR[64];
	const char *sig[32];
	type=getbitu(buff,24,12);

	/* decode msm header */
	if ((ncell=decode_msm_head(m_fp,buff,sys,&sync,&iod,&h,&i,&tow))<0) return -1;

	/*r:range   rr:phase-range rate  */
	for (j=0;j<h.nsat;j++)
	{
		r[j]=rr[j]=0.0; ex[j]=15;
	}
	for (j=0;j<ncell;j++) pr[j]=cp[j]=rrf[j]=-1E16;

	/* decode satellite data */
	for (j=0;j<h.nsat;j++) /* range */
	{ 
		rng  =getbitu(buff,i, 8); i+= 8;
		if (rng!=255)
		{
			RfR[j]=rng;
			r[j]=rng*RANGE_MS;
		}
	}
	for (j=0;j<h.nsat;j++) /* extended info */
	{ 
		ex[j]=getbitu(buff,i, 4); i+= 4;//GLONASS Satellite Frequency Channel Number  is used as extended satellite information  
	}
	for (j=0;j<h.nsat;j++) 
	{
		rng_m=getbitu(buff,i,10); i+=10;
		if (r[j]!=0.0) 
		{
			RfRm[j]=rng_m;		
			r_m[j]=rng_m*P2_10*RANGE_MS;
		}		

	}
	for (j=0;j<h.nsat;j++)
	{ /* phase-range-rate */
		rate =getbits(buff,i,14); i+=14;
		if (rate!=-8192) 
		{
			rr[j]=rate*1.0;
		}
	}

	/* decode signal data */
	for (j=0;j<ncell;j++) 
	{ /* pseudorange */
		prv=getbits(buff,i,15); i+=15;
		if (prv!=-16384) 
		{
			PRs[j]=prv;
			pr[j]=prv*P2_24*RANGE_MS;
		}

	}
	for (j=0;j<ncell;j++) 
	{ /* phase-range */
		cpv=getbits(buff,i,22); i+=22;
		if (cpv!=-2097152) 
		{
			PhsR[j]=cpv;
			cp[j]=cpv*P2_29*RANGE_MS;
		}
	}
	for (j=0;j<ncell;j++)
	{ /* lock time */
		lock[j]=getbitu(buff,i,4); i+=4;
	}
	for (j=0;j<ncell;j++) 
	{ /* half-cycle ambiguity */
		half[j]=getbitu(buff,i,1); i+=1;
	}
	for (j=0;j<ncell;j++) 
	{ /* cnr */
		cnr[j]=getbitu(buff,i,6)*1.0; i+=6;
	}
	for (j=0;j<ncell;j++)
	{ /* phase range-rate */
		rrv=getbits(buff,i,15); i+=15;
		if (rrv!=-16384) rrf[j]=rrv*0.0001;
	}
	for(i=j=0;i<32;i++)
	{
		if(h.sigmask[i]!=0)
		{
			switch (sys)
			{
				case GPS:sig[j]=gps_sig[i];j++;break;
				case GLO:sig[j]=glo_sig[i];j++;break;
				case GAL:sig[j]=gal_sig[i];j++;break;
				case BDS:sig[j]=bds_sig[i];j++;break;
				default: break;
			}
		}
	}

	if (counter_epoch[Mes_type] == 0)
	{

		fprintf(m_fp, "=====================================================================\n");
		fprintf(m_fp, "Epoch Time| PRN | Sig|   Pseduorange_i|   Phase Range_i| PR - CrPhsR|\n");
		fprintf(m_fp, "=====================================================================\n");

	}
	for (i = j = 0; i<32; i++)
	{
		if (h.nsig == 0)
			return sync ? 0 : 1;
		else
		{
			int temp = h.sats[i / h.nsig];
			if (temp != 0)
			{
				fprintf(m_fp, "%10.3f|%5d|%4.4s|", tow,h.sats[i / h.nsig], sig[i%h.nsig]);

				if (h.cellmask[i] == 0)
				{
					fprintf(m_fp, "\n");
				}
				else
				{
					pr[j] += (r[i / h.nsig] + r_m[i / h.nsig]);
					cp[j] += (r[i / h.nsig] + r_m[i / h.nsig]);
					fprintf(m_fp, "%16.7f|%16.7f|%12.6f|\n", pr[j], cp[j], pr[j] - cp[j]);
					j++;
				}

			}

		}

	}


	return sync?0:1;

}

/*content of MSM6 */
static int decode_msm6(FILE *m_fp, unsigned char *buff,unsigned int Meslen, int sys)
{
	msm_h_t h={0};
	double r[64],r_m[64],pr[64],cp[64],cnr[64],tow;
	int i,j,type,sync,iod,ncell,rng,rng_m,prv,cpv,lock[64],half[64],RfR[64],RfRm[64],PRs[64],PhsR[64];
	const char *sig[32];
	type=getbitu(buff,24,12);

	/* decode msm header */
	if ((ncell=decode_msm_head(m_fp,buff,sys,&sync,&iod,&h,&i,&tow))<0) return -1;
	for (j=0;j<h.nsat;j++) r[j]=0.0;
	for (j=0;j<ncell;j++) pr[j]=cp[j]=-1E16;

	/* decode satellite data */
	for (j=0;j<h.nsat;j++)
	{ /* range */
		rng  =getbitu(buff,i, 8); i+= 8;
		if (rng!=255)
		{
			RfR[j]=rng;
			r[j]=rng*RANGE_MS;
		}		

	}
	for (j=0;j<h.nsat;j++)
	{
		rng_m=getbitu(buff,i,10); i+=10;
		if (r[j]!=0.0) 
		{
			RfRm[j]=rng_m;
			r_m[j]=rng_m*P2_10*RANGE_MS;
		}
	}
	/* decode signal data */
	for (j=0;j<ncell;j++) 
	{ /* pseudorange */
		prv=getbits(buff,i,20); i+=20;
		if (prv!=-524288)
		{
			PRs[j]=prv;
			pr[j]=prv*P2_29*RANGE_MS;
		}

	}
	for (j=0;j<ncell;j++) 
	{ /* phaserange */
		cpv=getbits(buff,i,24); i+=24;
		if (cpv!=-8388608) 
		{
			PhsR[j]=cpv;
			cp[j]=cpv*P2_31*RANGE_MS;
		}
	}
	for (j=0;j<ncell;j++) 
	{ /* lock time */
		lock[j]=getbitu(buff,i,10); i+=10;
	}
	for (j=0;j<ncell;j++)
	{ /* half-cycle ambiguity */
		half[j]=getbitu(buff,i,1); i+=1;
	}
	for (j=0;j<ncell;j++)
	{ /* cnr */
		cnr[j]=getbitu(buff,i,10)*0.0625; i+=10;//scale is 2^-4=0.0625;
	}
	for(i=j=0;i<32;i++)
	{
		if(h.sigmask[i]!=0)
		{
			switch (sys)
			{
				case GPS:sig[j]=gps_sig[i];j++;break;
				case GLO:sig[j]=glo_sig[i];j++;break;
				case GAL:sig[j]=gal_sig[i];j++;break;
				case BDS:sig[j]=bds_sig[i];j++;break;
				default: break;
			}
		}
	}
	fprintf(m_fp,"======================================================================================================================================\n");
	fprintf(m_fp," PRN |RfR| RfR Int value|RfRm|RfR Mod value | Sig|   FPRs |Fine PhsR|   Pseduorange_i|   Phase Range_i| PR - CrPhsR|LTIndc|HAIndc| CNR\n");
	fprintf(m_fp,"======================================================================================================================================\n");
	for(i=j=0;i<32;i++)
	{	
		if(h.nsig==0)
			return sync?0:1;
		else		
		{
			int temp =h.sats[i/h.nsig];
			if(temp!=0)
			{
				fprintf(m_fp,"%5d|%3d|%14.3f|%4d|%14.6f|%4.4s|",h.sats[i/h.nsig],RfR[i/h.nsig],r[i/h.nsig],RfRm[i/h.nsig],r_m[i/h.nsig],sig[i%h.nsig]);	

				if(h.cellmask[i]==0)
				{			
					fprintf(m_fp,"\n");
				}
				else
				{	
					pr[j]+=(r[i/h.nsig]+r_m[i/h.nsig]);
					cp[j]+=(r[i/h.nsig]+r_m[i/h.nsig]);
					fprintf(m_fp,"%8d|%9d|%16.7f|%16.7f|%12.6f|%6d|%6d|%4d\n",PRs[j],PhsR[j],pr[j],cp[j],pr[j]-cp[j],lock[j],half[j],int(cnr[j]));
					j++;
				}
			}
		}		
		
	}

	fprintf(m_fp,"--------------------------------------------------------------------------------------------------------------------------------------\n\n");

	return sync?0:1;
}

/*content of MSM7 */
int decode_msm7(FILE *m_fp, unsigned char *buff, unsigned int Mes_type, unsigned int Meslen, int sys, WangEpoch *pEpoch, int nLeaps)
{
	msm_h_t h={0};
	double r[64],pr[64],cp[64],cnr[64],r_m[64],rr[64],rrf[64],tow;
	int i,j,type,sync,iod,ncell,rng,rng_m,prv,cpv,lock[64],half[64],RfR[64],RfRm[64],PRs[64],PhsR[64];

	const char *sig[32];

	type=getbitu(buff,24,12);

	/* decode msm header */
	if ((ncell=decode_msm_head(m_fp,buff,sys,&sync,&iod,&h,&i,&tow, LEAP_SECONDS))<0) return -1;

    if(sys == GPS && !(_constellationMask&RTK_GNSS_CONSTELLATION_GPS) ){
        return sync;
    }

    if(sys == GAL && !(_constellationMask&RTK_GNSS_CONSTELLATION_GALILEO) ){
        return sync;
    }

    if(sys == GLO && !(_constellationMask&RTK_GNSS_CONSTELLATION_GLONASS) ){
        return sync;
    }

    if(sys == BDS && !(_constellationMask&RTK_GNSS_CONSTELLATION_BEIDOU) ){
        return sync;
    }

    if (i+h.nsat*36+ncell*80>((int)Meslen*8)) {
        printf(" msm7%d length error: nsat=%d ncell=%d len=%d\n",type, h.nsat, ncell,Meslen);
        return -1;
    }

	long long itow = dInt2(tow);
	long long it = dInt2(pEpoch->t);
	if ((it != 0) && (it != itow))
	{
		return -2;
	}

	pEpoch->t = tow;

	/* decode satellite data */
    int extPos = i + h.nsat*8;
    int rng_mPos = extPos + h.nsat*4;
    int ratePos = rng_mPos + h.nsat*10;
    //int j;
	for (j = 0;j < h.nsat; j++)
	{ 
		r[j]=rr[j]=0.0; //ex[j]=15;
		//r[j]=0.0;

        /* range */
		int rng  =getbitu(buff, i, 8); i+= 8;
		if (rng!=255) 
		{
			RfR[j]=rng;
			r[j]=rng*RANGE_MS;
		}

	    /* extended info */
		//ex[j]=getbitu(buff,i, 4); i+= 4;
		getbitu(buff,extPos, 4); extPos += 4;

		int rng_m=getbitu(buff,rng_mPos,10); rng_mPos += 10;
		if (r[j]!=0.0)
		{
			RfRm[j]=rng_m;
			r_m[j]=rng_m*P2_10*RANGE_MS;
		}

        /*rough phase range rate */
		//rate =getbits(buff,i,14); i+=14;
		//if (rate!=-8192) rr[j]=rate*1.0;

		//getbits(buff,ratePos,14); ratePos += 14;
		rr[j] = getbits(buff,ratePos,14)*1.0; ratePos += 14;
    }
    i = ratePos;

	/* decode signal data */
    int phPos = i + ncell*20;
    int lckPos = phPos  + ncell*24;
    int cylPos = lckPos + ncell*10;
    int cnrPos = cylPos + ncell;
    int fRatePos = cnrPos + ncell*10;
    //for (j=0;j<ncell;j++) pr[j]=cp[j]=rrf[j]=-1E16;
	for (j = 0;j < ncell;j++){
        pr[j] = cp[j] = -1E16;

	    /* pseudo range */
		int prv = getbits(buff, i, 20); i += 20;
		if (prv != -524288)
		{
			PRs[j]=prv;
			pr[j]=prv*P2_29*RANGE_MS;
		}
	    
        /* phase-range */
		int cpv=getbits(buff,phPos,24); phPos += 24;
		if (cpv!=-8388608) 
		{
			PhsR[j]=cpv;
			cp[j]=cpv*P2_31*RANGE_MS;
		}
	    
        /* lock time */
		lock[j]=getbitu(buff,lckPos ,10); lckPos += 10;
	    
        /* half-cycle amiguity */
		half[j]=getbitu(buff,cylPos ,1); cylPos += 1;
	    
        /* cnr */
		cnr[j]=getbitu(buff,cnrPos ,10)*0.0625; cnrPos += 10;
	    
        /* fine phase range-rate */
		//rrv=getbits(buff,i,15); i+=15;
		//if (rrv!=-16384) rrf[j]=rrv*0.0001;

		//getbits(buff,fRatePos ,15); fRatePos +=15;
		//2020-11-24 calc doppler
		rrf[j]=getbits(buff,fRatePos ,15)*0.0001; fRatePos +=15;

	}
    //printf("RTK Engine,Decode msm7   ----10\n");
    //int i;
	for(i = j = 0; i < 32 ;i++)
	{
		if(h.sigmask[i]!=0)
		{
			switch (sys)
			{
				case GPS:	sig[j]	=	gps_sig[i];		j++;	break;
				case GLO:	sig[j]	=	glo_sig[i];		j++;	break;
				case GAL:	sig[j]	=	gal_sig[i];		j++;	break;
				case BDS:	sig[j]	=	bds_sig[i];		j++;	break;					
				case QZSS:	sig[j]	=	qzss_sig[i];	j++;	break;

				default:break;
			}
		}
	}

    //printf("RTK Engine,Decode msm7   ----11\n");
	/********************************************原打印结果*************************************************/
    /*
	fprintf(m_fp,"=================================================================================================================================================================\n");
	fprintf(m_fp," PRN |RfR| RfR Int value|EX_IN|RfRm|RfR Mod value |R_Phsr_R| Sig|  FPRs  |Fine PhsR|   Pseduorange_i|   Phase Range_i| PR - CrPhsR|LTIndc|HAIndc| CNR|F_Phsr_rate\n");
	fprintf(m_fp,"=================================================================================================================================================================\n");
	for(i=j=0;i<32;i++)
	{	
		if(h.nsig==0)
			return sync?0:10;
		else
		{
			int temp =h.sats[i/h.nsig];
			if(temp!=0)
			{
				fprintf(m_fp,"%5d|%3d|%14.3f|%5d|%4d|%14.6f|%8d|%4.4s|",h.sats[i/h.nsig],RfR[i/h.nsig],r[i/h.nsig],ex[i/h.nsig],RfRm[i/h.nsig],r_m[i/h.nsig],int(rr[i/h.nsig]),sig[i%h.nsig]);	

				if(h.cellmask[i]==0)
				{			
					fprintf(m_fp,"\n");
				}
				else
				{	
					pr[j]+=(r[i/h.nsig]+r_m[i/h.nsig]);
					cp[j]+=(r[i/h.nsig]+r_m[i/h.nsig]);
					fprintf(m_fp,"%8d|%9d|%16.7f|%16.7f|%12.6f|%6d|%6d|%4d|%7.4f\n",PRs[j],PhsR[j],pr[j],cp[j],pr[j]-cp[j],lock[j],half[j],int(cnr[j]),rrf[j]);
					j++;
				}

			}

		}
	
	}
    fprintf(m_fp,"-----------------------------------------------------------------------------------------------------------------------------------------------------------------\n\n");
	return sync?0:1;
    */
	/**************************************************************************************************************/
    /*	
    if (m_fp)
	{
		fprintf(m_fp, "===================================================================================================================================\n");
		fprintf(m_fp, " PRN |RfR| RfR Int value|RfRm|RfR Mod value | Sig|  FPRs  |Fine PhsR|   Pseduorange_i|   Phase Range_i| PR - CrPhsR|LTIndc|HAIndc| CNR\n");
		fprintf(m_fp, "===================================================================================================================================\n");
	}
    */
    
	//LOG_DEBUG("===================================================================================================================================\n");
	//LOG_DEBUG(" PRN |RfR| RfR Int value|RfRm|RfR Mod value | Sig|  FPRs  |Fine PhsR|   Pseduorange_i|   Phase Range_i| PR - CrPhsR|LTIndc|HAIndc| CNR\n");
	//LOG_DEBUG("===================================================================================================================================\n");

	//printf("===================================================================================================================================\n");
	//printf(" PRN |RfR| RfR Int value|RfRm|RfR Mod value | Sig|  FPRs  |Fine PhsR|   Pseduorange_i|   Phase Range_i| PR - CrPhsR|LTIndc|HAIndc| CNR\n");
	//printf("===================================================================================================================================\n");

    int loops = h.nsat * h.nsig;
	//for (i = j = 0; i < h.nsat * h.nsig; i++)
	for (i = j = 0; i < loops ; i++)
	{
        if(h.nsig==0) {
            return sync;
        }
		else
		{
            //printf("RTK Engine,Loop i:%d,j:%d,h.nsig:%d,h.nsig:%d\n", i, j, i/h.nsig, i%h.nsig);
            int satsPos = i / h.nsig;
            int nsigMod = i % h.nsig;
			//int temp =h.sats[i/h.nsig];
			int temp = h.sats[satsPos];
			if(temp!=0)
			{
                
				if (m_fp)
					fprintf(m_fp,"%5d|%3d|%14.3f|%4d|%14.6f|%4.4s|",h.sats[i/h.nsig],RfR[i/h.nsig],r[i/h.nsig],RfRm[i/h.nsig],r_m[i/h.nsig],sig[i%h.nsig]);
				
	            //LOG_DEBUG(" PRN |RfR| RfR Int value|RfRm|RfR Mod value | Sig|  FPRs  |Fine PhsR|   Pseduorange_i|   Phase Range_i| PR - CrPhsR|LTIndc|HAIndc| CNR\n");
				//LOG_DEBUG("%5d|%3d|%14.3f|%4d|%14.6f|%4.4s|",h.sats[satsPos], RfR[satsPos], r[satsPos], RfRm[satsPos], r_m[satsPos], sig[nsigMod]);
				//printf("%5d|%3d|%14.3f|%4d|%14.6f|%4.4s|",h.sats[satsPos], RfR[satsPos], r[satsPos], RfRm[satsPos], r_m[satsPos], sig[nsigMod]);		
                
				if(h.cellmask[i]==0)
				{
                    
					if (m_fp)
						fprintf(m_fp,"\n");
                    
					printf("\n");
				}
				else
				{
					//if (r[i / h.nsig] == 0.0)
					if (r[satsPos] == 0.0)
					{
						j++;
						continue;
					}

					//pr[j]+=(r[i/h.nsig]+r_m[i/h.nsig]);//r[i/h.nsig]+r_m[i/h.nsig]是变化量
					pr[j] += (r[satsPos] + r_m[satsPos]);//r[i/h.nsig]+r_m[i/h.nsig]是变化量
					//cp[j]+=(r[i/h.nsig]+r_m[i/h.nsig]);
					cp[j] += (r[satsPos] + r_m[satsPos]);
                    
					if (m_fp)
						fprintf(m_fp,"%8d|%9d|%16.7f|%16.7f|%12.6f|%6d|%6d|%4d\n",PRs[j],PhsR[j],pr[j],cp[j],pr[j]-cp[j],lock[j],half[j],(int)cnr[j]);
                    
                   
					//LOG_DEBUG("%8d|%9d|%16.7f|%16.7f|%12.6f|%6d|%6d|%4d\n",PRs[j], PhsR[j], pr[j], cp[j],pr[j] - cp[j], lock[j], half[j],(int)cnr[j]);
					//printf("%8d|%9d|%16.7f|%16.7f|%12.6f|%6d|%6d|%4d\n",PRs[j], PhsR[j], pr[j], cp[j],pr[j] - cp[j], lock[j], half[j],(int)cnr[j]);
                    
					int iIndex, iType;
					//int id = h.sats[i / h.nsig];
					int id = h.sats[satsPos];
					switch (sys)
					{
					case GPS:id = id + MIN_GPS_PRN - 1; break;
					case GLO:id = id + MIN_GLO_PRN - 1; break;
					case GAL:id = id + MIN_GAL_PRN - 1; break;
					case BDS:id = id + MIN_BD2_PRN - 1; break;
					default: break;
					}

					/**2020-11-29, debug only GPS*/
					//if (id >= MIN_BD2_PRN) continue;

					if (bFindObsIndex(pEpoch, id, sig[nsigMod], iIndex, iType))
					{
						int glo_offset[24] = {
							+1, -4, +5, +6, +1, -4, +5, +6, -2, -7, +0, -1,
							-2, -7, +0, -1, +4, -3, +3, +2, +4, -3, +3, +2
						};
						double dWaveLength;
						int nlock = (int)tow * 32;
						nlock &= 0x001FFFFF;
						switch (iType)
						{
						case 0:
						    dWaveLength = (LIGHT / GPS_F1);
							/*L1 phase lock loop.*/
							if (MIN_GPS_PRN <= id && id <= MAX_GPS_PRN)
								dWaveLength = (LIGHT / GPS_F1);
							else if (MIN_GLO_PRN <= id && id <= MAX_GLO_PRN)
								dWaveLength = (LIGHT / (GLO_L1_BASE_FRE + glo_offset[id - MIN_GLO_PRN] * GLO_L1_SUB_BAND));
							else if (MIN_BD2_PRN <= id && id <= MAX_BD2_PRN)
								dWaveLength = (LIGHT / BD2_F1);
							else if (MIN_GAL_PRN <= id && id <= MAX_GAL_PRN)
								dWaveLength = (LIGHT / GAL_F1);

							pEpoch->pbMode1[iIndex] = 1;
							pEpoch->pdL1[iIndex] = cp[j] / dWaveLength;
							pEpoch->pdR1[iIndex] = pr[j];
							pEpoch->pbSN1[iIndex] = (int)cnr[j];
							pEpoch->pdD1[iIndex] = (-(rr[satsPos]+rrf[j])/dWaveLength);
							pEpoch->pdwSlip1[iIndex] = lock[j];
							pEpoch->pdwSlip1[iIndex] = nlock + id * 32;
							//printf("L1 pbID:%d, phase Index:%d,pdL1:%f,pdR1:%f,pbSN1:%d,pbwSlip1,%d\n",id, iIndex, pEpoch->pdL1[iIndex],pEpoch->pdR1[iIndex],pEpoch->pbSN1[iIndex],pEpoch->pdwSlip1[iIndex] );
							break;
						case 1:
							/*L2 phase lock loop.*/
							/*
                            if (pEpoch->pdL1[iIndex] == 0.0 || pEpoch->pdR1[iIndex] == 0.0)
  							{
  								pEpoch->pbID[iIndex] = 0;
  								pEpoch->n--;
  							}
  							else
  							*/
							{
						        dWaveLength = (LIGHT / GPS_F2);
								if (MIN_GPS_PRN <= id && id <= MAX_GPS_PRN)
									dWaveLength = (LIGHT / GPS_F2);
								else if (MIN_GLO_PRN <= id && id <= MAX_GLO_PRN)
									dWaveLength = (LIGHT / (GLO_L2_BASE_FRE + glo_offset[id - MIN_GLO_PRN] * GLO_L2_SUB_BAND));
								else if (MIN_BD2_PRN <= id && id <= MAX_BD2_PRN)
									dWaveLength = (LIGHT / BD2_F2);
							    else if (MIN_GAL_PRN <= id && id <= MAX_GAL_PRN)
								    dWaveLength = (LIGHT / GAL_F2);

								pEpoch->pbMode2[iIndex] = 1;
								pEpoch->pdL2[iIndex] = cp[j] / dWaveLength;
								pEpoch->pdP2[iIndex] = pr[j];
								pEpoch->pbSN2[iIndex] = (int)cnr[j];
								pEpoch->pdD2[iIndex] = (-(rr[satsPos]+rrf[j])/dWaveLength);
								pEpoch->pdwSlip2[iIndex] = lock[j];
								pEpoch->pdwSlip2[iIndex] = nlock + (id)* 32;
                                //printf("L2 pbID:%d, phase Index:%d,pdL2:%f,pdP2:%f,pbSN2:%d,pbwSlip2,%d\n", id, iIndex, pEpoch->pdL2[iIndex],pEpoch->pdP2[iIndex],pEpoch->pbSN2[iIndex],pEpoch->pdwSlip2[iIndex] );
							}
							break;
						case 2:
							/* L5 phase lock loop.*/
							/*
                            if (MIN_GPS_PRN <= id && id <= MAX_GPS_PRN)
  								dWaveLength = (LIGHT / GPS_F5);
  							else if (MIN_GLO_PRN <= id && id <= MAX_GLO_PRN)
  								dWaveLength = (LIGHT / (GLO_L1_BASE_FRE + glo_offset[id - MIN_GLO_PRN] * GLO_L1_SUB_BAND));
  							else if (MIN_BD2_PRN <= id && id <= MAX_BD2_PRN)
 								dWaveLength = (LIGHT / BD2_F3);
  							pEpoch->pbMode5[iIndex] = 1;
 							pEpoch->pdL5[iIndex] = cp[j] / dWaveLength;
  							pEpoch->pdR5[iIndex] = pr[j];
  							pEpoch->pbSN5[iIndex] = int(cnr[j]);
  							pEpoch->pdwSlip5[iIndex] = lock[j];
  							pEpoch->pdwSlip5[iIndex] = nlock + (id) * 32;
  							*/
							break;
						default:
							break;
						}
					}
					j++;
				}

			}
		}
	}

	if (m_fp)
		fprintf(m_fp,"--------------------------------------------------------------------------------------------------------------------------------------\n\n");


	return sync;
}

/*Coordinate*/
unsigned int decode_type1005(FILE *m_fp1005, unsigned char *buff,unsigned int Meslen, Coordinate *pCoord)
{
	const unsigned int Mes_type = 1005;
	/*8bit preamble,  6bit reserve, 10bits Meslen*/
	//unsigned int staid,IRTF,GPS_ind,GLO_ind,Gli_ind,sta_ind,Sro_ind,QC_ind;
	//double ARP[3];
	int i = 24 + 12;//将头部的24位直接跳过，进入信息内部,由于信息内部开头是Mestype在main中已经解码过，在此直接跳过；

	pCoord->staid	  = getbitu(buff,i,12);  i+=12;
	pCoord->IRTF	  = getbitu(buff,i, 6);  i+= 6;
	pCoord->GPS_ind   = getbitu(buff,i, 1);  i+= 1;//0 - No GPS service supported ,1 - GPS service supported 
	pCoord->GLO_ind   = getbitu(buff,i, 1);  i+= 1;
	pCoord->Gli_ind   = getbitu(buff,i, 1);  i+= 1;
	pCoord->sta_ind   = getbitu(buff,i, 1);  i+= 1;	//Reference-Station Indicator 
	pCoord->ARP[0]	  = getbits_38(buff,i);  i+=38;
	pCoord->ARP[0]    = pCoord->ARP[0]*0.0001;		
	pCoord->Sro_ind   = getbitu(buff,i, 1);  i+= 1+1;  //Single Receiver Oscillator  Indicator
	pCoord->ARP[1]    = getbits_38(buff,i);  i+=38;
	pCoord->ARP[1]    = pCoord->ARP[1]*0.0001;		
	pCoord->QC_ind    = getbitu(buff,i, 2);  i+= 2;    //Quarter Cycle Indicator
	pCoord->ARP[2]    = getbits_38(buff,i);
	pCoord->ARP[2]    = pCoord->ARP[2]*0.0001;		

	if (m_fp1005 != NULL){
		fprintf(m_fp1005,"---------------------------------\n");
		fprintf(m_fp1005,"Mes_type:  %4d\n",Mes_type);
		fprintf(m_fp1005,"RefStaID:  %4d\n",pCoord->staid);
		fprintf(m_fp1005,"IRTF:      %4d\n",pCoord->IRTF);
		fprintf(m_fp1005,"GPS_ind:      %4d\n",pCoord->GPS_ind);
		fprintf(m_fp1005,"GLO_ind:      %4d\n",pCoord->GLO_ind);
		fprintf(m_fp1005,"Gli_ind:      %4d\n",pCoord->Gli_ind);
		fprintf(m_fp1005,"RefStation Indicator:      %4d\n",pCoord->sta_ind);
		fprintf(m_fp1005,"SingleReceiverOscillatorIndicator:      %4d\n",pCoord->Sro_ind);
		fprintf(m_fp1005,"Quarter Cycle Indicator:      %4d\n",pCoord->QC_ind);
		fprintf(m_fp1005,"ARP_ECEF_X:%13.4f\n",pCoord->ARP[0]*0.0001);
		fprintf(m_fp1005,"ARP_ECEF_Y:%13.4f\n",pCoord->ARP[1]*0.0001);
		fprintf(m_fp1005,"ARP_ECEF_Z:%13.4f\n",pCoord->ARP[2]*0.0001);
		fprintf(m_fp1005,"---------------------------------\n\n");
	}
	return 1;
}

unsigned int decode_type1033(FILE *m_fp1033,unsigned char *buff, unsigned int Meslen)
{
	const unsigned int Mes_type = 1033;
	int i = 24 + 12/*,j*/;
	unsigned int j, staid,N,ANTid,M,I,J,K;
	char ANT_descri[32],ANT_seri[32],Rec_type[32],Rec_ver[32],Rec_seri[32];

	staid = getbitu(buff,i,12); i+=12;
	N     = getbitu(buff,i, 8); i+= 8;

	for(j=0;j<N;j++)
	{
		ANT_descri[j]=getbitu(buff,i, 8);
		i+= 8;
	}

	ANTid = getbitu(buff,i, 8); i+= 8;
	M  = getbitu(buff,i, 8); i+= 8;  

	for(j=0;j<M;j++)
	{
		ANT_seri[j]=getbitu(buff,i, 8);
		i+= 8;
	}

	I  = getbitu(buff,i, 8); i+= 8;  

	for(j=0;j<I;j++)
	{
		Rec_type[j]=getbitu(buff,i, 8);
		i+= 8;
	}

	J = getbitu(buff,i, 8); i+= 8; 

	for(j=0;j<J;j++)
	{
		Rec_ver[j]=getbitu(buff,i, 8);
		i+= 8;
	}

	K = getbitu(buff,i, 8); i+= 8; 

	for(j=0;j<K;j++)
	{
		Rec_seri[j]=getbitu(buff,i, 8);
		i+= 8;
	}

	fprintf(m_fp1033,"--------------------------------------\n");
	fprintf(m_fp1033,"Mes_type:  %4d\n",Mes_type);
	fprintf(m_fp1033,"RefStaID:  %4d\n",staid);
	fprintf(m_fp1033,"ANT descripor counter:  %d\n",N);
	fprintf(m_fp1033,"ANT descripor:");

	for(j=0;j<N;j++)
	{
		fprintf(m_fp1033,"%c",ANT_descri[j]);
	}

	fprintf(m_fp1033,"\nANT serialnum counter:  %d\n",M);
	fprintf(m_fp1033,"ANT serial num:");

	for(j=0;j<M;j++)
	{
		fprintf(m_fp1033,"%c",ANT_seri[j]);
	}

	fprintf(m_fp1033,"\nRec descripor counter:  %d\n",I);

	fprintf(m_fp1033,"Rec descripor :");
	for(j=0;j<I;j++)
	{
		fprintf(m_fp1033,"%c",Rec_type[j]);
	}

	fprintf(m_fp1033,"\nRec Version counter:  %d\n",J);
	fprintf(m_fp1033,"Rec Version :");

	for(j=0;j<J;j++)
	{
		fprintf(m_fp1033,"%c",Rec_ver[j]);
	}

	fprintf(m_fp1033,"\nRec serial num counter:  %d\n",K);
	fprintf(m_fp1033,"Rec serial num:");

	for(j=0;j<K;j++)
	{
		fprintf(m_fp1033,"%c",Rec_seri[j]);
	}

	fprintf(m_fp1033,"\n--------------------------------------\n\n");

	return 1;
}

unsigned int decode_type1045(FILE *m_fp1045, unsigned char *buff, unsigned int Meslen)
{	
	const unsigned int Mes_type = 1045;
	int i=24+12,prn,week,e5a_hs,e5a_dvs,rsv,iode,sva;
	double toc,sqrtA,idot,f2,f1,f0,crs,deln,M0,cuc,e,cus,toes,cic,OMG0,cis,i0,crc,omg,OMGd,tgd;
	prn   =getbitu(buff,i, 6);              i+= 6;
	week  =getbitu(buff,i,12);              i+=12; /* gst-week */
	iode  =getbitu(buff,i,10);              i+=10;
	sva   =getbitu(buff,i, 8);              i+= 8;
	idot  =getbits(buff,i,14)*P2_43*SC2RAD; i+=14;
	toc   =getbitu(buff,i,14)*60.0;         i+=14;
	f2    =getbits(buff,i, 6)*P2_59;        i+= 6;
	f1    =getbits(buff,i,21)*P2_46;        i+=21;
	f0    =getbits(buff,i,31)*P2_34;        i+=31;
	crs   =getbits(buff,i,16)*P2_5;         i+=16;
	deln  =getbits(buff,i,16)*P2_43*SC2RAD; i+=16;
	M0    =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
	cuc   =getbits(buff,i,16)*P2_29;        i+=16;
	e     =getbitu(buff,i,32)*P2_33;        i+=32;
	cus   =getbits(buff,i,16)*P2_29;        i+=16;
	sqrtA =getbitu(buff,i,32)*P2_19;		i+=32;
	toes  =getbitu(buff,i,14)*60.0;         i+=14;
	cic   =getbits(buff,i,16)*P2_29;        i+=16;
	OMG0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
	cis   =getbits(buff,i,16)*P2_29;        i+=16;
	i0    =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
	crc   =getbits(buff,i,16)*P2_5;         i+=16;
	omg   =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
	OMGd  =getbits(buff,i,24)*P2_43*SC2RAD; i+=24;
	tgd   =getbits(buff,i,10)*P2_32;        i+=10; /* E5a/E1 */
	e5a_hs    =getbitu(buff,i, 2);          i+= 2; /* OSHS */
	e5a_dvs   =getbitu(buff,i, 1);          i+= 1; /* OSDVS */
	rsv       =getbitu(buff,i, 7);
	return 1;
}

unsigned int decode_type1046(FILE *m_fp1046,unsigned char *buff, unsigned int Meslen, ServerEphemeris* pEphem)
{	
    //printf("RTK Engine Debug, decode_type1046 Entrance......\n");
	const unsigned int Mes_type = 1046;
	int i=24+12,prn,week,e5b_hs,e5b_dvs,E1B_hs,E1B_dvs,rsv,iode,sva;
	double toc,sqrtA,idot,f2,f1,f0,crs,deln,M0,cuc,e,cus,toe,cic,OMG0,cis,i0,crc,omg,OMGd,tgd1,tgd2;

    pEphem->wSize = sizeof(ServerEphemeris);
	pEphem->uMsgID = 1046;

	prn   =getbitu(buff,i, 6);              i+= 6;
	pEphem->ID = prn + MIN_GAL_PRN - 1;

	week  =getbitu(buff,i,12)+1024;         i+=12; /* gst-week */
	pEphem->week = week;

	iode  =getbitu(buff,i,10);              i+=10;
	pEphem->iode = iode;

	sva   =getbitu(buff,i, 8);              i+= 8;
	idot  =getbits(buff,i,14)*P2_43*SC2RAD; i+=14;
	pEphem->itoet = idot;

	toc   =getbitu(buff,i,14)*60.0;         i+=14;
	pEphem->toc = toc;

	f2    =getbits(buff,i, 6)*P2_59;        i+= 6;
	pEphem->af2 = f2;

	f1    =getbits(buff,i,21)*P2_46;        i+=21;
	pEphem->af1 = f1;

	f0    =getbits(buff,i,31)*P2_34;        i+=31;
	pEphem->af0 = f0;

	crs   =getbits(buff,i,16)*P2_5;         i+=16;
	pEphem->Crs = crs;

	deln  =getbits(buff,i,16)*P2_43*SC2RAD; i+=16;
	pEphem->deltan = deln;

	M0    =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    pEphem->Ms0 = M0;

	cuc   =getbits(buff,i,16)*P2_29;        i+=16;
    pEphem->Cuc = cuc;

	e     =getbitu(buff,i,32)*P2_33;        i+=32;
    pEphem->es = e;

	cus   =getbits(buff,i,16)*P2_29;        i+=16;
    pEphem->Cus = cus;

	sqrtA =getbitu(buff,i,32)*P2_19;        i+=32;
    pEphem->roota = sqrtA;

	toe   =getbitu(buff,i,14)*60.0;         i+=14;
    pEphem->toe = toe;

	cic   =getbits(buff,i,16)*P2_29;        i+=16;
    pEphem->Cic = cic;

	OMG0  =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    pEphem->omega0 = OMG0;

	cis   =getbits(buff,i,16)*P2_29;        i+=16;
    pEphem->Cis = cis;

	i0    =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    pEphem->i0 = i0;

	crc   =getbits(buff,i,16)*P2_5;         i+=16;
    pEphem->Crc = crc;

	omg   =getbits(buff,i,32)*P2_31*SC2RAD; i+=32;
    pEphem->ws = omg;

	OMGd  =getbits(buff,i,24)*P2_43*SC2RAD; i+=24;
    pEphem->omegaot = OMGd;

	tgd1  =getbits(buff,i,10)*P2_32;        i+=10; /* E5a/E1 */
    pEphem->tgd = tgd1;//??

	tgd2  =getbits(buff,i,10)*P2_32;        i+=10; /*E5B/E1*/
    pEphem->tgd2 = tgd2;//??

    e5b_hs    =getbitu(buff,i, 2);          i+= 2; /* OSHS */
	e5b_dvs   =getbitu(buff,i, 1);          i+= 1; /* OSDVS */
	E1B_hs    =getbitu(buff,i,2);           i+= 2;
	E1B_dvs   =getbitu(buff,i,1);           i+= 1;


    pEphem->bHealth=(e5b_hs<<7)+(e5b_dvs<<6)+(E1B_hs<<1)+(E1B_dvs<<0);
	if (pEphem->bHealth)
		pEphem->blFlag = false;
	else
	{
		if (sqrtA < 4000 || sqrtA > 7000)

			pEphem->blFlag = false;
		else
			pEphem->blFlag = true;
	}

	rsv       =getbitu(buff,i, 2);		    i+= 2;	
    /*
	fprintf(m_fp1046, "===================================================================================================================================\n");
	fprintf(m_fp1046, "prn|week|iode|SVa|    idot    |    toc   |   toe    |     f2     |     f1     |     f0     |    deln    |      M0    |       e    |\n");
	fprintf(m_fp1046, "===================================================================================================================================\n");
	fprintf(m_fp1046, "%3d|%4d|%4d|%3d|%12.4e|%10.4f|%10.4f|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|\n", prn, week, iode, sva, toc, toe, f2, f1, f0, deln, M0, e);

	fprintf(m_fp1046, "======================================================================================================================================================================================\n");
	fprintf(m_fp1046, "   crs      |   cuc      |   cus      |   cic      |   cis      |   crc      |  sqrtA  |   OMG0     |   omega    |    OMGd    |   tgd1     |    tgd2   |e5b_hs|e5b_dvs|E1B_hs|E1B_dvs|\n");
	fprintf(m_fp1046, "======================================================================================================================================================================================\n");
	fprintf(m_fp1046, "%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%9.3f|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%6d|%7d|%6d|%7d|\n\n", crs, cuc, cus, cic, cic, cis, crc, sqrtA, OMG0, omg, OMGd, tgd1, tgd2, e5b_hs, e5b_dvs, E1B_hs, E1B_dvs);

	fflush(m_fp1046);
    */
    
    #ifdef _VERBOSE
	printf("===================================================================================================================================\n");
	printf("prn|week|iode|SVa|    idot    |    toc   |   toe    |     f2     |     f1     |     f0     |    deln    |      M0    |       e    |\n");
	printf("===================================================================================================================================\n");
	//printf("%3d|%4d|%4d|%3d|%12.4e|%10.4f|%10.4f|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|\n", prn, week, iode, sva, toc, toe, f2, f1, f0, deln, M0, e);
	printf("%3d|%4d|%4d|%3d|%12.4e|%10.4f|%10.4f|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|\n", prn, week, iode, sva, toc, toe, f2, f1, f0, deln, M0, e);

	printf("======================================================================================================================================================================================\n");
	printf("   crs      |   cuc      |   cus      |   cic      |   cis      |   crc      |  sqrtA  |   OMG0     |   omega    |    OMGd    |   tgd1     |    tgd2   |e5b_hs|e5b_dvs|E1B_hs|E1B_dvs|Health|\n");
	printf("======================================================================================================================================================================================\n");
	printf("%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%9.3f|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%7d|%6d|%7d|%7d|%7d|\n\n", crs, cuc, cus, cic, cic, cis, crc, sqrtA, OMG0, omg, OMGd, tgd1, tgd2, e5b_hs, e5b_dvs, E1B_hs, E1B_dvs, pEphem->bHealth);
    #endif
    return 1;
}

/*ssr1,4 header*/
static int decode_ssr1_head(FILE *m_fp, unsigned char *buff, int sys, unsigned int Meslen,int *sync, int *iod,	double *udint, int *refd, int *hsize)
{
	double tod,tow;

	int i=24+12,nsat,udi,provid=0,solid=0,ns=6;

	if (i+(sys==GLO?53:50+ns)>((int)(Meslen+3)*8)) return -1;

	if (sys==GLO)
	{
		tod=getbitu(buff,i,17); i+=17;
		tow = tod;
	
		if (tow > 604799)
			tow -= 604800;
	}
	else
	{
		tow=getbitu(buff,i,20); i+=20;
		
		if (tow > 604799)
			tow -= 604800;
	}
	udi   =getbitu(buff,i, 4); i+= 4;
	*sync =getbitu(buff,i, 1); i+= 1;
	*refd =getbitu(buff,i, 1); i+= 1; /* satellite ref datum */
	*iod  =getbitu(buff,i, 4); i+= 4; /* iod */
	provid=getbitu(buff,i,16); i+=16; /* provider id */
	solid =getbitu(buff,i, 4); i+= 4; /* solution id */
	nsat  =getbitu(buff,i,ns); i+=ns;
	*udint=ssrudint[udi];
	*hsize=i;


	fprintf(m_fp,"GPS Epoch Time:  %f\n",tow);
	fprintf(m_fp,"SSR Update Interval:  %d\n",udi);
	fprintf(m_fp,"Multiple Message Indicator:  %d\n",*sync);
	fprintf(m_fp,"Reference Datum:  %d\n",*refd);
	fprintf(m_fp,"iod:  %d\n",*iod);
	fprintf(m_fp,"provid:  %d\n",provid);
	fprintf(m_fp,"solid:  %d\n",solid);
	fprintf(m_fp,"Nsat:  %d\n",nsat);
	return nsat;
}

static int decode_ssr1(FILE * m_fp, unsigned char *buff,unsigned int Meslen, int sys)
{
	double udint,deph[3],ddeph[3];
	int i,j,type,sync,iod,nsat,prn,iode,iodcrc,refd=0,np,ni,nj,offp;

	type=getbitu(buff,24,12);	
	if ((nsat=decode_ssr1_head(m_fp, buff, sys, Meslen, &sync, &iod, &udint, &refd, &i))<0) 	
		return -1;

	switch (sys)
	{
		case GPS: np=6; ni= 8; nj= 0; offp=  0; break;
		case GLO: np=5; ni= 8; nj= 0; offp=  0; break;
		case GAL: np=6; ni= 8; nj= 0; offp=  0; break;
		case BDS: np=6; ni= 8; nj= 0; offp=  0; break;
		default: break;
	}
	fprintf(m_fp,"Mes_type:  %4d\n",type);
	fprintf(m_fp,"===========================================================\n");
	fprintf(m_fp,"prn|IODE|  dr   |   dat |   dct |   d_dr |  d_dat |   d_dct\n");
	fprintf(m_fp,"===========================================================\n");
	for (j=0;j<nsat;j++) 
	{
		prn     =getbitu(buff,i,np)+offp; i+=np;
		iode    =getbitu(buff,i,ni);      i+=ni;
		iodcrc  =getbitu(buff,i,nj);      i+=nj;
		deph [0]=getbits(buff,i,22)*1E-4; i+=22;
		deph [1]=getbits(buff,i,20)*4E-4; i+=20;
		deph [2]=getbits(buff,i,20)*4E-4; i+=20;
		ddeph[0]=getbits(buff,i,21)*1E-6; i+=21;
		ddeph[1]=getbits(buff,i,19)*4E-6; i+=19;
		ddeph[2]=getbits(buff,i,19)*4E-6; i+=19;


		fprintf(m_fp,"%3d|%4d|%7.4f|%7.4f|%7.4f|%7.6f|%7.6f|%7.6f\n",prn,iode, deph [0], deph [1], deph [2],ddeph[0],ddeph[1],ddeph[2]);

	}
	fprintf(m_fp,"===========================================================\r\n\r\n");
	return sync?0:10;
}

/* decode ssr 2,3,5,6 message header -----------------------------------------*/
static int decode_ssr2_head(FILE *m_fp,unsigned char *buff,int sys, unsigned int Meslen, int *sync, int *iod, double *udint, int *hsize)
{
	double tod,tow;	
	int i=24+12,nsat,udi,provid=0,solid=0,ns=6;

	if (i+(sys==GLO?52:49+ns)>(Meslen+3)*8) return -1;

	if (sys==GLO) 
	{
		tod=getbitu(buff,i,17); i+=17;
		tow = tod;		
		if (tow > 604799)
			tow -= 604800;
	}
	else 
	{
		tow=getbitu(buff,i,20); i+=20;	
		if (tow > 604799)
			tow -= 604800;
	}
	udi   =getbitu(buff,i, 4); i+= 4;
	*sync =getbitu(buff,i, 1); i+= 1;
	*iod  =getbitu(buff,i, 4); i+= 4;
	provid=getbitu(buff,i,16); i+=16; /* provider id */
	solid =getbitu(buff,i, 4); i+= 4; /* solution id */
	nsat  =getbitu(buff,i,ns); i+=ns;
	*udint=ssrudint[udi];
	*hsize=i;/*header length*/

	fprintf(m_fp,"GPS Epoch Time:  %f\n",tow);
	fprintf(m_fp,"SSR Update Interval:  %d\n",udi);
	fprintf(m_fp,"Multiple Message Indicator:  %d\n",*sync);
	fprintf(m_fp,"iod:  %d\n",*iod);
	fprintf(m_fp,"provid:  %d\n",provid);
	fprintf(m_fp,"solid:  %d\n",solid);
	fprintf(m_fp,"Nsat:  %d\n",nsat);
	return nsat;
}


/* decode ssr 2: clock corrections -------------------------------------------*/
static int decode_ssr2(FILE *m_fp, unsigned char *buff, unsigned int Meslen, int sys)
{
	double udint,dclk[3];
	int i,j,type,sync,iod,nsat,prn,np,offp;

	type=getbitu(buff,24,12);

	if ((nsat=decode_ssr2_head(m_fp, buff, sys, Meslen, &sync, &iod, &udint, &i))<0) 	 		
		return -1;

	switch (sys)
	{
		case GPS: np=6; offp=  0; break;
		case GLO: np=5; offp=  0; break;
		case GAL: np=6; offp=  0; break;
		case BDS: np=6; offp=  0; break;
			
		default: return sync?0:10;
	}
	fprintf(m_fp,"Mes_type:  %4d\n",type);
	fprintf(m_fp,"==============================\n");
	fprintf(m_fp,"prn|Clock C0|Clock C1|Clock C2\n");
	fprintf(m_fp,"==============================\n");
	for (j=0;j<nsat;j++) 
	{
		prn    =getbitu(buff,i,np)+offp; i+=np;
		dclk[0]=getbits(buff,i,22)*1E-4; i+=22;
		dclk[1]=getbits(buff,i,21)*1E-6; i+=21;
		dclk[2]=getbits(buff,i,27)*2E-8; i+=27;
		fprintf(m_fp,"%3d|%8.4f|%8.4f|%8.4f|\n",prn,dclk[0],dclk[1],dclk[2]);
	}
	fprintf(m_fp,"===============================\r\n\r\n");
	return sync?0:10;
}

/*解码SSR时用到*/
static const int codes_gps[] = 
{
	CODE_L1C, CODE_L1P, CODE_L1W, CODE_L1Y, CODE_L1M, CODE_L2C, CODE_L2D, CODE_L2S,
	CODE_L2L, CODE_L2X, CODE_L2P, CODE_L2W, CODE_L2Y, CODE_L2M, CODE_L5I, CODE_L5Q,
	CODE_L5X
};
static const int codes_glo[] = 
{
	CODE_L1C, CODE_L1P, CODE_L2C, CODE_L2P
};
static const int codes_gal[] =
{
	CODE_L1A, CODE_L1B, CODE_L1C, CODE_L1X, CODE_L1Z, CODE_L5I, CODE_L5Q, CODE_L5X,
	CODE_L7I, CODE_L7Q, CODE_L7X, CODE_L8I, CODE_L8Q, CODE_L8X, CODE_L6A, CODE_L6B,
	CODE_L6C, CODE_L6X, CODE_L6Z
};

static const int codes_bds[] = 
{
	CODE_L1I, CODE_L1Q, CODE_L1X, CODE_L7I, CODE_L7Q, CODE_L7X, CODE_L6I, CODE_L6Q,
	CODE_L6X
};



/* decode ssr 3: satellite code biases ---------------------------------------*/
static int decode_ssr3(FILE *m_fp, unsigned char *buff, unsigned int Meslen, int sys)
{
	const int *codes;

	double udint, bias, cbias[MAXCODE];
	int i, j, k, type, mode, sync, iod, nsat, prn, nbias, np, offp, ncode;

	type = getbitu(buff, 24, 12);//24表示的是8 bits preamble,6 bits reversed  10 bits message length;


	if ((nsat = decode_ssr2_head(m_fp, buff, sys, Meslen, &sync, &iod, &udint, &i))<0)//解码后将头的长度赋值给i输出
		return -1;

	fprintf(m_fp, "Mes_type:  %4d\n", type);
	fprintf(m_fp, "===========================\n");
	fprintf(m_fp, " prn | nbias | mode | bias \n");
	fprintf(m_fp, "===========================\n");

	switch (sys) 
	{
		case GPS: np = 6; offp = 0; codes = codes_gps; ncode = 17; break;//GPS :1059 Code Bias
		case GLO: np = 5; offp = 0; codes = codes_glo; ncode =  4; break;//GLONASS:1065 code bias
		case GAL: np = 6; offp = 0; codes = codes_gal; ncode = 19; break;	
		case BDS: np = 6; offp = 0; codes = codes_bds; ncode =  9; break;
	
		default: return sync ? 0 : 10;
	}



	for (j = 0; j < nsat && i + 5 + np <= (Meslen+3) * 8; j++)
	{
		prn = getbitu(buff, i, np) + offp; i += np;//satllite ID 
		nbias = getbitu(buff, i, 5);      i += 5;  //number of code biases processed

		for (k = 0; k<MAXCODE; k++) cbias[k] = 0.0;//赋初值
		for (k = 0; k<nbias&&i + 19 <= (Meslen+3) * 8; k++)
		{
			mode = getbitu(buff, i, 5);      i += 5;//signal and tracking mode indicator  0-31
			bias = getbits(buff, i, 14)*0.01; i += 14;//code bias  +-81.91m				
			
			if (mode <= ncode)  
			{
				cbias[codes[mode] - 1] = (float)bias;
				fprintf(m_fp, "%5d|%7d|%6d|%6.2f|\n", prn, nbias, mode, (float)bias);
			}
			else 
			{
				fprintf(m_fp, "rtcm3 %d not supported mode: mode=%d\n", type, mode);
			}	
		}	
	}

	fprintf(m_fp, "===========================\n\n");
	return sync ? 0 : 10;
}
/* decode ssr 4: combined orbit and clock corrections ------------------------*/
static int decode_ssr4(FILE *m_fp, unsigned char *buff, unsigned int Meslen, int sys)
{
	double udint, deph[3], ddeph[3], dclk[3];
	int i, j, type, nsat, sync, iod, prn, iode, iodcrc, refd = 0, np, ni, nj, offp;

	type = getbitu(buff, 24, 12);

	if ((nsat = decode_ssr1_head(m_fp, buff, sys, Meslen, &sync, &iod, &udint, &refd, &i))<0)
	{
		fprintf(m_fp, "rtcm3 %d length error: len=%d\n", type, Meslen);
		return -1;
	}

	fprintf(m_fp, "Mes_type:  %4d\n", type);
	fprintf(m_fp, "=============================================================================================================================================================================\n");
	fprintf(m_fp, " prn |iode|iodcrc|delta radial|delta along-track|delta cross-track|dot delta radial|dot delta along-track|dot delta cross-track|delta clock C0|delta clock C1|delta clock C2|\n");
	fprintf(m_fp, "=============================================================================================================================================================================\n");

	switch (sys) 
	{
		case GPS: np = 6; ni =  8; nj =  0; offp = 0; break;
		case GLO: np = 5; ni =  8; nj =  0; offp = 0; break;
		case GAL: np = 6; ni = 10; nj =  0; offp = 0; break;	
		case BDS: np = 6; ni = 10; nj = 24; offp = 0; break;
	
		default: return sync ? 0 : 10;
	}
	for (j = 0; j<nsat&&i + 191 + np + ni + nj <= (Meslen+3) * 8; j++)
	{
		prn     = getbitu(buff, i, np) + offp; i += np;//GPS Satellite ID 
		iode    = getbitu(buff, i, ni);        i += ni;//GPS IODE
		iodcrc  = getbitu(buff, i, nj);        i += nj;//仅北斗存在这一项,参考RTKLIB
		deph[0] = getbits(buff, i, 22)*1E-4;   i += 22;//delta radial
		deph[1] = getbits(buff, i, 20)*4E-4;   i += 20;//Delta along-track
		deph[2] = getbits(buff, i, 20)*4E-4;   i += 20;//delta cross-track
		ddeph[0] = getbits(buff, i, 21)*1E-6;  i += 21;//Dot delta radial
		ddeph[1] = getbits(buff, i, 19)*4E-6;  i += 19;//dot delta along-track
		ddeph[2] = getbits(buff, i, 19)*4E-6;  i += 19;//dot delta cross-track

		dclk[0] = getbits(buff, i, 22)*1E-4; i += 22;//delta clock C0
		dclk[1] = getbits(buff, i, 21)*1E-6; i += 21;//delta clock C1
		dclk[2] = getbits(buff, i, 27)*2E-8; i += 27;//delta clock C2

		fprintf(m_fp, "%5d|%4d|%6d|%12.4f|%17.4f|%17.4f|%16.6f|%21.6f|%21.6f|%14.4f|%14.4f|%14.4f|\n", prn, iode, iodcrc, deph[0], deph[1], deph[2], ddeph[0], ddeph[1], ddeph[2], dclk[0], dclk[1], dclk[2]);

	}

	fprintf(m_fp, "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
	return sync ? 0 : 10;
}


/*GPS ephemeris*/
unsigned int decode_type1019(FILE *m_fp, unsigned char *buff, unsigned int Meslen, ServerEphemeris* pEphem)
{
  	pEphem->wSize = sizeof(ServerEphemeris);
  	pEphem->uMsgID = 1019;

	int i = 24 + 12;
	unsigned int satID, WN, Sv_acc, L2_code, iode, toc, iodc, toe, Sv_hlth, flag, fit;
	double iodt, f2, f1, f0,e, crs, delt_n, M0, cuc, cus, sqrtA, cic, Omega0, cis, i0, crc, omega, Omegadot, tgd;

	pEphem->ID = satID = getbitu(buff, i, 6);		i += 6;
	pEphem->week = WN = getbitu(buff, i, 10);		i += 10;//0-1023,1980-1-6 0点
	if (pEphem->week < 1024)
		pEphem->week += 2048;

	Sv_acc = getbitu(buff, i, 4);		i += 4;//URA：Units meter
	//compile error
	//__int16 iUraArray[] = { 2, 3, 4, 6, 8, 12, 20, 36, 72, 150, 300, 600, 1200, 2200, 4500, 9000 };
	int16_t iUraArray[] = { 2, 3, 4, 6, 8, 12, 20, 36, 72, 150, 300, 600, 1200, 2200, 4500, 9000 };
	if (Sv_acc >= 16) Sv_acc = 0;
	pEphem->accuracy = iUraArray[Sv_acc];

	L2_code = getbitu(buff, i, 2);		i += 2;//00:reserved  01:P code on; 10:C/A code on;   11:L2C on
	pEphem->itoet = iodt = getbits(buff, i, 14)*P2_43*SC2RAD; i += 14;
	pEphem->iode = iode = getbitu(buff, i, 8);		i += 8;
	pEphem->toc = toc = getbitu(buff, i, 16) * 16; i += 16;//scale is 2^4 有效范围是607784
	/*pEphem->tow = (int)(floor(toc));*/
	pEphem->af2 = f2 = getbits(buff, i, 8)*P2_55; i += 8;
	pEphem->af1 = f1 = getbits(buff, i, 16)*P2_43; i += 16;
	pEphem->af0 = f0 = getbits(buff, i, 22)*P2_31; i += 22;
	pEphem->iodc = iodc = getbitu(buff, i, 10); i += 10;//iodc的低8位和iode包含相同的bit和序列；
	/*//pEphem->tgd2 = 0;*/
	pEphem->Crs = crs = getbits(buff, i, 16)*P2_5; i += 16;
	pEphem->deltan = delt_n = getbits(buff, i, 16)*P2_43*SC2RAD; i += 16;
	pEphem->Ms0 = M0 = getbits(buff, i, 32)*P2_31*SC2RAD; i += 32;
	pEphem->Cuc = cuc = getbits(buff, i, 16)*P2_29; i += 16;

	pEphem->es = e = getbitu(buff, i, 32)*P2_33; i += 32;
	pEphem->Cus = cus = getbits(buff, i, 16)*P2_29; i += 16;
	pEphem->roota = sqrtA = getbitu(buff, i, 32)*P2_19; i += 32;

	pEphem->toe = toe = getbitu(buff, i, 16) * 16; i += 16;//有效范围是604784
	pEphem->Cic = cic = getbits(buff, i, 16)*P2_29; i += 16;
	pEphem->omega0 = Omega0 = getbits(buff, i, 32)*P2_31*SC2RAD; i += 32;
	pEphem->Cis = cis = getbits(buff, i, 16)*P2_29;		 i += 16;
	pEphem->i0 = i0 = getbits(buff, i, 32)*P2_31*SC2RAD; i += 32;
	pEphem->Crc = crc = getbits(buff, i, 16)*P2_5;		 i += 16;
	pEphem->ws = omega = getbits(buff, i, 32)*P2_31*SC2RAD;  i += 32;
	pEphem->omegaot = Omegadot = getbits(buff, i, 24)*P2_43*SC2RAD; i += 24;
	tgd = getbits(buff, i, 8)*P2_31; i += 8;
	pEphem->bHealth = Sv_hlth = getbitu(buff, i, 6); i += 6;
	if (pEphem->bHealth)
		//pEphem->blFlag = FALSE;compile error
		pEphem->blFlag = false;
	else
	{
		if (sqrtA < 4000 || sqrtA > 7000)
			//pEphem->blFlag = FALSE;compile error

			pEphem->blFlag = false;
		else
			pEphem->blFlag = true;
	}

	flag = getbitu(buff, i, 1); i += 1;//0:L2 Pcode NAV data on  1:L2 P code NAV data off
	fit = getbitu(buff, i, 1); i += 1;
	/*
  	pEphem->bIonoValid = 0;
  	pEphem->m_wIdleTime = 130;
    */
    /*
	if (m_fp)
	{
		fprintf(m_fp, "===================================================================================================================\n");
		fprintf(m_fp, "prn| WN |Sv_accuracy|L2_code|    iodt    |  iode | iodc  |  toc  |  toe  |      f2    |      f1    |       f0     |\n");
		fprintf(m_fp, "===================================================================================================================\n");
		fprintf(m_fp, "%3d|%4d|%11d|%7d|%12.4e|%7d|%7d|%7d|%7d|%12.5e|%12.5e|%12.5e|\n", satID, WN, Sv_acc, L2_code, iodt, iode, iodc, toc, toe, f2, f1, f0);

		fprintf(m_fp, "==================================================================================================================================\n");
		fprintf(m_fp, "   delt_n   |     M0     |      e     |     i0     |     crs    |     cuc    |     cus    |    crc     |    cic     |    cis     |\n");
		fprintf(m_fp, "==================================================================================================================================\n");
		fprintf(m_fp, "%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|\n", delt_n, M0, e, i0, crs, cuc, cus, crc, cic, cis);

		fprintf(m_fp, "==========================================================================\n");
		fprintf(m_fp, "   omega    |  Omegadot  |   sqrtA    |    tgd     | Sv_hlth | flag | fit|\n");
		fprintf(m_fp, "==========================================================================\n");
		fprintf(m_fp, "%12.5e|%12.5e|%12.5e|%12.5e|%9d|%6d|%4d|\n\n", omega, Omegadot, sqrtA, tgd, Sv_hlth, flag, fit);

		fflush(m_fp);

	}
    */
    #ifdef _VERBOSE
    printf("===================================================================================================================\n");
    printf("prn| WN |Sv_accuracy|L2_code|    iodt    |  iode | iodc  |  toc  |  toe  |      f2    |      f1    |       f0     |\n");
	printf("===================================================================================================================\n");
	printf("%3d|%4d|%11d|%7d|%12.4e|%7d|%7d|%7d|%7d|%12.5e|%12.5e|%12.5e|\n", satID, WN, Sv_acc, L2_code, iodt, iode, iodc, toc, toe, f2, f1, f0);

	printf("==================================================================================================================================\n");
	printf("   delt_n   |     M0     |      e     |     i0     |     crs    |     cuc    |     cus    |    crc     |    cic     |    cis     |\n");
	printf("==================================================================================================================================\n");
	printf("%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|\n", delt_n, M0, e, i0, crs, cuc, cus, crc, cic, cis);

	printf("==========================================================================\n");
	printf("   omega    |  Omegadot  |   sqrtA    |    tgd     | Sv_hlth | flag | fit|\n");
	printf("==========================================================================\n");
	printf("%12.5e|%12.5e|%12.5e|%12.5e|%9d|%6d|%4d|\n\n", omega, Omegadot, sqrtA, tgd, Sv_hlth, flag, fit);
    #endif
	return 1;
}


/*GLONASS ephemeris
 * */
unsigned int decode_type1020(FILE *m_fp, unsigned char *buff, unsigned int Meslen, ServerGloEphemeris* pEphem, int nLeaps)
{
	pEphem->dwSize = sizeof(ServerGloEphemeris);
	pEphem->uMsgID = 1020;

	int i = 24 + 12;//跳过 message type 的12bits
	unsigned int tk,satID, fre_num, alm_hth, alm_hth_ind, p1, t_h, t_m, t_s, Bn, p2, tb, p3, M_p, M_l3, M_l5, En, M_p4, MF_t, MN_t, M_M, AOAD, NA, MN4, rev;
	double xn_1, xn, xn_2, yn_1, yn, yn_2, zn_1, zn, zn_2, garmn, tao_n, delt_taon,tao_c,tao_gps;

	satID = getbitu(buff, i, 6);  i += 6;
	pEphem->bSlot = satID + MIN_GLO_PRN - 1;
	pEphem->rfchnl = fre_num = getbitu(buff, i, 5);  i += 5;
	alm_hth = getbitu(buff, i, 1);  i += 1;
	alm_hth_ind = getbitu(buff, i, 1);  i += 1;//0：alamanac health is not available,1:alamanac health is available
	pEphem->P1 = p1 = getbitu(buff, i, 2);  i += 2;

	pEphem->hh = t_h = getbitu(buff, i, 5); i += 5;//hour
	pEphem->mm = t_m = getbitu(buff, i, 6); i += 6;//minute
	pEphem->ss = t_s = getbitu(buff, i, 1) * 30; i += 1;//seconde
	tk = t_h * 3600 + t_m * 60 + t_s;
	Bn = getbitu(buff, i, 1);			i += 1;//Bn 表示星历的健康状况
	pEphem->Bn = (Bn & 0x80) >> 7;
	pEphem->health = pEphem->Bn;
	pEphem->P2 = p2 = getbitu(buff, i, 1);			i += 1;
	tb = getbitu(buff, i, 7) * 15;		i += 7;	//minute
	pEphem->tb = tb * 60;
	pEphem->xvel = xn_1 = getbitg(buff, i, 24)*P2_20;	i += 24;//inS 最高位表示符号，后面表示绝对值
	pEphem->xpos = xn = getbitg(buff, i, 27)*P2_11;		i += 27;
	pEphem->xacc = xn_2 = getbitg(buff, i, 5)*P2_30;	i += 5;
	pEphem->yvel = yn_1 = getbitg(buff, i, 24)*P2_20;	i += 24;
	pEphem->ypos = yn = getbitg(buff, i, 27)*P2_11;		i += 27;
	pEphem->yacc = yn_2 = getbitg(buff, i, 5)*P2_30;	i += 5;
	pEphem->zvel = zn_1 = getbitg(buff, i, 24)*P2_20;	i += 24;
	pEphem->zpos = zn = getbitg(buff, i, 27)*P2_11;		i += 27;
	pEphem->zacc = zn_2 = getbitg(buff, i, 5)*P2_30;	i += 5;

	p3 = getbitu(buff, i, 1);			i += 1;
	pEphem->gamman = garmn = getbitg(buff, i, 11)*P2_40;	i += 11;
	M_p = getbitu(buff, i, 2);			i += 2;
	M_l3 = getbitu(buff, i, 1);			i += 1;
	pEphem->taun = tao_n = getbitg(buff, i, 22)*P2_30;	i += 22;
	pEphem->deltataun = delt_taon = getbitg(buff, i, 5)*P2_30;	i += 5;
	pEphem->En = En = getbitu(buff, i, 5);			i += 5;//the age of GLONASS navigation data
	M_p4 = getbitu(buff, i, 1);			i += 1;
	pEphem->FT = MF_t = getbitu(buff, i, 4);			i += 4;
	pEphem->NT = MN_t = getbitu(buff, i, 11);			i += 11;
	pEphem->M = M_M = getbitu(buff, i, 2);			i += 2;
	AOAD = getbitu(buff, i, 1);			i += 1;
	NA = getbitu(buff, i, 11);			i += 11;
	tao_c = getbitg(buff, i, 32)*P2_31;	i += 32;
	pEphem->N4 = MN4 = getbitu(buff, i, 5) * 4;		i += 5;
	tao_gps = getbitg(buff, i, 22)*P2_30;	i += 22;
	M_l5 = getbitu(buff, i, 1);			i += 1;
	rev = getbitu(buff, i, 7);			i += 7;

	int UTC[6];
	vGetGlonassUTC(pEphem->tb, MN_t, MN4, UTC);
	int iYMD = UTC[0] * 10000 + UTC[1] * 100 + UTC[2];
	int lDate = iYMDtoSumDay(iYMD);
	pEphem->week = ::iInt(lDate / 7);// 记录星期数 陆小路
	pEphem->tog = ::iInt((lDate % 7) * 86400 + UTC[3] * 3600 + UTC[4] * 60 + UTC[5]) - 3 * 3600 + nLeaps;// GLONASS星历关键数据(GPS时转为GLONASS时间）

	if (pEphem->health)
		//pEphem->bValid = FALSE;//compile error

		pEphem->bValid = false;
	else
		pEphem->bValid = true;
	
	if (m_fp)
	{
		fprintf(m_fp, "============================================================================================================\n");
		fprintf(m_fp, "prn| fre_num |alm_hth|alm_hth_ind| p1 |   tk   | Bn  |  p2  |  tb  |      xn    |     xn_1    |     xn_2   |\n");
		fprintf(m_fp, "============================================================================================================\n");
		fprintf(m_fp, "%3d|%9d|%7d|%11d|%4d|%8d|%7d|%7d|%7d|%12.5e|%12.5e|%12.5e|\n", satID, fre_num, alm_hth, alm_hth_ind, p1, tk, Bn, p2, tb, xn, xn_1, xn_2);

		fprintf(m_fp, "=========================================================================================================================\n");
		fprintf(m_fp, "     yn     |    yn_1    |    yn_2    |     zn     |    zn_1    |    zn_2    |  p3  |    garmn   |M_p| M_l3|  tao_n     |\n");
		fprintf(m_fp, "=========================================================================================================================\n");
		fprintf(m_fp, "%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%12.5e|%6d|%12.5e|%3d|%5d|%12.5e|\n", yn, yn_1, yn_2, zn, zn_1, zn_2, p3, garmn, M_p, M_l3, tao_n);


		fprintf(m_fp, "==================================================================================\n");
		fprintf(m_fp, "  delt_taon | En |M_p4|MF_t|MN_t|M_M|AOAD| NA |    tao_c   |MN4|  tao_gps   |M_l5|\n");
		fprintf(m_fp, "==================================================================================\n");
		fprintf(m_fp, "%12.5e|%4d|%4d|%4d|%4d|%3d|%4d|%4d|%12.5e|%3d|%12.5e|%3d|\n\n", delt_taon, En, M_p4, MF_t, MN_t, M_M, AOAD, NA, tao_c, MN4, tao_gps, M_l5);
		fflush(m_fp);
	}


	return 1;
}

/*BDS ephemeris*/
unsigned int decode_type1042(FILE *m_fp, unsigned char *buff, unsigned int Meslen, ServerEphemeris* pEphem)
{
	pEphem->wSize = sizeof(ServerEphemeris);
	pEphem->uMsgID = 1042;
	int i = 24 + 12;//跳过 message type 的12bits

	unsigned int satID, WN, URAI, AODE, toc, AODC, toe, health;
	double IDOT, a2, a1, a0, crs, deltn, M0, cuc, e, cus, sqrtA, cic, Omega0;
	double cis, i0, crc, omega, dot_Omega, tgd1, tgd2;

	satID = getbitu(buff, i, 6); i += 6;
	pEphem->ID = satID + MIN_BD2_PRN - 1;
	pEphem->week = WN = getbitu(buff, i, 13); i += 13;
	if (pEphem->week < 1356)
	{// BD星期调整为GPS星期
		pEphem->week += 1356;
	}

	URAI	= getbitu(buff, i, 4); i += 4;
        //compile error
	//__int16 iUraArray[] = { 2, 3, 4, 6, 8, 12, 20, 36, 72, 150, 300, 600, 1200, 2200, 4500, 9000 };
	int16_t iUraArray[] = { 2, 3, 4, 6, 8, 12, 20, 36, 72, 150, 300, 600, 1200, 2200, 4500, 9000 };
	if (URAI >= 16) URAI = 0;
	pEphem->accuracy = iUraArray[URAI];

	pEphem->itoet = IDOT = getbits(buff, i, 14)*P2_43*SC2RAD; i += 14;
	pEphem->iode = AODE = getbitu(buff, i, 5); i += 5;

	pEphem->toc = toc = getbitu(buff, i, 17) * 8;	  i += 17;
	/*/pEphem->tow = (int)(floor(toc));*/
	pEphem->af2 = a2 = getbits(buff, i, 11)*P2_66; i += 11;
	pEphem->af1 = a1 = getbits(buff, i, 22)*P2_50; i += 22;
	pEphem->af0 = a0 = getbits(buff, i, 24)*P2_33; i += 24;

	pEphem->iodc = AODC = getbitu(buff, i, 5); i += 5;
	pEphem->Crs = crs = getbits(buff, i, 18)*P2_6; i += 18;
	pEphem->deltan = deltn = getbits(buff, i, 16)*P2_43*SC2RAD; i += 16;
	pEphem->Ms0 = M0 = getbits(buff, i, 32)*P2_31*SC2RAD; i += 32;
	pEphem->Cuc = cuc = getbits(buff, i, 18)*P2_31; i += 18;
	pEphem->es = e = getbitu(buff, i, 32)*P2_33; i += 32;
	pEphem->Cus = cus = getbits(buff, i, 18)*P2_31; i += 18;
	pEphem->roota = sqrtA = getbitu(buff, i, 32)*P2_19; i += 32;

	pEphem->toe = toe = getbitu(buff, i, 17) * 8; i += 17;
	pEphem->Cic = cic = getbits(buff, i, 18)*P2_31; i += 18;
	pEphem->omega0 = Omega0 = getbits(buff, i, 32)*P2_31*SC2RAD; i += 32;
	pEphem->Cis = cis = getbits(buff, i, 18)*P2_31; i += 18;
	pEphem->i0 = i0 = getbits(buff, i, 32)*P2_31*SC2RAD; i += 32;
	pEphem->Crc = crc = getbits(buff, i, 18)*P2_6; i += 18;
	pEphem->ws = omega = getbits(buff, i, 32)*P2_31*SC2RAD; i += 32;
	pEphem->omegaot = dot_Omega = getbits(buff, i, 24)*P2_43*SC2RAD; i += 24;
	pEphem->tgd = tgd1 = getbits(buff, i, 10)*0.1*1E-9; i += 10;//?解码的单位是秒，不知道编码的单位是？
	pEphem->tgd2 = tgd2 = getbits(buff, i, 10)*0.1*1E-9; i += 10;
	pEphem->bHealth = health = getbitu(buff, i, 1); i += 1;
	if (pEphem->bHealth)
		//pEphem->blFlag = FALSE;//compile error

		pEphem->blFlag = false;
	else
	{
		if ((sqrtA < 4000.) || (sqrtA > 7000.))
			//pEphem->blFlag = FALSE;//compile error

			pEphem->blFlag = false;
		else
			pEphem->blFlag = true;
	}

	/*
  	pEphem->bIonoValid = 0;// 没有电离层参数
 	pEphem->m_wIdleTime = 130;
 	*/
    /*
	if (m_fp)
	{
		fprintf(m_fp, "=====================================================================================================================\n");
		fprintf(m_fp, "prn| week|URAI|   IDOT     |AODE|AODC|  toc |  toe |      a2    |      a1    |      a0    |    deltn   |      M0    |\n");
		fprintf(m_fp, "=====================================================================================================================\n");
		fprintf(m_fp, "%3d|%5d|%4d|%12.4e|%4d|%4d|%6d|%6d|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|\n", satID, WN, URAI, IDOT, AODE, AODC, toc, toe, a2, a1, a0, deltn, M0);


		fprintf(m_fp, "===================================================================================================================\n");
		fprintf(m_fp, "     crs    |     crc    |     cuc    |     cus    |     cic    |     cis    |     e      |   sqrtA  |  Omega0    |\n");
		fprintf(m_fp, "===================================================================================================================\n");
		fprintf(m_fp, "%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%10.4f|%12.4e|\n", crs, crc, cuc, cus, cic, cis, e, sqrtA, Omega0);

		fprintf(m_fp, "========================================================================\n");
		fprintf(m_fp, "    i0      |    omega   |  dot_Omega |    tgd1    |    tgd2    |health|\n");
		fprintf(m_fp, "========================================================================\n");
		fprintf(m_fp, "%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%6d|\n\n", i0, omega, dot_Omega, tgd1, tgd2, health);

		fflush(m_fp);
	}
    */
    
    #ifdef _VERBOSE
    printf("=====================================================================================================================\n");
	printf("prn| week|URAI|   IDOT     |AODE|AODC|  toc |  toe |      a2    |      a1    |      a0    |    deltn   |      M0    |\n");
	printf("=====================================================================================================================\n");
	printf("%3d|%5d|%4d|%12.4e|%4d|%4d|%6d|%6d|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|\n", satID, WN, URAI, IDOT, AODE, AODC, toc, toe, a2, a1, a0, deltn, M0);

    printf("===================================================================================================================\n");
	printf("     crs    |     crc    |     cuc    |     cus    |     cic    |     cis    |     e      |   sqrtA  |  Omega0    |\n");
	printf("===================================================================================================================\n");
	printf("%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%10.4f|%12.4e|\n", crs, crc, cuc, cus, cic, cis, e, sqrtA, Omega0);

	printf("========================================================================\n");
	printf("    i0      |    omega   |  dot_Omega |    tgd1    |    tgd2    |health|\n");
	printf("========================================================================\n");
	printf("%12.4e|%12.4e|%12.4e|%12.4e|%12.4e|%6d|\n\n", i0, omega, dot_Omega, tgd1, tgd2, health);
    #endif
	return 1;
}

/*iType: 0..L1/B1; 1..L2/B2; 2..L5/B3*/
bool bFindObsIndex(WangEpoch *pEpoch, int id, const char* strShort, int& iIndex, int& iType)
{
	bool bOK = false, bUpdateObsNum = false;
	int i;
    //2020-06-30,fixed
	//if (id <= 0 || (id > MAX_GLO_PRN && id < MIN_BD2_PRN) || id > MAX_BD2_PRN) return bOK; //这里仅处理GPS和BD2数据
	if (id <= 0 || (id > MAX_GLO_PRN && id < MIN_GAL_PRN) || id > MAX_BD2_PRN) return bOK; //这里仅处理GPS和BD2数据

	iIndex = -1;
	for (i = 0; i < MAXSAT; i++)
	{
		if (pEpoch->pbID[i] == id)
		{
			iIndex = i;
			break;
		}
		else if (pEpoch->pbID[i] == 0)
		{
			pEpoch->pbID[i] = id;
			iIndex = i;
			bUpdateObsNum = true;	//可能需要更新卫星数
			break;
		}
	}

	if (strstr(strShort, "L1") /* GPS,GLO,BD,GAL */
		|| strstr(strShort, "G1") || strstr(strShort, "B1")
        || strstr(strShort, "E1"))
	{//L1
		iType = 0;
	}
	else if (strstr(strShort, "L2") /* GPS,GLO,BD,GAL */
		|| strstr(strShort, "G2") || strstr(strShort, "B2")
        || strstr(strShort, "E5B"))
	{//L2
		iType = 1;
	}
	else if (strstr(strShort, "L5") || strstr(strShort, "B3")
	|| strstr(strShort, "E5A")) //2020-11-24 E5A,affect iType
	{//L5
		iType = 2;
	}
	else if (strstr(strShort, "   "))
	{//出现错误行
		//ASSERT(FALSE);//compile error
		assert(false);
		//return FALSE;
		return false;
	}

	if (iIndex >= 0 && iType >= 0)
	{
		if (bUpdateObsNum)
		{
			pEpoch->n++;
		}
		bOK = true;
	}

	return bOK;
}

