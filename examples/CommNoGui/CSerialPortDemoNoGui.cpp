#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#define imsleep(microsecond) Sleep(microsecond) // ms
#else
#include <unistd.h>
#define imsleep(microsecond) usleep(1000 * microsecond) // ms
#endif

#include <vector>

#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"
using namespace itas109;

std::string char2hexstr(const char *str, int len)
{
    static const char hexTable[17] = "0123456789ABCDEF";

    std::string result;
    for (int i = 0; i < len; ++i)
    {
        result += "0x";
        result += hexTable[(unsigned char)str[i] / 16];
        result += hexTable[(unsigned char)str[i] % 16];
        result += " ";
    }
    return result;
}


unsigned short CRC16_CCITT_FALSE(unsigned char* puchMsg, unsigned int usDataLen)
{
    unsigned short wCRCin = 0xFFFF;
    unsigned short wCPoly = 0x1021;
    unsigned char wChar = 0;

    while (usDataLen--)
    {
        wChar = *(puchMsg++);
        wCRCin ^= (wChar << 8);

        for (int i = 0; i < 8; i++)
        {
            if (wCRCin & 0x8000)
            {
                wCRCin = (wCRCin << 1) ^ wCPoly;
            }
            else
            {
                wCRCin = wCRCin << 1;
            }
        }
    }
    return (wCRCin);
}


int countRead = 0;

std::vector<unsigned char>txt;
unsigned short dataLength = 0;
int unm = 0;
class MyListener : public CSerialPortListener
{
public:
    MyListener(CSerialPort *sp)
        : p_sp(sp){};

    void onReadEvent(const char *portName, unsigned int readBufferLen)
    {
        if (readBufferLen > 0)
        {
           
           // char *data = new char[readBufferLen + 1]; // '\0'
            char* data = new char[readBufferLen];
            if (data)
            {
                // read
                int recLen = p_sp->readData(data, readBufferLen);
                //int recLen = p_sp->readAllData(data);
                if (recLen > 0)
                {
                   // data[recLen] = '\0';
                    if ((unsigned char)data[0]==0xFD)
                    {
                     
                        dataLength = MAKEWORD((unsigned char)data[2], (unsigned char)data[1]);
                     
                       
                        for (size_t i = 0; i < readBufferLen; i++)
                        {
                                txt.push_back(data[i]);
                        }
                       
                    }
                    else
                    {
                        
                        for (size_t i = 0; i < readBufferLen; i++)
                        {
                            txt.push_back(data[i]);
                        }
                    }

                    std::cout << portName << " - Count: " << ++countRead << ", Length: " << recLen /*<< ", Str: " << data << ", Hex: " << char2hexstr(data, recLen).c_str()*/
                              << std::endl;

                    // return receive data
                   // p_sp->writeData(data, recLen);
                }

                delete[] data;
                data = NULL;

                if ((dataLength+2) == txt.size())
                {
                    
                    unsigned short CRC = CRC16_CCITT_FALSE(txt.data(), txt.size()-2);
                    unsigned char aaaa = txt.at(txt.size()-2);
                    unsigned char aaaa1 = txt.at(txt.size() - 1);
                    unsigned short crc1 = MAKEWORD((unsigned char)aaaa1, (unsigned char)aaaa);
                    if (CRC == crc1)
                    {
                        unm++;
                        std::cout << "1====" << txt.size() << "========"<<unm<<std::endl;
                        txt.clear();

                    }

                   
                }
            }
        }
    };

private:
    CSerialPort *p_sp;
};



//#define CHONG_QING_SIX_LOCK_FRAME_MAX_LENGTH        4096
//#define CHONG_QING_SIX_LOCK_DATA_HEADER_LENGTH      6
//
//int RecvData()
//{
//    DWORD recv_len = 0;
//    int dataLength = 0;
//    int sumDataLength = 0;
//    int nRemainSize = 0;
//    int lastPos = 0;
//    BYTE recvbuf[CHONG_QING_SIX_LOCK_FRAME_MAX_LENGTH], databuf[CHONG_QING_SIX_LOCK_FRAME_MAX_LENGTH];
//    char oneFrameData[1024];
//
//    memset(recvbuf, 0, sizeof(recvbuf));
//    memset(databuf, 0, sizeof(databuf));
//
//    //收到服务端消息
//    //接受数据，处理粘包，拆分包
//    recv_len = (int)recv(m_Socket, (char*)recvbuf, CHONG_QING_SIX_LOCK_FRAME_MAX_LENGTH, 0);
//    if (recv_len > 0)
//    {
//        memcpy(databuf + lastPos, recvbuf, recv_len);
//        lastPos += recv_len;
//        //判断消息缓冲区的数据长度大于消息头
//        while (lastPos >= CHONG_QING_SIX_LOCK_DATA_HEADER_LENGTH)
//        {
//            //包头做判断，如果包头错误，收到的数据全部清空
//            if (databuf[0] == 0xEF && databuf[1] == 0xEF && databuf[2] == 0xEF && databuf[3] == 0xEF)
//            {
//                dataLength = MAKEWORD(databuf[4], databuf[5]);
//                sumDataLength = CHONG_QING_SIX_LOCK_DATA_HEADER_LENGTH + dataLength + 6;
//                //判断消息缓冲区的数据长度大于消息体
//                if (lastPos >= sumDataLength)
//                {
//                    //CRC校验
//                    if (CheckSum((byte*)databuf, dataLength + CHONG_QING_SIX_LOCK_DATA_HEADER_LENGTH + 2))
//                    {
//                        memcpy(oneFrameData, databuf, sumDataLength);
//                        //处理数据
//                        DealData(oneFrameData);
//                        //剩余未处理消息缓冲区数据的长度
//                        nRemainSize = lastPos - sumDataLength;
//                        //将未处理的数据前移
//                        if (nRemainSize > 0)
//                        {
//                            memcpy(databuf, databuf + (dataLength + CHONG_QING_SIX_LOCK_DATA_HEADER_LENGTH + 6), nRemainSize);
//                            lastPos = nRemainSize;
//                        }
//                    }
//                    else
//                    {
//                        if (nRemainSize > 0)
//                        {
//                            memcpy(databuf, databuf + sumDataLength, nRemainSize);
//                        }
//
//                        lastPos = nRemainSize;
//                    }
//                }
//                else
//                {
//                    break;
//                }
//            }
//            else      //寻找下一个包头
//            {
//                BOOL isFind = FALSE;
//                int nFindStart = 0;
//                for (int k = 1; k < lastPos; k++)
//                {
//                    if (databuf[k] == 0xEF && databuf[k + 1] == 0xEF && databuf[k + 2] == 0xEF && databuf[k + 3] == 0xEF)
//                    {
//                        nFindStart = k;
//                        isFind = TRUE;
//                        break;
//                    }
//                }
//                if (isFind == TRUE)
//                {
//                    memcpy(databuf, databuf + nFindStart, lastPos - nFindStart);
//
//                    lastPos = lastPos - nFindStart;
//                }
//                else
//                {
//                    memset(databuf, 0, sizeof(databuf));
//                    lastPos = 0;
//                    break;
//                }
//            }
//        }
//    }
//    return 0;
//}

int main()
{
    CSerialPort sp;
    std::cout << "Version: " << sp.getVersion() << std::endl << std::endl;

    MyListener listener(&sp);

    std::vector<SerialPortInfo> m_availablePortsList = CSerialPortInfo::availablePortInfos();

    std::cout << "availableFriendlyPorts: " << std::endl;

    for (size_t i = 1; i <= m_availablePortsList.size(); ++i)
    {
        SerialPortInfo serialPortInfo = m_availablePortsList[i - 1];
        std::cout << i << " - " << serialPortInfo.portName << " " << serialPortInfo.description << " " << serialPortInfo.hardwareId << std::endl;
    }

    if (m_availablePortsList.size() == 0)
    {
        std::cout << "No valid port" << std::endl;
    }
    else
    {
     /*   std::cout << std::endl;

        int input = -1;
        do
        {
            std::cout << "Please Input The Index Of Port(1 - " << m_availablePortsList.size() << ")" << std::endl;

            std::cin >> input;

            if (input >= 1 && input <= m_availablePortsList.size())
            {
                break;
            }
        } while (true);

        const char *portName = m_availablePortsList[input - 1].portName;
        std::cout << "Port Name: " << portName << std::endl;*/

        sp.init("COM3",              // windows:COM1 Linux:/dev/ttyS0
            2000000, // baudrate
                itas109::ParityNone,   // parity
                itas109::DataBits8,    // data bit
                itas109::StopOne,      // stop bit
                itas109::FlowNone,     // flow
                1024*35                   // read buffer size
        );
        sp.setReadIntervalTimeout(0); // read interval timeout 0ms

        sp.open();
        std::cout << "Open " << sp.getPortName() << (sp.isOpen() ? " Success" : " Failed") << std::endl;

        // connect for read
        sp.connectReadEvent(&listener);

        // write hex data
        char hex[5];
        hex[0] = 0x31;
        hex[1] = 0x32;
        hex[2] = 0x33;
        hex[3] = 0x34;
        hex[4] = 0x35;
       // sp.writeData(hex, sizeof(hex));

        // write str data
      //  sp.writeData("itas109", 7);

        for (;;)
        {
            imsleep(1);
        }
    }

    return 0;
}