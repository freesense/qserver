
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
	/** @brief      ����ҵ�����������֤������֮���������
	 *
	 *  ��ҵ���������ʱ���͸���֤����������֤�������յ���ԭ������
	 *  @retval     string��xmlЭ���ַ���
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

    /** @brief     �����û���¼��xmlЭ���ַ���
     *
     *  �������������û���¼ʱ����
     *  @param     lpUser���û���
     *  @param     lpPwd����¼����
     *  @retval    string��xmlЭ���ַ���
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

	/** @brief      �����û��ǳ�ʱ��xmlЭ���ַ���
	 *
	 *  ���������˵��û��˳�ʱ����
	 *  @param      lpUser���û���
	 *  @param      nUserID����֤���������ص��û�ID
	 *  @retval     string��xmlЭ���ַ���
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

	/** @brief      ����֪ͨ�û���֪ͨxmlЭ���ַ���
	 *
	 *  ��֤������֪ͨҵ�����������ĳ�û�����֪ͨ��Ϣ���������Ƿ�Ҫ��ֹ�û���¼
	 *  @param      nUserID���û�����֤�������е�ΨһID
	 *  @param      lpMessage��֪ͨ��Ϣ
	 *  @param      bConn���û������Ƿ񱣳֣�true-���֣�false-�Ͽ�
	 *  @retval     string��xmlЭ���ַ���
	 *  @warning    
	 *  @see        <?xml version="1.0" encoding="utf-8" standalone="yes" ?><UserNotify><User Connect="0">97</User><Message>���㲻ˬ�������ɣ�</Message></UserNotify>
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

	/** @brief      ����֪ͨ��֤�������ķ�������¼xmlЭ���ַ���
	 *
	 *  ����������¼ʱ��֪ͨ��֤����������ʱ��������û���û���¼����������������û����������Ժ�����û���¼����
	 *  @param      lpFingerPrinter��������ָ��
	 *  @param      lpDevList���豸�б�
	 *  @retval     string��xmlЭ���ַ���
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

	/** @brief      ����֪ͨ��֤�������ķ�������¼xmlЭ���ַ���
	 *
	 *  ����������¼ʱ��֪ͨ��֤����������ʱ��������û���û���¼����������������û����������Ժ�����û���¼����
	 *  @param      lpFingerPrinter��������ָ��
	 *  @retval     string��xmlЭ���ַ���
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

	/** @brief      ����֪ͨ��֤�������ķ�������¼xmlЭ���ַ���
	 *
	 *  ��������֤�������ɹ�ʱ��֪ͨ��֤������Ӧ�÷��������Ѿ���¼���û�
	 *  @param      pfnGetUserAccount����ȡ�û����Ƶ�ģ�庯������ģ�庯����ԭ��Ϊ:
					template <class T> bool GetUserAccount(char *lpAccount, T *idx)��
					����false��ʾ�޺����û�
	 *  @param      pIdxSvr��������ָ�Ƽ�������
	 *  @param      pIdxUser���û����Ƽ�������
	 *  @retval     string��xmlЭ���ַ���
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
			if (ir == 1)	//�����˷�����ָ��
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
	/** @brief     �����ѵ�¼�豸�б��xmlЭ���ַ���
     *
     *  ���ͻ����û���¼ʱ����
     *  @param     plstDev���ѵ�¼�豸�б�
     *  @retval    string��xmlЭ���ַ���
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

	/** @brief     �����û���¼��xmlЭ���ַ���
     *
     *  ���ͻ����û���¼ʱ����
     *  @param     lpUser���û���
     *  @param     lpPwd����¼����
     *  @retval    string��xmlЭ���ַ���
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

	/** @brief     ������־���ݵ��ַ���
     *
     *  ���ͻ�������־���ݸ���������ʱ����
	 *  @param     lpModule��������־��ģ������
	 *  @param     usLevel����־����
     *  @param     lpLogMsg����־����
     *  @retval    string��xmlЭ���ַ���
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


	/** @brief     ����xml�����ļ�
     *
	 *  @param     lpDevName���豸����
	 *  @param     XmlPath�������ļ�·��
     *  @retval    string��xmlЭ���ַ���
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

	/** @brief    �ϴ�xml�����ļ�
     *
	 *  @param     lpXmlPath�������ļ�·��
	 *  @param     lpXmlData�������ļ�����
	 *  @param     nSize�������ļ���С
     *  @retval    string��xmlЭ���ַ���
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
	/** @brief     ����xml�����ļ�����
     *
	 *  @param     lpDevName���豸����
	 *  @param     lpXmlData�������ļ�����
	 *  @param     nSize�������ļ���С
	 *  @param     XmlName���ļ�����
     *  @retval    string��xmlЭ���ַ���
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

	/** @brief     ����CC�����ͻ��˳�����ַ���
     *
     *  ��CC�����ͻ��˳���ʱ����
	 *  @param     lpExecutor����������
	 *  @param     lpParams��ִ�в���
     *  @param     lpWorkDir������Ŀ¼
     *  @retval    string��xmlЭ���ַ���
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


	/** @brief     ����CC�����ͻ��˳�����ַ���
     *
     *  ��CC�����ͻ��˳�����ַ���ʱ����
	 *  @param     lpExecutor����������
	 *  @param     lpParams��ִ�в���
     *  @param     lpWorkDir������Ŀ¼
     *  @retval    string��xmlЭ���ַ���
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
