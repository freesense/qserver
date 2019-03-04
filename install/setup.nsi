; �ýű�ʹ�� HM VNISEdit �ű��༭���򵼲���
; $R9 ���濴�Ź�����˿ڣ������������޸�����ʹ��
; ���Ź������ַ�̶�ʹ��127.0.0.1

Var SHOW_INST_WATCHDOG              ; �Ƿ���ʾ���Ź�����ҳ��
Var SHOW_INST_FEED                  ; �Ƿ���ʾFeed����ҳ��

; ��װ�����ʼ���峣��
!define PRODUCT_NAME "�����������"
!define PRODUCT_VERSION "2.0"
!define PRODUCT_PUBLISHER "�����н���Ͷ�ʲƾ���Ѷ���޹�˾"
!define PRODUCT_WEB_SITE "http://www.investoday.com.cn"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\QuoteConsole.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

SetCompressor lzma

BrandingText "�����н���Ͷ�ʲƾ���Ѷ���޹�˾"
SetFont "Tahoma" 8

ReserveFile "${NSISDIR}\Plugins\InstallOptions.dll"
ReserveFile "install_watchdog.ini"
ReserveFile "install_feed.ini"

; ------ MUI �ִ����涨�� (1.67 �汾���ϼ���) ------
!include "MUI.nsh"
!include "FileFunc.nsh"
!include "InstallOptions.nsh"
!include "xml.nsh"
!include "LogicLib.nsh"

; MUI Ԥ���峣��
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; �����������·���ʾ��
!define MUI_COMPONENTSPAGE_SMALLDESC

; ��ӭҳ��
!insertmacro MUI_PAGE_WELCOME
; ���Э��ҳ��
;!insertmacro MUI_PAGE_LICENSE "licence.txt"

!define MUI_COMPONENTSPAGE_TEXT_TOP "QuoteFeed���Ե�����װ��һ̨�������ϣ�QuoteFarm��\
QuoteServer��װ����һ̨�������ϡ�����QuoteFarm��QuoteServer���Թ���ͬһ̨QuoteFeed��������"
!define MUI_COMPONENTSPAGE_TEXT_COMPLIST "����ݲ���ƻ���ϸѡ��Ҫ�����ģ�����չ���"

; ���ѡ��ҳ��
!insertmacro MUI_PAGE_COMPONENTS
; ��װĿ¼ѡ��ҳ��
!insertmacro MUI_PAGE_DIRECTORY
; ��װ����ҳ��
!insertmacro MUI_PAGE_INSTFILES

; ���ÿ��Ź�
Page custom install_watchdog_page validate_install_watchdog
Page custom install_feed_page validate_install_feed

!define MUI_FINISHPAGE_LINK_LOCATION "http://www.investoday.com.cn"
!define MUI_FINISHPAGE_LINK "���ʽ���Ͷ����ҳ"

!define MUI_FINISHPAGE_RUN_TEXT "���������ۿ�̨���������µ�K���ļ�"
!define MUI_FINISHPAGE_RUN "$INSTDIR\QuoteConsole"

!define MUI_FINISHPAGE_SHOWREADME_TEXT "�Ķ���ά�ֲ�"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\QuoteServerͨѶЭ��.doc"

!define MUI_FINISHPAGE_TEXT "������ɰ�װ��${PRODUCT_NAME} ${PRODUCT_VERSION}��������������������ǰ������ϸ�Ķ���ά�ֲᣬ�޸ĸ�������������ļ���"

; ��װ���ҳ��
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME

!define MUI_UNCONFIRMPAGE_TEXT_TOP "ж�س���ǿ�н������̣�ɾ����ط���ɾ����װĿ¼�³�������������ļ���$\r$\n$\r$\n$_CLICK"

!insertmacro MUI_UNPAGE_CONFIRM

; ��װж�ع���ҳ��
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

; ��װ�����������������
!insertmacro MUI_LANGUAGE "SimpChinese"

; ��װԤ�ͷ��ļ�
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; ------ MUI �ִ����涨����� ------

!include "Sections.nsh"
!include "LogicLib.nsh"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "quotesetup.exe"
InstallDir "D:\jrtzhq2\run"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
ShowInstDetails show
ShowUnInstDetails show

Section "����֧���ļ�" SECACQUIRE
  SectionIn RO
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "..\binr\commxr.dll"
  File "..\binr\proxy_auth_server.dll"
  File "..\binr\msvcm80.dll"
  File "..\binr\msvcp80.dll"
  File "..\binr\msvcr80.dll"
  File "..\binr\Microsoft.VC80.CRT.manifest"
  File "..\binr\Microsoft.VC80.MFC.manifest"
  File "..\doc\����Ͷ������ϵͳ��ά�ֲ�.doc"
  CreateDirectory "$SMPROGRAMS\�����������"
  CreateShortCut "$SMPROGRAMS\�����������\��ά�ֲ�.lnk" "$INSTDIR\����Ͷ������ϵͳ��ά�ֲ�.doc"
SectionEnd

SectionGroup "QuoteFeed" SEC01
  Section "������" SEC_FEED_MAIN
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    StrCpy $SHOW_INST_FEED "show"
    File "..\binr\QuoteFeed.exe"
    File "..\binr\QuoteFeed.xml"
;    ExecWait 'cmd.exe /c "sc create QuoteFeed binpath= $\"$INSTDIR\QuoteFeed.exe$\" displayname= ����Ͷ��[QuoteFeed]"'
  SectionEnd
  
  Section "�������" SEC_FEED_HS
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\SHFeed.dll"
  SectionEnd
  
  Section "�۹���ʱ����" SEC_FEED_HK
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\HKFeed.dll"
  SectionEnd
SectionGroupEnd

SectionGroup "QuoteFarm" SEC02
  Section "������" SEC_FARM_MAIN
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\QuoteFarm.exe"
    File "..\binr\QuoteFarm.xml"
    File "..\binr\holiday.txt"
    ExecWait 'cmd.exe /c "sc create QuoteFarm binpath= $\"$INSTDIR\QuoteFarm.exe$\" displayname= ����Ͷ��[QuoteFarm] start= auto"'
  SectionEnd
  
  Section "���״�ϲ��" SEC_MATCH_ADDIN
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\tradematch.dll"
    File "..\binr\tradematch.xml"
  SectionEnd
SectionGroupEnd

SectionGroup "QuoteServer" SEC03
  Section "������" SEC_SERVER_MAIN
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\QuoteServer.exe"
    File "..\binr\QuoteServer.xml"
    ExecWait 'cmd.exe /c "sc create QuoteServer binpath= $\"$INSTDIR\QuoteServer.exe$\" displayname= ����Ͷ��[QuoteServer] start= auto"'
  SectionEnd
  
  Section "����ҵ����չ���" SEC_QEXTEND
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\qextend.dll"
    File "..\binr\ntwdblib.dll"
    File "..\binr\qextend.xml"
  SectionEnd
  
  Section "�������ݲ��" SEC_TIANMA_ADDIN
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\TianmaData.dll"
    File "..\binr\TianmaData.xml"
  SectionEnd
SectionGroupEnd

Section "�����ۿ�̨" SEC04
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "..\binr\Rar.exe"
  File "..\binr\QuoteConsole.exe"
  File "..\binr\QuoteConsole.ini"
  File "..\binr\QuoteConsole.mdb"
  File "..\binr\mfc80.dll"
  File "..\binr\mfc80u.dll"
  File "..\binr\mfcm80.dll"
  File "..\binr\mfcm80u.dll"
  
  CreateShortCut "$SMPROGRAMS\�����������\�����ۿ�̨.lnk" "$INSTDIR\QuoteConsole.exe"
  CreateShortCut "$DESKTOP\�����ۿ�̨.lnk" "$INSTDIR\QuoteConsole.exe"
  CreateShortCut "$SMSTARTUP\�����ۿ�̨.lnk" "$INSTDIR\QuoteConsole.exe"
  
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\QuoteConsole.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\QuoteConsole.exe"
SectionEnd

Section "��ط������" SEC05
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  StrCpy $SHOW_INST_WATCHDOG "show"
  File "..\binr\WatchDog.exe"
  File "..\binr\WatchDog.xml"
  ExecWait 'cmd.exe /c "sc create WatchDog binpath= $\"$INSTDIR\WatchDog.exe$\" displayname= ����Ͷ��[Daemon] start= auto"'
  ExecWait 'cmd.exe /c "sc config WatchDog type= interact type= own"'
SectionEnd

Section /o "Э��ת������" SEC06
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "..\binr\qpx.exe"
  File "..\binr\qpx.xml"
  ExecWait 'cmd.exe /c "sc create qpx binpath= $\"$INSTDIR\qpx.exe$\" displayname= ����Ͷ��[Э��ת��] start= auto"'
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\�����������\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\�����������\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

#-- ���� NSIS �ű��༭�������� Function ���α�������� Section ����֮���д���Ա��ⰲװ�������δ��Ԥ֪�����⡣--#

; �����������
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SECACQUIRE} "���밲װ�Ļ���֧���ļ���"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "Ϊ����ϵͳ�ṩ��������ת������"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_FEED_MAIN} "���밲װ������ת��������"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_FEED_HK} "�ṩ�۹�����ת����"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_FEED_HS} "�ṩ�����������ת����"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "ʵʱ�����ڴ����ݿ⣬��װ�г�֮��Ĳ��졣"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_FARM_MAIN} "���밲װ���ṩͳһ���������ݽӿڡ�"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_MATCH_ADDIN} "�ṩģ�⽻�״�Ϲ��ܣ�������Mssql���ݿ⡣"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} "Ϊ�û��ṩ�ⲿ���ݷ��ʷ���"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_SERVER_MAIN} "���밲װ���÷�����ͨ�����ݷ��ʽӿ��ṩͳһ�Ķ������"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_TIANMA_ADDIN} "����Ͷ������ҵ������������Mssql���ݿ⡣"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_QEXTEND} "�ṩ���а��������չҵ�񣬲���ҵ����ҪMssql���ݿ�֧�֡�"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} "�ṩK������ά�����������ʵʱ��صȹ��ܣ�������ÿ̨�������϶���װ��"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC05} "���Ҫ���������������ۿ�̨�ļ�ط�Χ֮�ڣ����밲װ�������������ÿ̨�������϶���װ��"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC06} "�ṩ��������1.0��2.0֮���Э��ת������"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

/******************************
 *  �����ǰ�װ�����ж�ز���  *
 ******************************/

Section Uninstall
  ExecWait 'cmd.exe /c "taskkill /f /im qpx.exe"'
  ExecWait 'cmd.exe /c "taskkill /f /im QuoteFarm.exe"'
  ExecWait 'cmd.exe /c "taskkill /f /im QuoteServer.exe"'
  ExecWait 'cmd.exe /c "taskkill /f /im QuoteFeed.exe"'
  ExecWait 'cmd.exe /c "taskkill /f /im QuoteConsole.exe"'
  ExecWait 'cmd.exe /c "taskkill /f /im WatchDog.exe"'
  
  ExecWait 'cmd.exe /c "sc delete qpx"'
  ExecWait 'cmd.exe /c "sc delete WatchDog"'
  ExecWait 'cmd.exe /c "sc delete QuoteServer"'
  ExecWait 'cmd.exe /c "sc delete QuoteFarm"'
  
  Delete "$SMPROGRAMS\�����������\Uninstall.lnk"
  Delete "$SMPROGRAMS\�����������\Website.lnk"
  Delete "$DESKTOP\�����ۿ�̨.lnk"
  Delete "$SMPROGRAMS\�����������\�����ۿ�̨.lnk"
  Delete "$SMSTARTUP\�����ۿ�̨.lnk"
  Delete "$SMPROGRAMS\�����������\��ά�ֲ�.lnk"

  RMDir "$SMPROGRAMS\�����������"

  SetOutPath $TEMP
  
; �������������ļ�����Ŀ¼�����ⶪʧ���ݣ����°�װʱҪ��������
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.exe"
  Delete "$INSTDIR\*.manifest"
  Delete "$INSTDIR\*.doc"
  Delete "$INSTDIR\*.lnk"
  Delete "$INSTDIR\*.url"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd

#-- ���� NSIS �ű��༭�������� Function ���α�������� Section ����֮���д���Ա��ⰲװ�������δ��Ԥ֪�����⡣--#

Function .onInit
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "itfq_install") i .r1 ?e'
  Pop $R0

  StrCmp $R0 0 +3
  MessageBox MB_OK|MB_ICONEXCLAMATION "��װ�����Ѿ������С�"
  Abort
  
  StrCpy $SHOW_INST_WATCHDOG "no"
  StrCpy $SHOW_INST_FEED "no"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "install_watchdog.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "install_feed.ini"
FunctionEnd

LangString INSTALL_FEED_TITLE ${LANG_SIMPCHINESE} "QuoteFeed����"
LangString INSTALL_FEED_SUBTITLE ${LANG_SIMPCHINESE} "����QuoteFeed�ķ���˿ڣ��Լ���������Դ����"

Function install_feed_page
  StrCmp $SHOW_INST_FEED "show" 0 noshow
    ${xml::LoadFile} "quotefeed.xml" $9
    ${xml::GotoPath} "QuoteFeed/����Դ[1]" $9
    ${xml::GetAttribute} "����" $1 $9
    !insertmacro MUI_INSTALLOPTIONS_WRITE "install_feed.ini" "Field 7" "Text" $1
    ${xml::GotoPath} "" $9
    ${xml::GotoPath} "QuoteFeed/����Դ[2]" $9
    ${xml::GetAttribute} "����" $1 $9
    !insertmacro MUI_INSTALLOPTIONS_WRITE "install_feed.ini" "Field 3" "Text" $1
    ${xml::Unload}
    
    !insertmacro MUI_HEADER_TEXT "$(INSTALL_FEED_TITLE)" "$(INSTALL_FEED_SUBTITLE)"
    !insertmacro INSTALLOPTIONS_DISPLAY "install_feed.ini"
  noshow:
FunctionEnd

Function validate_install_feed
  ${xml::LoadFile} "quotefeed.xml" $9

  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 2" "State"
  ${xml::GotoPath} "QuoteFeed/����/�����ַ" $9
  ${xml::SetText} $0 $9
  ${xml::GotoPath} "" $9
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 8" "State"
  ${xml::GotoPath} "QuoteFeed/����Դ[1]" $9
  ${If} $0 == 0
    ${xml::SetAttribute} "״̬" "��ͣ" $9
  ${Else}
    ${xml::SetAttribute} "״̬" "����" $9
  ${EndIf}
  ${xml::GotoPath} "" $9
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 4" "State"
  ${xml::GotoPath} "QuoteFeed/����Դ[2]" $9
  ${If} $0 == 0
    ${xml::SetAttribute} "״̬" "��ͣ" $9
  ${Else}
    ${xml::SetAttribute} "״̬" "����" $9
  ${EndIf}
  ${xml::GotoPath} "" $9
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 6" "State"
  ${xml::GotoPath} "QuoteFeed/����Դ[2]/Params[1]" $9
  ${xml::SetText} $0 $9
  ${xml::GotoPath} "" $9
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 10" "State"
  ${xml::GotoPath} "QuoteFeed/����Դ[1]/Params[1]" $9
  ${xml::SetText} $0 $9
  ${xml::GotoPath} "" $9
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 12" "State"
  ${xml::GotoPath} "QuoteFeed/����Դ[1]/Params[2]" $9
  ${xml::SetText} $0 $9
  ${xml::GotoPath} "" $9
  
  ${xml::GotoPath} "QuoteFeed/�豸���/�豸�������˿�" $9
  ${xml::SetText} $R9 $9

  ${xml::SaveFile} "" $9
  ${xml::Unload}
FunctionEnd

LangString INSTALL_WATCHDOG_TITLE ${LANG_SIMPCHINESE} "�����������"
LangString INSTALL_WATCHDOG_SUBTITLE ${LANG_SIMPCHINESE} "�����豸�������ķ���˿ڣ��Լ�������ĵķ����ַ�Ͷ˿�"

Function install_watchdog_page
  StrCmp $SHOW_INST_WATCHDOG "show" 0 noshow
    !insertmacro MUI_HEADER_TEXT "$(INSTALL_WATCHDOG_TITLE)" "$(INSTALL_WATCHDOG_SUBTITLE)"
    !insertmacro INSTALLOPTIONS_DISPLAY "install_watchdog.ini"
  noshow:
FunctionEnd

Function validate_install_watchdog
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_watchdog.ini" "Field 2" "State"
  StrCmp $0 "" +1 +3
    MessageBox MB_OK "������ĵ�ַ����Ϊ��"
    Abort
  !insertmacro MUI_INSTALLOPTIONS_READ $1 "install_watchdog.ini" "Field 4" "State"
  StrCmp $1 "" +1 +3
    MessageBox MB_OK "������Ķ˿ڲ���Ϊ��"
    Abort
  !insertmacro MUI_INSTALLOPTIONS_READ $R9 "install_watchdog.ini" "Field 6" "State"
  StrCmp $R9 "" +1 +3
    MessageBox MB_OK "�������˿ڲ���Ϊ��"
    Abort
  !insertmacro MUI_INSTALLOPTIONS_READ $3 "install_watchdog.ini" "Field 7" "State"
  
  ${xml::LoadFile} "watchdog.xml" $9
  ${xml::GotoPath} "WatchDog/�������ĵ�ַ" $9
  ${xml::SetText} $0 $9
  ${xml::GotoPath} "" $9
  ${xml::GotoPath} "WatchDog/�������Ķ˿�" $9
  ${xml::SetText} $1 $9
  ${xml::GotoPath} "" $9
  ${xml::GotoPath} "WatchDog/�����ַ" $9
  ${xml::SetText} $R9 $9
  ${xml::SaveFile} "" $9
  ${xml::Unload}
  
  ${xml::LoadFile} "qpx.xml" $9
  ${If} $9 == 0
    ${xml::GotoPath} "qpx/�豸���/�豸������IP" $9
    ${xml::SetText} "127.0.0.1" $9
    ${xml::GotoPath} "" $9
    ${xml::GotoPath} "qpx/�豸���/�豸�������˿�" $9
    ${xml::SetText} $R9 $9
    ${xml::SaveFile} "" $9
    ${xml::Unload}
  ${EndIf}

  ${xml::LoadFile} "QuoteFarm.xml" $9
  ${If} $9 == 0
    ${xml::GotoPath} "QuoteFarm/�豸���/�豸������IP" $9
    ${xml::SetText} "127.0.0.1" $9
    ${xml::GotoPath} "" $9
    ${xml::GotoPath} "QuoteFarm/�豸���/�豸�������˿�" $9
    ${xml::SetText} $R9 $9
    ${xml::SaveFile} "" $9
    ${xml::Unload}
  ${EndIf}

  ${xml::LoadFile} "QuoteFeed.xml" $9
  ${If} $9 == 0
    ${xml::GotoPath} "QuoteFeed/�豸���/�豸������IP" $9
    ${xml::SetText} "127.0.0.1" $9
    ${xml::GotoPath} "" $9
    ${xml::GotoPath} "QuoteFeed/�豸���/�豸�������˿�" $9
    ${xml::SetText} $R9 $9
    ${xml::SaveFile} "" $9
    ${xml::Unload}
  ${EndIf}

  ${xml::LoadFile} "QuoteServer.xml" $9
  ${If} $9 == 0
    ${xml::GotoPath} "QuoteServer/�豸���/�豸������IP" $9
    ${xml::SetText} "127.0.0.1" $9
    ${xml::GotoPath} "" $9
    ${xml::GotoPath} "QuoteServer/�豸���/�豸�������˿�" $9
    ${xml::SetText} $R9 $9
    ${xml::SaveFile} "" $9
    ${xml::Unload}
  ${EndIf}

  IntCmp $3 1 0 +2
    ExecWait 'cmd.exe /c $\"$INSTDIR\WatchDog.exe -o$\"'
FunctionEnd

;Function un.onInit
;  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "uninstall_confirm.ini"
;FunctionEnd

Function .onSelChange
; QuoteFeed����������뱻ѡ�񣬷���������������ܰ�װ
  SectionGetFlags ${SEC_FEED_HS} $0
  SectionGetFlags ${SEC_FEED_HK} $1
  IntOp $0 $0 & ${SF_SELECTED}
  IntOp $1 $1 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} 0 +2
  SectionSetFlags ${SEC_FEED_MAIN} 1
  IntCmp $1 ${SF_SELECTED} 0 +2
  SectionSetFlags ${SEC_FEED_MAIN} 1
  
; QuoteFarm����������밲װ���������в�������ܰ�װ
  SectionGetFlags ${SEC_MATCH_ADDIN} $0
  IntOp $0 $0 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} 0 +2
  SectionSetFlags ${SEC_FARM_MAIN} 1
  
; QuoteServer����������밲װ���������в�������ܰ�װ
  SectionGetFlags ${SEC_TIANMA_ADDIN} $0
  IntOp $0 $0 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} 0 +2
  SectionSetFlags ${SEC_SERVER_MAIN} 1
FunctionEnd

