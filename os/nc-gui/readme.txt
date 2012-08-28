Here are the prerequisites for compiling the statics gui on windows: 
- Visual Studio 10
- qt-vs-addin-1.1.11-opensource.exe
- QtSdk-Online-win-x86-v1_2_1.exe
- qt-win-opensource-4.8.2-vs2010.exe

Visual Studio opens with an annoing error message about a missing qt version....
This can be fixed by selecting Qt->Qt Project Settings and selecting version 4.8.1

Here are the prerequisites for compiling the the statistics gui on X11 ubuntu: 
http://www.wikihow.com/Install-Qt-SDK-on-Ubuntu-Linux
svn co https://qwt.svn.sourceforge.net/svnroot/qwt/branches/qwt-6.0 
export LD_LIBRARY_PATH=/usr/local/qwt-6.0.2-svns
