#include <Common/Log.h>
#include "PayloadExchangeVotes.h"

namespace Elastos {
    namespace ElaWallet {

        PayloadExchangeVotes::PayloadExchangeVotes() : _version(0) {

        }

        PayloadExchangeVotes::~PayloadExchangeVotes() {

        }

        size_t PayloadExchangeVotes::EstimateSize() const {
            size_t s = 0;
            s += sizeof(_version);
            s += _stakeAddress.size();
            return s;
        }

        void PayloadExchangeVotes::Serialize(ByteStream &stream, bool extend) const {
            stream.WriteUint8(_version);
            stream.WriteBytes(_stakeAddress);
        }

        bool PayloadExchangeVotes::Deserialize(const ByteStream &stream, bool extend) {
            if (!stream.ReadUint8(_version)) {
                Log::error("PayloadExchangeVotes deserialize ver err");
                return false;
            }

            if (!stream.ReadBytes(_stakeAddress)) {
                Log::error("PayloadExchangeVotes deserialize addr err");
                return false;
            }
            return true;
        }

        nlohmann::json PayloadExchangeVotes::ToJson() const {
            nlohmann::json j;
            j["Version"] = _version;
            j["StakeAddress"] = _stakeAddress.GetHex();
            return j;
        }

        void PayloadExchangeVotes::FromJson(const nlohmann::json &j) {
            _version = j["Version"];
            _stakeAddress.SetHex(j["StakeAddress"]);
        }

        IOutputPayload &PayloadExchangeVotes::operator=(const IOutputPayload &payload) {
            try {
                const PayloadExchangeVotes &p = dynamic_cast<const PayloadExchangeVotes &>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of PayloadExchangeVotes");
            }

            return *this;
        }

        PayloadExchangeVotes &PayloadExchangeVotes::operator=(const PayloadExchangeVotes &payload) {
            _version = payload._version;
            _stakeAddress = payload._stakeAddress;
            return *this;
        }

        bool PayloadExchangeVotes::operator==(const IOutputPayload &payload) const {
            try {
                const PayloadExchangeVotes &p = dynamic_cast<const PayloadExchangeVotes &>(payload);
                return _version == p._version &&
                       _stakeAddress == p._stakeAddress;
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of PayloadExchangeVotes");
            }

            return false;
        }

    }
}