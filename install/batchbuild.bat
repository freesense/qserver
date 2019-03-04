del /q /f batchbuild.log

devenv ../commx/commx.sln /build debug /out batchbuild.log
devenv ../commx/commx.sln /build release /out batchbuild.log

copy ..\commx\public\*.h ..\public\commx /y
copy ..\bind\commxd.lib ..\public\lib /y
copy ..\binr\commxr.lib ..\public\lib /y
copy ..\bind\commxd.dll ..\public\lib /y
copy ..\binr\commxr.dll ..\public\lib /y

devenv ../KLine_Exe/KLine_Exe.sln /build debug /out batchbuild.log
devenv ../KLine_Exe/KLine_Exe.sln /build release /out batchbuild.log

devenv ../qpx/qpx.sln /build debug /out batchbuild.log
devenv ../qpx/qpx.sln /build release /out batchbuild.log

devenv ../QuoteFarm/QuoteFarm.sln /build debug /out batchbuild.log
devenv ../QuoteFarm/QuoteFarm.sln /build release /out batchbuild.log

devenv ../QuoteFeed/QuoteFeed.sln /build debug /out batchbuild.log
devenv ../QuoteFeed/QuoteFeed.sln /build release /out batchbuild.log

devenv ../QuoteServer/QuoteServer.sln /build debug /out batchbuild.log
devenv ../QuoteServer/QuoteServer.sln /build release /out batchbuild.log

devenv ../TianmaData/TianmaData.sln /build debug /out batchbuild.log
devenv ../TianmaData/TianmaData.sln /build release /out batchbuild.log

devenv ../tradematch/tradematch.sln /build debug /out batchbuild.log
devenv ../tradematch/tradematch.sln /build release /out batchbuild.log

devenv ../WatchDog/WatchDog.sln /build debug /out batchbuild.log
devenv ../WatchDog/WatchDog.sln /build release /out batchbuild.log
