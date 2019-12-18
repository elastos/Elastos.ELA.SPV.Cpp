// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/Attribute.h>
#include <Plugin/Transaction/IDTransaction.h>
#include <Plugin/Transaction/Payload/DIDInfo.h>
#include <Common/Utils.h>
#include <Common/Log.h>
#include <Common/ElementSet.h>

using namespace Elastos::ElaWallet;


TEST_CASE("Transaction Serialize and Deserialize", "[Transaction]") {
	Log::registerMultiLogger();
	srand(time(nullptr));

	SECTION("transaction Serialize test") {
		Transaction tx1;
		initTransaction(tx1, Transaction::TxVersion::Default);

		ByteStream stream;
		tx1.Serialize(stream);

		REQUIRE(tx1.EstimateSize() == stream.GetBytes().size());

		Transaction tx2;
		REQUIRE(tx2.Deserialize(stream));

		verifyTransaction(tx1, tx2, false);

		tx2 = tx1;

		verifyTransaction(tx1, tx2, true);
	}

	SECTION("transaction set") {
		ElementSet<TransactionPtr> txSet;

		for (size_t i = 0; i < 30; ++i) {
			TransactionPtr tx1(new Transaction());
			initTransaction(*tx1, Transaction::TxVersion::V09);
			REQUIRE(txSet.Insert(tx1));
			REQUIRE(txSet.Size() == i + 1);
			REQUIRE(txSet.Contains(tx1));
			REQUIRE(txSet.Contains(tx1->GetHash()));

			TransactionPtr tx2(new Transaction(*tx1));
			REQUIRE(tx1->GetHash() == tx2->GetHash());
			REQUIRE(tx1.get() != tx2.get());
			REQUIRE(!txSet.Insert(tx2));
			REQUIRE(txSet.Size() == i + 1);
		}
	}

}

TEST_CASE("Convert to and from json", "[Transaction]") {
	srand(time(nullptr));

	SECTION("to and from json") {
		Transaction tx1;

		initTransaction(tx1, Transaction::TxVersion::V09);

		nlohmann::json txJson = tx1.ToJson();

		Transaction tx2;
		tx2.FromJson(txJson);

		verifyTransaction(tx1, tx2, true);
	}

	SECTION("from json string") {
		nlohmann::json j = R"({
			"Attributes":[{"Data":"31393532323632373233","Usage":0}],
			"BlockHeight":2147483647,
			"Fee":20000,
			"Inputs":[
			{
				"Index":1,
				"Sequence":0,
				"TxHash":"a693bd76ef3aa8c2001ae11cf7b26c3fa8a2a35385cb160e739ffb1edebe263b"
			}],
			"IsRegistered":false,
			"LockTime":0,
			"Outputs":[
			{
				"FixedIndex": 0,
				"Address":"ERSqjfWDwTYw7iLrCZYLHKzSzEYzF4QZUz",
				"Amount":2300000000,
				"AssetId":"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
				"OutputLock":0,
				"OutputType":1,
				"Payload":
				{
					"Version":0,
					"VoteContent":[
					{
						"Candidates":[
							{"Candidate":"03330ee8520088b7f578a9afabaef0c034fa31fe1354cb3a14410894f974132800"},
							{"Candidate":"033c495238ca2b6bb8b7f5ae172363caea9a55cf245ffb3272d078126b1fe3e7cd"},
							{"Candidate":"0337e6eaabfab6321d109d48e135190560898d42a1d871bfe8fecc67f4c3992250"},
							{"Candidate":"03c78467b91805c95ada2530513069bef1f1f1e7b756861381ab534efa6d94e40a"},
							{"Candidate":"021d59a84d2243111e39e8c2af0a5089127d142d52b18c3e4bf744e0c6f8af44e0"},
							{"Candidate":"036417ab256114a32bcff38f3e10f0384cfa9238afa41a163017687b3ce1fa17f2"},
							{"Candidate":"02e578a6f4295765ad3be4cdac9be15de5aedaf1ae76e86539bb54c397e467cd5e"},
							{"Candidate":"02ddd829f3495a2ce76d908c3e6e7d4505e12c4718c5af4b4cbff309cfd3aeab88"},
							{"Candidate":"03c7b1f234d5d16472fcdd24d121e4cd224e1074f558a3eb1a6a146aa91dcf9c0d"},
							{"Candidate":"03b688e0124580de452c400e01c628a690527e8742b6fa4645026dbc70155d7c8b"},
							{"Candidate":"03bc2c2b75009a3a551e98bf206730501ecdf46e71b0405840ff1d5750094bd4ff"},
							{"Candidate":"0230d383546d154d67cfafc6091c0736c0b26a8c7c16e879ef8011d91df976f1fb"},
							{"Candidate":"028fb1a85f6a30a516b9e3516d03267403a3af0c96d73b0284ca0c1165318531ff"},
							{"Candidate":"02db921cfb4bf504c83038212aafe52cc1d0a07eb71a399a0d2162fe0cd4d47720"},
							{"Candidate":"033fb33f39276b93d3474cf7999887bed16c3211ee7f904399eeead4c480d7d592"},
							{"Candidate":"030e4b487daf8e14dbd7023e3f6f475d00145a1f1cc87be4b8d58a4291ab0a3b1a"},
							{"Candidate":"0234048d3ee92a7d34fbe3da22bc69583b1785e8f6684c9f4f11804c518cb4e53d"},
							{"Candidate":"0203c80103bb094b5870f6b99b0bc6ab857fa87bab1896fc845108bba7aafbfe3c"},
							{"Candidate":"0210694f4ab518037bc2dcc3f5e1a1030e8a36821ab019c10f29d4a894b8034498"},
							{"Candidate":"02771568d40c1b20f3cbc2f4de327d3f61ae1a97a3e4a014838d267c818f2f999e"},
							{"Candidate":"02d1c315626710a4f556ee56f1978787e07d464b2287170e7789f2cb1ca60ece11"},
							{"Candidate":"03ba357f743e5dcab39dcd60a0a62f9ad573eae0d911291fd30846891f5ce29987"},
							{"Candidate":"038796d13f0ed94b2587ba2e13ca99b3cafd4d5cea2b08b2d06b841ed10d177a51"}
						],
						"Type":0
					}]
				},
				"ProgramHash":"8d3cab1fc7ed2da20317e2b111f0f91ff5f0f45a21"
			},
			{
				"FixedIndex": 1,
				"Address":"ERSqjfWDwTYw7iLrCZYLHKzSzEYzF4QZUz",
				"Amount":2687533220000,
				"AssetId":"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
				"OutputLock":0,
				"OutputType":0,
				"Payload":null,
				"ProgramHash":"8d3cab1fc7ed2da20317e2b111f0f91ff5f0f45a21"
			}],
			"PayLoad":null,
			"PayloadVersion":0,
			"Programs":[
			{
				"Code":"2102676d08015cdabae42647de70667a3f33f56037495d1d10a00092f9361b5c6cc0ac",
				"Parameter":"40be01660cb4501c7741ea7437a284cdcb5dca33c1e01b6b1a47e4814135c36430eeb19f8b2c89162bb45396e5f34c3a65f5ac05ec90f342a6ead2a48b6beb8a2f"
			}],
			"Remark":"",
			"Timestamp":118,
			"TxHash":"caed058f3a852547f5d44323ed7c97efd9876fab171f3db24416faee5fe1b63a",
			"Type":2,
			"Version":9
		})"_json;

		Transaction tx;
		tx.FromJson(j);

		uint256 hash = tx.GetHash();

		REQUIRE(j["TxHash"] == hash.GetHex());
		REQUIRE(tx.IsSigned());
	}

	SECTION("Deserialize from raw tx") {
		std::string rawtx = "09020001000a31393532323632373233013b26bede1efb9f730e16cb8553a3a2a83f6cb2f71ce11a00c2a83aef76bd93a601000000000002b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3003717890000000000000000215af4f0f51ff9f011b1e21703a22dedc71fab3c8d01000100172103330ee8520088b7f578a9afabaef0c034fa31fe1354cb3a14410894f97413280021033c495238ca2b6bb8b7f5ae172363caea9a55cf245ffb3272d078126b1fe3e7cd210337e6eaabfab6321d109d48e135190560898d42a1d871bfe8fecc67f4c39922502103c78467b91805c95ada2530513069bef1f1f1e7b756861381ab534efa6d94e40a21021d59a84d2243111e39e8c2af0a5089127d142d52b18c3e4bf744e0c6f8af44e021036417ab256114a32bcff38f3e10f0384cfa9238afa41a163017687b3ce1fa17f22102e578a6f4295765ad3be4cdac9be15de5aedaf1ae76e86539bb54c397e467cd5e2102ddd829f3495a2ce76d908c3e6e7d4505e12c4718c5af4b4cbff309cfd3aeab882103c7b1f234d5d16472fcdd24d121e4cd224e1074f558a3eb1a6a146aa91dcf9c0d2103b688e0124580de452c400e01c628a690527e8742b6fa4645026dbc70155d7c8b2103bc2c2b75009a3a551e98bf206730501ecdf46e71b0405840ff1d5750094bd4ff210230d383546d154d67cfafc6091c0736c0b26a8c7c16e879ef8011d91df976f1fb21028fb1a85f6a30a516b9e3516d03267403a3af0c96d73b0284ca0c1165318531ff2102db921cfb4bf504c83038212aafe52cc1d0a07eb71a399a0d2162fe0cd4d4772021033fb33f39276b93d3474cf7999887bed16c3211ee7f904399eeead4c480d7d59221030e4b487daf8e14dbd7023e3f6f475d00145a1f1cc87be4b8d58a4291ab0a3b1a210234048d3ee92a7d34fbe3da22bc69583b1785e8f6684c9f4f11804c518cb4e53d210203c80103bb094b5870f6b99b0bc6ab857fa87bab1896fc845108bba7aafbfe3c210210694f4ab518037bc2dcc3f5e1a1030e8a36821ab019c10f29d4a894b80344982102771568d40c1b20f3cbc2f4de327d3f61ae1a97a3e4a014838d267c818f2f999e2102d1c315626710a4f556ee56f1978787e07d464b2287170e7789f2cb1ca60ece112103ba357f743e5dcab39dcd60a0a62f9ad573eae0d911291fd30846891f5ce2998721038796d13f0ed94b2587ba2e13ca99b3cafd4d5cea2b08b2d06b841ed10d177a51b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3a08076bd7102000000000000215af4f0f51ff9f011b1e21703a22dedc71fab3c8d0000000000014140be01660cb4501c7741ea7437a284cdcb5dca33c1e01b6b1a47e4814135c36430eeb19f8b2c89162bb45396e5f34c3a65f5ac05ec90f342a6ead2a48b6beb8a2f232102676d08015cdabae42647de70667a3f33f56037495d1d10a00092f9361b5c6cc0ac";
		bytes_t bytes(rawtx);
		ByteStream stream(bytes);

		Transaction tx;
		REQUIRE(tx.Deserialize(stream));

		uint256 hash = tx.GetHash();
		REQUIRE("caed058f3a852547f5d44323ed7c97efd9876fab171f3db24416faee5fe1b63a" == hash.GetHex());
	}

	SECTION("tx verify signed") {
		nlohmann::json encodedTx = nlohmann::json::parse(
			"{\"Algorithm\":\"base64\",\"ChainID\":\"ELA\",\"Data\":\"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\",\"Fee\":0,\"ID\":\"19978272\"}");
		TransactionPtr tx(new Transaction());

		REQUIRE((encodedTx.find("Algorithm") != encodedTx.end() && encodedTx.find("Data") != encodedTx.end() &&
				 encodedTx.find("ChainID") != encodedTx.end()));

		std::string algorithm, data, chainID;

		algorithm = encodedTx["Algorithm"].get<std::string>();
		data = encodedTx["Data"].get<std::string>();
		chainID = encodedTx["ChainID"].get<std::string>();

		bytes_t rawHex;
		REQUIRE(algorithm == "base64");
		rawHex.setBase64(data);

		ByteStream stream(rawHex);
		REQUIRE(tx->Deserialize(stream, true));

		REQUIRE(tx->IsSigned());
	}
}

TEST_CASE("new tx with type and payload", "[IDTransaction]") {
	nlohmann::json didPayloadJSON = R"(
{"header":{"specification":"elastos/did/1.0","operation":"create"},"payload":"eyJleHBpcmVzIjoiMTk3MC0wMS0wMVQwODozMzo0NFoiLCJpZCI6ImRpZDplbGFzdG9zOmlvTkRBMlBqTGczZHB6WE54Y0tYUEJVdHFnS0IydmpmM0oiLCJwcm9vZiI6eyJzaWduYXR1cmVWYWx1ZSI6IjViUVhlQUVmeWdtYVhkbmlMcEI0MTM4a01jRHFLdHVObi1BSXVMbHNRT1RBX3lZcEdZQi1lWnB3TTJuOHM3QWphbVpNeXlMelQyNkxEeFVpQjdOZ2RRIiwidHlwZSI6IkVDRFNBc2VjcDI1NnIxIn0sInB1YmxpY0tleSI6W3siaWQiOiIjcHJpbWFyeSIsInB1YmxpY0tleUJhc2U1OCI6IjI0YUV0ckthUVN0MjFncm9qSlpEV0J4YUQzVkV2RGptY01jN1g1MkFSMXM1cyIsInR5cGUiOiJFQ0RTQXNlY3AyNTZyMSJ9XSwidmVyaWZpYWJsZUNyZWRlbnRpYWwiOlt7ImNyZWRlbnRpYWxTdWJqZWN0Ijp7ImRpZE5hbWUiOiJ0ZXN0In0sImV4cGlyYXRpb25EYXRlIjoiIiwiaWQiOiJkaWQ6ZWxhc3Rvczppb05EQTJQakxnM2RwelhOeGNLWFBCVXRxZ0tCMnZqZjNKIiwiaXNzdWFuY2VEYXRlIjoiMjAxOS0xMi0xOFQxNzo0NDowNFoiLCJpc3N1ZXIiOiIiLCJwcm9vZiI6eyJzaWduYXR1cmUiOiIiLCJ0eXBlIjoiIiwidmVyaWZpY2F0aW9uTWV0aG9kIjoiIn0sInR5cGUiOlsiU2VsZlByb2NsYWltZWRDcmVkZW50aWFsIl19XX0","proof":{"verificationMethod":"#primary","signature":"f-wfLWQ3paOaxkya7iWzMZecUoG5gmVvG6ofUc6EvvvRSa8yWQuppOwdoN9_g7aDB4U0Qjap-57Q-5P9xdW3Tw"}}
)"_json;


	PayloadPtr payload = PayloadPtr(new DIDInfo());
	payload->FromJson(didPayloadJSON, 0);
	TransactionPtr tx1 = TransactionPtr(new IDTransaction(IDTransaction::didTransaction, payload));
	initTransaction(*tx1, Transaction::TxVersion::Default);

	ByteStream stream;
	tx1->Serialize(stream);

	TransactionPtr tx2 = TransactionPtr(new IDTransaction());
	tx2->Deserialize(stream);

	verifyTransaction(*tx1, *tx2, false);

	DIDInfo *didInfo = dynamic_cast<DIDInfo *>(tx2->GetPayload());

	REQUIRE(didInfo->IsValid());
	const DIDHeaderInfo &header = didInfo->DIDHeader();
	REQUIRE(header.Specification() == "elastos/did/1.0");
	REQUIRE(header.Operation() == "create");

	const DIDPayloadInfo &didPayloadInfo = didInfo->DIDPayload();
	REQUIRE(didPayloadInfo.ID() == "did:elastos:ioNDA2PjLg3dpzXNxcKXPBUtqgKB2vjf3J");
	REQUIRE(didPayloadInfo.PublicKeyInfo().size() == 1);
	REQUIRE(didPayloadInfo.PublicKeyInfo()[0].ID() == "#primary");
	REQUIRE(didPayloadInfo.PublicKeyInfo()[0].PublicKeyBase58() == "24aEtrKaQSt21grojJZDWBxaD3VEvDjmcMc7X52AR1s5s");
}
