#ifndef MODULE_UID_H
#define MODULE_UID_H

#include <Framework/robotmodule_mt.h>

class QextSerialPort;
class QextPortInfo;
class PortSettings;

class Module_UID : public RobotModule_MT {
    Q_OBJECT
public:

    Module_UID(QString moduleId);
    Module_UID(QString moduleId, QString deviceId);
    ~Module_UID();

    // inherited from RobotModule
    QWidget* createView(QWidget* parent);
    // inherited from RobotModule
    QList<RobotModule*> getDependencies();

    enum Errorcode
    {
        E_NO_ERROR = 0xF000,

        // I2C ist auf low gezogen
        E_I2C_LOW = 0x01,

        // Konnte kein Start senden
        E_I2C_START = 0x02,

        // Slave antwortet nicht auf seine Schreibadresse
        E_I2C_MT_SLA_ACK = 0x04,

        // Slave antwortet nicht auf seine Leseadresse
        E_I2C_MR_SLA_ACK = 0x08,

        // Kein Acknowledge vom Slave auf an ihn gesendete Daten
        E_I2C_MT_DATA_ACK = 0x10,

        // Kein Acknowledge vom Slave beim Empfang von Daten
        E_I2C_MR_DATA_ACK = 0x20,

        // Erwartet kein Ack, bekommt aber eins
        E_I2C_MR_DATA_NACK = 0x40,

        // Serielle Schnittstelle "ausgetimed"...
        E_UART_TIMEOUT = 0x80,

        // read less bytes than expected
        E_SHORT_READ = 0x0100,

        // read too much bytes
        E_EXTRA_READ = 0x0200,

        // got an error while reading from serial port
        E_USB = 0x0400,

        // the UID is not responding ???
        //E_UID = 0x0800,
    };

    /**
     *	Enumerator fuer die I2C Geschwindigkeiten
     */
    enum I2CSpeed
    {
        kHz100 = 0xCC,
        kHz200 = 0xF0,
        khz400 = 0xFF
    };


public slots:
    /** \brief  UID Available
     *
     *  Ueberprueft ob UID vorhanden
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

    /** \brief I2C-Scan Methode
     *
     *  Scannt nach angeschlossenen I2C-Slaves
     *
     *  @return                     Liste der gefundenen Slaves
     */
    QVector<unsigned char> I2C_Scan();

    /** \brief I2C-Speed Methode
     *
     *  Setzt die Geschwindigkeit fuer die I2C-Kommunikation (Standard: 200Khz)
     *
     *  @param[in]      speed       Geschwindigkeit (100khz, 200khz, 400khz)
     *  @return                     TRUE wenn Ausfuehrung erfolgreich
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
    bool I2C_Read(unsigned char address, short byteCount, char* result);
    void I2C_Read(unsigned char address, short byteCount, char* result, bool status);
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
    bool I2C_ReadRegisters(unsigned char address, unsigned char reg, short byteCount, char* result);
    void I2C_ReadRegisters(unsigned char address, unsigned char reg, short byteCount, char *result, bool status);

    /** \brief I2C-Write Methode
     *
     *  Sendet Daten an einen Slave
     *
     *	@param[in]      address     Die Adresse des Slaves
     *  @param[in]      data        Array der zu schreibenden Daten
     *	@param[in]      byteCount   Anzahl Bytes der zu schreibenden Daten
     *  @return                     TRUE wenn Schreiben erfolgreich
     */
    bool I2C_Write(unsigned char address, const char* data, short byteCount);
    void I2C_Write(unsigned char address, const char* data, short byteCount, bool* status);


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
    bool I2C_WriteRegister(unsigned char address, unsigned char reg, const char* data, short byteCount);

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
     *  Setzt die Phase fuer die SPI-Kommunikation
     *
     *  @param[in]      phase
     *  <img src="SPI.png" alt="SPI">
     *  @return                     TRUE wenn Ausfuehrung erfolgreich
     */
    bool SPI_SetPHA(bool phase);

    /** \brief SPI-SetPol Methode
     *
     *  Setzt die Polaritaet fuer die SPI-Kommunikation
     *
     *  @param[in]      pol
     *  <img src="SPI.png" alt="SPI">
     *  @return                     TRUE wenn Ausfuehrung erfolgreich
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
    bool SPI_Read(unsigned char address, short byteCount, char* result);

    /** \brief SPI-Write Register Methode
     *
     *  Schreibt Daten an einen SPI Slave
     *
     *	@param[in]      address     Die Adresse des Slaves
     *  @param[in]      data        Array der zu schreibenden Daten
     *	@param[in]      byteCount   Anzahl Bytes der zu schreibenden Daten
     *  @return                     TRUE wenn Schreiben erfolgreich
     */
    bool SPI_Write(unsigned char address, const char* data, short byteCount);

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
    bool SPI_WriteRead(unsigned char address, const char* data, short byteCount, char* result);

    QString getLastError();
    void getLastError(QString &err);

    /**
      * returns true if the last problem is a slave problem, i.e. its not a fault of the uid.
      */
    bool isSlaveProblem();

    // inherited from RobotModule
    void reset();
    // inherited from RobotModule
    void terminate();

private:
    QextSerialPort* uid;
    PortSettings* portSettings;
    int lastError;

    QextSerialPort* findUIDPort();
    QextSerialPort* tryOpenPort(QString id, QextPortInfo* port);

    /**
      * may return false, when the command could not be sent to the uid.
      */
    bool SendCommand(const char* sequence, unsigned char length);
    bool SendCommand2(const QByteArray& send, char* recv=0, int recv_length=0);
    bool SendCheckCommand(const QByteArray& send, char* recv=0, int recv_length=0);
    bool CheckErrorcode();
    void doHealthCheck();
    unsigned char countBitsSet( unsigned char bitmask );

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
};

#endif // MODULE_UID_H
