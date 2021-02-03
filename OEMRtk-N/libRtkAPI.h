#pragma once

#include <stdint.h>
#include "Common.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**解算结果回调函数
*@pRtkResult, solutionFormat:1-- RTK_result, 2-char*  解算结果
*/
//typedef void (*pRtkReport)(const RTK_result *pRtkResult);
typedef void (*pRtkReport)(const void *pRtkResult);

/*初始化Rtk库
*@callback,回调解算结果,不能为空指针,否则没有返回结果
*@return,0:表示成功;其他数值表示错误
*/
//int rtk_init(pRtkReport callback);
int rtk_init(pRtkReport callback, uint16_t constellationMask, uint16_t solutionFormat);

/**传递Rtcm报文,星历、观测量等数据
*@source:0:移动站;1:参考站
*@msgType:0:RTCM;其他暂不支持
*@msg:消息内容
*@len:消息内容长度
*@return:0:成功;其他值表示错误代码
*/
//int PushRtcm(int source, int msgType, const char* msg, int len);
int PushRtcm(int source, const char* msg, int len);

/**移动站定位信息
*@msg,移动站GGA数据
*@len,移动站GGA数据长度
*/
//int PushGGA(const char* msg, int len);
int PushGGA(int source, const char* msg, int len);

/**
*司南自定义星历，观测量数据结构；功能与传递RTCM报文相同
*/
//@{
/**传递星历
*@pEphem:GPS或者Galio星历
*@return:0:成功;其他值表示错误代码
*/
int PushEphemeris(const ServerEphemeris *pEphem);

/**传递星历
*@pEphem:Glonas星历
*@return:0:成功;其他值表示错误代码
*/
int PushGloEphemeris(const ServerGloEphemeris *pEphem);

/**传递观测量
*@source:0:移动站;1:参考站
*@pEpoch:观测量数据结构
*@return:0:成功;其他值表示错误代码
*/
int PushEpoch(int source, const WangEpoch *pEpoch);
//}@

/**结束解算，释放解算库资源
*/
void Clearup();

#ifdef __cplusplus
}
#endif
