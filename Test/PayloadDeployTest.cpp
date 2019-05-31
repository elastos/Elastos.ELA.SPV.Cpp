// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"

#include <SDK/Plugin/Transaction/Payload/PayloadDeploy.h>
#include <SDK/Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("PayloadDeploy test", "[PayloadDeploy]") {
	Log::registerMultiLogger();

	SECTION("Serialize and deserialize") {
		PayloadDeploy p1;
		FunctionCode functionCode;
		functionCode.code = getRandBytes(50);
		functionCode.parameters.push_back(ParameterType::String);
		functionCode.parameters.push_back(ParameterType::Signature);
		functionCode.parameters.push_back(ParameterType::PublicKey);
		functionCode.returnType = ParameterType::Boolean;
		functionCode.codeHash = getRandUInt168();
		p1.SetFunctionCode(functionCode);

		p1.SetName("payloadTest");
		p1.SetVersion("1.0");
		p1.SetAuthor("zxb");
		p1.SetEmail("email");
		p1.SetDescription("desc");
		p1.SetProgramHash(getRandUInt168());
		p1.SetGas(3223);

		ByteStream stream;

		p1.Serialize(stream, 0);

		PayloadDeploy p2;

		REQUIRE(p2.Deserialize(stream, 0));

		const FunctionCode fcode = p2.GetFunctionCode();
		REQUIRE(fcode.codeHash == functionCode.codeHash);
		REQUIRE(fcode.parameters == functionCode.parameters);
		REQUIRE(fcode.returnType == functionCode.returnType);
		REQUIRE(fcode.code == functionCode.code);

		REQUIRE(p1.GetName() == p2.GetName());
		REQUIRE(p1.GetVersion() == p2.GetVersion());
		REQUIRE(p1.GetAuthor() == p2.GetAuthor());
		REQUIRE(p1.GetEmail() == p2.GetEmail());
		REQUIRE(p1.GetDescription() == p2.GetDescription());
		REQUIRE(p1.GetProgramHash() == p2.GetProgramHash());
		REQUIRE(p1.GetGas() == p2.GetGas());
	}

	SECTION("to json and from json") {
		PayloadDeploy p1;
		FunctionCode functionCode;
		functionCode.code = getRandBytes(50);
		functionCode.parameters.push_back(ParameterType::String);
		functionCode.parameters.push_back(ParameterType::Signature);
		functionCode.parameters.push_back(ParameterType::PublicKey);
		functionCode.returnType = ParameterType::Boolean;
		functionCode.codeHash = getRandUInt168();
		p1.SetFunctionCode(functionCode);

		p1.SetName("payloadTest");
		p1.SetVersion("1.0");
		p1.SetAuthor("zxb");
		p1.SetEmail("email");
		p1.SetDescription("desc");
		p1.SetProgramHash(getRandUInt168());
		p1.SetGas(3223);

		nlohmann::json p1Json = p1.ToJson(0);

		PayloadDeploy p2;
		p2.FromJson(p1Json, 0);

		const FunctionCode fcode = p2.GetFunctionCode();
		REQUIRE(fcode.codeHash == functionCode.codeHash);
		REQUIRE(fcode.parameters == functionCode.parameters);
		REQUIRE(fcode.returnType == functionCode.returnType);
		REQUIRE(fcode.code == functionCode.code);

		REQUIRE(p1.GetName() == p2.GetName());
		REQUIRE(p1.GetVersion() == p2.GetVersion());
		REQUIRE(p1.GetAuthor() == p2.GetAuthor());
		REQUIRE(p1.GetEmail() == p2.GetEmail());
		REQUIRE(p1.GetDescription() == p2.GetDescription());
		REQUIRE(p1.GetProgramHash() == p2.GetProgramHash());
		REQUIRE(p1.GetGas() == p2.GetGas());
	}
}