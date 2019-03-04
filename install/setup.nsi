; 该脚本使用 HM VNISEdit 脚本编辑器向导产生
; $R9 保存看门狗服务端口，供其他服务修改配置使用
; 看门狗服务地址固定使用127.0.0.1

Var SHOW_INST_WATCHDOG              ; 是否显示看门狗配置页面
Var SHOW_INST_FEED                  ; 是否显示Feed配置页面

; 安装程序初始定义常量
!define PRODUCT_NAME "今日行情服务"
!define PRODUCT_VERSION "2.0"
!define PRODUCT_PUBLISHER "深圳市今日投资财经资讯有限公司"
!define PRODUCT_WEB_SITE "http://www.investoday.com.cn"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\QuoteConsole.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

SetCompressor lzma

BrandingText "深圳市今日投资财经资讯有限公司"
SetFont "Tahoma" 8

ReserveFile "${NSISDIR}\Plugins\InstallOptions.dll"
ReserveFile "install_watchdog.ini"
ReserveFile "install_feed.ini"

; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
!include "MUI.nsh"
!include "FileFunc.nsh"
!include "InstallOptions.nsh"
!include "xml.nsh"
!include "LogicLib.nsh"

; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; 区段描述在下方显示。
!define MUI_COMPONENTSPAGE_SMALLDESC

; 欢迎页面
!insertmacro MUI_PAGE_WELCOME
; 许可协议页面
;!insertmacro MUI_PAGE_LICENSE "licence.txt"

!define MUI_COMPONENTSPAGE_TEXT_TOP "QuoteFeed可以单独安装在一台服务器上，QuoteFarm和\
QuoteServer安装在另一台服务器上。多组QuoteFarm和QuoteServer可以共享同一台QuoteFeed服务器。"
!define MUI_COMPONENTSPAGE_TEXT_COMPLIST "请根据部署计划仔细选择要部署的模块和扩展插件"

; 组件选择页面
!insertmacro MUI_PAGE_COMPONENTS
; 安装目录选择页面
!insertmacro MUI_PAGE_DIRECTORY
; 安装过程页面
!insertmacro MUI_PAGE_INSTFILES

; 配置看门狗
Page custom install_watchdog_page validate_install_watchdog
Page custom install_feed_page validate_install_feed

!define MUI_FINISHPAGE_LINK_LOCATION "http://www.investoday.com.cn"
!define MUI_FINISHPAGE_LINK "访问今日投资首页"

!define MUI_FINISHPAGE_RUN_TEXT "运行行情综控台，下载最新的K线文件"
!define MUI_FINISHPAGE_RUN "$INSTDIR\QuoteConsole"

!define MUI_FINISHPAGE_SHOWREADME_TEXT "阅读运维手册"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\QuoteServer通讯协议.doc"

!define MUI_FINISHPAGE_TEXT "您已完成安装“${PRODUCT_NAME} ${PRODUCT_VERSION}”，建议您在启动服务前首先仔细阅读运维手册，修改各个服务的配置文件。"

; 安装完成页面
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME

!define MUI_UNCONFIRMPAGE_TEXT_TOP "卸载程序将强行结束进程，删除相关服务，删除安装目录下除配置外的所有文件。$\r$\n$\r$\n$_CLICK"

!insertmacro MUI_UNPAGE_CONFIRM

; 安装卸载过程页面
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"

; 安装预释放文件
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; ------ MUI 现代界面定义结束 ------

!include "Sections.nsh"
!include "LogicLib.nsh"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "quotesetup.exe"
InstallDir "D:\jrtzhq2\run"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
ShowInstDetails show
ShowUnInstDetails show

Section "基础支持文件" SECACQUIRE
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
  File "..\doc\今日投资行情系统运维手册.doc"
  CreateDirectory "$SMPROGRAMS\今日行情服务"
  CreateShortCut "$SMPROGRAMS\今日行情服务\运维手册.lnk" "$INSTDIR\今日投资行情系统运维手册.doc"
SectionEnd

SectionGroup "QuoteFeed" SEC01
  Section "主程序" SEC_FEED_MAIN
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    StrCpy $SHOW_INST_FEED "show"
    File "..\binr\QuoteFeed.exe"
    File "..\binr\QuoteFeed.xml"
;    ExecWait 'cmd.exe /c "sc create QuoteFeed binpath= $\"$INSTDIR\QuoteFeed.exe$\" displayname= 今日投资[QuoteFeed]"'
  SectionEnd
  
  Section "沪深股市" SEC_FEED_HS
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\SHFeed.dll"
  SectionEnd
  
  Section "港股延时行情" SEC_FEED_HK
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\HKFeed.dll"
  SectionEnd
SectionGroupEnd

SectionGroup "QuoteFarm" SEC02
  Section "主程序" SEC_FARM_MAIN
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\QuoteFarm.exe"
    File "..\binr\QuoteFarm.xml"
    File "..\binr\holiday.txt"
    ExecWait 'cmd.exe /c "sc create QuoteFarm binpath= $\"$INSTDIR\QuoteFarm.exe$\" displayname= 今日投资[QuoteFarm] start= auto"'
  SectionEnd
  
  Section "交易撮合插件" SEC_MATCH_ADDIN
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\tradematch.dll"
    File "..\binr\tradematch.xml"
  SectionEnd
SectionGroupEnd

SectionGroup "QuoteServer" SEC03
  Section "主程序" SEC_SERVER_MAIN
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\QuoteServer.exe"
    File "..\binr\QuoteServer.xml"
    ExecWait 'cmd.exe /c "sc create QuoteServer binpath= $\"$INSTDIR\QuoteServer.exe$\" displayname= 今日投资[QuoteServer] start= auto"'
  SectionEnd
  
  Section "基础业务扩展插件" SEC_QEXTEND
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\qextend.dll"
    File "..\binr\ntwdblib.dll"
    File "..\binr\qextend.xml"
  SectionEnd
  
  Section "天马数据插件" SEC_TIANMA_ADDIN
    SetOutPath "$INSTDIR"
    SetOverwrite ifnewer
    File "..\binr\TianmaData.dll"
    File "..\binr\TianmaData.xml"
  SectionEnd
SectionGroupEnd

Section "行情综控台" SEC04
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
  
  CreateShortCut "$SMPROGRAMS\今日行情服务\行情综控台.lnk" "$INSTDIR\QuoteConsole.exe"
  CreateShortCut "$DESKTOP\行情综控台.lnk" "$INSTDIR\QuoteConsole.exe"
  CreateShortCut "$SMSTARTUP\行情综控台.lnk" "$INSTDIR\QuoteConsole.exe"
  
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\QuoteConsole.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\QuoteConsole.exe"
SectionEnd

Section "监控服务组件" SEC05
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  StrCpy $SHOW_INST_WATCHDOG "show"
  File "..\binr\WatchDog.exe"
  File "..\binr\WatchDog.xml"
  ExecWait 'cmd.exe /c "sc create WatchDog binpath= $\"$INSTDIR\WatchDog.exe$\" displayname= 今日投资[Daemon] start= auto"'
  ExecWait 'cmd.exe /c "sc config WatchDog type= interact type= own"'
SectionEnd

Section /o "协议转换服务" SEC06
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "..\binr\qpx.exe"
  File "..\binr\qpx.xml"
  ExecWait 'cmd.exe /c "sc create qpx binpath= $\"$INSTDIR\qpx.exe$\" displayname= 今日投资[协议转换] start= auto"'
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\今日行情服务\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\今日行情服务\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

#-- 根据 NSIS 脚本编辑规则，所有 Function 区段必须放置在 Section 区段之后编写，以避免安装程序出现未可预知的问题。--#

; 区段组件描述
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SECACQUIRE} "必须安装的基础支持文件。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "为行情系统提供行情数据转换服务。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_FEED_MAIN} "必须安装的数据转换主程序。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_FEED_HK} "提供港股数据转换。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_FEED_HS} "提供沪深股市数据转换。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "实时行情内存数据库，封装市场之间的差异。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_FARM_MAIN} "必须安装，提供统一的行情数据接口。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_MATCH_ADDIN} "提供模拟交易撮合功能，依赖于Mssql数据库。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} "为用户提供外部数据访问服务。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_SERVER_MAIN} "必须安装，该服务器通过数据访问接口提供统一的对外服务。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_TIANMA_ADDIN} "天马投资数据业务插件，依赖于Mssql数据库。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC_QEXTEND} "提供排行榜等行情扩展业务，部分业务需要Mssql数据库支持。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} "提供K线数据维护，行情服务实时监控等功能，建议在每台服务器上都安装。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC05} "如果要将本机纳入行情综控台的监控范围之内，必须安装此组件，建议在每台服务器上都安装。"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC06} "提供今日行情1.0和2.0之间的协议转换服务。"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

/******************************
 *  以下是安装程序的卸载部分  *
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
  
  Delete "$SMPROGRAMS\今日行情服务\Uninstall.lnk"
  Delete "$SMPROGRAMS\今日行情服务\Website.lnk"
  Delete "$DESKTOP\行情综控台.lnk"
  Delete "$SMPROGRAMS\今日行情服务\行情综控台.lnk"
  Delete "$SMSTARTUP\行情综控台.lnk"
  Delete "$SMPROGRAMS\今日行情服务\运维手册.lnk"

  RMDir "$SMPROGRAMS\今日行情服务"

  SetOutPath $TEMP
  
; 保留所有配置文件和子目录，以免丢失数据，重新安装时要重新配置
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

#-- 根据 NSIS 脚本编辑规则，所有 Function 区段必须放置在 Section 区段之后编写，以避免安装程序出现未可预知的问题。--#

Function .onInit
  System::Call 'kernel32::CreateMutexA(i 0, i 0, t "itfq_install") i .r1 ?e'
  Pop $R0

  StrCmp $R0 0 +3
  MessageBox MB_OK|MB_ICONEXCLAMATION "安装程序已经在运行。"
  Abort
  
  StrCpy $SHOW_INST_WATCHDOG "no"
  StrCpy $SHOW_INST_FEED "no"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "install_watchdog.ini"
  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "install_feed.ini"
FunctionEnd

LangString INSTALL_FEED_TITLE ${LANG_SIMPCHINESE} "QuoteFeed配置"
LangString INSTALL_FEED_SUBTITLE ${LANG_SIMPCHINESE} "配置QuoteFeed的服务端口，以及行情数据源属性"

Function install_feed_page
  StrCmp $SHOW_INST_FEED "show" 0 noshow
    ${xml::LoadFile} "quotefeed.xml" $9
    ${xml::GotoPath} "QuoteFeed/行情源[1]" $9
    ${xml::GetAttribute} "命名" $1 $9
    !insertmacro MUI_INSTALLOPTIONS_WRITE "install_feed.ini" "Field 7" "Text" $1
    ${xml::GotoPath} "" $9
    ${xml::GotoPath} "QuoteFeed/行情源[2]" $9
    ${xml::GetAttribute} "命名" $1 $9
    !insertmacro MUI_INSTALLOPTIONS_WRITE "install_feed.ini" "Field 3" "Text" $1
    ${xml::Unload}
    
    !insertmacro MUI_HEADER_TEXT "$(INSTALL_FEED_TITLE)" "$(INSTALL_FEED_SUBTITLE)"
    !insertmacro INSTALLOPTIONS_DISPLAY "install_feed.ini"
  noshow:
FunctionEnd

Function validate_install_feed
  ${xml::LoadFile} "quotefeed.xml" $9

  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 2" "State"
  ${xml::GotoPath} "QuoteFeed/公共/服务地址" $9
  ${xml::SetText} $0 $9
  ${xml::GotoPath} "" $9
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 8" "State"
  ${xml::GotoPath} "QuoteFeed/行情源[1]" $9
  ${If} $0 == 0
    ${xml::SetAttribute} "状态" "暂停" $9
  ${Else}
    ${xml::SetAttribute} "状态" "正常" $9
  ${EndIf}
  ${xml::GotoPath} "" $9
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 4" "State"
  ${xml::GotoPath} "QuoteFeed/行情源[2]" $9
  ${If} $0 == 0
    ${xml::SetAttribute} "状态" "暂停" $9
  ${Else}
    ${xml::SetAttribute} "状态" "正常" $9
  ${EndIf}
  ${xml::GotoPath} "" $9
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 6" "State"
  ${xml::GotoPath} "QuoteFeed/行情源[2]/Params[1]" $9
  ${xml::SetText} $0 $9
  ${xml::GotoPath} "" $9
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 10" "State"
  ${xml::GotoPath} "QuoteFeed/行情源[1]/Params[1]" $9
  ${xml::SetText} $0 $9
  ${xml::GotoPath} "" $9
  
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_feed.ini" "Field 12" "State"
  ${xml::GotoPath} "QuoteFeed/行情源[1]/Params[2]" $9
  ${xml::SetText} $0 $9
  ${xml::GotoPath} "" $9
  
  ${xml::GotoPath} "QuoteFeed/设备监控/设备服务器端口" $9
  ${xml::SetText} $R9 $9

  ${xml::SaveFile} "" $9
  ${xml::Unload}
FunctionEnd

LangString INSTALL_WATCHDOG_TITLE ${LANG_SIMPCHINESE} "精灵服务配置"
LangString INSTALL_WATCHDOG_SUBTITLE ${LANG_SIMPCHINESE} "配置设备服务器的服务端口，以及监控中心的服务地址和端口"

Function install_watchdog_page
  StrCmp $SHOW_INST_WATCHDOG "show" 0 noshow
    !insertmacro MUI_HEADER_TEXT "$(INSTALL_WATCHDOG_TITLE)" "$(INSTALL_WATCHDOG_SUBTITLE)"
    !insertmacro INSTALLOPTIONS_DISPLAY "install_watchdog.ini"
  noshow:
FunctionEnd

Function validate_install_watchdog
  !insertmacro MUI_INSTALLOPTIONS_READ $0 "install_watchdog.ini" "Field 2" "State"
  StrCmp $0 "" +1 +3
    MessageBox MB_OK "监控中心地址不能为空"
    Abort
  !insertmacro MUI_INSTALLOPTIONS_READ $1 "install_watchdog.ini" "Field 4" "State"
  StrCmp $1 "" +1 +3
    MessageBox MB_OK "监控中心端口不能为空"
    Abort
  !insertmacro MUI_INSTALLOPTIONS_READ $R9 "install_watchdog.ini" "Field 6" "State"
  StrCmp $R9 "" +1 +3
    MessageBox MB_OK "精灵服务端口不能为空"
    Abort
  !insertmacro MUI_INSTALLOPTIONS_READ $3 "install_watchdog.ini" "Field 7" "State"
  
  ${xml::LoadFile} "watchdog.xml" $9
  ${xml::GotoPath} "WatchDog/控制中心地址" $9
  ${xml::SetText} $0 $9
  ${xml::GotoPath} "" $9
  ${xml::GotoPath} "WatchDog/控制中心端口" $9
  ${xml::SetText} $1 $9
  ${xml::GotoPath} "" $9
  ${xml::GotoPath} "WatchDog/服务地址" $9
  ${xml::SetText} $R9 $9
  ${xml::SaveFile} "" $9
  ${xml::Unload}
  
  ${xml::LoadFile} "qpx.xml" $9
  ${If} $9 == 0
    ${xml::GotoPath} "qpx/设备监控/设备服务器IP" $9
    ${xml::SetText} "127.0.0.1" $9
    ${xml::GotoPath} "" $9
    ${xml::GotoPath} "qpx/设备监控/设备服务器端口" $9
    ${xml::SetText} $R9 $9
    ${xml::SaveFile} "" $9
    ${xml::Unload}
  ${EndIf}

  ${xml::LoadFile} "QuoteFarm.xml" $9
  ${If} $9 == 0
    ${xml::GotoPath} "QuoteFarm/设备监控/设备服务器IP" $9
    ${xml::SetText} "127.0.0.1" $9
    ${xml::GotoPath} "" $9
    ${xml::GotoPath} "QuoteFarm/设备监控/设备服务器端口" $9
    ${xml::SetText} $R9 $9
    ${xml::SaveFile} "" $9
    ${xml::Unload}
  ${EndIf}

  ${xml::LoadFile} "QuoteFeed.xml" $9
  ${If} $9 == 0
    ${xml::GotoPath} "QuoteFeed/设备监控/设备服务器IP" $9
    ${xml::SetText} "127.0.0.1" $9
    ${xml::GotoPath} "" $9
    ${xml::GotoPath} "QuoteFeed/设备监控/设备服务器端口" $9
    ${xml::SetText} $R9 $9
    ${xml::SaveFile} "" $9
    ${xml::Unload}
  ${EndIf}

  ${xml::LoadFile} "QuoteServer.xml" $9
  ${If} $9 == 0
    ${xml::GotoPath} "QuoteServer/设备监控/设备服务器IP" $9
    ${xml::SetText} "127.0.0.1" $9
    ${xml::GotoPath} "" $9
    ${xml::GotoPath} "QuoteServer/设备监控/设备服务器端口" $9
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
; QuoteFeed的主程序必须被选择，否则所有组件都不能安装
  SectionGetFlags ${SEC_FEED_HS} $0
  SectionGetFlags ${SEC_FEED_HK} $1
  IntOp $0 $0 & ${SF_SELECTED}
  IntOp $1 $1 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} 0 +2
  SectionSetFlags ${SEC_FEED_MAIN} 1
  IntCmp $1 ${SF_SELECTED} 0 +2
  SectionSetFlags ${SEC_FEED_MAIN} 1
  
; QuoteFarm的主程序必须安装，否则所有插件都不能安装
  SectionGetFlags ${SEC_MATCH_ADDIN} $0
  IntOp $0 $0 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} 0 +2
  SectionSetFlags ${SEC_FARM_MAIN} 1
  
; QuoteServer的主程序必须安装，否则所有插件都不能安装
  SectionGetFlags ${SEC_TIANMA_ADDIN} $0
  IntOp $0 $0 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} 0 +2
  SectionSetFlags ${SEC_SERVER_MAIN} 1
FunctionEnd

