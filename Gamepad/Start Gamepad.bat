@echo off
rem
rem This file is generated
rem
echo Setting up a MinGW/Qt only environment...
echo -- QTDIR set to C:\Qt\2010.03\qt
echo -- PATH set to C:\Qt\2010.03\qt\bin
echo -- Adding C:\Qt\2010.03\bin to PATH
echo -- Adding %SystemRoot%\System32 to PATH
echo -- QMAKESPEC set to win32-g++
set QTDIR=C:\Qt\2010.03\qt
set PATH=C:\Qt\2010.03\qt\bin
set PATH=%PATH%;C:\Qt\2010.03\bin;C:\Qt\2010.03\mingw\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++

C:
cd c:\Hanse\build

gamepad
