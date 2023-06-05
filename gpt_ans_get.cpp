#include "gpt_ans_get.h"

using namespace std;

const int BUF_SIZE = 65536;
string head = "python chat.py ";
string pipename = "\\\\.\\pipe\\my_pipe";

void python_s(string _in)
{
	_in = head + _in;
	system(_in.c_str());
	return;
}


string gpt3_5_ans_get(string s,int t)
{;

	thread py(python_s, to_string(t));
	py.detach();
	string pipename = "\\\\.\\pipe\\my_pipe";
	pipename = pipename + to_string(t);

	// 创建命名管道
	cout << "start create" << endl;
	HANDLE hPipe = NULL;

	WCHAR wcharTemp[256];
	MultiByteToWideChar(CP_ACP, 0, pipename.c_str(), -1, wcharTemp, sizeof(wcharTemp) / sizeof(wcharTemp[0]));

	hPipe = CreateNamedPipe(
		//L"\\\\.\\pipe\\my_pipe",
		wcharTemp,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE |
		PIPE_READMODE_MESSAGE |
		PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		BUF_SIZE,
		BUF_SIZE,
		0,
		NULL);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		cout << "Create Read Pipe Error" << endl;
		return FALSE;
	}

	// 等待python端的连接
	if (!ConnectNamedPipe(hPipe, NULL))
	{
		cout << "Connect Failed" << endl;
		return FALSE;
	}

	cout << "connected" << endl;
	DWORD dwReturn = 0;
	char szBuffer[BUF_SIZE] = { 0 };

	//写入python数据
	if (!WriteFile(hPipe, s.c_str(), s.size(), &dwReturn, NULL))
	{
		cout << "Write Failed" << endl;
	}

	string ans = "";
	// 读取python端数据
	memset(szBuffer, 0, BUF_SIZE);
	if (ReadFile(hPipe, szBuffer, BUF_SIZE, &dwReturn, NULL))
	{
		szBuffer[dwReturn] = '\0';
		ans = szBuffer;
		cout << "Read succesfully" << endl;
		cout << "cans:" << ans << endl;
	}
	else
	{
		cout << "Read Failed" << endl;
	}

	return ans;
}