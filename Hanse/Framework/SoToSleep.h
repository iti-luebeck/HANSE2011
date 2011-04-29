#ifndef SOTOSLEEP_H
#define SOTOSLEEP_H

#endif // SOTOSLEEP_H
class sotoSleep : public QThread
{
        public:
                static void sleep(unsigned long secs)
                {
                        QThread::sleep(secs);
                }
                static void msleep(unsigned long msecs)
                {
                        QThread::msleep(msecs);
                }
                static void usleep(unsigned long usecs)
                {
                        QThread::usleep(usecs);
                }
};
