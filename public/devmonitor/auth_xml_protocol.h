
#ifndef __AUTH_XML_PROTOCOL_H__
#define __AUTH_XML_PROTOCOL_H__

#pragma warning(disable:4786)

#include "../xml/tinyxml.h"
#include <string>
#include <list>
#include <map>

using namespace std;
typedef std::map<unsigned long, std::string> CDevMap;

class CAuthProtoclXML
{
public:
	/** @brief      产生业务服务器与认证服务器之间的心跳包
	 *
	 *  由业务服务器定时发送给认证服务器，认证服务器收到后原样返回
	 *  @retval     string，xml协议字符串
	 *  @warning    
	 *  @see        <?xml version="1.0" encoding="utf-8" standalone="yes" ?><HeartBeat />
	 *  @ingroup    
	 *  @exception  
	 */
	string HeartBeat()
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "utf-8", "yes");
        xmlDoc.InsertEndChild(Declaration);
		
        TiXmlElement* pRootElm = new TiXmlElement("HeartBeat");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();
		
        //xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;

    	return sxml;
	};

    /** @brief     生成用户登录的xml协议字符串
     *
     *  当服务器端有用户登录时调用
     *  @param     lpUser，用户名
     *  @param     lpPwd，登录密码
     *  @retval    string，xml协议字符串
     *  @warning
     *  @see	   <?xml version="1.0" encoding="utf-8" standalone="yes" ?><UserLogin><User Pwd="123456">testUser</User></UserLogin>
     *  @ingroup
     *  @exception
     */
    string UserLogin(const char *lpUser, const char *lpPwd)
    {
        TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "utf-8", "yes");
        xmlDoc.InsertEndChild(Declaration);

        TiXmlElement* pRootElm = new TiXmlElement("UserLogin");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();

        TiXmlElement *pChildeElm = new TiXmlElement("User");
        pChildeElm->SetAttribute("Pwd", lpPwd);
        pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

        TiXmlText *pText = new TiXmlText(lpUser);
        pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;

        //xmlDoc.Print();
	    string sxml;
    	sxml << xmlDoc;
    	return sxml;
    };

	/** @brief      生成用户登出时的xml协议字符串
	 *
	 *  当服务器端的用户退出时调用
	 *  @param      lpUser，用户名
	 *  @param      nUserID，认证服务器返回的用户ID
	 *  @retval     string，xml协议字符串
	 *  @warning    
	 *  @see        <?xml version="1.0" encoding="utf-8" standalone="yes" ?><UserLogout><User>testUser</User></UserLogout>
	 *  @ingroup    
	 *  @exception  
	 */
	string UserLogout(const char *lpUser)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "utf-8", "yes");
        xmlDoc.InsertEndChild(Declaration);

        TiXmlElement* pRootElm = new TiXmlElement("UserLogout");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();

        TiXmlElement *pChildeElm = new TiXmlElement("User");
        pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

        TiXmlText *pText = new TiXmlText(lpUser);
        pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;

        //xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	};

	/** @brief      产生通知用户的通知xml协议字符串
	 *
	 *  认证服务器通知业务服务器，向某用户发出通知消息，并决定是否要终止用户登录
	 *  @param      nUserID，用户在认证服务器中的唯一ID
	 *  @param      lpMessage，通知消息
	 *  @param      bConn，用户连接是否保持，true-保持，false-断开
	 *  @retval     string，xml协议字符串
	 *  @warning    
	 *  @see        <?xml version="1.0" encoding="utf-8" standalone="yes" ?><UserNotify><User Connect="0">97</User><Message>看你不爽，滚蛋吧！</Message></UserNotify>
	 *  @ingroup    
	 *  @exception  
	 */
	string UserNotify(unsigned int nUserID, const char *lpMessage, bool bConn)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "utf-8", "yes");
        xmlDoc.InsertEndChild(Declaration);
		
        TiXmlElement* pRootElm = new TiXmlElement("UserNotify");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();
		
        TiXmlElement *pChildeElm = new TiXmlElement("User");
        pChildeElm->SetAttribute("Connect", bConn);
        pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

		char szID[256];
		sprintf_s(szID, 256, "%d", nUserID);
        TiXmlText *pText = new TiXmlText(szID);
        pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;

		pChildeElm = new TiXmlElement("Message");
		pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

		pText = new TiXmlText(lpMessage);
		pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;
		
        //xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	};

	/** @brief      产生通知认证服务器的服务器登录xml协议字符串
	 *
	 *  当服务器登录时，通知认证服务器，此时服务器上没有用户登录，如果服务器上有用户，可以在稍后调用用户登录功能
	 *  @param      lpFingerPrinter，服务器指纹
	 *  @param      lpDevList，设备列表
	 *  @retval     string，xml协议字符串
	 *  @warning    
	 *  @see        <?xml version="1.0" encoding="utf-8" standalone="yes" ?><ServerLogin><Server FingerPrinter = "server002"></Server></ServerLogin>
	 *  @ingroup    
	 *  @exception  
	 */
	string ServerLogin(const char *lpFingerPrinter, const char *lpDevList)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "gbk", "yes");
        xmlDoc.InsertEndChild(Declaration);

		TiXmlElement* pRootElm = new TiXmlElement("ServerLogin");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();

		TiXmlElement *pChildeElm  = new TiXmlElement("Server");
		pChildeElm->SetAttribute("FingerPrinter", lpFingerPrinter);
		pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

		TiXmlText *pText = new TiXmlText(lpDevList);
		pChildeElm = pNode->ToElement();
		pChildeElm->InsertEndChild(*pText);
		delete pText;
		
		//xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	}

	/** @brief      产生通知认证服务器的服务器登录xml协议字符串
	 *
	 *  当服务器登录时，通知认证服务器，此时服务器上没有用户登录，如果服务器上有用户，可以在稍后调用用户登录功能
	 *  @param      lpFingerPrinter，服务器指纹
	 *  @retval     string，xml协议字符串
	 *  @warning    
	 *  @see        <?xml version="1.0" encoding="utf-8" standalone="yes" ?><ServerLogout><Server FingerPrinter = "server002"></Server></ServerLogout>
	 *  @ingroup    
	 *  @exception  
	 */
	string ServerLogout(const char *lpFingerPrinter)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "gbk", "yes");
        xmlDoc.InsertEndChild(Declaration);

		TiXmlElement* pRootElm = new TiXmlElement("ServerLogout");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();

		TiXmlElement *pServer = new TiXmlElement("Server");
		pServer->SetAttribute("FingerPrinter", lpFingerPrinter);
		pNode = pRootElm->InsertEndChild(*pServer);
		delete pServer;
		pServer = pNode->ToElement();

		//xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	}

	/** @brief      产生通知认证服务器的服务器登录xml协议字符串
	 *
	 *  当连接认证服务器成功时，通知认证服务器应用服务器上已经登录的用户
	 *  @param      pfnGetUserAccount，获取用户名称的模板函数，该模板函数的原型为:
					template <class T> bool GetUserAccount(char *lpAccount, T *idx)，
					返回false表示无后续用户
	 *  @param      pIdxSvr，服务器指纹检索索引
	 *  @param      pIdxUser，用户名称检索索引
	 *  @retval     string，xml协议字符串
	 *  @warning    
	 *  @see        <?xml version="1.0" encoding="utf-8" standalone="yes" ?><ServerLogin><Server FingerPrinter = "server002"><User>user1</User><User>leo</User><User>smith john</User><User>Bye-bye</User></Server><Server FingerPrinter = "IBM-XINL"><User>Bill</User></Server></ServerLogin>
	 *  @ingroup    
	 *  @exception  
	 */
	template <class T, class VS, class VU>
	string ServerLogin(T pfnGetUserAccount, VS *pIdxSvr, VU *pIdxUser)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "gbk", "yes");
        xmlDoc.InsertEndChild(Declaration);

		TiXmlElement* pRootElm = new TiXmlElement("ServerLogin");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();

		char szName[256];
		TiXmlElement *pServer = NULL;
		while (1)
		{
			int ir = pfnGetUserAccount(szName, pIdxSvr, pIdxUser);
			if (ir == 1)	//返回了服务器指纹
			{
				pServer = new TiXmlElement("Server");
				pServer->SetAttribute("FingerPrinter", szName);
				pNode = pRootElm->InsertEndChild(*pServer);
				delete pServer;
				pServer = pNode->ToElement();
			}
			else if (ir == 2)
			{
				TiXmlElement *pChildeElm = new TiXmlElement("User");
				pNode = pServer->InsertEndChild(*pChildeElm);
				delete pChildeElm;

				TiXmlText *pText = new TiXmlText(szName);
				pChildeElm = pNode->ToElement();
				pChildeElm->InsertEndChild(*pText);
				delete pText;
			}
			else
				break;
		}

		//xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	};

	//ad by dj
	string ServerLogin(const char* pDogVer)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "utf-8", "yes");
        xmlDoc.InsertEndChild(Declaration);

		TiXmlElement* pRootElm = new TiXmlElement("ServerLogin");
		if (pDogVer != NULL)
			pRootElm->SetAttribute("Version", pDogVer);

        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();

		//xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	};
	/** @brief     发送已登录设备列表的xml协议字符串
     *
     *  当客户端用户登录时调用
     *  @param     plstDev，已登录设备列表
     *  @retval    string，xml协议字符串
     *  @warning
     *  @see	   <?xml version="1.0" encoding="utf-8" standalone="yes" ?><UserAuth><User Pwd="123456">testUser</User><Product>Pc-client</Product></UserAuth>
     *  @ingroup
     *  @exception
     */
	string SendLoginDevList(CDevMap* pmapDev)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "utf-8", "yes");
        xmlDoc.InsertEndChild(Declaration);

		TiXmlElement* pRootElm = new TiXmlElement("LoginDevList");
		TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
		pRootElm = pNode->ToElement();

		CDevMap::iterator pos = pmapDev->begin();
		while(pos != pmapDev->end())
		{
			TiXmlElement *pChildeElm = new TiXmlElement("DevName");
			char szProID[20] = {0};
			sprintf_s(szProID, 20, "%d", pos->first);
			pChildeElm->SetAttribute("ProID", szProID);
			pNode = pRootElm->InsertEndChild(*pChildeElm);
			delete pChildeElm;

			TiXmlText *pText = new TiXmlText(pos->second);
			pChildeElm = pNode->ToElement();
			pChildeElm->InsertEndChild(*pText);
			delete pText;

			pos++;
		}

		//xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	}

	/** @brief     生成用户登录的xml协议字符串
     *
     *  当客户端用户登录时调用
     *  @param     lpUser，用户名
     *  @param     lpPwd，登录密码
     *  @retval    string，xml协议字符串
     *  @warning
     *  @see	   <?xml version="1.0" encoding="utf-8" standalone="yes" ?><UserAuth><User Pwd="123456">testUser</User><Product>Pc-client</Product></UserAuth>
     *  @ingroup
     *  @exception
     */
	string UserAuth(const char *lpUser, const char *lpPwd, const char *lpProduct)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "utf-8", "yes");
        xmlDoc.InsertEndChild(Declaration);
		
        TiXmlElement* pRootElm = new TiXmlElement("UserAuth");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();
		
        TiXmlElement *pChildeElm = new TiXmlElement("User");
        pChildeElm->SetAttribute("Pwd", lpPwd);
        pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;
		
        TiXmlText *pText = new TiXmlText(lpUser);
        pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;

		pChildeElm = new TiXmlElement("Product");
		pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

		pText = new TiXmlText(lpProduct);
        pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;
		
        //xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	};

	/** @brief     生成日志传递的字符串
     *
     *  当客户端有日志传递给控制中心时调用
	 *  @param     lpModule，发送日志的模块名称
	 *  @param     usLevel，日志级别
     *  @param     lpLogMsg，日志内容
     *  @retval    string，xml协议字符串
     *  @warning
     *  @see	   <?xml version="1.0" encoding="utf-8" standalone="yes" ?><Log IP="111.111.111.111" Module="QuoteServer" Level="1">Server Started.</Log>
     *  @ingroup
     *  @exception
     */
	string Log(const char *lpModule, unsigned short usLevel, const char *lpLogMsg,const char *strLocalAddress)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "gbk", "yes");
        xmlDoc.InsertEndChild(Declaration);

        TiXmlElement* pRootElm = new TiXmlElement("Log");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();

        pRootElm->SetAttribute("Module", lpModule);
		char szLevel[21];
		sprintf_s(szLevel, 21, "%d", usLevel);
		pRootElm->SetAttribute("Level", szLevel);
		pRootElm->SetAttribute("IP", strLocalAddress);

		TiXmlText *pText = new TiXmlText(lpLogMsg);
        pRootElm->InsertEndChild(*pText);
		delete pText;

		//xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	}


	/** @brief     下载xml配置文件
     *
	 *  @param     lpDevName，设备名称
	 *  @param     XmlPath，配置文件路径
     *  @retval    string，xml协议字符串
     *  @warning
     *  @ingroup
     *  @exception
     */
	string DownloadXml(const char *lpDevName, const char *lpXmlPath)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "gbk", "yes");
        xmlDoc.InsertEndChild(Declaration);

        TiXmlElement* pRootElm = new TiXmlElement("DownloadXml");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();

        pRootElm->SetAttribute("DevName", lpDevName);

		TiXmlText *pText = new TiXmlText(lpXmlPath);
        pRootElm->InsertEndChild(*pText);
		delete pText;

		//xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	}

	/** @brief    上传xml配置文件
     *
	 *  @param     lpXmlPath，配置文件路径
	 *  @param     lpXmlData，配置文件数据
	 *  @param     nSize，配置文件大小
     *  @retval    string，xml协议字符串
     *  @warning
     *  @ingroup
     *  @exception
	 */
	string UploadXml(const char *lpXmlPath, const char *lpXmlData, int nSize)
	{
		TiXmlDocument xmlDoc;
		TiXmlDeclaration Declaration("1.0", "gbk", "yes");
		xmlDoc.InsertEndChild(Declaration);

		TiXmlElement* pRootElm = new TiXmlElement("UploadXml");
		TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
		pRootElm = pNode->ToElement();

		pRootElm->SetAttribute("XmlPath", lpXmlPath);
		char szDataSize[20];
		sprintf_s(szDataSize, 20, "%d", nSize);
		pRootElm->SetAttribute("DataSize", szDataSize);

		TiXmlText *pText = new TiXmlText(lpXmlData);
		pRootElm->InsertEndChild(*pText);
		delete pText;

		//xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
		return sxml;
	}
	/** @brief     发送xml配置文件数据
     *
	 *  @param     lpDevName，设备名称
	 *  @param     lpXmlData，配置文件数据
	 *  @param     nSize，配置文件大小
	 *  @param     XmlName，文件名称
     *  @retval    string，xml协议字符串
     *  @warning
     *  @ingroup
     *  @exception
     */

	string SendXmlData(const char *lpDevName, const char *lpXmlData, int nSize, const char *lpXmlName)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "gbk", "yes");
        xmlDoc.InsertEndChild(Declaration);

        TiXmlElement* pRootElm = new TiXmlElement("XmlData");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();

        pRootElm->SetAttribute("DevName", lpDevName);
		char szDataSize[20];
		sprintf_s(szDataSize, 20, "%d", nSize);
		pRootElm->SetAttribute("DataSize", szDataSize);
		pRootElm->SetAttribute("XmlName", lpXmlName);

		TiXmlText *pText = new TiXmlText(lpXmlData);
        pRootElm->InsertEndChild(*pText);
		delete pText;

		//xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	}

	/** @brief     生成CC启动客户端程序的字符串
     *
     *  当CC启动客户端程序时调用
	 *  @param     lpExecutor，程序名称
	 *  @param     lpParams，执行参数
     *  @param     lpWorkDir，工作目录
     *  @retval    string，xml协议字符串
     *  @warning
     *  @see	   <?xml version="1.0" encoding="utf-8" standalone="yes" ?><Start><appName>testapp</appName><param>1</param><workDir>d:\\</workDir></Start>
     *  @ingroup
     *  @exception
     */
	string StartApp(const char *lpExecutor, const char *lpParams, const char *lpWorkDir)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "gbk", "yes");
        xmlDoc.InsertEndChild(Declaration);

		TiXmlElement* pRootElm = new TiXmlElement("Start");
		TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
		pRootElm = pNode->ToElement();

		TiXmlElement *pChildeElm = new TiXmlElement("appName");
        pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

        TiXmlText *pText = new TiXmlText(lpExecutor);
        pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;

		pChildeElm = new TiXmlElement("param");
        pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

        pText = new TiXmlText(lpParams);
        pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;

		pChildeElm = new TiXmlElement("workDir");
        pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

        pText = new TiXmlText(lpWorkDir);
        pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;

        //xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;

    	return sxml;
	}


	/** @brief     生成CC启动客户端程序的字符串
     *
     *  当CC启动客户端程序的字符串时调用
	 *  @param     lpExecutor，程序名称
	 *  @param     lpParams，执行参数
     *  @param     lpWorkDir，工作目录
     *  @retval    string，xml协议字符串
     *  @warning
     *  @see	   <?xml version="1.0" encoding="utf-8" standalone="yes" ?><Stop><appName>testapp</appName><param>1</param><workDir>d:\\</workDir></Stop>
     *  @ingroup
     *  @exception
     */
	string StopApp(const char *lpExecutor, const char *lpParams, const char *lpWorkDir)
	{
		TiXmlDocument xmlDoc;
        TiXmlDeclaration Declaration("1.0", "gbk", "yes");
        xmlDoc.InsertEndChild(Declaration);

        TiXmlElement* pRootElm = new TiXmlElement("Stop");
        TiXmlNode *pNode = xmlDoc.InsertEndChild(*pRootElm);
		delete pRootElm;
        pRootElm = pNode->ToElement();

        TiXmlElement *pChildeElm = new TiXmlElement("appName");
        pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

        TiXmlText *pText = new TiXmlText(lpExecutor);
        pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;

		pChildeElm = new TiXmlElement("param");
        pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

        pText = new TiXmlText(lpParams);
        pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;

		pChildeElm = new TiXmlElement("workDir");
        pNode = pRootElm->InsertEndChild(*pChildeElm);
		delete pChildeElm;

        pText = new TiXmlText(lpWorkDir);
        pChildeElm = pNode->ToElement();
        pChildeElm->InsertEndChild(*pText);
		delete pText;

        //xmlDoc.Print();
		string sxml;
		sxml << xmlDoc;
    	return sxml;
	}
	string m_strLocalAddress;
};

#endif
