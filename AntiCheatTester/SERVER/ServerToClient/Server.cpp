#include "Server.h"
void ServerToClient::Listen()
{
    if (server.is_running())
    {
        // fail
        return;
    }

    std::cout << "Now listening\n";
    std::cout << "Curl up\n";

    server.Post("/", [this](const httplib::Request &req, httplib::Response &res)
        {
            Network::Data d;
            json parsed = json::parse(req.body, nullptr, false);  
            if (parsed.is_discarded()) {
                res.status = PAYLOAD_MALFORMED;
                return;
            }

            std::lock_guard<std::mutex> lock(dataMutex);
            d.body   = std::move(parsed);
            d.ipAddr = req.remote_addr;
            d.port   = static_cast<uint64_t>(req.remote_port);

            // do other stuff
            // handle payload here
            std::cout << "payload is here\n";

            res.status = PAYLOAD_SENT;

        }
    );

    server.listen("0.0.0.0", 8080);
}