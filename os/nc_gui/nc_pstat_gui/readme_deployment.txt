X11: 

export LD_PRELOAD=$PWD/release_nc/libqwt.so.6:$PWD/release_nc/libQtGui.so.4:$PWD/release_nc/libQtCore.so.4:$PWD/release_nc/libQtSvg.so.4 
./nc_pstats_gui

Windows: 
cd release_nc_win
nc_pstats_gui