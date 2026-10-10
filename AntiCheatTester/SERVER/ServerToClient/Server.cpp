#include "Server.h"

void ServerToClient::Listen()
{
    if (server.is_running())
    {
        // fail
        return;
    }

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


            res.status = PAYLOAD_SENT;

        }
    );

    server.listen("0.0.0.0", 8080);
}