// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__GROUPEDASSET_H__
#define __ELASTOS_SDK__GROUPEDASSET_H__

#include <SDK/Common/ElementSet.h>
#include <SDK/Common/Lockable.h>
#include <SDK/Account/SubAccount.h>
#include <SDK/Common/BigInt.h>

#include <map>
#include <boost/function.hpp>
#include <boost/weak_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Wallet;
		class Asset;
		class Transaction;
		class TransactionOutput;
		class TransactionInput;
		class UTXO;
		class VoteContent;
		typedef boost::shared_ptr<Asset> AssetPtr;
		typedef boost::shared_ptr<UTXO> UTXOPtr;
		typedef std::vector<UTXOPtr> UTXOArray;
		typedef boost::shared_ptr<TransactionOutput> OutputPtr;
		typedef std::vector<OutputPtr> OutputArray;
		typedef boost::shared_ptr<TransactionInput> InputPtr;
		typedef std::vector<InputPtr> InputArray;
		typedef std::vector<VoteContent> VoteContentArray;

		class GroupedAsset {
		public:
			GroupedAsset();

			GroupedAsset(Wallet *parent, const AssetPtr &asset);

			GroupedAsset(const GroupedAsset &proto);

			GroupedAsset &operator=(const GroupedAsset &proto);

			UTXOArray GetUTXOs(const std::string &addr) const;

			const UTXOArray &GetVoteUTXO() const;

			const UTXOArray &GetCoinBaseUTXOs() const;

			BigInt GetBalance() const;

			nlohmann::json GetBalanceInfo();

			TransactionPtr CreateRetrieveDepositTx(const OutputArray &outputs,
												   const Address &fromAddress,
												   const std::string &memo);

			TransactionPtr Vote(const VoteContent &voteContent, const std::string &memo, bool max);

			TransactionPtr Consolidate(const std::string &memo);

			TransactionPtr CreateTxForOutputs(const std::vector<OutputPtr> &outputs,
											  const Address &fromAddress,
											  const std::string &memo,
											  bool max);

			void AddFeeForTx(TransactionPtr &tx);

			const AssetPtr &GetAsset() const;

			bool AddUTXO(const UTXOPtr &o);

			bool AddCoinBaseUTXO(const UTXOPtr &o);

			bool RemoveSpentUTXO(const std::vector<InputPtr> &inputs);

			bool RemoveSpentUTXO(const InputPtr &input);

			bool RemoveSpentUTXO(const uint256 &hash, uint16_t n);

			void GetSpentCoinbase(const InputArray &inputs, std::vector<uint256> &spentCoinbase) const;

			bool UpdateLockedBalance();

			bool ContainUTXO(const UTXOPtr &o) const;

		private:
			uint64_t CalculateFee(uint64_t feePerKB, size_t size) const;

		private:
			BigInt _balance, _balanceVote, _balanceDeposit, _balanceLocked;
			UTXOArray _utxos, _utxosVote, _utxosCoinbase, _utxosDeposit, _utxosLocked;

			AssetPtr _asset;

			Wallet *_parent;
		};

		typedef boost::shared_ptr<GroupedAsset> GroupedAssetPtr;

	}
}


#endif //__ELASTOS_SDK__GROUPEDASSET_H__
