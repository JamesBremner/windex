#pragma once
#include <iostream>
#include <windows.h>
#include <vector>
#include <future>
#include <functional>
#include "wex.h"

namespace wex
{
    /**  @brief read / write to COM serial port

For sample code, see https://github.com/JamesBremner/windex/blob/master/demo/com.cpp
*/
    class com : public gui
    {
    public:
        /** CTOR
        @param[in] parent window which will receive messages when async read has completed
    */
        com(gui *parent)
            : gui(parent),
              myCOMHandle(0),
              myfOverlapped(true),
              myfCTSFlowControl(true)
        {
        }
        /// @name Setters
        ///@{

        /// Set port number to which connection will be made
        void port(const std::string &port)
        {
            if (atoi(port.c_str()) < 10)
                myPortNumber = "COM" + port;
            else
                myPortNumber = "\\\\.\\COM" + port;
        }

        /** Enable/Disable open connection overlapped
         * @param[in] f true to enable overlapped, default true
         * 
         * If the COM port is opened overlapped
         *   - read/writes are asynvhronous
         *   - port csn be used for both reading and writing
         * 
         * This must be called before the port is opened.
         */
        void overlapped(bool f = true)
        {
            myfOverlapped = f;
        }

        /** Enable/Disable CTS flow control
         * @param[in] f true to enable, default true
         * 
         * If this is TRUE, the CTS (clear-to-send) signal is monitored for output flow control.
         * If this is TRUE and CTS is turned off, output is suspended until CTS is sent again.
         * 
         * This must be called before the port is opened.
         */
        void CTSFlowControl(bool f = true)
        {
            myfCTSFlowControl = f;
        }

        /** Configure device
        @param[in] controlString The device-control information.

        The device must be open.

        Control sring format
        [baud=b][parity=p][data=d][stop=s][to={on|off}][xon={on|off}][odsr={on|off}][octs={on|off}][dtr={on|off|hs}][rts={on|off|hs|tg}][idsr={on|off}]

        For example, the following string specifies a baud rate of 1200, no parity, 8 data bits, and 1 stop bit:

        baud=1200 parity=N data=8 stop=1
    */
        void DeviceControlString(const std::string &controlString)
        {
            // Initalize all the configuration parameters to zero
            // BuildCommDCBA is supposed to do this,
            // but does not seem to do so - in particular the CTS flow is not diabled
            DCB dcbSerialParams = {0};

            if (!myCOMHandle)
                return;
            if (!BuildCommDCBA(
                    controlString.c_str(),
                    &dcbSerialParams))
                return;

            // Force the CTS control to state requested
            dcbSerialParams.fOutxCtsFlow = myfCTSFlowControl;

            SetCommState(myCOMHandle, &dcbSerialParams);
        }
        void baud(int rate)
        {
            if (!isOpen())
                return;
            DCB dcbSerialParams = {0}; // Initializing DCB structure
            dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
            GetCommState(myCOMHandle, &dcbSerialParams);
            dcbSerialParams.BaudRate = rate;
            dcbSerialParams.fOutxCtsFlow = 0;
            SetCommState(myCOMHandle, &dcbSerialParams);
        }

        /// @name Getters
        ///@{
        const std::string &portNumber() const
        {
            return myPortNumber;
        }
        int baud()
        {
            if (!isOpen())
                return 0;
            DCB dcbSerialParams = {0}; // Initializing DCB structure
            dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
            GetCommState(myCOMHandle, &dcbSerialParams);
            return dcbSerialParams.BaudRate;
        }
                /// true if connected
        bool isOpen()
        {
            return myCOMHandle != 0;
        }
               /** Get human readable port configuration
         * @return string
         */
        std::string configText()
        {
            std::stringstream ss;
            if (!isOpen())
            {
                ss << "COM not connected\n";
                return ss.str();
            }
            if (myfOverlapped)
                ss << "overlapped\n";
            else
                ss << "not overlapped\n";
            _COMMCONFIG cfg;
            cfg.dcb = {0};
            DWORD sz = sizeof(cfg);
            if (!GetCommConfig(
                    myCOMHandle, // Handle to the Serial port
                    &cfg,
                    &sz))
            {
                ss << "GetCommConfig FAILED\n";
                return ss.str();
            }
            DCB dcb = cfg.dcb;
            ss << "\nBaudRate " << dcb.BaudRate
               << "\nfBinary " << dcb.fBinary
               << "\nfParity " << dcb.fParity
               << "\nfOutxCtsFlow " << dcb.fOutxCtsFlow
               << "\nfOutxDsrFlow " << dcb.fOutxDsrFlow
               << "\nfDtrControl " << dcb.fDtrControl
               << "\nfDsrSensitivity " << dcb.fDsrSensitivity
               << "\nfTXContinueOnXoff " << dcb.fTXContinueOnXoff
               << "\nfOutX " << dcb.fOutX
               << "\nfInX " << dcb.fInX
               << "\nfErrorChar " << dcb.fErrorChar
               << "\nfBinary " << dcb.fNull
               << "\nfNull " << dcb.fRtsControl
               << "\nfAbortOnError " << dcb.fAbortOnError
               << "\nXonLim " << dcb.XonLim
               << "\nXoffLim " << dcb.XoffLim
               << "\nByteSize " << dcb.ByteSize
               << "\nParity " << dcb.Parity
               << "\nStopBits " << dcb.StopBits
               << "\nXonChar " << dcb.XonChar
               << "\nXoffChar " << dcb.XoffChar
               << "\nErrorChar " << dcb.ErrorChar
               << "\nEofChar " << dcb.EofChar
               << "\nEvtChar " << dcb.EvtChar << "\n";
            return ss.str();
        }
                std::string &errorMsg()
        {
            return myError;
        }

        ///@}

        /** Open connection to port
        @return true if succesful

        Opens with default configuration "baud=9600 parity=N data=8 stop=1"

        Reconfigure with DeviceControlString()

        On error, a mesage will be available by calling errorMsg();
    */
        bool open()
        {
            if (!myPortNumber.length())
                return false;

            DWORD dwFlagsAndAttributes = 0;
            if (myfOverlapped)
                dwFlagsAndAttributes = FILE_FLAG_OVERLAPPED;

            myCOMHandle = CreateFile(
                myPortNumber.c_str(),         //port name
                GENERIC_READ | GENERIC_WRITE, //Read/Write
                0,                            // No Sharing
                NULL,                         // No Security
                OPEN_EXISTING,                // Open existing port only
                dwFlagsAndAttributes,
                //0,
                NULL); // Null for Comm Devices

            if (myCOMHandle == INVALID_HANDLE_VALUE)
            {
                DWORD dw = GetLastError();

                LPVOID lpMsgBuf;
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    dw,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&lpMsgBuf,
                    0, NULL);
                std::cout << (char *)lpMsgBuf;

                std::cout << "Cannot open COM at "
                          << myPortNumber << " error " << myPortNumber << "\n";
                myError = myPortNumber;
                switch (dw)
                {
                case 2:
                    myError += " There seems to be no device connected to this port";
                    break;
                case 5:
                    myError += " This port seems to be in use by another application";
                    break;
                case 21:
                    myError += " Driver reports device is not ready";
                    break;
                default:
                    myError += " This port will not open, error " + std::to_string(dw);
                }

                LocalFree(lpMsgBuf);

                myCOMHandle = 0;
                return false;
            }

            myError = "";

            DeviceControlString(
                "baud=9600 parity=N data=8 stop=1 octs=off");

            // empty the input buffer
            PurgeComm(myCOMHandle, PURGE_RXCLEAR);

            return true;
        }
        void close()
        {
            if (myCOMHandle)
            {
                CloseHandle(myCOMHandle);
            }
        }

        /** blocking read from COM port
        @param[in] needed byte count, -1 to read whatever becomes available

        Data will be read into a vector of bytes,
        com::ReadData() provides a reference to this.

    */
        void read(int needed)
        {
            DWORD NoBytesRead;

            int chunk; // size of next chunk of data to be read
            int totalBytesRead = 0;

            // ensure there is room for the data read
            if (needed > 0)
                myRcvbuffer.resize(needed);

            // loop reading chunks of data as they arrive
            do
            {
                // wait for data
                int waiting = waitForData();

                if (needed < 0)
                {
                    needed = waiting;
                    chunk = waiting;
                    myRcvbuffer.resize(needed);
                }
                else if (waiting >= needed)
                {
                    // read all we need
                    chunk = needed;
                }
                else
                {
                    // read all that is available
                    chunk = waiting;
                }

                // read chunk
                _OVERLAPPED over;
                memset(&over, 0, sizeof(over));
                bool ret = ReadFile(
                    myCOMHandle,                         //Handle of the Serial port
                    myRcvbuffer.data() + totalBytesRead, // pointer to buffer
                    chunk,                               // amount to read
                    &NoBytesRead,                        //Number of bytes read
                    &over);
                if (!ret)
                {
                    throw std::runtime_error("windex com read block failed " + std::to_string(GetLastError()));
                }

                // update data counts
                totalBytesRead += chunk;
                needed -= chunk;

                //std::cout << "COM read block read " << totalBytesRead << "\n";
            } while (needed > 0);
        }

        /// get reference to buffer containg data that was read
        std::vector<unsigned char>
        readData()
        {
            return myRcvbuffer;
        }

        /** non-blocking read from COM port
        @param[in] bytes byte count to be read, -1 to read whatever becomes available

      This will return imediatly.

      When the specified bytes have been read
      the parent window event <pr>asyncReadComplete</pre>
      will invoke its read handler in the thread that created the parent window.
      No multithreading is required by the application code.

      This uses the message <pre>id = WM_APP+1</pre>,
      which must NOT be used anywhere in the application code.

    */
        void read_async(
            int bytes)
        {
            //std::cout << "com read_async " << bytes << "\n";

            // start blocking read in own thread
            myFuture = std::async(
                std::launch::async, // insist on starting immediatly
                &com::read,
                this,
                bytes);

            // start waiting for read completion in own thread
            myThread = new std::thread(read_sync_wait, this);

            /* There are now two threads running
            one doing the reading
            and the other waiting for the read to be finished

            We can return now and get on with something else
        */
        }
 
        /// Write buffer of data to the COM port
        int write(const std::vector<unsigned char> &buffer)
        {
            std::cout << "Write buffersize " << buffer.size() << "\n";
            std::cout << COMConfigText();

            _OVERLAPPED over;
            memset(&over, 0, sizeof(over));
            DWORD dNoOfBytesWritten;
            bool ret = WriteFile(
                myCOMHandle,        // Handle to the Serial port
                buffer.data(),      // Data to be written to the port
                buffer.size(),      //No of bytes to write
                &dNoOfBytesWritten, //Bytes written
                &over);
            if (!ret)
            {
                int syserrno = GetLastError();
                std::cout << "write failed " << syserrno << "\n";
                int count = 0;
                if (syserrno == 997)
                {
                    std::cout << "polling ..." << std::endl;
                    while (1)
                    {
                        HasOverlappedIoCompleted(&over);
                        if (over.Internal != STATUS_PENDING)
                        {
                            //for better or worse the write has completed
                            break;
                        }

                        count++;
                        if (count > 10000)
                        {
                            std::cout << "async write timed out\n";
                            return 0;
                        }
                        if (!(count % 1000))
                            std::cout << " ." << std::endl;

                        // Yield
                        Sleep(0);
                    }
                    std::cout << "async write completed\n";
                    return buffer.size();
                }
            }
            //std::cout << "write " << dNoOfBytesWritten << "\n";
            return dNoOfBytesWritten;
        }

        ///  Write string of data to the COM port
        int write(const std::string &msg)
        {
            std::vector<unsigned char> buffer(msg.size());
            memcpy(buffer.data(), msg.data(), msg.size());
            return write(buffer);
        }

    private:
        std::string myPortNumber;
        HANDLE myCOMHandle;
        std::future<void> myFuture;
        std::thread *myThread;
        std::vector<unsigned char> myRcvbuffer;
        std::string myError;
        bool myfOverlapped;
        bool myfCTSFlowControl;

        int
        waitForData()
        {
            DWORD dwEventMask;
            _COMSTAT comstat;
            DWORD errors;

            while (1)
            {
                // check for bytes waiting to be read
                ClearCommError(
                    myCOMHandle,
                    &errors,
                    &comstat);
                int waiting = comstat.cbInQue;
                if (waiting)
                    return waiting;

                // wait for some data to arrive
                SetCommMask(myCOMHandle, EV_RXCHAR);
                WaitCommEvent(myCOMHandle, &dwEventMask, NULL);
            }
        }
        void read_sync_wait()
        {
            // check if read is complete from time to time
            const int check_interval_msecs = 50;
            while (myFuture.wait_for(std::chrono::milliseconds(check_interval_msecs)) == std::future_status::timeout)
            {
                //std::cout << '.' << std::flush;
                // read still running, loop and check again after an interval
            }

            // read complete
            // send WM_APP+1 message to parent window
            // which will be handled, not in this thread,
            // but in the thread that created the window.

            PostMessageA(
                myParent->handle(),
                WM_APP + 1,
                myID,
                0);

            // return, terminating the wait thread
            return;
        }
    };
}
