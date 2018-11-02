ubuntu16.04

#--------编译链接decode_rtp_h264.c接收端
gcc Client.c -o client

#--------编译链接rtp_send_h264.c发送端
gcc Server.c -o server


#注意事项
IP地址都需要按自己情况修改
端口号需避免与其它用途冲突
先打开server，再打开client

#发送端,服务端：
--------720p.h264.raw为h264原始码流文件
使用方法usage: %s <H264_filename> [port设置监听端口]
e.g.	./server 720p.h264.raw 6669

#接收端,客户端：
使用方法usage: %s save_filename [server_ip] [server_port服务器监听端口]
e.g.	./client client_recv.h264 127.0.0.1 6669
server_ip 设为服务端ip，用来发送请求

##发送端，循环播放
循环发送,默认循环1次，之后询问是否继续循环发送
输入循环次数(0为中止发送)，即可按次数循环，每当循环次数达到都会询问是否继续循环

![image](http://github.com/peipielvcm/UDP_RTP_H264/raw/master/ServerClient.png)