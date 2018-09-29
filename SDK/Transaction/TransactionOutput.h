// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONOUTPUT_H__
#define __ELASTOS_SDK_TRANSACTIONOUTPUT_H__

#include <boost/shared_ptr.hpp>

#include "BRInt.h"
#include "CMemBlock.h"
#include "Plugin/Interface/ELAMessageSerializable.h"

namespace Elastos {
	namespace ElaWallet {

		class TransactionOutput :
				public ELAMessageSerializable {

		public:
			TransactionOutput();

			TransactionOutput(const TransactionOutput &output);

			TransactionOutput(uint64_t amount, const CMBlock &script, int signType);

			~TransactionOutput();

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			std::string getAddress() const;

			void setAddress(const std::string &address);

			void setAddressSignType(int signType);

			int getAddressSignType() const;

			uint64_t getAmount() const;

			void setAmount(uint64_t amount);

			const CMBlock &getScript() const;

			const UInt256 &getAssetId() const;

			void setAssetId(const UInt256 &assetId);

			uint32_t getOutputLock() const;

			void setOutputLock(uint32_t outputLock);

			const UInt168 &getProgramHash() const;

			void setProgramHash(const UInt168 &hash);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

			void SetScript(const CMBlock &script, int signType);

			size_t GetSize() const;

		private:
			char _address[75];
			uint64_t _amount;
			CMBlock _script;
			UInt256 _assetId;
			uint32_t _outputLock;
			UInt168 _programHash;
			int _signType;
		};

		typedef boost::shared_ptr<TransactionOutput> TransactionOutputPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONOUTPUT_H__
