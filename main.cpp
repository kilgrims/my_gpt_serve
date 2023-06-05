#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <Ws2tcpip.h>
#include <string>
#include <thread>
#include <queue>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

#include "gpt_ans_get.h"

using namespace std;

const int MAX_HANDLE_NUM = 100;
queue<int> thread_num;

char recbuf[2048];//�������ݻ�����
const int BUFF_SIZE = 2048;
const char log_ui_filename[] = "./log_ui.html";
const char chat_ui_filename[] = "./chat_ui.html";

string html_head = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
string html_end = "\r\n\0";
string log_ui_s;
string chat_ui_s;
const string GET = "GET";
const string POST = "POST";


void thread_s(string _in,string* _out)
{
    *_out = gpt3_5_ans_get(_in,1);
    return;
}



void handle_request(int client_fd,int _thread_num)
{
    int numbytes;
    char buff[2048];
    if ((numbytes = recv(client_fd, buff, BUFF_SIZE, 0)) == -1)
    {
        cerr << "Failed to receive data: " << WSAGetLastError() << endl;
        closesocket(client_fd);
    }
    buff[numbytes] = 0;
    cout << buff << endl;

    string buff_s = buff;
    string http_type = buff_s.substr(0, buff_s.find(' '));

    if (http_type == GET)
    {
        string post_sent = html_head + chat_ui_s + html_end;
        cout << send(client_fd, post_sent.c_str(), post_sent.size(), 0) << " Bytes has sent" << endl;
        return;
    }
    else if (http_type == POST)
    {
        cout << "POST received" << endl;

        string post_rec = buff_s.substr(buff_s.find('{'));
        string post_sent = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=UTF-8\r\n\r\n";

        string result_s;
        result_s = gpt3_5_ans_get(post_rec, _thread_num);
        
        Sleep(2500);
        post_sent = post_sent + result_s + html_end;
        cout << "post_sent:" << post_sent << endl;
        
        int bytesRead;
        int bytesSent = send(client_fd, post_sent.c_str(), post_sent.length(), 0);
        cout << bytesSent << endl;
        if (bytesSent == SOCKET_ERROR)
        {
            std::cerr << "Failed to send data: " << WSAGetLastError() << std::endl;
        }
        else {
            cout << "post sent successfully" << endl;
        }
        return;
    }
    else
    {
        cerr << "http type error!" << endl;
        return;
    }
    return;
}

void thread_handle_catch(int _client_fd, int _tnum)
{
    handle_request(_client_fd, _tnum);
    closesocket(_client_fd);
    thread_num.push(_tnum);
    return;
}


void thread_handle(int _client_fd)
{
    int tnum = thread_num.front();
    cout << "thread start with: " << tnum << endl;
    thread_num.pop();
    thread  t(thread_handle_catch, _client_fd,tnum);
    t.detach();
    return;
}



void file_input(string& _in, istream& ss)
{
    string s;
    while (!ss.eof())
    {
        s.clear();
        getline(ss, s);
        _in = _in + s + '\n';
    }
    return;
}





int main() {
    //��ʼ���̹߳������
    for (int i = 1; i <= MAX_HANDLE_NUM; i++)
    {
        thread_num.push(i);
    }

    //��ȡui�ļ�
    fstream fs;
    fs.open(log_ui_filename);
    if (!fs.is_open())
    {
        cerr << "failed to open source file" << endl;
    }
    file_input(log_ui_s, fs);
    fs.close();

    fs.open(chat_ui_filename);
    if (!fs.is_open())
    {
        cerr << "failed to open source file" << endl;
    }
    file_input(chat_ui_s, fs);
    fs.close();


    WSADATA wsadata;//���ڳ�ʼ��winsock��
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
    SOCKADDR_IN svr_addr, cli_addr;//��������ַ��ͻ��˵�ַ
    int sin_len = sizeof(cli_addr);

    //�׽�����˿�����
    int my_socket = socket(AF_INET, SOCK_STREAM, 0);
    int my_port = 1181;

    //��������ַ����
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(my_port);

    //���׽��ֵ���������ַ
    if (bind(my_socket, (SOCKADDR*)&svr_addr, sizeof(svr_addr)) == -1)
    {
        closesocket(my_socket);
        cerr << "bind error" << endl;
    }

    listen(my_socket, MAX_HANDLE_NUM);//�����MAX_HANDLE_NUM���ͻ��˽��м���

    //����ѭ�������û�accept
    while (1)
    {
        int len = sizeof(SOCKADDR);
        int _client_fd = accept(my_socket, (SOCKADDR*)&cli_addr, &sin_len);
        cout << "\n\naccept received in " << inet_ntoa(cli_addr.sin_addr) << endl;//��ʾ���ܵ����û�ip��ַ
        if (_client_fd == -1)
        {
            cerr << "failed to connect accept" << endl;
            continue;
        }
        thread_handle(_client_fd);
    }


    closesocket(my_socket);
    WSACleanup();

    return 0;
}