
Framework und Module:
---------------------
Framework			Shared code für alle Module
HanseGUI			GUI, lädt alle Module. Erstellt die EXE
Module_ScanningSonar		Modul für das Scanning Sonar (nutzt qextserialport)
Module_ScanningSonar		Modul für das UID (ITI-BUs) (nutzt qextserialport)
#Module_...			Modul für ...
#Module_...			Modul für ...

Externe Libs:
-------------
qextserialport			SerialPort Library
#opencv				TODO

Test-Apps:
----------
Test_UID			Test-Proggie für das UID
