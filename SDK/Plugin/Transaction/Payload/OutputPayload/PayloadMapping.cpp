
#include <Common/Log.h>
#include "PayloadMapping.h"

namespace Elastos {
    namespace ElaWallet {

        PayloadMapping::PayloadMapping() : _version(0) {

        }

        PayloadMapping::~PayloadMapping() {

        }

        size_t PayloadMapping::EstimateSize() const {
            size_t s = 0;
            s += 1;
            s += _ownerPubKey.size();
            s += _sideProducerID.size();
            s += _signature.size();

            return s;
        }

        void PayloadMapping::Serialize(ByteStream &stream, bool extend) const {
            stream.WriteUint8(_version);
            stream.WriteVarBytes(_ownerPubKey);
            stream.WriteVarBytes(_sideProducerID);
            stream.WriteVarBytes(_signature);
        }

        bool PayloadMapping::Deserialize(const ByteStream &stream, bool extend) {
            if (!stream.ReadUint8(_version)) {
                Log::error("PayloadMapping deserialize ver err");
                return false;
            }

            if (!stream.ReadVarBytes(_ownerPubKey)) {
                Log::error("PayloadMapping deserialize owner pubkey err");
                return false;
            }

            if (!stream.ReadVarBytes(_sideProducerID)) {
                Log::error("PayloadMapping deserialize side producer id err");
                return false;
            }

            if (!stream.ReadVarBytes(_signature)) {
                Log::error("PayloadMapping deserialize sign err");
                return false;
            }
            return true;
        }

        nlohmann::json PayloadMapping::ToJson() const {
            nlohmann::json j;
            j["Version"] = _version;
            j["OwnerPubKey"] = _ownerPubKey.getHex();
            j["SideProducerID"] = _sideProducerID.getHex();
            j["Signature"] = _signature.getHex();
            return j;
        }

        void PayloadMapping::FromJson(const nlohmann::json &j) {
            _version = j["Version"];
            _ownerPubKey.setHex(j["OwnerPubKey"]);
            _sideProducerID.setHex(j["SideProducerID"]);
            _signature.setHex(j["Signature"]);
        }

        IOutputPayload &PayloadMapping::operator=(const IOutputPayload &payload) {
            try {
                const PayloadMapping &p = dynamic_cast<const PayloadMapping &>(payload);
                operator=(p);
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of PayloadMapping");
            }

            return *this;
        }

        PayloadMapping &PayloadMapping::operator=(const PayloadMapping &payload) {
            _version = payload._version;
            _ownerPubKey = payload._ownerPubKey;
            _sideProducerID = payload._sideProducerID;
            _signature = payload._signature;
            return *this;
        }

        bool PayloadMapping::operator==(const IOutputPayload &payload) const {
            try {
                const PayloadMapping &p = dynamic_cast<const PayloadMapping &>(payload);
                return _version == p._version &&
                       _ownerPubKey == p._ownerPubKey &&
                       _sideProducerID == p._sideProducerID &&
                       _signature == p._signature;
            } catch (const std::bad_cast &e) {
                Log::error("payload is not instance of PayloadMapping");
            }

            return false;
        }

    }
}