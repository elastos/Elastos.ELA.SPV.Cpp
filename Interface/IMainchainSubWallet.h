// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <ISubWallet.h>

#ifndef __ELASTOS_SDK_IMAINCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IMAINCHAINSUBWALLET_H__

namespace Elastos {
	namespace ElaWallet {

		class IMainchainSubWallet : public virtual ISubWallet {
		public:
			/**
			 * Virtual destructor.
			 */
			virtual ~IMainchainSubWallet() noexcept {}

			/**
			 * Deposit token from the main chain to side chains, such as ID chain or token chain, etc.
			 *
			 * @param fromAddress      If this address is empty, wallet will pick available UTXO automatically.
			 *                         Otherwise, wallet will pick UTXO from the specific address.
			 * @param sideChainID      Chain id of the side chain.
			 * @param amount           The amount that will be deposit to the side chain.
			 * @param sideChainAddress Receive address of side chain.
			 * @memo                   Remarks string. Can be empty string.
			 * @return                 The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateDepositTransaction(
					const std::string &fromAddress,
					const std::string &sideChainID,
					const std::string &amount,
					const std::string &sideChainAddress,
					const std::string &memo) = 0;

			/**
			 * Generate payload for registering or updating producer.
			 *
			 * @param ownerPublicKey The public key to identify a producer. Can't change later. The producer reward will
			 *                       be sent to address of this public key.
			 * @param nodePublicKey  The public key to identify a node. Can be update
			 *                       by CreateUpdateProducerTransaction().
			 * @param nickName       Nickname of producer.
			 * @param url            URL of producer.
			 * @param ipAddress      IP address of node. This argument is deprecated.
			 * @param location       Location code.
			 * @param payPasswd      Pay password is using for signing the payload with the owner private key.
			 *
			 * @return               The payload in JSON format.
			 */
			virtual nlohmann::json GenerateProducerPayload(
				const std::string &publicKey,
				const std::string &nodePublicKey,
				const std::string &nickName,
				const std::string &url,
				const std::string &ipAddress,
				uint64_t location,
				const std::string &payPasswd) const = 0;

			/**
			 * Generate payaload for unregistering producer.
			 *
			 * @param ownerPublicKey The public key to identify a producer.
			 * @param payPasswd      Pay password is using for signing the payload with the owner private key.
			 *
			 * @return               The payload in JSON format.
			 */
			virtual nlohmann::json GenerateCancelProducerPayload(
				const std::string &publicKey,
				const std::string &payPasswd) const = 0;

			/**
			 * Create register producer transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payload      Generate by GenerateProducerPayload().
			 * @param amount       Amount must lager than 500,000,000,000 sela
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRegisterProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &amount,
				const std::string &memo) = 0;

			/**
			 * Create update producer transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payload      Generate by GenerateProducerPayload().
			 * @param memo         Remarks string. Can be empty string.
			 *
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateUpdateProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo) = 0;

			/**
			 * Create cancel producer transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payload      Generate by GenerateCancelProducerPayload().
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateCancelProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo) = 0;

			/**
			 * Create retrieve deposit transaction.
			 *
			 * @param amount     The available amount to be retrieved back.
			 * @param memo       Remarks string. Can be empty string.
			 *
			 * @return           The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRetrieveDepositTransaction(
				const std::string &amount,
				const std::string &memo) = 0;

			/**
			 * Get owner public key.
			 *
			 * @return Owner public key.
			 */
			virtual std::string GetOwnerPublicKey() const = 0;

			/**
			 * Get address of owner public key
			 * @return Address of owner public key
			 */
			virtual std::string GetOwnerAddress() const = 0;

			/**
			 * Create vote transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param stake        Vote amount in sela. "-1" means max.
			 * @param publicKeys   Public keys array in JSON format.
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateVoteProducerTransaction(
					const std::string &fromAddress,
					const std::string &stake,
					const nlohmann::json &pubicKeys,
					const std::string &memo) = 0;

			/**
			 * Get vote information of current wallet.
			 *
			 * @return Vote information in JSON format. The key is the public key, and the value is the stake. Such as:
			 * {
			 * 	 "02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5": "10000000",
			 * 	 "02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D": "200000000",
			 * 	 "03E5B45B44BB1E2406C55B7DD84B727FAD608BA7B7C11A9C5FFBFEE60E427BD1DA": "5000000000"
			 * }
			 */
			virtual	nlohmann::json GetVotedProducerList() const = 0;

			/**
			 * Get information about whether the current wallet has been registered the producer.
			 *
			 * @return Information in JSON format. Such as:
			 * { "Status": "Unregistered", "Info": null }
			 *
			 * {
			 *    "Status": "Registered",
			 *    "Info": {
			 *      "OwnerPublicKey": "02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D",
			 *      "NodePublicKey": "02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5",
			 *      "NickName": "hello nickname",
			 *      "URL": "www.google.com",
			 *      "Location": 86,
			 *      "Address": 127.0.0.1,
			 *    }
			 * }
			 *
			 * { "Status": "Canceled", "Info": { "Confirms": 2016 } }
			 *
			 * { "Status": "ReturnDeposit", "Info": null }
			 */
			virtual nlohmann::json GetRegisteredProducerInfo() const = 0;


			/**
			 * Generate payload for registering or updating cr.
			 *
			 * @param crPublicKey    The public key to identify a cr. Can't change later.
			 * @param nickName       Nickname of cr.
			 * @param url            URL of cr.
			 * @param location       Location code.
			 * @param payPasswd      Pay password is using for signing the payload with the owner private key.
			 *
			 * @return               The payload in JSON format.
			 */
			virtual nlohmann::json GenerateCRInfoPayload(
					const std::string &crPublicKey,
					const std::string &nickName,
					const std::string &url,
					uint64_t location,
					const std::string &payPasswd) const = 0;

			/**
			 * Get cr owner DID.
			 *
			 * @return Owner cr DID.
			 */
			virtual std::string GetCROwnerDID() const = 0;

			/**
			 * Get cr owner public key.
			 *
			 * @return Owner cr public key.
			 */
			virtual std::string GetCROwnerPublicKey() const = 0;

			/**
			 * Generate payload for unregister or updating cr.
			 *
			 * @param crDID          The id of cr will unregister
			 * @param payPasswd      Pay password is using for signing the payload with the owner private key.
			 *
			 * @return               The payload in JSON format.
			 */
			virtual nlohmann::json GenerateUnregisterCRPayload(
					const std::string &crDID,
					const std::string &payPasswd) const = 0;

			/**
			 * Create register cr transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payloadJSON  Generate by GenerateCRInfoPayload().
			 * @param amount       Amount must lager than 500,000,000,000 sela
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRegisterCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &amount,
					const std::string &memo) = 0;

			/**
			 * Create update cr transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payloadJSON  Generate by GenerateCRInfoPayload().
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateUpdateCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &memo) = 0;

			/**
			 * Create unregister cr transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payloadJSON  Generate by GenerateUnregisterCRPayload().
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateUnregisterCRTransaction(
					const std::string &fromAddress,
					const nlohmann::json &payloadJSON,
					const std::string &memo) = 0;

			/**
			 * Create retrieve deposit cr transaction.
			 *
			 * @param amount     The available amount to be retrieved back.
			 * @param memo       Remarks string. Can be empty string.
			 * @return           The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRetrieveCRDepositTransaction(
					const std::string &amount,
					const std::string &memo) = 0;

			/**
			 * Create vote cr transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param votes        Candidate code and votes in JSON format. Such as:
			 *                     {
			 *                          "iYMVuGs1FscpgmghSzg243R6PzPiszrgj7": "100000000",
			 *                          "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP": "200000000"
			 *                     }
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateVoteCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &votes,
				const std::string &memo) = 0;

			/**
			 * Get CR vote information of current wallet.
			 *
			 * @return Vote information in JSON format. The key is the public key, and the value is the stake. Such as:
			 * {
			 * 	 "iYMVuGs1FscpgmghSzg243R6PzPiszrgj7": "10000000",
			 * 	 "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP": "200000000"
			 * }
			 */
			virtual	nlohmann::json GetVotedCRList() const = 0;

			/**
			 * Get information about whether the current wallet has been registered the producer.
			 *
			 * @return Information in JSON format. Such as:
			 * { "Status": "Unregistered", "Info": null }
			 *
			 * {
			 *    "Status": "Registered",
			 *    "Info": {
			 *      "CROwnerPublicKey": "02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D",
			 *      "CROwnerDID": "iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP",
			 *      "NickName": "hello nickname",
			 *      "URL": "www.google.com",
			 *      "Location": 86,
			 *    }
			 * }
			 *
			 * { "Status": "Canceled", "Info": { "Confirms": 2016 } }
			 *
			 * { "Status": "ReturnDeposit", "Info": null }
			 */
			virtual nlohmann::json GetRegisteredCRInfo() const = 0;

			/**
			 * Get summary or details of all types of votes
			 * @type if the type is empty, a summary of all types of votes will return. Otherwise, the details of the specified type will return.
			 * @return vote info in JSON format. Such as:
			 *
			 * summary:
			 *  [
			 *      {"Type": "Delegate", "Amount": "12345", "Timestamp": 1560888482, "Expiry": null},
			 *      {"Type": "CRC", "Amount": "56789", "Timestamp": 1560888482, "Expiry": 1561888000}
			 *  ]
			 *
			 * details:
			 *  [{
			 *      "Type": "Delegate",
			 *      "Amount": "200000000",
			 *      "Timestamp": 1560888482,
			 *      "Expiry": null,
			 *      "Votes": {"02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5": "10000000","02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D": "200000000"}
			 *  },
			 *  {
			 *      ...
			 *  }]
			 * or:
			 *  [{
			 *      "Type": "CRC",
			 *      "Amount": "300000000",
			 *      "Timestamp": 1560888482,
			 *      "Expiry": null,
			 *      "Votes": {"iYMVuGs1FscpgmghSzg243R6PzPiszrgj7": "10000000","iT42VNGXNUeqJ5yP4iGrqja6qhSEdSQmeP": "200000000"}
			 *  },
			 *  {
			 *      ...
			 *  }]
			 */
			virtual nlohmann::json GetVoteInfo(const std::string &type) const = 0;


			/**
			 *Sponsor generate proposal digest for sponsor signature.
			 *
			 * @param type             Proposal type, value is [0-5]
			 * @param sponsorPublicKey Public key of sponsor
			 * @param draftHash        The hash of draft proposal
			 * @param budgets          The budgets of proposal every stage. Such as ["300", "33", "344"]
			 * @param recipient        Address of budget payee. Such as "EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv"
			 *
			 * @return The proposal in JSON format contains the "Digest" field to be signed and then set the "Signature" field. Such as
			 * {
			 *    "Budgets":[324,266,234],
			 *    "DraftHash":"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *    "Recipient":"cbcb4cc55dc3fc6cc6e4663f747d8b75e25d21db21",
			 *    "SponsorPublicKey":"031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4",
			 *    "Type":0,
			 *    "Digest":"ae9ebae57a176f5d5f8693eee5a8192f219b3ae04922cc096acb7748fe7ceba7",
			 *    "Signature":""
			 * }
			 */
			virtual nlohmann::json SponsorProposalDigest(uint8_t type,
			                                             const std::string &sponsorPublicKey,
			                                             const std::string &draftHash,
			                                             const nlohmann::json &budgets,
			                                             const std::string &recipient) const = 0;

			/**
			 *CR sponsor generate proposal digest for cr signature.
			 *
			 * @param  sponsorSignedProposal Sponsor signed proposal
			 * @param  crSponsorDID     Did of sponsor. Such as "iYMVuGs1FscpgmghSzg243R6PzPiszrgj7"
			 * @return The proposal in JSON format contains the "Digest" field to be signed and then set the "CRSignature" field. Such as
			 * {
			 * 	  "Budgets":[324,266,234],
			 * 	  "CRSponsorDID":"cbcb4cc55dc3fc6cc6e4663f747d8b75e25d21db67",
			 * 	  "DraftHash":"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 * 	  "Recipient":"cbcb4cc55dc3fc6cc6e4663f747d8b75e25d21db21",
			 * 	  "Signature":"ff0ff9f45478f8f9fcd50b15534c9a60810670c3fb400d831cd253370c42a0af79f7f4015ebfb4a3791f5e45aa1c952d40408239dead3d23a51314b339981b76",
			 * 	  "SponsorPublicKey":"031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4",
			 * 	  "Type":0,
			 * 	  "Digest":"2a65e8ac29f5ba4e16a1def91fc4e210838900f24f0f651c5c414d8fd8aaf55d",
			 * 	  "CRSignature":""
			 * }
			 */
			virtual nlohmann::json CRSponsorProposalDigest(const nlohmann::json &sponsorSignedProposal,
			                                               const std::string &crSponsorDID) const = 0;

			/**
			 * Create CRC Proposal transaction.
			 *
			 * @param crSignedProposal CR sponsor signed proposal
			 * @param memo             Remarks string. Can be empty string.
			 * @return                 The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateCRCProposalTransaction(nlohmann::json crSignedProposal,
			                                                    const std::string &memo) = 0;

			/**
			 * Generate payload for review CRC proposal.
			 *
			 * @param proposalHash Hash value of proposal.
			 * @param voteResult   Opinion of the proposal. approve : 0, reject : 1, abstain : 2
			 * @param did          DID of CR. Such as  "iYMVuGs1FscpgmghSzg243R6PzPiszrgj7";
			 * @return             The proposal in JSON format contains the "Digest" field to be signed and then set the "Signature" field. Such as
			 * {
			 *   "DID":"270589bb89afdbeac2f6788c52386bafb941a24867",
			 *   "Digest":"01333218d8492fe59c1ee1e0830bf86bf97b37f71f73183f29a31385f3e0ba45",
			 *   "ProposalHash":"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
			 *   "Result":1,
			 *   "Signature":""
			 * }
			 */
			virtual nlohmann::json GenerateCRCProposalReview(const std::string &proposalHash,
			                                                 uint8_t voteResult,
			                                                 const std::string &did) const = 0;

			/**
			 * Create a review proposal transaction.
			 *
			 * @param proposalReview Generate by GenerateCRCProposalReview() and "Signature" is not empty.
			 * @param memo           Remarks string. Can be empty string.
			 * @return               The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateCRCProposalReviewTransaction(const nlohmann::json &proposalReview,
			                                                          const std::string &memo) = 0;

			/**
			 * Create vote crc proposal transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param votes        Proposal hash and votes in JSON format. Such as:
			 *                     {
			 *                          "109780cf45c7a6178ad674ac647545b47b10c2c3e3b0020266d0707e5ca8af7c": "100000000",
			 *                          "92990788d66bf558052d112f5498111747b3e28c55984d43fed8c8822ad9f1a7": "200000000"
			 *                     }
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateVoteCRCProposalTransaction(
					const std::string &fromAddress,
					const nlohmann::json &votes,
					const std::string &memo) = 0;

			/**
			 * Create impeachment crc transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param votes        CRC did and votes in JSON format. Such as:
			 *                     {
			 *                          "innnNZJLqmJ8uKfVHKFxhdqVtvipNHzmZs": "100000000",
			 *                          "iZFrhZLetd6i6qPu2MsYvE2aKrgw7Af4Ww": "200000000"
			 *                     }
			 * @param memo         Remarks string. Can be empty string.
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateImpeachmentCRCTransaction(
					const std::string &fromAddress,
					const nlohmann::json &votes,
					const std::string &memo) = 0;

		};

	}
}

#endif //__ELASTOS_SDK_IMAINCHAINSUBWALLET_H__
