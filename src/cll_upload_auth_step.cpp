#include "cll_upload_auth_step.h"
#include "xbox_live_helper.h"
#include <cll/event_batch.h>

void CllUploadAuthStep::setAccount(std::string const& cid) {
    std::lock_guard<std::recursive_mutex> lock (mutex);
    this->cid = cid;
    msaToken = std::string();
    xToken = std::string();
}

void CllUploadAuthStep::refreshTokens(bool force) {
    std::lock_guard<std::recursive_mutex> lock (mutex);
    if (msaToken.empty() || force)
        msaToken = XboxLiveHelper::getInstance().getCllMsaToken(cid);
    if (xToken.empty() || force)
        xToken = XboxLiveHelper::getCllXToken(force);
}

void CllUploadAuthStep::onRequest(cll::EventUploadRequest& request) {
    std::lock_guard<std::recursive_mutex> lock (mutex);
    refreshTokens(false);
    if (!msaToken.empty())
        request.headers.emplace_back("X-AuthMsaDeviceTicket", msaToken);
    if (!xToken.empty())
        request.headers.emplace_back("X-AuthXToken", xToken);
    std::map<std::string, std::string> tickets;
    char const* ptr = request.batch.getData();
    char const* end = ptr + request.batch.getDataSize();
    while (true) {
        char const* e = (char const*) memchr(ptr, '\n', end - ptr);
        if (e == nullptr)
            break;
        nlohmann::json event = nlohmann::json::parse(nlohmann::detail::input_adapter(ptr, e - ptr - 1));
        for (auto const& ticket : event["ext"]["android"]["tickets"]) {
            if (tickets.count(ticket) == 0) {
                auto tk = XboxLiveHelper::getCllXTicket(ticket);
                if (!tk.empty())
                    tickets.insert({ticket, "x:" + tk});
            }
        }
        ptr = e + 1;
    }
    if (!tickets.empty()) {
        std::stringstream ss;
        bool f = true;
        for (auto const& ticket : tickets) {
            if (!f)
                ss << ';';
            ss << '"' << ticket.first << "\"=\"" << ticket.second << '"';
            f = false;
        }
        request.headers.emplace_back("X-Tickets", ss.str());
    }
}

bool CllUploadAuthStep::onAuthenticationFailed() {
    refreshTokens(true);
    return true;
}