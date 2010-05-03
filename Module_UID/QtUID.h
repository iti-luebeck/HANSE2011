/**
 *  @file QtUID.h
 *
 *  @author Dipl-Inf. Christoph Osterloh
 *  @date 10.09.2009
 *
 *  @version 1.0
 *  Methoden hinzugefügt.
 *  @version 1.1
 *  SPI-Methoden hinzugefügt.
 *  @version 1.2
 *  Kommentare hinzugefügt (Doxygen).
 */

#ifndef QTUID_H
#define QTUID_H

#include <string>
#include <QtCore/QObject>
#include <QTime>
#include <QVector>
#include <QString>
#include <QtGlobal>
#include <QDebug>
#include "Module_UID_global.h"

class PortSettings;
class QextSerialPort;

typedef unsigned char byte;

class MODULE_UIDSHARED_EXPORT UID
{
public:

    /**
     *	Enumerator für die I2C Geschwindigkeiten
     */
    enum I2CSpeed
    {
        kHz100 = 0xCC,
        kHz200 = 0xF0,
        khz400 = 0xFF
    };

    /** \brief Konstruktor
     *
     *
     *
     *	@param[in] (QString Id) ID des zu suchenden UIDs
     */
    UID(QString Id);

    /**
     *	Schließt den Port
     */
    void ClosePort();

    /** \brief  UID Available
     *
     *  Überprüft ob UID vorhanden
     *
     *  @return                     TRUE, wenn UID gefunden wurde.
     */
    bool UID_Available();

    /** \brief  Idendity
     *
     *  Fragt die Identifikationskennung des UIDs ab
     *
     *  @return                     Name des UIDs.
     */
    QString UID_Identify();

    /** \brief  Revision
     *
     *  Fragt die Revisionsnummer des UID ab
     *
     *  @return                     Revisionsnummer des UID.
     */
    QString UID_Revision();

    /** \brief Startet I2C Acknowledge Modus
     *
     *  Für Slaves, die nach jedem Byte ein Acknowledge brauchen
     *  @return                     TRUE wenn Ausführung erfolgreich
     */
    bool I2C_EnterAckMode();

    /** \brief Beendet I2C Acknowledge Modus
     *
     *  Für Slaves, die nicht nach jedem Byte ein Acknowledge brauchen
     *  @return                     TRUE wenn Ausführung erfolgreich
     */
    bool I2C_LeaveAckMode();

    /** \brief I2C-Scan Methode
     *
     *  Scannt nach angeschlossenen I2C-Slaves
     *
     *  @return                     Liste der gefundenen Slaves
     */
    QVector<unsigned char> I2C_Scan();

    /** \brief I2C-Speed Methode
     *
     *  Setzt die Geschwindigkeit für die I2C-Kommunikation (Standard: 200Khz)
     *
     *  @param[in]      speed       Geschwindigkeit (100khz, 200khz, 400khz)
     *  @return                     TRUE wenn Ausführung erfolgreich
     */
    bool I2C_Speed(I2CSpeed speed);

    /** \brief I2C-Read Methode
     *
     *  Liest Daten von einem Slave
     *
     *	@param[in]      address     Die Adresse des Slaves
     *	@param[in]      byteCount   Anzahl Bytes der zu lesenden Daten
     *  @param[in]      result      Pointer auf das Zielarray der gelesenen Daten
     *  @return                     TRUE wenn Lesen erfolgreich
     */
    bool I2C_Read(unsigned char address, short byteCount, unsigned char* result);

    /** \brief I2C-Read Register Methode
     *
     *  Liest Daten aus einem Register von einem Slave
     *
     *	@param[in]      address     Die Adresse des Slaves
     *  @param[in]      reg         Nummer des zu lesenden Registers
     *	@param[in]      byteCount   Anzahl Bytes der zu lesenden Daten
     *  @param[in]  result      Pointer auf das Zielarray der gelesenen Daten
     *  @return                     TRUE wenn Lesen erfolgreich
     */
    bool I2C_ReadRegisters(unsigned char address, unsigned char reg, short byteCount, unsigned char* result);

    /** \brief SRF08 Methode
     *
     *  Spezielle Abfragemethode für den SRF08 Ultraschallsensor die es erlaubt zu überprüfen,
     *  ob eine gestartete Messung beendet wurde.
     *
     *	@param[in]      address     Die Adresse des SRF08
     *  @return                     TRUE wenn SRF08 fertig mit Messung
     */
    bool I2C_TestSRF08Ready(unsigned char address);

    /** \brief I2C-Write Methode
     *
     *  Sendet Daten an einen Slave
     *
     *	@param[in]      address     Die Adresse des Slaves
     *  @param[in]      data        Array der zu schreibenden Daten
     *	@param[in]      byteCount   Anzahl Bytes der zu schreibenden Daten
     *  @return                     TRUE wenn Schreiben erfolgreich
     */
    bool I2C_Write(unsigned char address, unsigned char* data, short byteCount);

    /** \brief I2C-Write Register Methode
     *
     *  Sendet Daten an Register eines Slaves
     *
     *	@param[in]      address     Die Adresse des Slaves
     *  @param[in]      reg         Zielregister
     *  @param[in]      data        Array der zu schreibenden Daten
     *	@param[in]      byteCount   Anzahl Bytes der zu schreibenden Daten
     *  @return                     TRUE wenn Schreiben erfolgreich
     */
    bool I2C_WriteRegister(unsigned char address, unsigned char reg, unsigned char* data, short byteCount);

    
    
    
    /** \brief Direct Access I2C-Start Methode
     *
     *  Führt einen Start aus.
     *  @return                     TRUE wenn Start erfolgreich
     */
    bool I2C_DA_Start();
    
    /** \brief Direct Access I2C-Stop Methode
     *
     *  Führt einen Stop aus.
     *  @return                     TRUE wenn Stop erfolgreich
     */
    bool I2C_DA_Stop();
    
    /** \brief Direct Access I2C-Repeat Start Methode
     *
     *  Führt einen Repeat Start aus.
     *  @return                     TRUE wenn RepStart erfolgreich
     */
    bool I2C_DA_RepStart();
   
    /** \brief Direct Access I2C-Send Ack Methode
     *
     *  Empfängt ein Acknowledge vom Slave
     *  @return                     TRUE wenn Acknowledge erfolgreich empfangen
     */
    bool I2C_DA_SendAck();
    /** \brief Direct Access I2C-Send Ack Methode
     *
     *  Sendet ein Acknowledge an den Slave
     *  @return                     TRUE wenn Acknowledge erfolgreich gesendet
     */
    bool I2C_DA_ReceiveAck();
    
    /** \brief Direct Access I2C-Receive Byte Ack Methode
     *
     *  Empfängt ein Byte mit Acknowledge vom Slave
     *  @return                     TRUE wenn Byte und Acknowledge erfolgreich empfangen
     */
    bool I2C_DA_ReceiveByteAck();
    
    /** \brief Direct Access I2C-Receive Byte Methode
     *
     *  Empfängt ein Byte ohne Acknowledge vom Slave
     *  @return                     TRUE wenn Byte erfolgreich empfangen
     */
    bool I2C_DA_ReceiveByteNoAck();
    
    /** \brief Direct Access I2C-Send Byte Ack Methode
     *
     *  Sendet ein Byte mit Acknowledge an Slave
     *  @return                     TRUE wenn Byte und Acknowledge erfolgreich gesendet
     */
    bool I2C_DA_SendByteAck(unsigned char data);

    /** \brief Direct Access I2C-Send Byte Methode
     *
     *  Sendet ein Byte ohne Acknowledge an Slave
     *  @return                     TRUE wenn Byte erfolgreich gesendet
     */
    bool I2C_DA_SendByteNoAck(unsigned char data);


    /** \brief ADC Methode
     *
     *  Liest den AD-Wandler des ATmega aus. Der Kanal, an dessen Stelle in der Bitmaske eine 1 steht wird ausgelesen.
     *  Mehrere Kanäle können "gleichzeitig" ausgelesen werden.
     *
     *  @param[in]      bitmask     Die Bitmaske für den AD-Wandler
     *  @param[in]      result      Pointer auf das Ergebnisarray
     *  @return                     TRUE wenn Ausführung erfolgreich
     */
    bool UID_ADC(unsigned char bitmask, unsigned char* result);

    /** \brief SPI-Speed Methode
     *
     *  Setzt die Geschwindigkeit für die SPI-Kommunikation
     *
     *  @param[in]      speed       1=clk/2<br>
     *                              2=clk/4<br>
     *                              3=clk/8<br>
     *                              4=clk/16<br>
     *                              5=clk/32<br>
     *                              6=clk/64<br>
     *                              7=clk/128<br>
     *  @return                     TRUE wenn Ausführung erfolgreich
     */
    bool SPI_Speed(unsigned char speed);

    /** \brief SPI-SetPHA Methode
     *
     *  Setzt die Phase für die SPI-Kommunikation
     *
     *  @param[in]      phase
     *  <img src="SPI.png" alt="SPI">
     *  @return                     TRUE wenn Ausführung erfolgreich
     */
    bool SPI_SetPHA(bool phase);

    /** \brief SPI-SetPol Methode
     *
     *  Setzt die Polarität für die SPI-Kommunikation
     *
     *  @param[in]      pol
     *  <img src="SPI.png" alt="SPI">
     *  @return                     TRUE wenn Ausführung erfolgreich
     */
    bool SPI_SetPOL(bool pol);

    /** \brief SPI-Read Methode
     *
     *  Liest Daten von einem SPI Slave
     *
     *	@param[in]      address     Die Adresse des Slaves
     *	@param[in]      byteCount   Anzahl Bytes der zu lesenden Daten
     *  @param[in]      result      Pointer auf das Zielarray der gelesenen Daten
     *  @return                     TRUE wenn Lesen erfolgreich
     */
    bool SPI_Read(unsigned char address, short byteCount, unsigned char* result);

    /** \brief SPI-Write Register Methode
     *
     *  Schreibt Daten an einen SPI Slave
     *
     *	@param[in]      address     Die Adresse des Slaves
     *  @param[in]      data        Array der zu schreibenden Daten
     *	@param[in]      byteCount   Anzahl Bytes der zu schreibenden Daten
     *  @return                     TRUE wenn Schreiben erfolgreich
     */
    bool SPI_Write(unsigned char address, unsigned char* data, short byteCount);

    /** \brief SPI-WriteRead Register Methode
     *
     *  Schreibt Daten an einen SPI Slave und liest gleichzeitig Ergebnis
     *
     *	@param[in]      address     Die Adresse des Slaves
     *  @param[in]      data        Array der zu schreibenden Daten
     *	@param[in]      byteCount   Anzahl Bytes der zu schreibenden Daten
     *  @param[in]      result      Pointer auf das Zielarray der gelesenen Daten
     *  @return                     TRUE wenn Schreiben erfolgreich
     */
    bool SPI_WriteRead(unsigned char address, unsigned char* data, short byteCount, unsigned char* result);

private:

    enum Opcode
    {
        ADC_ADC = 0x00,

        UID_IDENTIFY = 0x05,
        UID_REVISION = 0x0C,

        I2C_ENTERACKMODE = 0x03,
        I2C_LEAVEACKMODE = 0x06,
        I2C_READ = 0x09,
        I2C_READREGISTER = 0x0A,
        I2C_SCAN = 0x0F,
        I2C_SPEED = 0x11,
        I2C_TESTSRF08READY = 0x2E,
        I2C_WRITE = 0x18,
        I2C_WRITEREGISTER = 0x1B,
        I2C_DA_RECEIVEACK = 0x1D,
        I2C_DA_RECEIVEBYTEACK = 0x1E,
        I2C_DA_RECEIVEBYTENOACK = 0x21,
        I2C_DA_REPSTART = 0x22,
        I2C_DA_SENDACK = 0x24,
        I2C_DA_SENDBYTEACK = 0x27,
        I2C_DA_SENDBYTENOACK = 0x28,
        I2C_DA_START = 0x2B,
        I2C_DA_STOP = 0x2D,

        SPI_SPEED = 0x14,
        SPI_SETPHA = 0x62,
        SPI_SETPOL = 0x64,
        SPI_READ = 0x12,
        SPI_WRITE = 0x17,
        SPI_READWRITE = 0x6A
    };

    QextSerialPort* port;
    PortSettings* portSettings;

    QextSerialPort* ScanForUIDs( QString Id );
    QString IdentifyCommand(unsigned char* sequence);
    bool SendCommand(unsigned char* sequence, unsigned char length, int msec);
    bool WaitForSerialData( short amount );
    unsigned char countBitsSet( unsigned char bitmask );

    bool UID_available;

};


#endif // QTUID_H
