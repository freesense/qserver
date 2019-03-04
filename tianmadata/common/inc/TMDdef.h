//TMDdef.h
#ifndef _TMDDEF_H_
#define _TMDDEF_H_
#include "TianmaDataExport.h"
#include "../../public/protocol.h"
/////////////////////////////////////////////////////////////////////////////////
//客户端请求
#define STOCK_INFO      1001	//证券信息 
#define INDUSTRY_INFO	1002	//行业分类 
#define AREA_INFO		1003	//区域分类 
#define INDEX_INFO		1004	//指数分类 
#define HQ_INDEX        1005	//股票行情类指标 

#define PAYOFF_COLLECT	1006	//盈利预测汇总
#define PAYOFF_BILL     1007	//盈利预测明细
#define COMMENTARY_BILL 1008    //投资评级明细
#define IDX_INDEX		1009    //指数类指标
#define IDU_INDEX       1010    //行业类指标
#define SUBTAB_INFO     1011    //科目表
#define COMMENTARY_COLLECT 1012 //评级汇总
#define ANALYSER_INFO   1013	//分析师信息
#define MKT_FACTOR_IDX  1014	//市场因素指标
#define MKT_MIX_IDX     1015	//市场混合指标

#define ANALYSER_REMARK 1200	//分析师评论
#define ADD_REMARK      1201	//增加分析师评论

#define HIS_FINANCE_IDX 1300    //历史财务类指标
#define INDEX_HIS_DATA  1301    //指标历史数据
#define IDU_REPRE_DATA  1302    //行业表现数据
#define IDU_SORT_DATA   1303    //行业按给定因素排名数据


#define VERIFY_USERINFO	2000	//用户信息验证
#define MODIFY_PASSWORD	2001	//修改密码
#define SELSTKMOD_INFO  2004	//选股模型信息
#define SELSTKMOD_DATA	2005	//选股模型数据


//科目表的编码
#define  STK16_FBSC     1001
#define  STK17_FHIND    1002
#define  STK18_ASSET1   1003
#define  STK19_ASSET2   1004
#define  STK20_PROFIT1  1005
#define  STK21_PROFIT2  1006
#define  STK22_CASH1    1007
#define  STK23_CASH2    1008


//市场类别
#define MT_CN           "CN"	//大陆市场
#define MT_HK           "HK"	//香港市场

//指数型指标定义
const unsigned short INDEX_IDX_CM0201=1001;//短期趋势
const unsigned short INDEX_IDX_CM0202=1002;//中期趋势
const unsigned short INDEX_IDX_CM0203=1003;//主要趋势
const unsigned short INDEX_IDX_CM0211=1004;//当前PE
const unsigned short INDEX_IDX_CM0212=1005;//过去财年PE
const unsigned short INDEX_IDX_CM0213=1006;//本财年PE
const unsigned short INDEX_IDX_CM0214=1007;//下一财年PE
const unsigned short INDEX_IDX_CM0215=1008;//3年最高PE
const unsigned short INDEX_IDX_CM0216=1009;//3年最低PE
const unsigned short INDEX_IDX_CM0217=1010;//当前PB
const unsigned short INDEX_IDX_CM0221=1011;//1周涨跌幅
const unsigned short INDEX_IDX_CM0222=1012;//2周涨跌幅
const unsigned short INDEX_IDX_CM0223=1013;//1月涨跌幅
const unsigned short INDEX_IDX_CM0224=1014;//3月涨跌幅
const unsigned short INDEX_IDX_CM0225=1015;//6月涨跌幅
const unsigned short INDEX_IDX_CM0226=1016;//1年涨跌幅


const unsigned short INDEX_IDX_CM0301=1017;//最近季度净利润增长率
const unsigned short INDEX_IDX_CM0302=1018;//最近季度净利润增长率
const unsigned short INDEX_IDX_CM0303=1019;//最近季度业务收入增长率
const unsigned short INDEX_IDX_CM0304=1020;//最近季度业务收入增长率

const unsigned short INDEX_IDX_CM0601=1021;// 价值因素 新增加
const unsigned short INDEX_IDX_CM0602=1022;// 增长因素 新增加
const unsigned short INDEX_IDX_CM0603=1023;// 回报动能 新增加
const unsigned short INDEX_IDX_CM0604=1024; //回报反转 新增加
const unsigned short INDEX_IDX_CM0605=1025; //盈利动能 新增加
const unsigned short INDEX_IDX_CM0606=1026; //评级变化 新增加
const unsigned short INDEX_IDX_CM0607=1027; //分析师评级 新增加
const unsigned short INDEX_IDX_CM0608=1028; //市值价格 新增加
const unsigned short INDEX_IDX_CM0609=1029; //RTDM因素 新增加
const unsigned short INDEX_IDX_CM0610=1030; //资金流/机构持仓比例 新增加

const unsigned short INDEX_IDX_CM0701=1031; //Earning Visib 新增加
const unsigned short INDEX_IDX_CM0702=1032; //Earning Momen 新增加
const unsigned short INDEX_IDX_CM0703=1033; //Upgrade/DownGrade 新增加
const unsigned short INDEX_IDX_CM0704=1034; //Volatility（3month） 新增加
const unsigned short INDEX_IDX_CM0705=1035; //AAII(Bull-Bear)% 新增加
const unsigned short INDEX_IDX_CM0706=1036; //过去一周起作用的市场因素 新增加
const unsigned short INDEX_IDX_CM0707=1037; //过去一月起作用的市场因素 新增加
const unsigned short INDEX_IDX_CM0708=1038; //过去一周表现最好的行业 新增加
const unsigned short INDEX_IDX_CM0709=1039; //过去一月表现最好的行业 新增加

//行业型指标定义
const unsigned short INDEX_IDU_CI0201 = 2001;//总市值
const unsigned short INDEX_IDU_CI0202 = 2002;//流通市值
const unsigned short INDEX_IDU_CI0210 = 2003;//基金持股比列
const unsigned short INDEX_IDU_CI0211 = 2004;//当前PE
const unsigned short INDEX_IDU_CI0212 = 2005;//过去财年PE
const unsigned short INDEX_IDU_CI0213 = 2006;//本财年PE
const unsigned short INDEX_IDU_CI0214 = 2007;//下一财年PE
const unsigned short INDEX_IDU_CI0215 = 2008;//3年最高PE
const unsigned short INDEX_IDU_CI0216 = 2009;//3年最低PE
const unsigned short INDEX_IDU_CI0217 = 2010;//PERelative
const unsigned short INDEX_IDU_CI0218 = 2011;//PEG(1年增长率)
const unsigned short INDEX_IDU_CI0219 = 2012;//PEG(3年增长率)
const unsigned short INDEX_IDU_CI0220 = 2013;//股价/最近季度每股业务收入
const unsigned short INDEX_IDU_CI0221 = 2014;//股价/最近季度每股业务收入
const unsigned short INDEX_IDU_CI0222 = 2015;//当前PB
const unsigned short INDEX_IDU_CI0223 = 2016;//股价/每股有形净资产
const unsigned short INDEX_IDU_CI0224 = 2017;//股价/每股现金流
const unsigned short INDEX_IDU_CI0225 = 2018;//股价/每股自由现金流
const unsigned short INDEX_IDU_CI0226 = 2019;//每股营运资本/股价
const unsigned short INDEX_IDU_CI0227 = 2020;//最近季度每股营运资本/股价
const unsigned short INDEX_IDU_CI0231 = 2021;//1周涨跌幅
const unsigned short INDEX_IDU_CI0232 = 2022;//1月涨跌幅
const unsigned short INDEX_IDU_CI0233 = 2023;//3月涨跌幅
const unsigned short INDEX_IDU_CI0234 = 2024;//6月涨跌幅
const unsigned short INDEX_IDU_CI0235 = 2025;//1年涨跌幅
const unsigned short INDEX_IDU_CI0236 = 2026;//1周相对涨跌幅
const unsigned short INDEX_IDU_CI0237 = 2027;//1月相对涨跌幅
const unsigned short INDEX_IDU_CI0238 = 2028;//3月相对涨跌幅
const unsigned short INDEX_IDU_CI0239 = 2029;//6月相对涨跌幅
const unsigned short INDEX_IDU_CI0240 = 2030;//1年相对涨跌幅
const unsigned short INDEX_IDU_CI0241 = 2031;//1周换手率
const unsigned short INDEX_IDU_CI0242 = 2032;//1月换手率
const unsigned short INDEX_IDU_CI0243 = 2033;//3月换手率
const unsigned short INDEX_IDU_CI0244 = 2034;//6月换手率
const unsigned short INDEX_IDU_CI0245 = 2035;//1年换手率
const unsigned short INDEX_IDU_CI0246 = 2036;//新增加
const unsigned short INDEX_IDU_CI0247 = 2037;//新增加

const unsigned short INDEX_IDU_CI0301 = 2038;//最近季度净利润增长率
const unsigned short INDEX_IDU_CI0302 = 2039;//最近季度净利润增长率
const unsigned short INDEX_IDU_CI0303 = 2040;//3年净利润复合增长率
const unsigned short INDEX_IDU_CI0304 = 2041;//最近季度业务收入增长率
const unsigned short INDEX_IDU_CI0305 = 2042;//最近季度业务收入增长率
const unsigned short INDEX_IDU_CI0306 = 2043;//3年业务收入复合增长率
const unsigned short INDEX_IDU_CI0307 = 2044;//最近季度现金流增长率
const unsigned short INDEX_IDU_CI0308 = 2045;//最近季度现金流增长率
const unsigned short INDEX_IDU_CI0309 = 2046;//最近季度自由现金流增长率
const unsigned short INDEX_IDU_CI0310 = 2047;//最近季度自由现金流增长率
const unsigned short INDEX_IDU_CI0311 = 2048;//自由现金流年平均增长率
const unsigned short INDEX_IDU_CI0312 = 2049;//最近季度资产周转率
const unsigned short INDEX_IDU_CI0313 = 2050;//最近季度资产周转率
const unsigned short INDEX_IDU_CI0314 = 2051;//最近季度库存周转率
const unsigned short INDEX_IDU_CI0315 = 2052;//最近季度库存周转率
const unsigned short INDEX_IDU_CI0316 = 2053;//最近季度应收帐周转率
const unsigned short INDEX_IDU_CI0317 = 2054;//最近季度应收帐周转率
const unsigned short INDEX_IDU_CI0318 = 2055;//最近季度毛利率
const unsigned short INDEX_IDU_CI0319 = 2056;//最近季度毛利率
const unsigned short INDEX_IDU_CI0320 = 2057;//3年平均毛利率
const unsigned short INDEX_IDU_CI0321 = 2058;//最近季度净利润率
const unsigned short INDEX_IDU_CI0322 = 2059;//最近季度净利润率
const unsigned short INDEX_IDU_CI0323 = 2060;//3年平均净利润率
const unsigned short INDEX_IDU_CI0324 = 2061;//最近季度营运利润率
const unsigned short INDEX_IDU_CI0325 = 2062;//最近季度营运利润率
const unsigned short INDEX_IDU_CI0326 = 2063;//3年平均营运利润率
const unsigned short INDEX_IDU_CI0327 = 2064;//最近季度资产回报率
const unsigned short INDEX_IDU_CI0328 = 2065;//最近季度资产回报率
const unsigned short INDEX_IDU_CI0329 = 2066;//3年平均资产回报率
const unsigned short INDEX_IDU_CI0330 = 2067;//最近季度股东权益回报率
const unsigned short INDEX_IDU_CI0331 = 2068;//最近季度股东权益回报率
const unsigned short INDEX_IDU_CI0332 = 2069;//3年平均股东权益回报率
const unsigned short INDEX_IDU_CI0333 = 2070;//最近季度投资回报率
const unsigned short INDEX_IDU_CI0334 = 2071;//最近季度投资回报率
const unsigned short INDEX_IDU_CI0335 = 2072;//3年平均投资回报率
const unsigned short INDEX_IDU_CI0336 = 2073;//3年ROA平均增长率
const unsigned short INDEX_IDU_CI0337 = 2074;//3年ROE平均增长率
const unsigned short INDEX_IDU_CI0338 = 2075;//最近季度营运利润率变化
const unsigned short INDEX_IDU_CI0339 = 2076;//最近季度营运利润率变化
const unsigned short INDEX_IDU_CI0340 = 2077;//最近季度流动比率
const unsigned short INDEX_IDU_CI0341 = 2078;//最近季度速动比率
const unsigned short INDEX_IDU_CI0342 = 2079;//最近季度长期负债/总资产
const unsigned short INDEX_IDU_CI0343 = 2080;//最近季度长期负债/股东权益
const unsigned short INDEX_IDU_CI0344 = 2081;//最近季度利息支出/盈利
const unsigned short INDEX_IDU_CI0345 = 2082;//最近季度利息支出/盈利
const unsigned short INDEX_IDU_CI0346 = 2083;//最近季度总负债/总资产
const unsigned short INDEX_IDU_CI0347 = 2084;//最近季度总负债/股东权益
const unsigned short INDEX_IDU_CI0348 = 2085;//最近季度利息保障倍数
const unsigned short INDEX_IDU_CI0349 = 2086;//最近季度利息保障倍数

const unsigned short INDEX_IDU_CI0401 = 2087;//评级分析，月本财年盈利调准比例
const unsigned short INDEX_IDU_CI0402 = 2088;//评级分析，月下一财年盈利调准比例
const unsigned short INDEX_IDU_CI0403 = 2089;//新增加
const unsigned short INDEX_IDU_CI0404 = 2090; //新增加
const unsigned short INDEX_IDU_CI0405 = 2091; //新增加
const unsigned short INDEX_IDU_CI0406 = 2092; //新增加
const unsigned short INDEX_IDU_CI0407 = 2093; //新增加
const unsigned short INDEX_IDU_CI0408 = 2094; //新增加
const unsigned short INDEX_IDU_CI0409 = 2095; //新增加
const unsigned short INDEX_IDU_CI0410 = 2096; //新增加
const unsigned short INDEX_IDU_CI0411 = 2097; //新增加
const unsigned short INDEX_IDU_CI0412 = 2098; //新增加
const unsigned short INDEX_IDU_CI0413 = 2099; //新增加

//股票类指标
const unsigned short INDEX_STK_CS0201 = 4001;//总市值
const unsigned short INDEX_STK_CS0202 = 4002;//流通市值
const unsigned short INDEX_STK_CS0203 = 4003;//(总市值+总负债)/最近季度业务收入
const unsigned short INDEX_STK_CS0209 = 4004;//高管持股比列
const unsigned short INDEX_STK_CS0210 = 4005;//基金持股比列
const unsigned short INDEX_STK_CS0211 = 4006;//当前PE
const unsigned short INDEX_STK_CS0212 = 4007;//过去财年PE
const unsigned short INDEX_STK_CS0213 = 4008;//本财年PE
const unsigned short INDEX_STK_CS0214 = 4009;//下一财年PE
const unsigned short INDEX_STK_CS0215 = 4010;//3年最高PE
const unsigned short INDEX_STK_CS0216 = 4011;//3年最低PE
const unsigned short INDEX_STK_CS0217 = 4012;//PERelative
const unsigned short INDEX_STK_CS0218 = 4013;//PEG(1年增长率)
const unsigned short INDEX_STK_CS0219 = 4014;//PEG(3年增长率)
const unsigned short INDEX_STK_CS0220 = 4015;//股价/最近季度每股业务收入
const unsigned short INDEX_STK_CS0221 = 4016;//股价/最近季度每股业务收入
const unsigned short INDEX_STK_CS0222 = 4017;//当前PB
const unsigned short INDEX_STK_CS0223 = 4018;//股价/每股有形净资产
const unsigned short INDEX_STK_CS0224 = 4019;//股价/每股现金流
const unsigned short INDEX_STK_CS0225 = 4020;//股价/每股自由现金流
const unsigned short INDEX_STK_CS0226 = 4021;//每股营运资本/股价
const unsigned short INDEX_STK_CS0227 = 4022;//最近季度每股营运资本/股价
const unsigned short INDEX_STK_CS0231 = 4023;//1周涨跌幅
const unsigned short INDEX_STK_CS0232 = 4024;//1月涨跌幅
const unsigned short INDEX_STK_CS0233 = 4025;//3月涨跌幅
const unsigned short INDEX_STK_CS0234 = 4026;//6月涨跌幅
const unsigned short INDEX_STK_CS0235 = 4027;//1年涨跌幅
const unsigned short INDEX_STK_CS0236 = 4028;//1周相对涨跌幅
const unsigned short INDEX_STK_CS0237 = 4029;//1月相对涨跌幅
const unsigned short INDEX_STK_CS0238 = 4030;//3月相对涨跌幅
const unsigned short INDEX_STK_CS0239 = 4031;//6月相对涨跌幅
const unsigned short INDEX_STK_CS0240 = 4032;//1年相对涨跌幅
const unsigned short INDEX_STK_CS0241 = 4033;//1周换手率
const unsigned short INDEX_STK_CS0242 = 4034;//1月换手率
const unsigned short INDEX_STK_CS0243 = 4035;//3月换手率
const unsigned short INDEX_STK_CS0244 = 4036;//6月换手率
const unsigned short INDEX_STK_CS0245 = 4037;//1年换手率
const unsigned short INDEX_STK_CS0246 = 4038;//2周平均成交量/3月平均成交量
const unsigned short INDEX_STK_CS0247 = 4039;//1月平均成交量/6月平均成交量

const unsigned short INDEX_STK_CS0301 = 4040;//最近季度每股盈利
const unsigned short INDEX_STK_CS0302 = 4041;//最近季度每股盈利
const unsigned short INDEX_STK_CS0303 = 4042;//最近季度每股可分配利润
const unsigned short INDEX_STK_CS0311 = 4043;//最近季度每股盈利增长率
const unsigned short INDEX_STK_CS0312 = 4044;//最近季度每股盈利增长率
const unsigned short INDEX_STK_CS0313 = 4045;//3年每股盈利复合增长率
const unsigned short INDEX_STK_CS0314 = 4046;//最近季度净利润增长率
const unsigned short INDEX_STK_CS0315 = 4047;//最近季度净利润增长率
const unsigned short INDEX_STK_CS0316 = 4048;//3年净利润复合增长率
const unsigned short INDEX_STK_CS0317 = 4049;//最近季度业务收入增长率
const unsigned short INDEX_STK_CS0318 = 4050;//最近季度业务收入增长率
const unsigned short INDEX_STK_CS0319 = 4051;//3年业务收入复合增长率
const unsigned short INDEX_STK_CS0320 = 4052;//最近季度现金流增长率
const unsigned short INDEX_STK_CS0321 = 4053;//最近季度现金流增长率
const unsigned short INDEX_STK_CS0322 = 4054;//最近季度自由现金流增长率
const unsigned short INDEX_STK_CS0323 = 4055;//最近季度自由现金流增长率
const unsigned short INDEX_STK_CS0324 = 4056;//自由现金流年平均增长率
const unsigned short INDEX_STK_CS0325 = 4057;//最近季度资产周转率
const unsigned short INDEX_STK_CS0326 = 4058;//最近季度资产周转率
const unsigned short INDEX_STK_CS0327 = 4059;//最近季度库存周转率
const unsigned short INDEX_STK_CS0328 = 4060;//最近季度库存周转率
const unsigned short INDEX_STK_CS0329 = 4061;//最近季度应收帐周转率
const unsigned short INDEX_STK_CS0330 = 4062;//最近季度应收帐周转率
const unsigned short INDEX_STK_CS0331 = 4063;//最近季度毛利率
const unsigned short INDEX_STK_CS0332 = 4064;//最近季度毛利率
const unsigned short INDEX_STK_CS0333 = 4065;//3年平均毛利率
const unsigned short INDEX_STK_CS0334 = 4066;//最近季度净利润率
const unsigned short INDEX_STK_CS0335 = 4067;//最近季度净利润率
const unsigned short INDEX_STK_CS0336 = 4068;//3年平均净利润率
const unsigned short INDEX_STK_CS0337 = 4069;//最近季度营运利润率
const unsigned short INDEX_STK_CS0338 = 4070;//最近季度营运利润率
const unsigned short INDEX_STK_CS0339 = 4071;//3年平均营运利润率
const unsigned short INDEX_STK_CS0340 = 4072;//最近季度资产回报率
const unsigned short INDEX_STK_CS0341 = 4073;//最近季度资产回报率
const unsigned short INDEX_STK_CS0342 = 4074;//3年平均资产回报率
const unsigned short INDEX_STK_CS0343 = 4075;//最近季度股东权益回报率
const unsigned short INDEX_STK_CS0344 = 4076;//最近季度股东权益回报率
const unsigned short INDEX_STK_CS0345 = 4077;//3年平均股东权益回报率
const unsigned short INDEX_STK_CS0346 = 4078;//最近季度投资回报率
const unsigned short INDEX_STK_CS0347 = 4079;//最近季度投资回报率
const unsigned short INDEX_STK_CS0348 = 4080;//3年平均投资回报率
const unsigned short INDEX_STK_CS0349 = 4081;//3年ROA平均增长率
const unsigned short INDEX_STK_CS0350 = 4082;//3年ROE平均增长率
const unsigned short INDEX_STK_CS0351 = 4083;//最近季度营运利润率变化
const unsigned short INDEX_STK_CS0352 = 4084;//最近季度营运利润率变化
const unsigned short INDEX_STK_CS0353 = 4085;//最近季度流动比率
const unsigned short INDEX_STK_CS0354 = 4086;//最近季度速动比率
const unsigned short INDEX_STK_CS0355 = 4087;//最近季度长期负债/总资产
const unsigned short INDEX_STK_CS0356 = 4088;//最近季度长期负债/股东权益
const unsigned short INDEX_STK_CS0357 = 4089;//最近季度利息支出/盈利
const unsigned short INDEX_STK_CS0358 = 4090;//最近季度利息支出/盈利
const unsigned short INDEX_STK_CS0359 = 4091;//最近季度总负债/总资产
const unsigned short INDEX_STK_CS0360 = 4092;//最近季度总负债/股东权益
const unsigned short INDEX_STK_CS0361 = 4093;//最近季度利息保障倍数
const unsigned short INDEX_STK_CS0362 = 4094;//最近季度利息保障倍数

const unsigned short INDEX_STK_CS0401 = 4095;//本财年平均预测eps
const unsigned short INDEX_STK_CS0402 = 4096;//本财年预测eps标准偏差
const unsigned short INDEX_STK_CS0403 = 4097;//1月本财年eps变化
const unsigned short INDEX_STK_CS0404 = 4098;//2月本财年eps变化
const unsigned short INDEX_STK_CS0405 = 4099;//3月本财年eps变化
const unsigned short INDEX_STK_CS0406 = 4100;//1周上调本财年盈利预测人数
const unsigned short INDEX_STK_CS0407 = 4101;//1月上调本财年盈利预测人数
const unsigned short INDEX_STK_CS0408 = 4102;//1周下调本财年盈利预测人数
const unsigned short INDEX_STK_CS0409 = 4103;//1月下调本财年盈利预测人数
const unsigned short INDEX_STK_CS0411 = 4104;//下一财年平均预测eps
const unsigned short INDEX_STK_CS0412 = 4105;//下一财年预测eps标准偏差
const unsigned short INDEX_STK_CS0413 = 4106;//1月下一财年eps变化
const unsigned short INDEX_STK_CS0414 = 4107;//2月下一财年eps变化
const unsigned short INDEX_STK_CS0415 = 4108;//3月下一财年eps变化
const unsigned short INDEX_STK_CS0416 = 4109;//1周上调下一财盈利预测人数
const unsigned short INDEX_STK_CS0417 = 4110;//1月上调下一财盈利预测人数
const unsigned short INDEX_STK_CS0418 = 4111;//1周下调下一财盈利预测人数
const unsigned short INDEX_STK_CS0419 = 4112;//1月下调下一财盈利预测人数
const unsigned short INDEX_STK_CS0420 = 4113;//上财年实际eps和预测eps偏差

const unsigned short INDEX_STK_AVG_VALUE = 4114;// 			Avg_Value;
const unsigned short INDEX_STK_TBU_NUM = 4115;// 			Tbu_Num;
const unsigned short INDEX_STK_BUY_NUM = 4116;//			Buy_Num;
const unsigned short INDEX_STK_NEU_NUM = 4117;//			Neu_Num;
const unsigned short INDEX_STK_RED_NUM = 4118;//			Red_Num;
const unsigned short INDEX_STK_SEL_NUM = 4119;//			Sel_Num;
const unsigned short INDEX_STK_AVG_PRICE = 4120;//			Avg_Price;
const unsigned short INDEX_STK_HIG_PRICE = 4121;//			Hig_Price;
const unsigned short INDEX_STK_LOW_PRICE = 4122;//			Low_Price;

//结构体
#pragma pack(push)
#pragma pack(1)
struct tagReturnMsgID
{
	SMsgID    id;
	char*     pBuf;
	int       nBufSize;
};

//包头
struct tagPackHead
{
	CommxHead       comxHead;     //通讯包头 
	unsigned short  nFuncNo;	  //功能号
	unsigned short  nReqCount;	  //请求个数
};

//请求和应答共同信息
struct tagComInfo
{
	unsigned short  nPacktype;      //报文类型
	unsigned char   MarketType[2];  //市场类别
};

//证券信息
struct tagStockInfo
{
	unsigned char   Sec_cd[8];	  //股票代码
	unsigned char   Sec_id[10];	  //证券标识
	unsigned char   Org_id[10];	  //公司代码
	unsigned char   Idu_cd[10];	  //行业代码
	unsigned char   Area_cd[10];  //区域代码
	unsigned char   Var_cl[2];    //证券品种代码
	unsigned char   Mkt_cl[1];    //交易市场代码
	unsigned char   Sec_snm[16];  //证券中文简称
	unsigned char   Sec_esnm[16]; //证券英文简称
	unsigned char   Sec_ssnm[4];  //证券拼音简称
	unsigned int    nIss_dt;       //发行日期
	unsigned int    nMkt_dt;      //上市日期
};

//行业分类信息
struct tagIndustryInfo
{
	unsigned char   Idu_cd[10];	  //行业代码
	unsigned char   Idu_nm[40];	  //行业名称
};

//指数分类信息
struct tagIndexInfo
{
	unsigned char   Idx_cd[10];	  //指数代码
	unsigned char   Var_cl[2];    //证券品种代码
	unsigned char   Mkt_cl[1];    //交易市场代码
	unsigned char   Idx_snm[16];  //证券中文简称
	unsigned char   Idx_esnm[16]; //证券英文简称
	unsigned char   Idx_ssnm[4];  //证券拼音简称
	unsigned short  nRecordCount; //此指数包含的股票总数
	//股票代码 (重复)
};

struct tagMkt_Idx
{
	unsigned char   Idx_cd[10];	  //指数代码
	unsigned char   Var_cl[2];    //证券品种代码
	unsigned char   Mkt_cl[1];    //交易市场代码
	unsigned char   Idx_snm[16];  //证券中文简称
	unsigned char   Idx_esnm[16]; //证券英文简称
	unsigned char   Idx_ssnm[4];  //证券拼音简称
	unsigned short  nRecordCount; //此指数包含的股票总数
	char*           pStockCode;   //股票代码 (重复), 每个8个字节
};

//市场类别和股票代码
struct tagMarketandStock
{
	unsigned char   MarketType[2];  //市场类别
	unsigned char   Sec_cd[8];	    //股票代码
};

//股票行情类指标 125
struct tagHQIndex
{
	unsigned char   MarketType[2];  //市场类别
	unsigned char   Sec_cd[8];	    //股票代码

	double   cs0201;		//总市值
	double   cs0202;		//流通市值
	double   cs0203;		//(总市值+总负债)/最近4季度业务收入
	double   cs0209;		//高管持股比列
	double   cs0210;		//基金持股比列

	double   cs0211;		//当前PE
	double   cs0212;		//过去财年PE
	double   cs0213;		//本财年PE
	double   cs0214;		//下一财年PE
	double   cs0215;		//3年最高PE

	double   cs0216;		//3年最低PE
	double   cs0217;		//PERelative
	double   cs0218;		//PEG(1年增长率)
	double   cs0219;		//PEG(3年增长率)
	double   cs0220;		//股价/最近1季度每股业务收入

	double   cs0221;		//股价/最近4季度每股业务收入
	double   cs0222;		//当前PB
	double   cs0223;		//股价/每股有形净资产
	double   cs0224;		//股价/每股现金流
	double   cs0225;		//股价/每股自由现金流

	double   cs0226;		//每股营运资本/股价
	double   cs0227;		//最近季度每股营运资本/股价
	double   cs0231;		//1周涨跌幅
	double   cs0232;		//1月涨跌幅
	double   cs0233;		//3月涨跌幅

	double   cs0234;		//6月涨跌幅
	double   cs0235;		//1年涨跌幅
	double   cs0236;		//1周相对涨跌幅
	double   cs0237;		//1月相对涨跌幅
	double   cs0238;		//3月相对涨跌幅

	double   cs0239;		//6月相对涨跌幅
	double   cs0240;		//1年相对涨跌幅
	double   cs0241;		//1周换手率
	double   cs0242;		//1月换手率
	double   cs0243;		//3月换手率

	double   cs0244;		//6月换手率
	double   cs0245;		//1年换手率
	double   cs0246;		//2周平均成交量/3月平均成交量
	double   cs0247;		//1月平均成交量/6月平均成交量
	double   cs0301;

	double   cs0302;
	double   cs0303;
	double   cs0311;
	double   cs0312;
	double   cs0313;

	double   cs0314;
	double   cs0315;
	double   cs0316;
	double   cs0317;
	double   cs0318;

	double   cs0319;
	double   cs0320;
	double   cs0321;
	double   cs0322;
	double   cs0323;

	double   cs0324;
	double   cs0325;
	double   cs0326;
	double   cs0327;
	double   cs0328;

	double   cs0329;
	double   cs0330;
	double   cs0331;
	double   cs0332;
	double   cs0333;

	double   cs0334;
	double   cs0335;
	double   cs0336;
	double   cs0337;
	double   cs0338;

	double   cs0339;
	double   cs0340;
	double   cs0341;
	double   cs0342;
	double   cs0343;

	double   cs0344;
	double   cs0345;
	double   cs0346;
	double   cs0347;
	double   cs0348;

	double   cs0349;
	double   cs0350;
	double   cs0351;
	double   cs0352;
	double   cs0353;

	double   cs0354;
	double   cs0355;
	double   cs0356;
	double   cs0357;
	double   cs0358;

	double   cs0359;
	double   cs0360;
	double   cs0361;
	double   cs0362;
	double   cs0401;

	double   cs0402;
	double   cs0403;
	double   cs0404;
	double   cs0405;
	double   cs0406;

	double   cs0407;
	double   cs0408;
	double   cs0409;
	double   cs0410;
	double   cs0411;

	double   cs0412;
	double   cs0413;
	double   cs0414;
	double   cs0415;
	double   cs0416;

	double   cs0417;
	double   cs0418;
	double   cs0419;
	double   cs0420;
	double   Avg_Value;

	double   Tbu_Num;
	double   Buy_Num;
	double   Neu_Num;
	double   Red_Num;
	double   Sel_Num;

	double   Avg_Price;
	double   Hig_Price;
	double   Low_Price;
};

//盈利预测汇总
struct tagPayOffCollect
{
	unsigned char IND_ID[6];   //指标代码
	unsigned char Rpt_Yr[4];   //预测年份
	unsigned char Rpt_Prd[4];  //预测时段
	unsigned char Sta_Prd[4];  //统计时段
	double  Avg_Value;		   //预测平均值
	double  Hig_Value;		   //预测最高
	double  Low_Value;		   //预测最低
	unsigned short Ana_Num;    //分析师人数
};

struct tagPayOffCollectEx
{
	unsigned char SEC_CD[8];
	tagPayOffCollect  collect;
};

//盈利预测明细
struct tagPayOffBill
{
	unsigned int  ORG_CL;         //研究机构代码
	unsigned char IND_ID[6];      //指标代码
	unsigned char For_Date[8];	  //预测时间YYYYMMDD
	unsigned char Rpt_Yr[4];	  //预测年份
	unsigned char Rpt_Prd[4];     //预测时段
	double For_Value;             //预测值
	unsigned char Author[100];     //作者
	double Dil_Value;             //按股本摊薄预测值	
	unsigned char Dil_Date[8];    //摊薄股本的变动日YYYYMMDD
};

//投资评级明细
struct tagCommentaryBill
{
	unsigned int  ORG_CL;         //研究机构代码
	unsigned char Gra_Date[8];	  //评级时间YYYYMMDD
	unsigned char Gra_Prd;        //评级时段
	double Gra_Value;             //评级指数
	double Tar_Price;             //目标价
	unsigned char Tar_Prd[3];    //价位时段
	unsigned char Author[100];      //作者
	double Exr_Price;             //除权的目标价位
	unsigned char Exr_Date[8];    //最近除权日期YYYYMMDD
};

struct tagCommentaryBillEx
{
	unsigned char SEC_CD[8];      //股票代码
	tagCommentaryBill bill;
};

//指数类指标  24   注意：大陆新增了字段，而港股没有增加，tagIDXIndex已经不一致啦，注意修改HK_MKT02_Qind.cpp
struct tagIDXIndex
{
	unsigned char   MarketType[2];  //市场类别
	unsigned char   IDX_CD[8];	    //指数代码
	
	unsigned char cm0201[10];       //短期趋势
	unsigned char cm0202[10];       //中期趋势
	unsigned char cm0203[10];       //主要趋势
	double cm0211;                  //当前PE
	double cm0212;                  //过去财年PE

	double cm0213;                  //本财年PE
	double cm0214;                  //下一财年PE
	double cm0215;                  //3年最高PE
	double cm0216;                  //3年最低PE
	double cm0217;                  //当前PB
	double cm0218;					//PEG(1年增长率)  新增
	double cm0219;					//PEG(3年增长率)  新增

	double cm0221;                  //1周涨跌幅
	double cm0222;					//2周涨跌幅  新增
	double cm0223;                  //1月涨跌幅
	double cm0224;                  //3月涨跌幅
	double cm0225;                  //6月涨跌幅
	double cm0226;                  //1年涨跌幅

	double cm0301;                  //最近1季度净利润增长率
	double cm0302;                  //最近4季度净利润增长率
	double cm0303;                  //最近1季度业务收入增长率
	double cm0304;                  //最近4季度业务收入增长率
};

//行业类指标 101 注意：大陆新增了字段，而港股没有增加，tagIduIndex已经不一致啦，注意修改HK_IDU02_Qind.cpp
struct tagIduIndex
{
	unsigned char   MarketType[2];  //市场类别
	unsigned char   IDU_CD[8];	    //行业代码
	
	double ci0201;                  //总市值
	double ci0202;                  //流通市值
	double ci0210;                  //基金持股比列
	double ci0211;                  //当前PE
	double ci0212;                  //过去财年PE

	double ci0213;                  //本财年PE
	double ci0214;                  //下一财年PE
	double ci0215;                  //3年最高PE
	double ci0216;                  //3年最低PE
	double ci0217;                  //PERelative

	double ci0218;                  //PEG(1年增长率)
	double ci0219;                  //PEG(3年增长率)
	double ci0220;                  //股价/最近1季度每股业务收入
	double ci0221;                  //股价/最近4季度每股业务收入
	double ci0222;                  //当前PB

	double ci0223;                  //股价/每股有形净资产
	double ci0224;                  //股价/每股现金流
	double ci0225;                  //股价/每股自由现金流
	double ci0226;                  //每股营运资本/股价
	double ci0227;                  //最近季度每股营运资本/股价

	double ci0231;                  //1周涨跌幅
	double ci0232;                  //1月涨跌幅
	double ci0233;                  //3月涨跌幅
	double ci0234;                  //6月涨跌幅
	double ci0235;                  //1年涨跌幅

	double ci0236;                  //1周相对涨跌幅
	double ci0237;                  //1月相对涨跌幅
	double ci0238;                  //3月相对涨跌幅
	double ci0239;                  //6月相对涨跌幅
	double ci0240;                  //1年相对涨跌幅

	double ci0241;                  //1周换手率
	double ci0242;                  //1月换手率
	double ci0243;                  //3月换手率
	double ci0244;                  //6月换手率
	double ci0245;                  //1年换手率

	double ci0246;				    //2周涨跌幅
	double ci0247;					//3月前3月涨跌幅

	double ci0301;                  //最近1季度净利润增长率
	double ci0302;                  //最近4季度净利润增长率
	double ci0303;                  //3年净利润复合增长率
	double ci0304;                  //最近1季度业务收入增长率
	double ci0305;                  //最近4季度业务收入增长率

	double ci0306;                  //3年业务收入复合增长率
	double ci0307;                  //最近1季度现金流增长率
	double ci0308;                  //最近4季度现金流增长率
	double ci0309;                  //最近1季度自由现金流增长率
	double ci0310;                  //最近4季度自由现金流增长率

	double ci0311;                  //自由现金流3年平均增长率
	double ci0312;                  //最近1季度资产周转率
	double ci0313;                  //最近4季度资产周转率
	double ci0314;                  //最近1季度库存周转率
	double ci0315;                  //最近4季度库存周转率

	double ci0316;                  //最近1季度应收帐周转率
	double ci0317;                  //最近4季度应收帐周转率
	double ci0318;                  //最近1季度毛利率
	double ci0319;                  //最近4季度毛利率
	double ci0320;                  //3年平均毛利率

	double ci0321;                  //最近1季度净利润率
	double ci0322;                  //最近4季度净利润率
	double ci0323;                  //3年平均净利润率
	double ci0324;                  //最近1季度营运利润率
	double ci0325;                  //最近4季度营运利润率

	double ci0326;                  //3年平均营运利润率
	double ci0327;                  //最近1季度资产回报率
	double ci0328;                  //最近4季度资产回报率
	double ci0329;                  //3年平均资产回报率
	double ci0330;                  //最近1季度股东权益回报率

	double ci0331;                  //最近4季度股东权益回报率
	double ci0332;                  //3年平均股东权益回报率
	double ci0333;                  //最近1季度投资回报率
	double ci0334;                  //最近4季度投资回报率
	double ci0335;                  //3年平均投资回报率
	
	double ci0336;                  //3年ROA平均增长率
	double ci0337;                  //3年ROE平均增长率
	double ci0338;                  //最近1季度营运利润率变化
	double ci0339;                  //最近4季度营运利润率变化
	double ci0340;                  //最近1季度流动比率

	double ci0341;                  //最近1季度速动比率
	double ci0342;                  //最近1季度长期负债/总资产
	double ci0343;                  //最近1季度长期负债/股东权益
	double ci0344;                  //最近1季度利息支出/盈利
	double ci0345;                  //最近4季度利息支出/盈利

	double ci0346;                  //最近1季度总负债/总资产
	double ci0347;                  //最近1季度总负债/股东权益
	double ci0348;                  //最近1季度利息保障倍数
	double ci0349;                  //最近4季度利息保障倍数
	double ci0401;                  //1月本财年盈利调准比例

	double ci0402;                  //1月下一财年盈利调准比例
	double ci0403;					//未来12月净利润增长率
	double ci0404;					//1月本财年盈利变化							
	double ci0405;					//2月本财年盈利变化
	double ci0406;					//3月本财年盈利变化

	double ci0407;					//1月下一财年盈利变化
	double ci0408;					//2月下一财年盈利变化
	double ci0409;					//3月下一财年盈利变化
	double ci0410;					//1月评级变化
	double ci0411;					//2月评级变化
	double ci0412;					//3月评级变化

	double ci0413;					//当前评级
};

//科目表
struct tagSubjectTable
{
	unsigned char IND_ID[10];    //指标代码
	unsigned char IND_NM[40];    //指标中文名称
	unsigned char IND_ENM[40];   //指标英文名称
	unsigned char Mea_Unit[10];  //计量单位
};

struct tagSubjectTableEx
{
	unsigned short TableNum;     //科目表的编码
	tagSubjectTable table;
};

struct tagCommentaryCollect
{
	unsigned char Sta_Prd[4];   //统计时段
	double Avg_Value;           //综合建议
	double Tbu_Num;             //强力买入人数
	double Buy_Num;             //买入人数
	double Neu_Num;             //观望人数
	double Red_Num;             //适度减持人数
	double Sel_Num;             //卖出人数
	double Avg_Price;			//综合目标价格
	double Hig_Price;           //最高目标价格
	double Low_Price;           //最低目标价格
};

struct tagCommentaryCollectEx
{
	unsigned char SEC_CD[8];      //股票代码
	tagCommentaryCollect collect;
};

struct tagVerifyUserInfo
{
	unsigned char UserID[16];
	unsigned char UserName[16];
	unsigned char Result[1];     //0失败，1通过
	unsigned char Role[64];
};

struct tagModifyPassword
{
	unsigned char UserID[16];
	unsigned char Result[1];     //0失败，1通过
};

struct tagAnalyserInfo
{
	unsigned char ANA_Account[20]; //分析师帐号
	unsigned char ANA_NM[20];	   //分析师姓名
	unsigned char Company[40];     //所属单位
};

struct tagAnalyserRemark
{
	unsigned char ANA_Account[20]; //分析师帐号
	unsigned char SEC_CD[8];       //股票代码
	unsigned char IDU_CD[10];      //行业代码
	unsigned char UPDT_TM[8];      //日期YYYYMMDD
	unsigned short CommentLen;	   //评论的长度
	//评论字符串，以零结尾
};

struct tagAddRemark
{
	unsigned char UPDT_TM[8];      //日期YYYYMMDD
	unsigned char ANA_Account[20]; //分析师帐号
	unsigned char SEC_CD[8];       //股票代码
	unsigned char IDU_CD[10];      //行业代码
	unsigned short CommentLen;	   //评论的长度
};

struct tagHisFinanceIdx
{
	unsigned char IND_ID[10];	   //指标代码
	unsigned char Rpt_Yr[4];	   //报告年度
	unsigned char Rpt_Prd[4];	   //报告时段
	double        Ind_Value;       //指标值
};

struct tagIdxHisData
{
	unsigned char Date[8];		  //日期
	unsigned char Value[20];	  //值
};

struct tagIduRepresentData
{
	unsigned short nFactorMark;   //因素标识ID
	double field1;
	double field2;
	double field3;
	double field4;
	double field5;
};

struct tagIduSortData
{
	unsigned char Idu_CD[10];    //行业编码
	double field1;
	double field2;
	double field3;
	double field4;
	double field5;
	double field6;
	double field7;
	double field8;
	double field9;
	double field10;
};

struct tagSelStkModInfo
{
	char MOD_CD[8];
	char MOD_NM[100];
};

struct tagSelStkModData
{
	char SEC_CD[8];
	char MKT_CL[2];
};

struct tagMktFactorIdx
{
	char IDX_CD[8];	   //指数代码
	char Dt_prd[5];	   //时间区间
	double cm0601;
	double cm0602;
	double cm0603;
	double cm0604;
	double cm0605;
	double cm0606;
	double cm0607;
	double cm0608;
	double cm0609;
	double cm0610;
};

struct tagMktMixIdx
{
	char IDX_CD[8];	   //指数代码
	double cm0701;
	double cm0702;
	double cm0703;
	double cm0704;
	double cm0705;

	char cm0706[50]; //过去一周起作用的市场因素
	char cm0707[50]; //过去一月起作用的市场因素
	char cm0708[50]; //过去一周表现最好的行业
	char cm0709[50]; //过去一月表现最好的行业
};

#pragma pack(pop)
/////////////////////////////////////////////////////////////////////////////////
#endif