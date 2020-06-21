#pragma once

#include <common/singleton.h>
#include <map>

class Protocol;
class Server;
class App : public Singleton<App>{
public:
    App();
    ~App();

public:
    void    add_server(short port, Protocol* protocol);
    Server* get_server(short port);

private:
    std::map<short, Server*>  m_mapServers;

};
