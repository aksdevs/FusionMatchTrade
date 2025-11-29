#include <iostream>
#include <string>

#include "forex.pb.h"

int main()
{
    forex::ForexRequest req;
    req.set_from_currency("USD");
    req.set_to_currency("EUR");

    std::string out;
    if (!req.SerializeToString(&out))
    {
        std::cerr << "Failed to serialize ForexRequest" << std::endl;
        return 2;
    }

    forex::ForexRequest parsed;
    if (!parsed.ParseFromString(out))
    {
        std::cerr << "Failed to parse ForexRequest" << std::endl;
        return 3;
    }

    if (parsed.from_currency() != "USD" || parsed.to_currency() != "EUR")
    {
        std::cerr << "Parsed message mismatch" << std::endl;
        return 4;
    }

    // Test response message
    forex::ForexResponse resp;
    resp.set_rate(1.2345);
    resp.set_timestamp("2025-11-28T00:00:00Z");

    std::string out2;
    if (!resp.SerializeToString(&out2))
    {
        std::cerr << "Failed to serialize ForexResponse" << std::endl;
        return 5;
    }

    forex::ForexResponse parsedResp;
    if (!parsedResp.ParseFromString(out2))
    {
        std::cerr << "Failed to parse ForexResponse" << std::endl;
        return 6;
    }

    if (parsedResp.rate() != 1.2345)
    {
        std::cerr << "Parsed response rate mismatch" << std::endl;
        return 7;
    }

    std::cout << "forex proto test passed" << std::endl;
    return 0;
}
