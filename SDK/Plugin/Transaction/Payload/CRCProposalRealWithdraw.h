/*
 * Copyright (c) 2020 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ELASTOS_SDK_CRCPROPOSALREALWITHDRAW_H__
#define __ELASTOS_SDK_CRCPROPOSALREALWITHDRAW_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class CRCProposalRealWithdraw : public IPayload {
		public:
			~CRCProposalRealWithdraw();

			CRCProposalRealWithdraw();

		public:
			size_t EstimateSize(uint8_t version) const override;

			void Serialize(ByteStream &stream, uint8_t version) const override;

			bool Deserialize(const ByteStream &stream, uint8_t version) override;

			nlohmann::json ToJson(uint8_t version) const override;

			void FromJson(const nlohmann::json &j, uint8_t version) override;

			bool IsValid(uint8_t version) const override;

			IPayload &operator=(const IPayload &payload) override;

			CRCProposalRealWithdraw &operator=(const CRCProposalRealWithdraw &payload);

			bool Equal(const IPayload &payload, uint8_t version) const override;

		private:
			std::vector<uint256> _withdrawTxHashes;
		};

	}
}

#endif
