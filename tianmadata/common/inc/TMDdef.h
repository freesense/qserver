//TMDdef.h
#ifndef _TMDDEF_H_
#define _TMDDEF_H_
#include "TianmaDataExport.h"
#include "../../public/protocol.h"
/////////////////////////////////////////////////////////////////////////////////
//�ͻ�������
#define STOCK_INFO      1001	//֤ȯ��Ϣ 
#define INDUSTRY_INFO	1002	//��ҵ���� 
#define AREA_INFO		1003	//������� 
#define INDEX_INFO		1004	//ָ������ 
#define HQ_INDEX        1005	//��Ʊ������ָ�� 

#define PAYOFF_COLLECT	1006	//ӯ��Ԥ�����
#define PAYOFF_BILL     1007	//ӯ��Ԥ����ϸ
#define COMMENTARY_BILL 1008    //Ͷ��������ϸ
#define IDX_INDEX		1009    //ָ����ָ��
#define IDU_INDEX       1010    //��ҵ��ָ��
#define SUBTAB_INFO     1011    //��Ŀ��
#define COMMENTARY_COLLECT 1012 //��������
#define ANALYSER_INFO   1013	//����ʦ��Ϣ
#define MKT_FACTOR_IDX  1014	//�г�����ָ��
#define MKT_MIX_IDX     1015	//�г����ָ��

#define ANALYSER_REMARK 1200	//����ʦ����
#define ADD_REMARK      1201	//���ӷ���ʦ����

#define HIS_FINANCE_IDX 1300    //��ʷ������ָ��
#define INDEX_HIS_DATA  1301    //ָ����ʷ����
#define IDU_REPRE_DATA  1302    //��ҵ��������
#define IDU_SORT_DATA   1303    //��ҵ������������������


#define VERIFY_USERINFO	2000	//�û���Ϣ��֤
#define MODIFY_PASSWORD	2001	//�޸�����
#define SELSTKMOD_INFO  2004	//ѡ��ģ����Ϣ
#define SELSTKMOD_DATA	2005	//ѡ��ģ������


//��Ŀ��ı���
#define  STK16_FBSC     1001
#define  STK17_FHIND    1002
#define  STK18_ASSET1   1003
#define  STK19_ASSET2   1004
#define  STK20_PROFIT1  1005
#define  STK21_PROFIT2  1006
#define  STK22_CASH1    1007
#define  STK23_CASH2    1008


//�г����
#define MT_CN           "CN"	//��½�г�
#define MT_HK           "HK"	//����г�

//ָ����ָ�궨��
const unsigned short INDEX_IDX_CM0201=1001;//��������
const unsigned short INDEX_IDX_CM0202=1002;//��������
const unsigned short INDEX_IDX_CM0203=1003;//��Ҫ����
const unsigned short INDEX_IDX_CM0211=1004;//��ǰPE
const unsigned short INDEX_IDX_CM0212=1005;//��ȥ����PE
const unsigned short INDEX_IDX_CM0213=1006;//������PE
const unsigned short INDEX_IDX_CM0214=1007;//��һ����PE
const unsigned short INDEX_IDX_CM0215=1008;//3�����PE
const unsigned short INDEX_IDX_CM0216=1009;//3�����PE
const unsigned short INDEX_IDX_CM0217=1010;//��ǰPB
const unsigned short INDEX_IDX_CM0221=1011;//1���ǵ���
const unsigned short INDEX_IDX_CM0222=1012;//2���ǵ���
const unsigned short INDEX_IDX_CM0223=1013;//1���ǵ���
const unsigned short INDEX_IDX_CM0224=1014;//3���ǵ���
const unsigned short INDEX_IDX_CM0225=1015;//6���ǵ���
const unsigned short INDEX_IDX_CM0226=1016;//1���ǵ���


const unsigned short INDEX_IDX_CM0301=1017;//������Ⱦ�����������
const unsigned short INDEX_IDX_CM0302=1018;//������Ⱦ�����������
const unsigned short INDEX_IDX_CM0303=1019;//�������ҵ������������
const unsigned short INDEX_IDX_CM0304=1020;//�������ҵ������������

const unsigned short INDEX_IDX_CM0601=1021;// ��ֵ���� ������
const unsigned short INDEX_IDX_CM0602=1022;// �������� ������
const unsigned short INDEX_IDX_CM0603=1023;// �ر����� ������
const unsigned short INDEX_IDX_CM0604=1024; //�ر���ת ������
const unsigned short INDEX_IDX_CM0605=1025; //ӯ������ ������
const unsigned short INDEX_IDX_CM0606=1026; //�����仯 ������
const unsigned short INDEX_IDX_CM0607=1027; //����ʦ���� ������
const unsigned short INDEX_IDX_CM0608=1028; //��ֵ�۸� ������
const unsigned short INDEX_IDX_CM0609=1029; //RTDM���� ������
const unsigned short INDEX_IDX_CM0610=1030; //�ʽ���/�����ֱֲ��� ������

const unsigned short INDEX_IDX_CM0701=1031; //Earning Visib ������
const unsigned short INDEX_IDX_CM0702=1032; //Earning Momen ������
const unsigned short INDEX_IDX_CM0703=1033; //Upgrade/DownGrade ������
const unsigned short INDEX_IDX_CM0704=1034; //Volatility��3month�� ������
const unsigned short INDEX_IDX_CM0705=1035; //AAII(Bull-Bear)% ������
const unsigned short INDEX_IDX_CM0706=1036; //��ȥһ�������õ��г����� ������
const unsigned short INDEX_IDX_CM0707=1037; //��ȥһ�������õ��г����� ������
const unsigned short INDEX_IDX_CM0708=1038; //��ȥһ�ܱ�����õ���ҵ ������
const unsigned short INDEX_IDX_CM0709=1039; //��ȥһ�±�����õ���ҵ ������

//��ҵ��ָ�궨��
const unsigned short INDEX_IDU_CI0201 = 2001;//����ֵ
const unsigned short INDEX_IDU_CI0202 = 2002;//��ͨ��ֵ
const unsigned short INDEX_IDU_CI0210 = 2003;//����ֹɱ���
const unsigned short INDEX_IDU_CI0211 = 2004;//��ǰPE
const unsigned short INDEX_IDU_CI0212 = 2005;//��ȥ����PE
const unsigned short INDEX_IDU_CI0213 = 2006;//������PE
const unsigned short INDEX_IDU_CI0214 = 2007;//��һ����PE
const unsigned short INDEX_IDU_CI0215 = 2008;//3�����PE
const unsigned short INDEX_IDU_CI0216 = 2009;//3�����PE
const unsigned short INDEX_IDU_CI0217 = 2010;//PERelative
const unsigned short INDEX_IDU_CI0218 = 2011;//PEG(1��������)
const unsigned short INDEX_IDU_CI0219 = 2012;//PEG(3��������)
const unsigned short INDEX_IDU_CI0220 = 2013;//�ɼ�/�������ÿ��ҵ������
const unsigned short INDEX_IDU_CI0221 = 2014;//�ɼ�/�������ÿ��ҵ������
const unsigned short INDEX_IDU_CI0222 = 2015;//��ǰPB
const unsigned short INDEX_IDU_CI0223 = 2016;//�ɼ�/ÿ�����ξ��ʲ�
const unsigned short INDEX_IDU_CI0224 = 2017;//�ɼ�/ÿ���ֽ���
const unsigned short INDEX_IDU_CI0225 = 2018;//�ɼ�/ÿ�������ֽ���
const unsigned short INDEX_IDU_CI0226 = 2019;//ÿ��Ӫ���ʱ�/�ɼ�
const unsigned short INDEX_IDU_CI0227 = 2020;//�������ÿ��Ӫ���ʱ�/�ɼ�
const unsigned short INDEX_IDU_CI0231 = 2021;//1���ǵ���
const unsigned short INDEX_IDU_CI0232 = 2022;//1���ǵ���
const unsigned short INDEX_IDU_CI0233 = 2023;//3���ǵ���
const unsigned short INDEX_IDU_CI0234 = 2024;//6���ǵ���
const unsigned short INDEX_IDU_CI0235 = 2025;//1���ǵ���
const unsigned short INDEX_IDU_CI0236 = 2026;//1������ǵ���
const unsigned short INDEX_IDU_CI0237 = 2027;//1������ǵ���
const unsigned short INDEX_IDU_CI0238 = 2028;//3������ǵ���
const unsigned short INDEX_IDU_CI0239 = 2029;//6������ǵ���
const unsigned short INDEX_IDU_CI0240 = 2030;//1������ǵ���
const unsigned short INDEX_IDU_CI0241 = 2031;//1�ܻ�����
const unsigned short INDEX_IDU_CI0242 = 2032;//1�»�����
const unsigned short INDEX_IDU_CI0243 = 2033;//3�»�����
const unsigned short INDEX_IDU_CI0244 = 2034;//6�»�����
const unsigned short INDEX_IDU_CI0245 = 2035;//1�껻����
const unsigned short INDEX_IDU_CI0246 = 2036;//������
const unsigned short INDEX_IDU_CI0247 = 2037;//������

const unsigned short INDEX_IDU_CI0301 = 2038;//������Ⱦ�����������
const unsigned short INDEX_IDU_CI0302 = 2039;//������Ⱦ�����������
const unsigned short INDEX_IDU_CI0303 = 2040;//3�꾻���󸴺�������
const unsigned short INDEX_IDU_CI0304 = 2041;//�������ҵ������������
const unsigned short INDEX_IDU_CI0305 = 2042;//�������ҵ������������
const unsigned short INDEX_IDU_CI0306 = 2043;//3��ҵ�����븴��������
const unsigned short INDEX_IDU_CI0307 = 2044;//��������ֽ���������
const unsigned short INDEX_IDU_CI0308 = 2045;//��������ֽ���������
const unsigned short INDEX_IDU_CI0309 = 2046;//������������ֽ���������
const unsigned short INDEX_IDU_CI0310 = 2047;//������������ֽ���������
const unsigned short INDEX_IDU_CI0311 = 2048;//�����ֽ�����ƽ��������
const unsigned short INDEX_IDU_CI0312 = 2049;//��������ʲ���ת��
const unsigned short INDEX_IDU_CI0313 = 2050;//��������ʲ���ת��
const unsigned short INDEX_IDU_CI0314 = 2051;//������ȿ����ת��
const unsigned short INDEX_IDU_CI0315 = 2052;//������ȿ����ת��
const unsigned short INDEX_IDU_CI0316 = 2053;//�������Ӧ������ת��
const unsigned short INDEX_IDU_CI0317 = 2054;//�������Ӧ������ת��
const unsigned short INDEX_IDU_CI0318 = 2055;//�������ë����
const unsigned short INDEX_IDU_CI0319 = 2056;//�������ë����
const unsigned short INDEX_IDU_CI0320 = 2057;//3��ƽ��ë����
const unsigned short INDEX_IDU_CI0321 = 2058;//������Ⱦ�������
const unsigned short INDEX_IDU_CI0322 = 2059;//������Ⱦ�������
const unsigned short INDEX_IDU_CI0323 = 2060;//3��ƽ����������
const unsigned short INDEX_IDU_CI0324 = 2061;//�������Ӫ��������
const unsigned short INDEX_IDU_CI0325 = 2062;//�������Ӫ��������
const unsigned short INDEX_IDU_CI0326 = 2063;//3��ƽ��Ӫ��������
const unsigned short INDEX_IDU_CI0327 = 2064;//��������ʲ��ر���
const unsigned short INDEX_IDU_CI0328 = 2065;//��������ʲ��ر���
const unsigned short INDEX_IDU_CI0329 = 2066;//3��ƽ���ʲ��ر���
const unsigned short INDEX_IDU_CI0330 = 2067;//������ȹɶ�Ȩ��ر���
const unsigned short INDEX_IDU_CI0331 = 2068;//������ȹɶ�Ȩ��ر���
const unsigned short INDEX_IDU_CI0332 = 2069;//3��ƽ���ɶ�Ȩ��ر���
const unsigned short INDEX_IDU_CI0333 = 2070;//�������Ͷ�ʻر���
const unsigned short INDEX_IDU_CI0334 = 2071;//�������Ͷ�ʻر���
const unsigned short INDEX_IDU_CI0335 = 2072;//3��ƽ��Ͷ�ʻر���
const unsigned short INDEX_IDU_CI0336 = 2073;//3��ROAƽ��������
const unsigned short INDEX_IDU_CI0337 = 2074;//3��ROEƽ��������
const unsigned short INDEX_IDU_CI0338 = 2075;//�������Ӫ�������ʱ仯
const unsigned short INDEX_IDU_CI0339 = 2076;//�������Ӫ�������ʱ仯
const unsigned short INDEX_IDU_CI0340 = 2077;//���������������
const unsigned short INDEX_IDU_CI0341 = 2078;//��������ٶ�����
const unsigned short INDEX_IDU_CI0342 = 2079;//������ȳ��ڸ�ծ/���ʲ�
const unsigned short INDEX_IDU_CI0343 = 2080;//������ȳ��ڸ�ծ/�ɶ�Ȩ��
const unsigned short INDEX_IDU_CI0344 = 2081;//���������Ϣ֧��/ӯ��
const unsigned short INDEX_IDU_CI0345 = 2082;//���������Ϣ֧��/ӯ��
const unsigned short INDEX_IDU_CI0346 = 2083;//��������ܸ�ծ/���ʲ�
const unsigned short INDEX_IDU_CI0347 = 2084;//��������ܸ�ծ/�ɶ�Ȩ��
const unsigned short INDEX_IDU_CI0348 = 2085;//���������Ϣ���ϱ���
const unsigned short INDEX_IDU_CI0349 = 2086;//���������Ϣ���ϱ���

const unsigned short INDEX_IDU_CI0401 = 2087;//�����������±�����ӯ����׼����
const unsigned short INDEX_IDU_CI0402 = 2088;//��������������һ����ӯ����׼����
const unsigned short INDEX_IDU_CI0403 = 2089;//������
const unsigned short INDEX_IDU_CI0404 = 2090; //������
const unsigned short INDEX_IDU_CI0405 = 2091; //������
const unsigned short INDEX_IDU_CI0406 = 2092; //������
const unsigned short INDEX_IDU_CI0407 = 2093; //������
const unsigned short INDEX_IDU_CI0408 = 2094; //������
const unsigned short INDEX_IDU_CI0409 = 2095; //������
const unsigned short INDEX_IDU_CI0410 = 2096; //������
const unsigned short INDEX_IDU_CI0411 = 2097; //������
const unsigned short INDEX_IDU_CI0412 = 2098; //������
const unsigned short INDEX_IDU_CI0413 = 2099; //������

//��Ʊ��ָ��
const unsigned short INDEX_STK_CS0201 = 4001;//����ֵ
const unsigned short INDEX_STK_CS0202 = 4002;//��ͨ��ֵ
const unsigned short INDEX_STK_CS0203 = 4003;//(����ֵ+�ܸ�ծ)/�������ҵ������
const unsigned short INDEX_STK_CS0209 = 4004;//�߹ֹܳɱ���
const unsigned short INDEX_STK_CS0210 = 4005;//����ֹɱ���
const unsigned short INDEX_STK_CS0211 = 4006;//��ǰPE
const unsigned short INDEX_STK_CS0212 = 4007;//��ȥ����PE
const unsigned short INDEX_STK_CS0213 = 4008;//������PE
const unsigned short INDEX_STK_CS0214 = 4009;//��һ����PE
const unsigned short INDEX_STK_CS0215 = 4010;//3�����PE
const unsigned short INDEX_STK_CS0216 = 4011;//3�����PE
const unsigned short INDEX_STK_CS0217 = 4012;//PERelative
const unsigned short INDEX_STK_CS0218 = 4013;//PEG(1��������)
const unsigned short INDEX_STK_CS0219 = 4014;//PEG(3��������)
const unsigned short INDEX_STK_CS0220 = 4015;//�ɼ�/�������ÿ��ҵ������
const unsigned short INDEX_STK_CS0221 = 4016;//�ɼ�/�������ÿ��ҵ������
const unsigned short INDEX_STK_CS0222 = 4017;//��ǰPB
const unsigned short INDEX_STK_CS0223 = 4018;//�ɼ�/ÿ�����ξ��ʲ�
const unsigned short INDEX_STK_CS0224 = 4019;//�ɼ�/ÿ���ֽ���
const unsigned short INDEX_STK_CS0225 = 4020;//�ɼ�/ÿ�������ֽ���
const unsigned short INDEX_STK_CS0226 = 4021;//ÿ��Ӫ���ʱ�/�ɼ�
const unsigned short INDEX_STK_CS0227 = 4022;//�������ÿ��Ӫ���ʱ�/�ɼ�
const unsigned short INDEX_STK_CS0231 = 4023;//1���ǵ���
const unsigned short INDEX_STK_CS0232 = 4024;//1���ǵ���
const unsigned short INDEX_STK_CS0233 = 4025;//3���ǵ���
const unsigned short INDEX_STK_CS0234 = 4026;//6���ǵ���
const unsigned short INDEX_STK_CS0235 = 4027;//1���ǵ���
const unsigned short INDEX_STK_CS0236 = 4028;//1������ǵ���
const unsigned short INDEX_STK_CS0237 = 4029;//1������ǵ���
const unsigned short INDEX_STK_CS0238 = 4030;//3������ǵ���
const unsigned short INDEX_STK_CS0239 = 4031;//6������ǵ���
const unsigned short INDEX_STK_CS0240 = 4032;//1������ǵ���
const unsigned short INDEX_STK_CS0241 = 4033;//1�ܻ�����
const unsigned short INDEX_STK_CS0242 = 4034;//1�»�����
const unsigned short INDEX_STK_CS0243 = 4035;//3�»�����
const unsigned short INDEX_STK_CS0244 = 4036;//6�»�����
const unsigned short INDEX_STK_CS0245 = 4037;//1�껻����
const unsigned short INDEX_STK_CS0246 = 4038;//2��ƽ���ɽ���/3��ƽ���ɽ���
const unsigned short INDEX_STK_CS0247 = 4039;//1��ƽ���ɽ���/6��ƽ���ɽ���

const unsigned short INDEX_STK_CS0301 = 4040;//�������ÿ��ӯ��
const unsigned short INDEX_STK_CS0302 = 4041;//�������ÿ��ӯ��
const unsigned short INDEX_STK_CS0303 = 4042;//�������ÿ�ɿɷ�������
const unsigned short INDEX_STK_CS0311 = 4043;//�������ÿ��ӯ��������
const unsigned short INDEX_STK_CS0312 = 4044;//�������ÿ��ӯ��������
const unsigned short INDEX_STK_CS0313 = 4045;//3��ÿ��ӯ������������
const unsigned short INDEX_STK_CS0314 = 4046;//������Ⱦ�����������
const unsigned short INDEX_STK_CS0315 = 4047;//������Ⱦ�����������
const unsigned short INDEX_STK_CS0316 = 4048;//3�꾻���󸴺�������
const unsigned short INDEX_STK_CS0317 = 4049;//�������ҵ������������
const unsigned short INDEX_STK_CS0318 = 4050;//�������ҵ������������
const unsigned short INDEX_STK_CS0319 = 4051;//3��ҵ�����븴��������
const unsigned short INDEX_STK_CS0320 = 4052;//��������ֽ���������
const unsigned short INDEX_STK_CS0321 = 4053;//��������ֽ���������
const unsigned short INDEX_STK_CS0322 = 4054;//������������ֽ���������
const unsigned short INDEX_STK_CS0323 = 4055;//������������ֽ���������
const unsigned short INDEX_STK_CS0324 = 4056;//�����ֽ�����ƽ��������
const unsigned short INDEX_STK_CS0325 = 4057;//��������ʲ���ת��
const unsigned short INDEX_STK_CS0326 = 4058;//��������ʲ���ת��
const unsigned short INDEX_STK_CS0327 = 4059;//������ȿ����ת��
const unsigned short INDEX_STK_CS0328 = 4060;//������ȿ����ת��
const unsigned short INDEX_STK_CS0329 = 4061;//�������Ӧ������ת��
const unsigned short INDEX_STK_CS0330 = 4062;//�������Ӧ������ת��
const unsigned short INDEX_STK_CS0331 = 4063;//�������ë����
const unsigned short INDEX_STK_CS0332 = 4064;//�������ë����
const unsigned short INDEX_STK_CS0333 = 4065;//3��ƽ��ë����
const unsigned short INDEX_STK_CS0334 = 4066;//������Ⱦ�������
const unsigned short INDEX_STK_CS0335 = 4067;//������Ⱦ�������
const unsigned short INDEX_STK_CS0336 = 4068;//3��ƽ����������
const unsigned short INDEX_STK_CS0337 = 4069;//�������Ӫ��������
const unsigned short INDEX_STK_CS0338 = 4070;//�������Ӫ��������
const unsigned short INDEX_STK_CS0339 = 4071;//3��ƽ��Ӫ��������
const unsigned short INDEX_STK_CS0340 = 4072;//��������ʲ��ر���
const unsigned short INDEX_STK_CS0341 = 4073;//��������ʲ��ر���
const unsigned short INDEX_STK_CS0342 = 4074;//3��ƽ���ʲ��ر���
const unsigned short INDEX_STK_CS0343 = 4075;//������ȹɶ�Ȩ��ر���
const unsigned short INDEX_STK_CS0344 = 4076;//������ȹɶ�Ȩ��ر���
const unsigned short INDEX_STK_CS0345 = 4077;//3��ƽ���ɶ�Ȩ��ر���
const unsigned short INDEX_STK_CS0346 = 4078;//�������Ͷ�ʻر���
const unsigned short INDEX_STK_CS0347 = 4079;//�������Ͷ�ʻر���
const unsigned short INDEX_STK_CS0348 = 4080;//3��ƽ��Ͷ�ʻر���
const unsigned short INDEX_STK_CS0349 = 4081;//3��ROAƽ��������
const unsigned short INDEX_STK_CS0350 = 4082;//3��ROEƽ��������
const unsigned short INDEX_STK_CS0351 = 4083;//�������Ӫ�������ʱ仯
const unsigned short INDEX_STK_CS0352 = 4084;//�������Ӫ�������ʱ仯
const unsigned short INDEX_STK_CS0353 = 4085;//���������������
const unsigned short INDEX_STK_CS0354 = 4086;//��������ٶ�����
const unsigned short INDEX_STK_CS0355 = 4087;//������ȳ��ڸ�ծ/���ʲ�
const unsigned short INDEX_STK_CS0356 = 4088;//������ȳ��ڸ�ծ/�ɶ�Ȩ��
const unsigned short INDEX_STK_CS0357 = 4089;//���������Ϣ֧��/ӯ��
const unsigned short INDEX_STK_CS0358 = 4090;//���������Ϣ֧��/ӯ��
const unsigned short INDEX_STK_CS0359 = 4091;//��������ܸ�ծ/���ʲ�
const unsigned short INDEX_STK_CS0360 = 4092;//��������ܸ�ծ/�ɶ�Ȩ��
const unsigned short INDEX_STK_CS0361 = 4093;//���������Ϣ���ϱ���
const unsigned short INDEX_STK_CS0362 = 4094;//���������Ϣ���ϱ���

const unsigned short INDEX_STK_CS0401 = 4095;//������ƽ��Ԥ��eps
const unsigned short INDEX_STK_CS0402 = 4096;//������Ԥ��eps��׼ƫ��
const unsigned short INDEX_STK_CS0403 = 4097;//1�±�����eps�仯
const unsigned short INDEX_STK_CS0404 = 4098;//2�±�����eps�仯
const unsigned short INDEX_STK_CS0405 = 4099;//3�±�����eps�仯
const unsigned short INDEX_STK_CS0406 = 4100;//1���ϵ�������ӯ��Ԥ������
const unsigned short INDEX_STK_CS0407 = 4101;//1���ϵ�������ӯ��Ԥ������
const unsigned short INDEX_STK_CS0408 = 4102;//1���µ�������ӯ��Ԥ������
const unsigned short INDEX_STK_CS0409 = 4103;//1���µ�������ӯ��Ԥ������
const unsigned short INDEX_STK_CS0411 = 4104;//��һ����ƽ��Ԥ��eps
const unsigned short INDEX_STK_CS0412 = 4105;//��һ����Ԥ��eps��׼ƫ��
const unsigned short INDEX_STK_CS0413 = 4106;//1����һ����eps�仯
const unsigned short INDEX_STK_CS0414 = 4107;//2����һ����eps�仯
const unsigned short INDEX_STK_CS0415 = 4108;//3����һ����eps�仯
const unsigned short INDEX_STK_CS0416 = 4109;//1���ϵ���һ��ӯ��Ԥ������
const unsigned short INDEX_STK_CS0417 = 4110;//1���ϵ���һ��ӯ��Ԥ������
const unsigned short INDEX_STK_CS0418 = 4111;//1���µ���һ��ӯ��Ԥ������
const unsigned short INDEX_STK_CS0419 = 4112;//1���µ���һ��ӯ��Ԥ������
const unsigned short INDEX_STK_CS0420 = 4113;//�ϲ���ʵ��eps��Ԥ��epsƫ��

const unsigned short INDEX_STK_AVG_VALUE = 4114;// 			Avg_Value;
const unsigned short INDEX_STK_TBU_NUM = 4115;// 			Tbu_Num;
const unsigned short INDEX_STK_BUY_NUM = 4116;//			Buy_Num;
const unsigned short INDEX_STK_NEU_NUM = 4117;//			Neu_Num;
const unsigned short INDEX_STK_RED_NUM = 4118;//			Red_Num;
const unsigned short INDEX_STK_SEL_NUM = 4119;//			Sel_Num;
const unsigned short INDEX_STK_AVG_PRICE = 4120;//			Avg_Price;
const unsigned short INDEX_STK_HIG_PRICE = 4121;//			Hig_Price;
const unsigned short INDEX_STK_LOW_PRICE = 4122;//			Low_Price;

//�ṹ��
#pragma pack(push)
#pragma pack(1)
struct tagReturnMsgID
{
	SMsgID    id;
	char*     pBuf;
	int       nBufSize;
};

//��ͷ
struct tagPackHead
{
	CommxHead       comxHead;     //ͨѶ��ͷ 
	unsigned short  nFuncNo;	  //���ܺ�
	unsigned short  nReqCount;	  //�������
};

//�����Ӧ��ͬ��Ϣ
struct tagComInfo
{
	unsigned short  nPacktype;      //��������
	unsigned char   MarketType[2];  //�г����
};

//֤ȯ��Ϣ
struct tagStockInfo
{
	unsigned char   Sec_cd[8];	  //��Ʊ����
	unsigned char   Sec_id[10];	  //֤ȯ��ʶ
	unsigned char   Org_id[10];	  //��˾����
	unsigned char   Idu_cd[10];	  //��ҵ����
	unsigned char   Area_cd[10];  //�������
	unsigned char   Var_cl[2];    //֤ȯƷ�ִ���
	unsigned char   Mkt_cl[1];    //�����г�����
	unsigned char   Sec_snm[16];  //֤ȯ���ļ��
	unsigned char   Sec_esnm[16]; //֤ȯӢ�ļ��
	unsigned char   Sec_ssnm[4];  //֤ȯƴ�����
	unsigned int    nIss_dt;       //��������
	unsigned int    nMkt_dt;      //��������
};

//��ҵ������Ϣ
struct tagIndustryInfo
{
	unsigned char   Idu_cd[10];	  //��ҵ����
	unsigned char   Idu_nm[40];	  //��ҵ����
};

//ָ��������Ϣ
struct tagIndexInfo
{
	unsigned char   Idx_cd[10];	  //ָ������
	unsigned char   Var_cl[2];    //֤ȯƷ�ִ���
	unsigned char   Mkt_cl[1];    //�����г�����
	unsigned char   Idx_snm[16];  //֤ȯ���ļ��
	unsigned char   Idx_esnm[16]; //֤ȯӢ�ļ��
	unsigned char   Idx_ssnm[4];  //֤ȯƴ�����
	unsigned short  nRecordCount; //��ָ�������Ĺ�Ʊ����
	//��Ʊ���� (�ظ�)
};

struct tagMkt_Idx
{
	unsigned char   Idx_cd[10];	  //ָ������
	unsigned char   Var_cl[2];    //֤ȯƷ�ִ���
	unsigned char   Mkt_cl[1];    //�����г�����
	unsigned char   Idx_snm[16];  //֤ȯ���ļ��
	unsigned char   Idx_esnm[16]; //֤ȯӢ�ļ��
	unsigned char   Idx_ssnm[4];  //֤ȯƴ�����
	unsigned short  nRecordCount; //��ָ�������Ĺ�Ʊ����
	char*           pStockCode;   //��Ʊ���� (�ظ�), ÿ��8���ֽ�
};

//�г����͹�Ʊ����
struct tagMarketandStock
{
	unsigned char   MarketType[2];  //�г����
	unsigned char   Sec_cd[8];	    //��Ʊ����
};

//��Ʊ������ָ�� 125
struct tagHQIndex
{
	unsigned char   MarketType[2];  //�г����
	unsigned char   Sec_cd[8];	    //��Ʊ����

	double   cs0201;		//����ֵ
	double   cs0202;		//��ͨ��ֵ
	double   cs0203;		//(����ֵ+�ܸ�ծ)/���4����ҵ������
	double   cs0209;		//�߹ֹܳɱ���
	double   cs0210;		//����ֹɱ���

	double   cs0211;		//��ǰPE
	double   cs0212;		//��ȥ����PE
	double   cs0213;		//������PE
	double   cs0214;		//��һ����PE
	double   cs0215;		//3�����PE

	double   cs0216;		//3�����PE
	double   cs0217;		//PERelative
	double   cs0218;		//PEG(1��������)
	double   cs0219;		//PEG(3��������)
	double   cs0220;		//�ɼ�/���1����ÿ��ҵ������

	double   cs0221;		//�ɼ�/���4����ÿ��ҵ������
	double   cs0222;		//��ǰPB
	double   cs0223;		//�ɼ�/ÿ�����ξ��ʲ�
	double   cs0224;		//�ɼ�/ÿ���ֽ���
	double   cs0225;		//�ɼ�/ÿ�������ֽ���

	double   cs0226;		//ÿ��Ӫ���ʱ�/�ɼ�
	double   cs0227;		//�������ÿ��Ӫ���ʱ�/�ɼ�
	double   cs0231;		//1���ǵ���
	double   cs0232;		//1���ǵ���
	double   cs0233;		//3���ǵ���

	double   cs0234;		//6���ǵ���
	double   cs0235;		//1���ǵ���
	double   cs0236;		//1������ǵ���
	double   cs0237;		//1������ǵ���
	double   cs0238;		//3������ǵ���

	double   cs0239;		//6������ǵ���
	double   cs0240;		//1������ǵ���
	double   cs0241;		//1�ܻ�����
	double   cs0242;		//1�»�����
	double   cs0243;		//3�»�����

	double   cs0244;		//6�»�����
	double   cs0245;		//1�껻����
	double   cs0246;		//2��ƽ���ɽ���/3��ƽ���ɽ���
	double   cs0247;		//1��ƽ���ɽ���/6��ƽ���ɽ���
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

//ӯ��Ԥ�����
struct tagPayOffCollect
{
	unsigned char IND_ID[6];   //ָ�����
	unsigned char Rpt_Yr[4];   //Ԥ�����
	unsigned char Rpt_Prd[4];  //Ԥ��ʱ��
	unsigned char Sta_Prd[4];  //ͳ��ʱ��
	double  Avg_Value;		   //Ԥ��ƽ��ֵ
	double  Hig_Value;		   //Ԥ�����
	double  Low_Value;		   //Ԥ�����
	unsigned short Ana_Num;    //����ʦ����
};

struct tagPayOffCollectEx
{
	unsigned char SEC_CD[8];
	tagPayOffCollect  collect;
};

//ӯ��Ԥ����ϸ
struct tagPayOffBill
{
	unsigned int  ORG_CL;         //�о���������
	unsigned char IND_ID[6];      //ָ�����
	unsigned char For_Date[8];	  //Ԥ��ʱ��YYYYMMDD
	unsigned char Rpt_Yr[4];	  //Ԥ�����
	unsigned char Rpt_Prd[4];     //Ԥ��ʱ��
	double For_Value;             //Ԥ��ֵ
	unsigned char Author[100];     //����
	double Dil_Value;             //���ɱ�̯��Ԥ��ֵ	
	unsigned char Dil_Date[8];    //̯���ɱ��ı䶯��YYYYMMDD
};

//Ͷ��������ϸ
struct tagCommentaryBill
{
	unsigned int  ORG_CL;         //�о���������
	unsigned char Gra_Date[8];	  //����ʱ��YYYYMMDD
	unsigned char Gra_Prd;        //����ʱ��
	double Gra_Value;             //����ָ��
	double Tar_Price;             //Ŀ���
	unsigned char Tar_Prd[3];    //��λʱ��
	unsigned char Author[100];      //����
	double Exr_Price;             //��Ȩ��Ŀ���λ
	unsigned char Exr_Date[8];    //�����Ȩ����YYYYMMDD
};

struct tagCommentaryBillEx
{
	unsigned char SEC_CD[8];      //��Ʊ����
	tagCommentaryBill bill;
};

//ָ����ָ��  24   ע�⣺��½�������ֶΣ����۹�û�����ӣ�tagIDXIndex�Ѿ���һ������ע���޸�HK_MKT02_Qind.cpp
struct tagIDXIndex
{
	unsigned char   MarketType[2];  //�г����
	unsigned char   IDX_CD[8];	    //ָ������
	
	unsigned char cm0201[10];       //��������
	unsigned char cm0202[10];       //��������
	unsigned char cm0203[10];       //��Ҫ����
	double cm0211;                  //��ǰPE
	double cm0212;                  //��ȥ����PE

	double cm0213;                  //������PE
	double cm0214;                  //��һ����PE
	double cm0215;                  //3�����PE
	double cm0216;                  //3�����PE
	double cm0217;                  //��ǰPB
	double cm0218;					//PEG(1��������)  ����
	double cm0219;					//PEG(3��������)  ����

	double cm0221;                  //1���ǵ���
	double cm0222;					//2���ǵ���  ����
	double cm0223;                  //1���ǵ���
	double cm0224;                  //3���ǵ���
	double cm0225;                  //6���ǵ���
	double cm0226;                  //1���ǵ���

	double cm0301;                  //���1���Ⱦ�����������
	double cm0302;                  //���4���Ⱦ�����������
	double cm0303;                  //���1����ҵ������������
	double cm0304;                  //���4����ҵ������������
};

//��ҵ��ָ�� 101 ע�⣺��½�������ֶΣ����۹�û�����ӣ�tagIduIndex�Ѿ���һ������ע���޸�HK_IDU02_Qind.cpp
struct tagIduIndex
{
	unsigned char   MarketType[2];  //�г����
	unsigned char   IDU_CD[8];	    //��ҵ����
	
	double ci0201;                  //����ֵ
	double ci0202;                  //��ͨ��ֵ
	double ci0210;                  //����ֹɱ���
	double ci0211;                  //��ǰPE
	double ci0212;                  //��ȥ����PE

	double ci0213;                  //������PE
	double ci0214;                  //��һ����PE
	double ci0215;                  //3�����PE
	double ci0216;                  //3�����PE
	double ci0217;                  //PERelative

	double ci0218;                  //PEG(1��������)
	double ci0219;                  //PEG(3��������)
	double ci0220;                  //�ɼ�/���1����ÿ��ҵ������
	double ci0221;                  //�ɼ�/���4����ÿ��ҵ������
	double ci0222;                  //��ǰPB

	double ci0223;                  //�ɼ�/ÿ�����ξ��ʲ�
	double ci0224;                  //�ɼ�/ÿ���ֽ���
	double ci0225;                  //�ɼ�/ÿ�������ֽ���
	double ci0226;                  //ÿ��Ӫ���ʱ�/�ɼ�
	double ci0227;                  //�������ÿ��Ӫ���ʱ�/�ɼ�

	double ci0231;                  //1���ǵ���
	double ci0232;                  //1���ǵ���
	double ci0233;                  //3���ǵ���
	double ci0234;                  //6���ǵ���
	double ci0235;                  //1���ǵ���

	double ci0236;                  //1������ǵ���
	double ci0237;                  //1������ǵ���
	double ci0238;                  //3������ǵ���
	double ci0239;                  //6������ǵ���
	double ci0240;                  //1������ǵ���

	double ci0241;                  //1�ܻ�����
	double ci0242;                  //1�»�����
	double ci0243;                  //3�»�����
	double ci0244;                  //6�»�����
	double ci0245;                  //1�껻����

	double ci0246;				    //2���ǵ���
	double ci0247;					//3��ǰ3���ǵ���

	double ci0301;                  //���1���Ⱦ�����������
	double ci0302;                  //���4���Ⱦ�����������
	double ci0303;                  //3�꾻���󸴺�������
	double ci0304;                  //���1����ҵ������������
	double ci0305;                  //���4����ҵ������������

	double ci0306;                  //3��ҵ�����븴��������
	double ci0307;                  //���1�����ֽ���������
	double ci0308;                  //���4�����ֽ���������
	double ci0309;                  //���1���������ֽ���������
	double ci0310;                  //���4���������ֽ���������

	double ci0311;                  //�����ֽ���3��ƽ��������
	double ci0312;                  //���1�����ʲ���ת��
	double ci0313;                  //���4�����ʲ���ת��
	double ci0314;                  //���1���ȿ����ת��
	double ci0315;                  //���4���ȿ����ת��

	double ci0316;                  //���1����Ӧ������ת��
	double ci0317;                  //���4����Ӧ������ת��
	double ci0318;                  //���1����ë����
	double ci0319;                  //���4����ë����
	double ci0320;                  //3��ƽ��ë����

	double ci0321;                  //���1���Ⱦ�������
	double ci0322;                  //���4���Ⱦ�������
	double ci0323;                  //3��ƽ����������
	double ci0324;                  //���1����Ӫ��������
	double ci0325;                  //���4����Ӫ��������

	double ci0326;                  //3��ƽ��Ӫ��������
	double ci0327;                  //���1�����ʲ��ر���
	double ci0328;                  //���4�����ʲ��ر���
	double ci0329;                  //3��ƽ���ʲ��ر���
	double ci0330;                  //���1���ȹɶ�Ȩ��ر���

	double ci0331;                  //���4���ȹɶ�Ȩ��ر���
	double ci0332;                  //3��ƽ���ɶ�Ȩ��ر���
	double ci0333;                  //���1����Ͷ�ʻر���
	double ci0334;                  //���4����Ͷ�ʻر���
	double ci0335;                  //3��ƽ��Ͷ�ʻر���
	
	double ci0336;                  //3��ROAƽ��������
	double ci0337;                  //3��ROEƽ��������
	double ci0338;                  //���1����Ӫ�������ʱ仯
	double ci0339;                  //���4����Ӫ�������ʱ仯
	double ci0340;                  //���1������������

	double ci0341;                  //���1�����ٶ�����
	double ci0342;                  //���1���ȳ��ڸ�ծ/���ʲ�
	double ci0343;                  //���1���ȳ��ڸ�ծ/�ɶ�Ȩ��
	double ci0344;                  //���1������Ϣ֧��/ӯ��
	double ci0345;                  //���4������Ϣ֧��/ӯ��

	double ci0346;                  //���1�����ܸ�ծ/���ʲ�
	double ci0347;                  //���1�����ܸ�ծ/�ɶ�Ȩ��
	double ci0348;                  //���1������Ϣ���ϱ���
	double ci0349;                  //���4������Ϣ���ϱ���
	double ci0401;                  //1�±�����ӯ����׼����

	double ci0402;                  //1����һ����ӯ����׼����
	double ci0403;					//δ��12�¾�����������
	double ci0404;					//1�±�����ӯ���仯							
	double ci0405;					//2�±�����ӯ���仯
	double ci0406;					//3�±�����ӯ���仯

	double ci0407;					//1����һ����ӯ���仯
	double ci0408;					//2����һ����ӯ���仯
	double ci0409;					//3����һ����ӯ���仯
	double ci0410;					//1�������仯
	double ci0411;					//2�������仯
	double ci0412;					//3�������仯

	double ci0413;					//��ǰ����
};

//��Ŀ��
struct tagSubjectTable
{
	unsigned char IND_ID[10];    //ָ�����
	unsigned char IND_NM[40];    //ָ����������
	unsigned char IND_ENM[40];   //ָ��Ӣ������
	unsigned char Mea_Unit[10];  //������λ
};

struct tagSubjectTableEx
{
	unsigned short TableNum;     //��Ŀ��ı���
	tagSubjectTable table;
};

struct tagCommentaryCollect
{
	unsigned char Sta_Prd[4];   //ͳ��ʱ��
	double Avg_Value;           //�ۺϽ���
	double Tbu_Num;             //ǿ����������
	double Buy_Num;             //��������
	double Neu_Num;             //��������
	double Red_Num;             //�ʶȼ�������
	double Sel_Num;             //��������
	double Avg_Price;			//�ۺ�Ŀ��۸�
	double Hig_Price;           //���Ŀ��۸�
	double Low_Price;           //���Ŀ��۸�
};

struct tagCommentaryCollectEx
{
	unsigned char SEC_CD[8];      //��Ʊ����
	tagCommentaryCollect collect;
};

struct tagVerifyUserInfo
{
	unsigned char UserID[16];
	unsigned char UserName[16];
	unsigned char Result[1];     //0ʧ�ܣ�1ͨ��
	unsigned char Role[64];
};

struct tagModifyPassword
{
	unsigned char UserID[16];
	unsigned char Result[1];     //0ʧ�ܣ�1ͨ��
};

struct tagAnalyserInfo
{
	unsigned char ANA_Account[20]; //����ʦ�ʺ�
	unsigned char ANA_NM[20];	   //����ʦ����
	unsigned char Company[40];     //������λ
};

struct tagAnalyserRemark
{
	unsigned char ANA_Account[20]; //����ʦ�ʺ�
	unsigned char SEC_CD[8];       //��Ʊ����
	unsigned char IDU_CD[10];      //��ҵ����
	unsigned char UPDT_TM[8];      //����YYYYMMDD
	unsigned short CommentLen;	   //���۵ĳ���
	//�����ַ����������β
};

struct tagAddRemark
{
	unsigned char UPDT_TM[8];      //����YYYYMMDD
	unsigned char ANA_Account[20]; //����ʦ�ʺ�
	unsigned char SEC_CD[8];       //��Ʊ����
	unsigned char IDU_CD[10];      //��ҵ����
	unsigned short CommentLen;	   //���۵ĳ���
};

struct tagHisFinanceIdx
{
	unsigned char IND_ID[10];	   //ָ�����
	unsigned char Rpt_Yr[4];	   //�������
	unsigned char Rpt_Prd[4];	   //����ʱ��
	double        Ind_Value;       //ָ��ֵ
};

struct tagIdxHisData
{
	unsigned char Date[8];		  //����
	unsigned char Value[20];	  //ֵ
};

struct tagIduRepresentData
{
	unsigned short nFactorMark;   //���ر�ʶID
	double field1;
	double field2;
	double field3;
	double field4;
	double field5;
};

struct tagIduSortData
{
	unsigned char Idu_CD[10];    //��ҵ����
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
	char IDX_CD[8];	   //ָ������
	char Dt_prd[5];	   //ʱ������
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
	char IDX_CD[8];	   //ָ������
	double cm0701;
	double cm0702;
	double cm0703;
	double cm0704;
	double cm0705;

	char cm0706[50]; //��ȥһ�������õ��г�����
	char cm0707[50]; //��ȥһ�������õ��г�����
	char cm0708[50]; //��ȥһ�ܱ�����õ���ҵ
	char cm0709[50]; //��ȥһ�±�����õ���ҵ
};

#pragma pack(pop)
/////////////////////////////////////////////////////////////////////////////////
#endif