#ifndef __ELASTOS_SDK_OUTPUT_PAYLOADMAPPING_H
#define __ELASTOS_SDK_OUTPUT_PAYLOADMAPPING_H

#include <Plugin/Transaction/Payload/OutputPayload/IOutputPayload.h>

namespace Elastos {
    namespace ElaWallet {

        class PayloadMapping : public IOutputPayload {
        public:
            PayloadMapping();

            ~PayloadMapping();

            virtual size_t EstimateSize() const;

            virtual void Serialize(ByteStream &stream, bool extend = false) const;

            virtual bool Deserialize(const ByteStream &stream, bool extend = false);

            virtual nlohmann::json ToJson() const;

            virtual void FromJson(const nlohmann::json &j);

            virtual IOutputPayload &operator=(const IOutputPayload &payload);

            virtual PayloadMapping &operator=(const PayloadMapping &payload);

            virtual bool operator==(const IOutputPayload &payload) const;

        private:
            uint8_t _version;
            bytes_t _ownerPubKey;
            bytes_t _sideProducerID;
            bytes_t _signature;
        };

    }
}

#endif
