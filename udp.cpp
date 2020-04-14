#include <iostream>
#include <map>
#include <thread>
#include <winsock2.h>
//#pragma comment(lib, "ws2_32.lib")

using namespace std;

sockaddr_in localAddr;

void mySend(SOCKET serSocket, map<unsigned long long, sockaddr_in *> &player)
{
    sockaddr_in *remoteAddr = NULL;
    int nAddrLen = sizeof(sockaddr_in);

    char sendData[255];
    char addr[20];
    int ee;

    while (true)
    {
        cin.getline(sendData, 255);

        if (sendData[0] == '+')
        {
            remoteAddr = (sockaddr_in *)malloc(nAddrLen);
            remoteAddr->sin_family = AF_INET;
            sscanf(sendData, "+%s%u", addr, &ee);
            remoteAddr->sin_port = htons(ee);
            remoteAddr->sin_addr.S_un.S_addr = inet_addr(addr);
            player[((remoteAddr->sin_addr.S_un.S_addr) << 16) | remoteAddr->sin_port] = remoteAddr;
            continue;
        }
        for (map<unsigned long long, sockaddr_in *>::iterator iter = player.begin(); iter != player.end(); iter++)
        {
            remoteAddr = iter->second;
            sendto(serSocket, sendData, strlen(sendData), 0, (sockaddr *)remoteAddr, nAddrLen);
        }
        if (sendData[0] == '-')
        {
            sendto(serSocket, sendData, strlen(sendData), 0, (sockaddr *)&localAddr, nAddrLen);
            break;
        }
    }
}

void myRecv(SOCKET serSocket, map<unsigned long long, sockaddr_in *> &player)
{
    int nAddrLen = sizeof(sockaddr_in);
    sockaddr_in *remoteAddr = (sockaddr_in *)malloc(nAddrLen);

    char recvData[255];
    int ret;
    u_long addr;

    while (true)
    {
        ret = recvfrom(serSocket, recvData, 255, 0, (sockaddr *)remoteAddr, &nAddrLen);
        if (ret > 0)
        {
            recvData[ret] = 0;
            cout << inet_ntoa(remoteAddr->sin_addr) << ":";
            cout << ntohs(remoteAddr->sin_port) << ": ";
            cout << recvData << endl;

            addr = ((remoteAddr->sin_addr.S_un.S_addr) << 16) | remoteAddr->sin_port;
            if (!player.count(addr))
            {
                player[addr] = remoteAddr;
                remoteAddr = (sockaddr_in *)malloc(nAddrLen);
            }
            if (recvData[0] == '-')
            {
                if (addr == localAddr.sin_addr.S_un.S_addr)
                    break;
                else
                {
                    free(player[addr]);
                    player.erase(addr);
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        return 0;
    }
    SOCKET serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serSocket == INVALID_SOCKET)
    {
        printf("socket error !");
        return 0;
    }

    u_short port;
    cin >> port;
    getchar();

    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(port);
    localAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(port);
    serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(serSocket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
    {
        printf("bind error !");
        closesocket(serSocket);
        return 0;
    }

    map<unsigned long long, sockaddr_in *> player;

    thread sendThread(mySend, serSocket, std::ref(player));
    thread recvThread(myRecv, serSocket, std::ref(player));

    sendThread.join();
    recvThread.join();

    closesocket(serSocket);
    WSACleanup();
    return 0;
}
