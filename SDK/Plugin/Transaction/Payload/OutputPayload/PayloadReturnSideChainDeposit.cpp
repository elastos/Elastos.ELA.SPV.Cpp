
#include <Common/Log.h>
#include "PayloadReturnSideChainDeposit.h"

namespace Elastos {
    namespace ElaWallet {

        PayloadReturnSideChainDeposit::PayloadReturnSideChainDeposit() : _version(0) {

        }

        PayloadReturnSideChainDeposit::~PayloadReturnSideChainDeposit() {

        }

        size_t PayloadReturnSideChainDeposit::EstimateSize() const {
            size_t s = 0;
            s += sizeof(_version);
            s += _genesisBlockAddress.size();
            s += _depositTransactionHash.size();
            return s;
        }

        void PayloadReturnSideChainDeposit::Serialize(ByteStream &stream, bool extend) const {
            stream.WriteUint8(_version);
            stream.WriteVarString(_genesisBlockAddress);
            stream.WriteBytes(_depositTransactionHash);
        }

        bool PayloadReturnSideChainDeposit::Deserialize(const ByteStream &stream, bool extend) {
            if (!stream.ReadUint8(_version)) {
                Log::error("PayloadReturnSideChainDeposit deserialize ver err");
                return false;
            }

            if (!stream.ReadVarString(_genesisBlockAddress)) {
                Log::error("PayloadReturnSideChainDeposit deserialize addr err");
                return false;
            }

            if (!stream.ReadBytes(_depositTransactionHash)) {
                Log::error("PayloadReturnSideChainDeposit deserialize hash err");
                return false;
            }

            return true;
        }

        nlohmann::json PayloadReturnSideChainDeposit::ToJson() const {
            nlohmann::json j;
            j["Version"] = _version;
            j["GenesisBlockAddress"] = _genesisBlockAddress;
            j["DepositTransactionHash"] = _depositTransactionHash.GetHex();
            return j;
        }

        void PayloadReturnSideChainDeposit::FromJson(const nlohmann::json &j) {
            _version = j["Version"];
            _genesisBlockAddress = j["GenesisBlockAddress"];
            _depositTransactionHash.SetHex(j["DepositTransactionHash"]);
        }

        IOutputPayload &PayloadReturnSideChainDeposit::operator=(const IOutputPayload &payload) {
            try {
                const PayloadReturnSideChainDeposit &p = dynamic_cast<const PayloadReturnSideChainDeposit &>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of PayloadReturnSideChainDeposit");
            }

            return *this;
        }

        PayloadReturnSideChainDeposit &PayloadReturnSideChainDeposit::operator=(const PayloadReturnSideChainDeposit &payload) {
            _version = payload._version;
            _genesisBlockAddress = payload._genesisBlockAddress;
            _depositTransactionHash = payload._depositTransactionHash;
            return *this;
        }

        bool PayloadReturnSideChainDeposit::operator==(const IOutputPayload &payload) const {
            try {
                const PayloadReturnSideChainDeposit &p = dynamic_cast<const PayloadReturnSideChainDeposit &>(payload);
                return _version == p._version &&
                       _genesisBlockAddress == p._genesisBlockAddress &&
                       _depositTransactionHash == p._depositTransactionHash;
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of PayloadReturnSideChainDeposit");
            }

            return false;
        }

    }
}