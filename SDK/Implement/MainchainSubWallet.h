// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MAINCHAINSUBWALLET_H__
#define __ELASTOS_SDK_MAINCHAINSUBWALLET_H__

#include "SubWallet.h"
#include <Interface/IMainchainSubWallet.h>

namespace Elastos {
	namespace ElaWallet {

		class IOutputPayload;
		typedef boost::shared_ptr<IOutputPayload> OutputPayloadPtr;

		class MainchainSubWallet : public IMainchainSubWallet, public SubWallet {
		public:
			~MainchainSubWallet();

			virtual nlohmann::json CreateDepositTransaction(
					const std::string &fromAddress,
					const std::string &lockedAddress,
					const std::string &amount,
					const std::string &sideChainAddress,
					const std::string &memo);

			virtual nlohmann::json GenerateProducerPayload(
				const std::string &ownerPublicKey,
				const std::string &nodePublicKey,
				const std::string &nickName,
				const std::string &url,
				const std::string &ipAddress,
				uint64_t location,
				const std::string &payPasswd) const;

			virtual nlohmann::json GenerateCancelProducerPayload(
				const std::string &ownerPublicKey,
				const std::string &payPasswd) const;

			virtual nlohmann::json CreateRegisterProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &amount,
				const std::string &memo);

			virtual nlohmann::json CreateUpdateProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo);

			virtual nlohmann::json CreateCancelProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &emmo);

			virtual nlohmann::json CreateRetrieveDepositTransaction(
				const std::string &amount,
				const std::string &memo);

			virtual std::string GetOwnerPublicKey() const;

			virtual std::string GetOwnerAddress() const;

			virtual nlohmann::json CreateVoteProducerTransaction(
					const std::string &fromAddress,
					const std::string &stake,
					const nlohmann::json &publicKeys,
					const std::string &memo);

			virtual	nlohmann::json GetVotedProducerList() const;

			virtual nlohmann::json GetRegisteredProducerInfo() const;

			virtual std::string GetCROwnerDID() const;

			virtual std::string GetCROwnerPublicKey() const;

			virtual nlohmann::json GenerateCRInfoPayload(
					const std::string &crPublicKey,
					const std::string &nickName,
					const std::string &url,
					uint64_t location,
					const std::string &payPasswd) const;

			virtual nlohmann::json GenerateUnregisterCRPayload(
					const std::string &crPublicKey,
					const std::string &payPasswd) const;

			virtual nlohmann::json CreateRegisterCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &amount,
					const std::string &memo);

			virtual nlohmann::json CreateUpdateCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &memo);

			virtual nlohmann::json CreateUnregisterCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &memo);

			virtual nlohmann::json CreateRetrieveCRDepositTransaction(
					const std::string &amount,
					const std::string &memo);

			virtual nlohmann::json CreateVoteCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &votes,
				const std::string &memo);

			virtual	nlohmann::json GetVotedCRList() const;

			virtual nlohmann::json GetRegisteredCRInfo() const;

			virtual nlohmann::json GetVoteInfo(const std::string &type) const;

			virtual nlohmann::json SponsorSignProposal(uint8_t type,
			                                           const std::string &sponsorPublicKey,
			                                           const std::string &crSponsorDID,
			                                           const std::string &draftHash,
			                                           const std::vector<std::string> &budgets,
			                                           const std::string &recipient,
			                                           const std::string &payPasswd) const;

			virtual nlohmann::json CRSponsorSignProposal(uint8_t type,
			                                             const std::string &sponsorPublicKey,
			                                             const std::string &crSponsorDID,
			                                             const std::string &draftHash,
			                                             const std::vector<std::string> &budgets,
			                                             const std::string &recipient,
			                                             const std::string &signature,
			                                             const std::string &payPasswd) const;

			virtual nlohmann::json CreateCRCProposalTransaction(const nlohmann::json &proposal,
			                                                    const std::string &memo);

			virtual nlohmann::json GenerateCRCProposalReview(const std::string &proposalHash,
			                                                 uint8_t voteResult,
			                                                 const std::string &crDID,
			                                                 const std::string &payPasswd) const;

			virtual nlohmann::json CreateCRCProposalReviewTransaction(const nlohmann::json &proposalReview,
			                                                          const std::string &memo);

		protected:
			friend class MasterWallet;

			MainchainSubWallet(const CoinInfoPtr &info,
							   const ChainConfigPtr &config,
							   MasterWallet *parent);

			TransactionPtr CreateVoteTx(const VoteContent &voteContent, const std::string &memo, bool max);
		};

	}
}

#endif //__ELASTOS_SDK_MAINCHAINSUBWALLET_H__
