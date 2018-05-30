#pragma once

#include <memory>
#include <msa/client/service_launcher.h>
#include <msa/client/service_client.h>
#include <minecraft/Xbox.h>

class XboxLiveHelper {

private:
    static std::string const MSA_CLIENT_ID;
    static std::string const MSA_COBRAND_ID;

    msa::client::ServiceLauncher launcher;
    msa::client::ServiceClient client;

public:
    static XboxLiveHelper& getInstance() {
        static XboxLiveHelper instance;
        return instance;
    }

    XboxLiveHelper() : launcher(std::string()), client(launcher) {
    }

    void invokeMsaAuthFlow(std::function<void (std::string const& cid, std::string const& binaryToken)> success_cb,
                           std::function<void (simpleipc::rpc_error_code, std::string const&)> error_cb);

    xbox::services::xbox_live_result<xbox::services::system::token_and_signature_result> invokeXblLogin(
            std::string const& cid, std::string const& binaryToken);

    xbox::services::xbox_live_result<xbox::services::system::token_and_signature_result> invokeEventInit();


    simpleipc::client::rpc_call<std::shared_ptr<msa::client::Token>> requestXblToken(std::string const& cid,
                                                                                     bool silent);

    void requestXblToken(std::string const& cid, bool silent,
                         std::function<void (std::string const& cid, std::string const& binaryToken)> success_cb,
                         std::function<void (simpleipc::rpc_error_code, std::string const&)> error_cb);

};