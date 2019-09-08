// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PARAMCHECKER_H__
#define __ELASTOS_SDK_PARAMCHECKER_H__

#include "CommonConfig.h"
#include "BigInt.h"

#include <string>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

		namespace Error {
			typedef enum {
				InvalidArgument = 20001,
				InvalidPasswd = 20002,
				WrongPasswd = 20003,
				IDNotFound = 20004,
				CreateMasterWalletError = 20005,
				CreateSubWalletError = 20006,
				JsonArrayError = 20007,
				Mnemonic = 20008,
				PubKeyFormat = 20009,
				PubKeyLength = 20010,
				DepositParam = 20011,
				WithdrawParam = 20012,
				CreateTransactionExceedSize = 20013,
				CreateTransaction = 20014,
				Transaction = 20015,
				PathNotExist = 20016,
				PayloadRegisterID = 20017,
				SqliteError = 20018,
				DerivePurpose = 20019,
				WrongAccountType = 20020,
				WrongNetType = 20021,
				InvalidCoinType = 20022,
				NoCurrentMultiSinAccount = 20023,
				MultiSignersCount = 20024,
				MultiSign = 20025,
				KeyStore = 20026,
				LimitGap = 20027,
				Wallet =  20028,
				Key = 20029,
				HexString = 20030,
				SignType = 20031,
				Address = 20032,
				Sign = 20033,
				KeyStoreNeedPhrasePassword = 20034,
				BalanceNotEnough = 20035,
				JsonFormatError = 20036,
				VoteStakeError = 20037,
				GetTransactionInput = 20038,
				InvalidTransaction = 20039,
				GetUnusedAddress = 20040,
				AccountNotSupportVote = 20041,
				WalletNotContainTx = 20042,
				DepositAmountInsufficient = 20043,
				PrivateKeyNotFound = 20044,
				InvalidRedeemScript = 20045,
				AlreadySigned = 20046,
				EncryptError = 20047,
				VerifyError = 20048,
				TxPending = 20049,
				InvalidMnemonicWordCount = 20050,
				InvalidLocalStore = 20051,
				MasterWalletNotExist = 20052,
				InvalidAsset = 20053,
				ReadConfigFileError = 20054,
				InvalidChainID = 20055,
				UnSupportOldTx = 20056,
				UnsupportOperation = 20057,
				BigInt = 20058,
				DepositNotFound = 20059,
				TooMuchInputs = 20060,
				LastVoteConfirming = 20061,
				Other = 29999,
			} Code;
		}

		namespace Exception {
			typedef enum {
				LogicError,
				InvalidArgument,
			} Type;
		}

		class ErrorChecker {
		public:

			static nlohmann::json MakeErrorJson(Error::Code err, const std::string &msg);

			static nlohmann::json MakeErrorJson(Error::Code err, const std::string &msg, const BigInt &data);

			static void ThrowParamException(Error::Code err, const std::string &msg);

			static void ThrowLogicException(Error::Code err, const std::string &msg);

			static void CheckParam(bool condition, Error::Code err, const std::string &msg);

			static void CheckLogic(bool condition, Error::Code err, const std::string &msg);

			static void CheckCondition(bool condition, Error::Code err, const std::string &msg,
									   Exception::Type type = Exception::LogicError);

			static void CheckCondition(bool condition, Error::Code err, const std::string &msg, const BigInt &data,
									   Exception::Type type = Exception::LogicError);

			static void CheckPassword(const std::string &password, const std::string &msg);

			static void CheckPasswordWithNullLegal(const std::string &password, const std::string &msg);

			static void CheckParamNotEmpty(const std::string &argument, const std::string &msg);

			static void CheckJsonArray(const nlohmann::json &jsonData, size_t count, const std::string &msg);

			static void CheckPathExists(const boost::filesystem::path &path);

			static void CheckPrivateKey(const std::string &key);

		};

	}
}

#endif //__ELASTOS_SDK_PARAMCHECKER_H__
