
#include <Common/Log.h>
#include "PayloadWithdraw.h"


namespace Elastos {
    namespace ElaWallet {

        PayloadWithdraw::PayloadWithdraw() : _version(0) {

        }

        PayloadWithdraw::~PayloadWithdraw() {

        }

        size_t PayloadWithdraw::EstimateSize() const {
            size_t s = 0;
            s += sizeof(_version);
            s += _genesisBlockAddress.size();
            s += _sideChainTransactionHash.size();
            s += _targetData.size();
            return s;
        }

        void PayloadWithdraw::Serialize(ByteStream &stream, bool extend) const {
            stream.WriteUint8(_version);
            stream.WriteVarString(_genesisBlockAddress);
            stream.WriteBytes(_sideChainTransactionHash);
            stream.WriteVarBytes(_targetData);
        }

        bool PayloadWithdraw::Deserialize(const ByteStream &stream, bool extend) {
            if (!stream.ReadUint8(_version)) {
                Log::error("PayloadWithdraw deserialize ver err");
                return false;
            }

            if (!stream.ReadVarString(_genesisBlockAddress)) {
                Log::error("PayloadWithdraw deserialize addr err");
                return false;
            }

            if (!stream.ReadBytes(_sideChainTransactionHash)) {
                Log::error("PayloadWithdraw deserialize hash err");
                return false;
            }

            if (!stream.ReadVarBytes(_targetData)) {
                Log::error("PayloadWithdraw deserialize data err");
                return false;
            }
            return true;
        }

        nlohmann::json PayloadWithdraw::ToJson() const {
            nlohmann::json j;
            j["Version"] = _version;
            j["GenesisBlockAddress"] = _genesisBlockAddress;
            j["SideChainTransactionHash"] = _sideChainTransactionHash.GetHex();
            j["TargetData"] = _targetData.getHex();
            return j;
        }

        void PayloadWithdraw::FromJson(const nlohmann::json &j) {
            _version = j["Version"];
            _genesisBlockAddress = j["GenesisBlockAddress"];
            _sideChainTransactionHash.SetHex(j["SideChainTransactionHash"]);
            _targetData.setHex(j["TargetData"]);
        }

        IOutputPayload &PayloadWithdraw::operator=(const IOutputPayload &payload) {
            try {
                const PayloadWithdraw &p = dynamic_cast<const PayloadWithdraw &>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of PayloadWithdraw");
            }

            return *this;
        }

        PayloadWithdraw &PayloadWithdraw::operator=(const PayloadWithdraw &payload) {
            _version = payload._version;
            _genesisBlockAddress = payload._genesisBlockAddress;
            _sideChainTransactionHash = payload._sideChainTransactionHash;
            _targetData = payload._targetData;
            return *this;
        }

        bool PayloadWithdraw::operator==(const IOutputPayload &payload) const {
            try {
                const PayloadWithdraw &p = dynamic_cast<const PayloadWithdraw &>(payload);
                return _version == p._version &&
                       _genesisBlockAddress == p._genesisBlockAddress &&
                       _sideChainTransactionHash == p._sideChainTransactionHash &&
                       _targetData == p._targetData;
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of PayloadWithdraw");
            }

            return false;
        }

    }
}