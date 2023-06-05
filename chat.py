# -*- coding: UTF-8 -*-

from asyncio.subprocess import PIPE
from math import fabs
import win32file
import win32pipe
import struct
import threading
import time
import sys
import json
import openai
import deepai

openai.api_key="PUT YOUR API KEY HERE"


def chat_with_3_5(str_in):
    msg_load={}
    msg_load=json.loads(str_in)
    m=[]
    for key in msg_load.keys():
        m.append({"role":key,"content":msg_load[key]})

    ans=""
    ans=openai.ChatCompletion.create(model="gpt-3.5-turbo",messages=m).choices[0].message.content
    #if you want to use free gpt(deepai),delate the "#" in the next two lines,and add "#" to the line above

    #for chunk in deepai.ChatCompletion.create(m):
    #    ans=ans+chunk

    print("ans:")
    return ans




inputmsg="f"
PIPE_NAME = r'\\.\pipe\my_pipe'
PIPE_NAME=PIPE_NAME+str(sys.argv[1])
PIPE_BUFFER_SIZE = 65535


named_pipe = win32file.CreateFile(PIPE_NAME,
                                   win32file.GENERIC_READ | win32file.GENERIC_WRITE,
                                   win32file.FILE_SHARE_WRITE, None,
                                   win32file.OPEN_EXISTING, 0, None)
try:
    while True:
        try:
            data = win32file.ReadFile(named_pipe, PIPE_BUFFER_SIZE, None)

            if data is None or len(data) < 2:
                continue

            print ('#receive msg:', data[1])
            

            inputmsg=str(data[1],encoding="utf-8")
            print ('#inputmsg:',inputmsg)
            break
        except BaseException as e:
            print ("#exception:", e)
            break
finally:
    try:
        #win32pipe.DisconnectNamedPipe(named_pipe)
        pass
    except:
        pass

print("#python start rec")
sendmsg=chat_with_3_5(inputmsg)

def sendpipe(msg):
    try:
        win32file.WriteFile(named_pipe, msg.encode())
        global inputmsg
        inputmsg="quit"
    except:
        pass

while True:
    if inputmsg=="quit":
        try:
            win32file.CloseHandle(named_pipe)
        except:
            pass
        break
    sendpipe(sendmsg)

