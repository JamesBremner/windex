#pragma once
#include <iostream>
#include <windows.h>
#include <vector>
#include <future>
#include <functional>


//class tester
//{
//public:
//    std::future< void > myFuture;
//
//    void test( int& x )
//    {
//
//    }
//    void test2( std::vector<unsigned char>& buffer )
//    {
//
//    }
//    void callasync(
//        int& x,
//        std::vector<unsigned char>& buffer )
//    {
//        myFuture = std::async(
//                       std::launch::async,              // insist on starting immediatly
//                       &tester::test,
//                       this,
//                       std::ref( x ) );
//    }
//};


namespace wex
{

// https://www.xanthium.in/Serial-Port-Programming-using-Win32-API
class com
{
public:
    com()
        : myHandle( 0 )
    {

    }
    void port( const std::string& port )
    {
        if( atoi( port.c_str() ) < 10 )
            myPortNumber = "COM" + port;
        else
            myPortNumber = "\\\\.\\COM" + port;
    }
    bool open()
    {
        if ( ! myPortNumber.length() )
            return false;

        myHandle = CreateFile(
                       myPortNumber.c_str(),                //port name
                       GENERIC_READ | GENERIC_WRITE, //Read/Write
                       0,                            // No Sharing
                       NULL,                         // No Security
                       OPEN_EXISTING,// Open existing port only
                       0,            // Non Overlapped I/O
                       NULL);        // Null for Comm Devices

        if (myHandle == INVALID_HANDLE_VALUE)
        {
            myHandle = 0;
            return false;
        }

        DCB dcbSerialParams = { 0 }; // Initializing DCB structure
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        GetCommState(myHandle, &dcbSerialParams);
        dcbSerialParams.BaudRate = CBR_9600;  // Setting BaudRate = 9600
        dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
        dcbSerialParams.StopBits = ONESTOPBIT;// Setting StopBits = 1
        dcbSerialParams.Parity   = NOPARITY;  // Setting Parity = None
        SetCommState(myHandle, &dcbSerialParams);

        return true;
    }
    bool isOpen()
    {
        return myHandle != 0;
    }



    /// blocking read from COM port
    void read( int bytes )
    {
        DWORD dwEventMask;
        unsigned char c;
        DWORD NoBytesRead;

        SetCommMask(myHandle, EV_RXCHAR);
        WaitCommEvent(myHandle, &dwEventMask, NULL);
        myRcvbuffer.clear();
        do
        {
            ReadFile(
                myHandle,           //Handle of the Serial port
                &c,                //Temporary character
                1,                //Size of TempChar
                &NoBytesRead,    //Number of bytes read
                NULL);
            std::cout << std::hex << (int)c << " ";
            myRcvbuffer.push_back( c );
            if( bytes == (int)myRcvbuffer.size())
                break;
        }
        while (NoBytesRead > 0);
        std::cout << "\n";
    }

    std::vector<unsigned char>
    readData()
    {
        return myRcvbuffer;
    }


    /// non-blocking read from COM port
    void read_async(
        int bytes,
        std::function<void(void )> readHandler )
    {
        myFuture = std::async(
                       std::launch::async,              // insist on starting immediatly
                       &com::read,
                       this,
                       bytes );
    }

    int write( std::vector<unsigned char>& buffer )
    {
        std::cout << "buffersize " <<  buffer.size() << "\n";
        DWORD dNoOfBytesWritten;
        WriteFile(myHandle,        // Handle to the Serial port
                  buffer.data(),     // Data to be written to the port
                  buffer.size(),  //No of bytes to write
                  &dNoOfBytesWritten, //Bytes written
                  NULL);
        std::cout << "write " << dNoOfBytesWritten << "\n";
        return dNoOfBytesWritten;
    }
    int write( const std::string& msg )
    {
        std::vector<unsigned char> buffer( msg.size() );
        memcpy( buffer.data(), msg.data(), msg.size() );
        return write( buffer );
    }
private:
    std::string myPortNumber;
    HANDLE myHandle;
    std::future< void > myFuture;
    std::vector<unsigned char> myRcvbuffer;
};
}
