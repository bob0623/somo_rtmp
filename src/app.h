#pragma once

#include <common/singleton.h>
#include <map>
#include <string>

#define PROTOCOL_SOMO_UDP       1
#define PROTOCOL_RTMP           2
#define PROTOCOL_RTP            3

/**
 * Session belongs to a Server, but Sessions can talk cross Server. Ex: packets from rtmp Session can be forward to rtmp Session.
 * 
 * So App need to hold Server and Session seperatly!
 * 
 */
class Protocol;
class Server;
class Client;
class Session;
class App : public Singleton<App>{
public:
    App();
    ~App();

public:
    void    add_protocol(int proto);
    Protocol*   get_protocol(int proto);

    void    add_server(short port, int proto);
    void    add_server(short port, Protocol* protocol);
    Server* get_server(short port);
    void    clear_servers();

    Session* add_session(const std::string& stream, int proto);
    Session* add_session(const std::string& stream, Protocol* protocol);
    Session* get_session(const std::string& stream);
    void    clear_sessions();

    Client* add_client(const std::string& url, bool player);
    Client* add_forwarder(const std::string& url);
    void    clear_forwarders();

private:
    std::map<int, Protocol*>  m_mapProtocols;
    std::map<short, Server*>  m_mapServers;
    std::map<std::string, Session*>     m_mapSessions;
    std::map<std::string, Client*>      m_mapClients;
};
