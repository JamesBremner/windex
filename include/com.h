#pragma once
#include <iostream>
#include <windows.h>
#include <vector>
#include <future>
#include <functional>
#include "wex.h"

namespace wex
{

class com : public gui
{
public:
    /** CTOR
        @param[in] parent window which will receive messages when async read have completed
    */
    com( gui* parent )
        : gui( parent )
        , myCOMHandle( 0 )

    {
    }
    /// Set port number to wgich connection will be made
    void port( const std::string& port )
    {
        if( atoi( port.c_str() ) < 10 )
            myPortNumber = "COM" + port;
        else
            myPortNumber = "\\\\.\\COM" + port;
    }
    /** Open connection to port
        @return true if succesful
    */
    bool open()
    {
        if ( ! myPortNumber.length() )
            return false;

        myCOMHandle = CreateFile(
                          myPortNumber.c_str(),                //port name
                          GENERIC_READ | GENERIC_WRITE, //Read/Write
                          0,                            // No Sharing
                          NULL,                         // No Security
                          OPEN_EXISTING,// Open existing port only
                          FILE_FLAG_OVERLAPPED,
                          //0,
                          NULL);        // Null for Comm Devices

        if (myCOMHandle == INVALID_HANDLE_VALUE)
        {
            std::cout << "Cannot open COM\n";
            myCOMHandle = 0;
            return false;
        }

        DCB dcbSerialParams = { 0 }; // Initializing DCB structure
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        GetCommState(myCOMHandle, &dcbSerialParams);
        dcbSerialParams.BaudRate = CBR_9600;  // Setting BaudRate = 9600
        dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
        dcbSerialParams.StopBits = ONESTOPBIT;// Setting StopBits = 1
        dcbSerialParams.Parity   = NOPARITY;  // Setting Parity = None
        SetCommState(myCOMHandle, &dcbSerialParams);

        // empty the input buffer
        PurgeComm(myCOMHandle,PURGE_RXCLEAR);

        return true;
    }

    /// true if connected
    bool isOpen()
    {
        return myCOMHandle != 0;
    }

    /** blocking read from COM port
        @param[in] needed byte count
        Data will be read into myRcvbuffer vector
    */
    void read( int needed )
    {
        DWORD NoBytesRead;

        int chunk;          // size of next chunk of data to be read
        int totalBytesRead = 0;

        // ensure there is room for the data read
        myRcvbuffer.resize( needed );

        // loop reading chunks of data as they arrive
        while( needed > 0 )
        {
            // wait for data
            int waiting = waitForData();

            if( waiting >= needed )
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
                           myCOMHandle,                               //Handle of the Serial port
                           myRcvbuffer.data() + totalBytesRead,    // pointer to buffer
                           chunk,                                   // amount to read
                           &NoBytesRead,                            //Number of bytes read
                           &over );
            if( ! ret )
            {
                throw std::runtime_error("windex com read block failed "
                                         + std::to_string( GetLastError() ));
            }

            // update data counts
            totalBytesRead += chunk;
            needed -= chunk;

            //std::cout << "COM read block read " << totalBytesRead << "\n";
        }
    }

    /// get reference to buffer containg data that was read
    std::vector<unsigned char>
    readData()
    {
        return myRcvbuffer;
    }

    /** non-blocking read from COM port
        @param[in] bytes byte count to be read

      This will return imediatly.
      When the specified bytes have been read
      a message, id = WM_APP+1, will be sent to the parent window

    */
    void read_async(
        int bytes )
    {
        //std::cout << "com read_async " << bytes << "\n";

        // start blocking read in own thread
        myFuture = std::async(
                       std::launch::async,              // insist on starting immediatly
                       &com::read,
                       this,
                       bytes );

        // start waiting for read completion in own thread
        myThread = new std::thread(read_sync_wait, this);

        /* There are now two threads running
            one doing the reading
            and the other waiting for the read to be finished

            We can return now and get on with something else
        */
    }

    /// Write buffer of data to the COM port
    int write( const std::vector<unsigned char>& buffer )
    {
        std::cout << "buffersize " <<  buffer.size() << "\n";
        _OVERLAPPED over;
        memset(&over, 0, sizeof(over));
        DWORD dNoOfBytesWritten;
        bool ret = WriteFile(
                       myCOMHandle,        // Handle to the Serial port
                       buffer.data(),     // Data to be written to the port
                       buffer.size(),  //No of bytes to write
                       &dNoOfBytesWritten, //Bytes written
                       &over);
        if( ! ret )
        {
            std::cout << "write failed " << GetLastError() << "\n";
            return 0;
        }
        std::cout << "write " << dNoOfBytesWritten << "\n";
        return dNoOfBytesWritten;
    }

    ///  Write string of data to the COM port
    int write( const std::string& msg )
    {
        std::vector<unsigned char> buffer( msg.size() );
        memcpy( buffer.data(), msg.data(), msg.size() );
        return write( buffer );
    }
private:
    std::string myPortNumber;
    HANDLE myCOMHandle;
    std::future< void > myFuture;
    std::thread*        myThread;
    std::vector<unsigned char> myRcvbuffer;

    int waitForData()
    {
        DWORD dwEventMask;
        _COMSTAT comstat;
        DWORD errors;

        while( 1 )
        {
            // check for bytes waiting to be read
            ClearCommError(
                myCOMHandle,
                &errors,
                &comstat );
            int waiting = comstat.cbInQue;
            if( waiting )
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
        while (myFuture.wait_for(std::chrono::milliseconds(check_interval_msecs))==std::future_status::timeout)
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
            WM_APP+1,
            myID,
            0 );

        // return, terminating the wait thread
        return;
    }
};
}
