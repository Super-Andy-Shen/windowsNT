#include <iostream>
#include<Windows.h>
using namespace std;


int main()
{
    HANDLE h = NULL;
    char buffer[100] = { 0 };
    DWORD Read = 0;
    h = CreateFile("\\\\.\\myfirst123", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        printf("failed to open\n"); 
        system("pause");
    }
    printf("success to open device\n");
    system("pause");
    //读取
    bool success = ReadFile(h, buffer, 100, &Read, NULL);
    if (!success)
    {
        printf("failed to read\n");
        CloseHandle(h);
        system("pause");
        return 0;
    }
    printf("success to read: %s\n", buffer);
    system("pause");
    CloseHandle(h);
    system("pause");
    return 0;
}
