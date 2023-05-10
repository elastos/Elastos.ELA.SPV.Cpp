#ifndef __ELASTOS_SPVSDK_PAYLOADWITHDRAW_H
#define __ELASTOS_SPVSDK_PAYLOADWITHDRAW_H

#include <Plugin/Transaction/Payload/OutputPayload/IOutputPayload.h>

namespace Elastos {
    namespace ElaWallet {

        class PayloadWithdraw : public IOutputPayload {
        public:
            PayloadWithdraw();

            ~PayloadWithdraw();

            virtual size_t EstimateSize() const;

            virtual void Serialize(ByteStream &stream, bool extend = false) const;

            virtual bool Deserialize(const ByteStream &stream, bool extend = false);

            virtual nlohmann::json ToJson() const;

            virtual void FromJson(const nlohmann::json &j);

            virtual IOutputPayload &operator=(const IOutputPayload &payload);

            virtual PayloadWithdraw &operator=(const PayloadWithdraw &payload);

            virtual bool operator==(const IOutputPayload &payload) const;

        private:
            uint8_t _version;
            std::string _genesisBlockAddress;
            uint256 _sideChainTransactionHash;
            bytes_t _targetData;
        };

    }
}

#endif
