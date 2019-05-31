// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MAINCHAINSUBWALLET_H__
#define __ELASTOS_SDK_MAINCHAINSUBWALLET_H__

#include "SubWallet.h"
#include <Interface/IMainchainSubWallet.h>

namespace Elastos {
	namespace ElaWallet {

		class MainchainSubWallet : public IMainchainSubWallet, public SubWallet {
		public:
			~MainchainSubWallet();

			/**
			 * Deposit token from the main chain to side chains, such as ID chain or token chain, etc.
			 *
			 * @param fromAddress      If this address is empty, SDK will pick available UTXO automatically.
			 *                         Otherwise, pick UTXO from the specific address.
			 * @param lockedAddress    Locked address of each side chain. Can be got by
			 *                         ISidechainSubWallet::GetGenesisAddress() of side chain instance.
			 * @param amount           The amount that will be deposit to the side chain.
			 * @param sideChainAddress Receive address of side chain.
			 * @memo                   Remarks string. Can be empty string.
			 * @useVotedUTXO           If true, all voted UTXO will be picked. Otherwise,
			 *                         any voted UTXO will not be picked.
			 *
			 * @return                 The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateDepositTransaction(
					const std::string &fromAddress,
					const std::string &lockedAddress,
					uint64_t amount,
					const std::string &sideChainAddress,
					const std::string &memo,
					const std::string &remark,
					bool useVotedUTXO = false);

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
				const std::string &ownerPublicKey,
				const std::string &nodePublicKey,
				const std::string &nickName,
				const std::string &url,
				const std::string &ipAddress,
				uint64_t location,
				const std::string &payPasswd) const;

			/**
			 * Generate payaload for unregistering producer.
			 *
			 * @param ownerPublicKey The public key to identify a producer.
			 * @param payPasswd      Pay password is using for signing the payload with the owner private key.
			 *
			 * @return               The payload in JSON format.
			 */
			virtual nlohmann::json GenerateCancelProducerPayload(
				const std::string &ownerPublicKey,
				const std::string &payPasswd) const;

			/**
			 * Create register producer transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payload      Generate by GenerateProducerPayload().
			 * @param amount       Amount must lager than 500,000,000,000 sela
			 * @param memo         Remarks string. Can be empty string.
			 * @param useVotedUTXO If true, all voted UTXO will be picked. Otherwise, any voted UTXO will not be picked.
			 *
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRegisterProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				uint64_t amount,
				const std::string &memo,
				const std::string &remark,
				bool useVotedUTXO = false);

			/**
			 * Create update producer transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payload      Generate by GenerateProducerPayload().
			 * @param memo         Remarks string. Can be empty string.
			 * @param useVotedUTXO If true, all voted UTXO will be picked. Otherwise, any voted UTXO will not be picked.
			 *
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateUpdateProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &memo,
				const std::string &remark,
				bool useVotedUTXO = false);

			/**
			 * Create cancel producer transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param payload      Generate by GenerateCancelProducerPayload().
			 * @param memo         Remarks string. Can be empty string.
			 * @param useVotedUTXO If true, all voted UTXO will be picked. Otherwise, any voted UTXO will not be picked.
			 *
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateCancelProducerTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payload,
				const std::string &emmo,
				const std::string &remark,
				bool useVotedUTXO = false);

			/**
			 * Create retrieve deposit transaction.
			 *
			 * @param amount     The available amount to be retrieved back.
			 * @param memo       Remarks string. Can be empty string.
			 *
			 * @return           The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateRetrieveDepositTransaction(
				uint64_t amount,
				const std::string &memo,
				const std::string &remark);

			/**
			 * Get owner public key.
			 *
			 * @return Owner public key.
			 */
			virtual std::string GetPublicKeyForVote() const;

			/**
			 * Create vote transaction.
			 *
			 * @param fromAddress  If this address is empty, SDK will pick available UTXO automatically.
			 *                     Otherwise, pick UTXO from the specific address.
			 * @param stake        Vote amount in sela.
			 * @param publicKeys   Public keys array in JSON format.
			 * @param memo         Remarks string. Can be empty string.
			 * @param remark       Deprecated.
			 * @param useVotedUTXO If true, all voted UTXO will be picked. Otherwise, any voted UTXO will not be picked.
			 *
			 * @return             The transaction in JSON format to be signed and published.
			 */
			virtual nlohmann::json CreateVoteProducerTransaction(
					const std::string &fromAddress,
					uint64_t stake,
					const nlohmann::json &publicKeys,
					const std::string &memo,
					const std::string &remark,
					bool useVotedUTXO = false);

			/**
			 * Get vote information of current wallet.
			 *
			 * @return Vote information in JSON format. The key is the public key, and the value is the stake. Such as:
			 * {
			 * 	 {"02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5": 10000000},
			 * 	 {"02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D": 200000000},
			 * 	 {"03E5B45B44BB1E2406C55B7DD84B727FAD608BA7B7C11A9C5FFBFEE60E427BD1DA": 5000000000}
			 * }
			 */
			virtual	nlohmann::json GetVotedProducerList() const;

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
			virtual nlohmann::json GetRegisteredProducerInfo() const;

		protected:
			friend class MasterWallet;

			MainchainSubWallet(const CoinInfo &info,
							   const ChainParams &chainParams,
							   const PluginType &pluginTypes,
							   MasterWallet *parent);

			virtual nlohmann::json GetBasicInfo() const;

		};

	}
}

#endif //__ELASTOS_SDK_MAINCHAINSUBWALLET_H__
