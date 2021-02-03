#pragma once

class XPublic
{
private:
	double		m_pdFreqL1[MAXPRN];
	double		m_pdFreqL2[MAXPRN];
	double		m_pdFreqL3[MAXPRN];
	double		m_pdWaveL1[MAXPRN];		// L1/B1波长
	double		m_pdWaveL2[MAXPRN];		// L2/B2波长
	double		m_pdWaveL3[MAXPRN];		// L5/B3波长
	double		m_pdWaveLW[MAXPRN];		// (L1-L2)/(B1-B2)波长
	double		m_pdWaveLN[MAXPRN];		// 窄巷
	double		m_pdWaveW3[MAXPRN];		// (L1-L5)/(B1-B3)波长
	double		m_pdWaveN3[MAXPRN];		// (L1-L5)/(B1-B3)窄巷
	double		m_pdWaveEW[MAXPRN];		// 超宽巷波长
	double		m_pdIonoC2[MAXPRN];		// L2/B2电离层延迟系数
	double		m_pdIonoC3[MAXPRN];		// L5/B3电离层延迟系数
	double		m_pdIonoEW[MAXPRN];		// 超宽巷观测值的电离层延迟系数
public:
	//int			m_nLeapSecond;			// 闰秒，GLONASS解算需要使用
	//CNLock		m_XPublicLock;
public:
	XPublic()
	{
	//	this->m_nLeapSecond = LEAP_SECONDS;
		this->InitWaveLength();
	}

	~XPublic()
	{
	}

	inline void GetWaveLength(int id, double& dWaveL1, double& dWaveL2, double& dWaveL3)
	{
		//CNLockGuard mlock(&m_XPublicLock);
		dWaveL1 = this->m_pdWaveL1[id];
		dWaveL2 = this->m_pdWaveL2[id];
		dWaveL3 = this->m_pdWaveL3[id];
	}

	inline double dGetWideWaveLength(int id)
	{
		//CNLockGuard mlock(&m_XPublicLock);
		return this->m_pdWaveLW[id];
	}

	inline void GetFreq(int id, double& dFreqL1, double& dFreqL2, double& dFreqL3)
	{
		//CNLockGuard mlock(&m_XPublicLock);
		dFreqL1 = this->m_pdFreqL1[id];
		dFreqL2 = this->m_pdFreqL2[id];
		dFreqL3 = this->m_pdFreqL3[id];
	}

	/** 设置某颗卫星的L1/L2/L5的频率、相位以及对应的频率(单位：Hz)，注意L3不要为0*/
	inline void SetFreq(const int id, const double dF1, const double dF2, const double dF3)
	{
		//CNLockGuard mlock(&m_XPublicLock);
		this->m_pdFreqL1[id] = dF1;
		this->m_pdFreqL2[id] = dF2;
		this->m_pdFreqL3[id] = dF3;
		this->m_pdWaveL1[id] = LIGHT / dF1;
		this->m_pdWaveL2[id] = LIGHT / dF2;
		this->m_pdWaveL3[id] = LIGHT / dF3;

		this->m_pdWaveLW[id] = LIGHT / (dF1 - dF2);
		this->m_pdWaveLN[id] = LIGHT / (dF1 + dF2);
		this->m_pdWaveW3[id] = LIGHT / (dF1 - dF3);
		this->m_pdWaveN3[id] = LIGHT / (dF1 + dF3);
		this->m_pdWaveEW[id] = LIGHT / (dF2 - dF3);
		this->m_pdIonoC2[id] = (dF1 * dF1) / (dF2 * dF2);
		this->m_pdIonoC3[id] = (dF1 * dF1) / (dF3 * dF3);
		this->m_pdIonoEW[id] = (dF1 * dF1) / (dF2 * dF3);
	}

	/**初始化各颗卫星的频率、波长及电离层系数*/
	void InitWaveLength()
	{
		//CNLockGuard mlock(&m_XPublicLock);
		int i;
		double dF1, dF2, dF3;

		for(i = 0; i < MAXPRN; i++)
		{
			if(i >= MIN_BD2_PRN && i <= MAX_BD2_PRN) 
			{
				dF1 = BD2_F1;
				dF2 = BD2_F2;
				dF3 = BD2_F3;
			}
			else
			{
				dF1 = GPS_F1;
				dF2 = GPS_F2;
				dF3 = GPS_F5;
			}

			this->m_pdFreqL1[i] = dF1;
			this->m_pdFreqL2[i] = dF2;
			this->m_pdFreqL3[i] = dF3;
			this->m_pdWaveL1[i] = LIGHT / dF1;
			this->m_pdWaveL2[i] = LIGHT / dF2;
			this->m_pdWaveL3[i] = LIGHT / dF3;
			this->m_pdWaveLW[i] = LIGHT / (dF1 - dF2);
			this->m_pdWaveLN[i] = LIGHT / (dF1 + dF2);
			this->m_pdWaveW3[i] = LIGHT / (dF1 - dF3);
			this->m_pdWaveN3[i] = LIGHT / (dF1 + dF3);
			this->m_pdWaveEW[i] = LIGHT / (dF2 - dF3);
			this->m_pdIonoC2[i] = (dF1 * dF1) / (dF2 * dF2);
			this->m_pdIonoC3[i] = (dF1 * dF1) / (dF3 * dF3);
			this->m_pdIonoEW[i] = (dF1 * dF1) / (dF2 * dF3);
		}
	}

};

