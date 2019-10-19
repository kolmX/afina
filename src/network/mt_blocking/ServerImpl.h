#ifndef AFINA_NETWORK_MT_BLOCKING_SERVER_H
#define AFINA_NETWORK_MT_BLOCKING_SERVER_H

#include <atomic>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>
#include "afina/concurrency/Executor.h"
#include "afina/network/Server.h"

namespace spdlog {
class logger;
}

namespace Afina {
namespace Network {
namespace MTblocking {

/**
 * # Network resource manager implementation
 * Server that is spawning a separate thread for each connection
 */
class ServerImpl : public Server {
public:
    ServerImpl(std::shared_ptr<Afina::Storage> ps, std::shared_ptr<Logging::Service> pl);
    ~ServerImpl();

    // See Server.h
    void Start(uint16_t port, uint32_t, uint32_t) override;

    // See Server.h
    void Stop() override;

    // See Server.h
    void Join() override;

protected:
    /**
     * Method is running in the connection acceptor thread
     */
    void OnRun(const uint32_t n_workers);

private:

    enum class ConnectionState{
        idle,

        waitArgs,

        executing
    };

    void handleConnection(std::map<const int, ConnectionState>::iterator it);

    //Thread pool
    Afina::Concurrency::Executor executor;
    // Logger instance
    std::shared_ptr<spdlog::logger> _logger;

    // Atomic flag to notify threads when it is time to stop. Note that
    // flag must be atomic in order to safely publisj changes cross thread
    // bound

    std::atomic<bool> running;
    std::atomic<int> connections;
    // Server socket to accept connections on
    int _server_socket;
    std::map<const int, ConnectionState> active_clients_state;

    // Thread to run network on
    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cv;


    std::mutex _connections_mutex;

    std::mutex _state_mutex;
    std::condition_variable state_cv;

};

} // namespace MTblocking
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_MT_BLOCKING_SERVER_H
