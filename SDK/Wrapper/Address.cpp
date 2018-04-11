// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRAddress.h>
#include "Address.h"

namespace Elastos {
    namespace SDK {

        Address::Address(std::string address) {
            _address = nullptr;

            if (address.empty()) {
                return;
            }

            _address = boost::shared_ptr<BRAddress>(new BRAddress());
            size_t stringLen = address.size();
            size_t stringLenMax = sizeof(_address->s) - 1;

             if (stringLen > stringLenMax) {
                 stringLen = stringLenMax;
             }

            const char* stringChars = address.c_str();
            memcpy(_address->s, stringChars, stringLen);
        }

        Address::Address(boost::shared_ptr<BRAddress> address) {
            this->_address = address;
        }

        Address *Address::createAddress(std::string address) {
            Address* addr = nullptr;
            if (!address.empty()) {
                addr = new Address(address);
            }
            return addr;
        }

        Address *Address::fromScriptPubKey(ByteData script) {
            //todo complete me

            boost::shared_ptr<BRAddress> address = boost::shared_ptr<BRAddress>(new BRAddress());

            BRAddressFromScriptPubKey(address->s, sizeof(address->s), script.data, script.length);

            Address* addr =  new Address(address);
            return addr;
        }

        Address *Address::fromScriptSignature(ByteData script) {
            boost::shared_ptr<BRAddress> address = boost::shared_ptr<BRAddress>(new BRAddress());

            size_t scriptLen = script.length;
            BRAddressFromScriptSig(address->s, sizeof(address->s), script.data, scriptLen);

            printf("%s",address->s);
            Address* addr =  new Address(address);

            return addr;
        }

        std::string Address::stringify() const {
            //todo complete me
            BRAddress* address = getRaw();
            return address->s;
        }

        bool Address::isValid() {
            BRAddress* address = getRaw();
            bool res = BRAddressIsValid(address->s) ? true : false;
            return res;
        }

        ByteData Address::getPubKeyScript() {
            BRAddress* address = getRaw();

            size_t pubKeyLen = BRAddressScriptPubKey(NULL, 0, address->s);
            uint8_t pubKey[pubKeyLen];
            BRAddressScriptPubKey(pubKey, pubKeyLen, address->s);

            ByteData result(pubKey,pubKeyLen);
            return result;
        }
        std::string Address::toString() const {
            return stringify();
        }

        BRAddress* Address::getRaw() const {
            return _address.get();
        }
    }
}