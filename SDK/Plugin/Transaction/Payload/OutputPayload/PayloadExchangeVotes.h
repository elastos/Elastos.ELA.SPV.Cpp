
#ifndef __ELASTOS_SPVSDK_PAYLOADEXCHANGEVOTES_H
#define __ELASTOS_SPVSDK_PAYLOADEXCHANGEVOTES_H

#include <Plugin/Transaction/Payload/OutputPayload/IOutputPayload.h>

namespace Elastos {
    namespace ElaWallet {

        class PayloadExchangeVotes : public IOutputPayload {
        public:
            PayloadExchangeVotes();

            ~PayloadExchangeVotes();

            virtual size_t EstimateSize() const;

            virtual void Serialize(ByteStream &stream, bool extend = false) const;

            virtual bool Deserialize(const ByteStream &stream, bool extend = false);

            virtual nlohmann::json ToJson() const;

            virtual void FromJson(const nlohmann::json &j);

            virtual IOutputPayload &operator=(const IOutputPayload &payload);

            virtual PayloadExchangeVotes &operator=(const PayloadExchangeVotes &payload);

            virtual bool operator==(const IOutputPayload &payload) const;

        private:
            uint8_t _version;
            uint168 _stakeAddress;
        };

    }
}


#endif
