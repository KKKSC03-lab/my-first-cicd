#include<iostream>
#include<event2/event.h>
#include<signal.h>
#include<cstring>
using namespace std;
#define SPORT 5002

void client_cb(evutil_socket_t sock, short which, void *arg)
{
    cout<<"client_cb"<<endl;
    event *ev = (event *)arg;
    if(which & EV_TIMEOUT)
    {
        cout<<"timeout"<<endl;
        cout<<"event_free"<<endl;
        event_free(ev);
        evutil_closesocket(sock);
        return;
    }
    char buf[1024] = {0};
    int len = recv(sock, buf, sizeof(buf) - 1, 0);
    if(len > 0){
        cout<<buf<<endl;
        send(sock, "ok", 2, 0);
    }
    else{
        cout<<"."<<flush;
        event_free(ev);
        evutil_closesocket(sock);
    }
}

void listen_cb(evutil_socket_t sock, short which, void *arg)
{
    cout<<"listen_cb"<<endl;
    //读取连接信息
    sockaddr_in sin;
    socklen_t len = sizeof(sin);
    evutil_socket_t client = accept(sock, (sockaddr *)&sin, &len);
    char ip[17];
    evutil_inet_ntop(AF_INET, &sin.sin_addr, ip, sizeof(ip) - 1);
    cout<<"client ip is "<<ip<<endl;
    //客户端数据读取事件
    event_base *base = (event_base *)arg;
    event *ev = event_new(base, client, EV_READ|EV_PERSIST|EV_ET, client_cb, base);
    timeval time = {10, 0};
    event_add(ev, &time);
    return;
}

int main()
{
#ifdef win32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#else
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return -1;
#endif
    //定义event_base
    event_base *base = event_base_new();
    cout<<"event server start"<<endl;

    //定义socket
    evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        cout<<"socket error:"<<strerror(errno)<<endl;
        return -1;
    }

    //socket设置非阻塞和地址复用
    evutil_make_listen_socket_reuseable(sock);
    evutil_make_socket_nonblocking(sock);

    //绑定socket地址
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SPORT);
    int re = bind(sock, (sockaddr *)&sin, sizeof(sin));
    if(re < 0)
    {
        cerr<<"bind error"<<strerror(errno)<<endl;
        return -1;
    }
    //开始监听
    listen(sock, 10);
    //开始接受连接事件
    event *ev = event_new(base, sock, EV_READ|EV_PERSIST, listen_cb, base);
    event_add(ev, 0);
    //进入事件主循环
    event_base_dispatch(base);
    evutil_closesocket(sock);
    event_base_free(base);
    return 0;
}