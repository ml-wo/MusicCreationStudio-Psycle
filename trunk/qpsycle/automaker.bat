@REM Created By Graity_0... Released Under GPL Licence
@echo off
title Automaker QT 1.0 - Windows
COLOR 0%3
echo Welcome to Gravity_0 Automaker for QT
echo Qt is a registered trademaerk of Trolltech
pause
echo Running QMake
qmake
echo QMake Complete
if "%1" == "debug" goto DEBUG
make release
echo Make Complete

:DEBUG
make debug