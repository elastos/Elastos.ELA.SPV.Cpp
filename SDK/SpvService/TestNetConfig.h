// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TESTNET_H__
#define __ELASTOS_SDK_TESTNET_H__

namespace Elastos {
	namespace ElaWallet {
		const nlohmann::json DefaultTestNetConfig = R"(
{
	"NetType": "TestNet",
	"ELA": {
		"Index": 0,
		"MinFee": 10000,
		"FeePerKB": 10000,
		"DisconnectionTime": 300,
		"GenesisAddress": "",
		"ChainParameters": {
			"Services": 0,
			"MagicNumber": 2018111,
			"StandardPort": 21338,
			"TargetTimeSpan": 86400,
			"TargetTimePerBlock": 120,
			"DNSSeeds": [
				"node-testnet-005.elastos.org",
				"node-testnet-006.elastos.org",
				"node-testnet-007.elastos.org"
			],
			"CheckPoints": [
				[0,       "6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3", 1513936800, 486801407],
				[2016,    "99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767", 1517940844, 503906048],
				[14112,   "20931a5184a88116d76b8669fae9fe038ca3e5209d74a64ec78947142c16a95e", 1523380720, 494547269],
				[22176,   "a8baf0a258a9c1a760d09a2bf038bcfcceec4d2e652690bfc45c95c885487b62", 1525053347, 503359531],
				[30240,   "5a9f2456d12b5f335a4d1ecfad28cadd600b7a38957a72d0c087573f4aa8280e", 1526024685, 503359636],
				[38304,   "ea313df1089b8b001c40565aa5e90d17da51885a29459b966d0019ac7dd90002", 1526997348, 503361573],
				[46368,   "824f0efd718d680467f7707b5735d0d69c9215e9721ed2be57b1254d74c926a5", 1527950360, 503351874],
				[54432,   "80731ac2ad64f7fb673290bb3ede8319ea2bd9cc7057f7f1ffecf9c631cd2eab", 1528908425, 494408287],
				[62496,   "8cb6da886b6258e8a6daefce9d5566575bd2ba95b4bd684105954ab9e69be042", 1529864598, 493529298],
				[70560,   "551499f42d2dc3f8dc7875720590d437ccd91a60c309e38be972131a2bca65c1", 1530853521, 494674545],
				[78624,   "7946a656a6a241b55fbc2e96fbfd4681ac715c9bcaecef791e98baff5090c71c", 1531850116, 503352094],
				[86688,   "a25b7207c98cbbede93db67ca41a0ac308d198c556eea955ff63f52b0b70131a", 1532870279, 503379835],
				[94752,   "372df4fe8ebb396f8298d3fa596ab6c825fb9d16a7d93ba15ed5372ed9382dfb", 1533848032, 503381789],
				[102816,  "b5e07af958149f5332336be914dbeba25da11dbd81cc0be9421352cb6fb02379", 1534812379, 503379290],
				[110880,  "743823d8e5d1e74aa7aa8de9a20ca3b42dad4c1115b1631d6071d66d68f6a4f9", 1535780560, 503376620],
				[118944,  "0475ce0c0612ec22dedbd9a1c2fd734fff86b1e6a24d75c6874f2b62c3bc607a", 1536760555, 503383990],
				[127008,  "309e70fc82a3709260011426edb71cdbe2a67b2540868cdd572cc0a506a96e4e", 1537725085, 503381782],
				[135072,  "681fee1c6ec452b1c38b5aadb26b9b60371b8104136bcef628c5b828643aebf9", 1538701705, 503383651],
				[143136,  "22a3efa19fcfdbcd5a1be9c4e66dadf317f31c6de53ef41a5603dc1d04d6fa21", 1539918360, 503378173],
				[151200,  "0536033af4e39efb760d9070a98e546ba53cc16d36841d440d3e7686cb852f79", 1540978309, 503363606],
				[159264,  "a72b0aab079818986cca9fbee3155fe9d20c7cfb64a1aed3edfa603d6d688c6e", 1541946004, 503356074],
				[167328,  "0e81022030c88a719b5d60b638a9d9e2ae0b0688eb309ff3ab47c82975936daf", 1542969353, 503382380],
				[175392,  "a19f1d199dcea09cca591bfee11ca708d8cba8507247fe958dbf1ad23cf6f5e7", 1544083986, 503358989],
				[183456,  "728d851c3a623d45393d665267497abbee60458d6e7d48682e31794b9a8d20ee", 1545056278, 503357468],
				[191520,  "0bf0d6ec4aef082a08106100573a27d4fc2a2fd48f144883f28d743cb76b3bcb", 1546198659, 503557308],
				[199584,  "b33502f4b8a823e91d8e2b3ef3f62eef0ca0746aa1c61880aa1d7c3e80ea3346", 1547382331, 505455856],
				[207648,  "28525060cada3be8f6318beb11dc848c76eb6404ae418278255a37cfce61a95f", 1548515041, 520142669],
				[215712,  "455f8e3845b370837589f6e9124c30f5d06584479364c62abc0ef13b99e9caf7", 1549542011, 520186953],
				[223776,  "d033741010e231d052399e9f5a49cfc6fd2ef1538afd37504fef9270cd823ab1", 1550547179, 520228596],
				[231840,  "6ca4aafd19df42379062f677ad0b7147e63d42cf115d32d41e5fd201986426f1", 1551559204, 520321991],
				[239904,  "b35329ff34fca9317b0f769a528947e013fe4875a665d140b8b2c9545231f617", 1552561580, 520414824],
				[247968,  "d898fd66d184453cba7488adecc9e81f74c608fd4a7e8259af5e9d06422903d4", 1553787572, 520194365],
				[256032,  "a2f034826f919c955984c5da8fcba2b69455f5abdb9237a910c9f2415c71e410", 1554299711, 492511404],
				[264096,  "9fe4ac340b5c24dedfa8f62774d222c5ae926970cf85b423564f40f746dcb0e5", 1555335825, 486746015],
				[272160,  "0deb5e022ccdf5ec8b4757468e8bd8e2eab8896d361cc916ccaa2e23cd21f43f", 1556404861, 487013872],
				[280224,  "a8af9773ea238ae776e89f7e4f6531f7ed11c9de12c133d4216e2e37fa321257", 1557633529, 487473134],
				[288288,  "6875f3daf52bff1d2e2397b2b66d4e37a1caf8c6121478016193fba6dad0b472", 1558816377, 488838945],
				[296352,  "a2d75e8ab339352311c8dc70e09aa14d2cea39154933b630e4a7f39391e08e83", 1559787419, 487734156],
				[304416,  "84a68ab383a4f471b28c32a2386bff90ef6dcbbd64bd88f6feca28f165d56bab", 1560888482, 487920650],
				[312480,  "48248ab94a26fbb006b732caa0aa40aa3bfbb42945399f1e0c0be6dac1e4710d", 1562023733, 489202555],
				[320544,  "3fdeebe7233f12b914fd3eca9b59bf0c01de4bda3a0472115cd6da4ccd6cd5fa", 1562943936, 487173288],
				[326592,  "d958c2578c47ba8f5bf856a386de97182c76ca6bc603f61d80d3ef221b9ae561", 1563663687, 487109489],
				[330624,  "889012deea50d0a97cc4127de161c238f5e02da3b0dd4eb51f6b8d3cd1438f7e", 1564164262, 487223426],
				[332640,  "4f8fca79374f72f2d18f83109cf72aa46c43bfe711594bef84b168d2c1119bd6", 1564400524, 487166982],
				[334656,  "44c6c066cd39631f50878798774d86ce8d06b7e9cca8097444e923ee1da37745", 1564624201, 486946062]
			]
		}
	},
	"IDChain": {
		"Index": 1,
		"MinFee": 10000,
		"FeePerKB": 10000,
		"DisconnectionTime": 300,
		"GenesisAddress": "XKUh4GLhFJiqAMTF6HyWQrV9pK9HcGUdfJ",
		"ChainParameters": {
			"Services": 0,
			"MagicNumber": 2018102,
			"StandardPort": 21608,
			"TargetTimeSpan": 86400,
			"TargetTimePerBlock": 120,
			"DNSSeeds": [
				"node-testnet-011.elastos.org",
				"node-testnet-012.elastos.org",
				"node-testnet-013.elastos.org",
				"node-testnet-014.elastos.org",
				"node-testnet-015.elastos.org"
			],
			"CheckPoints": [
				[0,     "56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3", 1513936800, 486801407],
				[2016,  "9b3069a05478988d4c9d2d4b941af048680dbe92353bc8f0cf282766aa935edb", 1532276131, 505977014],
				[4032,  "6e8f5e21ddd736bb62dcd3ae445444702dc0b0ee560b4161877d0c8f0d9ad448", 1532628770, 510283913],
				[6048,  "8cef466bdc7c9ed79fdd6b36babb5b6670c916c07d87b473327460e57d21799b", 1533004749, 520187129],
				[8064,  "46a9dd847b62f278a26a93e3f7a4cc135b30754e07f2754d6ac0baf6870c8060", 1533346700, 520376318],
				[10080, "09cdb3d720a7095c7023241361ca8a317c75273fbdd0e5dc268667683e360780", 1533694521, 520890958],
				[12096, "2790468186eb341dfe06fc6b8a66ad48d2bfb597d0f612e2a917d703c951aee1", 1534057013, 521637984],
				[14112, "f5582a7bab4ec851172dd96b9c18d8a046081de65ed05f316e4dc14a5a8fe176", 1534506459, 536912421],
				[16128, "8b78d2fee6751c82d419e5eb98bbf83a1514a046ae9ffd1462aceaeed3f189ec", 1534771533, 536935584],
				[18144, "ba98bdbeb358fc2ac565dd3a6f706cef7dfdc60063f1f3517311744c3a908de4", 1535034489, 536952176],
				[20160, "b4d0dcec130dccd6cfa61880cdfe30259e90c69d1f83056f0495910cca05246b", 1535300757, 536980894],
				[22176, "9738541d86b7843be83926be997164615a3102e33edece26da351fdae70d2d6a", 1535557289, 536997059],
				[24192, "e9198d6fdd89d292f7e967266c22c59e245d015d93c0428626211965b8349d05", 1535828463, 537035498],
				[26208, "8636093320982e3707224cb19252c2f2de124bd1c480c3e4ba35f6d564e2d713", 1536086668, 537085786],
				[28224, "b98d6c3e7f424d44bf4f21f9093be7b2d29f2134137949769718c33ae22bd205", 1536361613, 537169686],
				[30240, "8c43de5c79cd48da6754b2098fcb230d525b7b0f1e44fc7c87b2b38064269991", 1536626736, 537255791],
				[32256, "a8000672bff80dcf7117062073df6f3cb511be0e49c2862f91432a00eeef053e", 1536886078, 537301386],
				[34272, "640298c39b33c1298d87e482115053177ae60b721e23801db0061bb4e24d393b", 1537146343, 537429158],
				[36288, "5d2a3ae65b42c7233e2fcbfc2a4f86eb24a693ab7e3b47c38830e57c956ff49c", 1537593491, 539041984],
				[38304, "3763dde6fa684f31615b4237a17116777bf807a19701c3d5d02604e9bdcea1a6", 1538035204, 545259519],
				[40320, "25ad2ec3bd7531301d367b4f7b1357e567665213c776744775fed7f80b9b0349", 1538398560, 545259519],
				[42336, "18086d3158974bbc1a331b194e0580504ed76058d88da84fc446b0020b63c329", 1538663566, 545259519],
				[44352, "fed3a4a23a70eb6aebe0db46d509b9f239c31f48c3d9e8c8a516ce4690fbc0b0", 1545058592, 545259519],
				[46368, "d177b074f16f0affcaade7c1da0484d7f10f6faf02e421b8b915496d6679bd82", 1548462469, 545259519],
				[48384, "99fa8265769bc8bb752c1b5614e4ac6c3114ecefb7de3484d77359a51e20b754", 1548730334, 545259519],
				[50400, "ed572ae38820c513227570edacc6aea88968776160fade9e291bcb5a0d877311", 1549118884, 545259519],
				[52416, "5295b35d8ebcedd0ae482416b28491441a2de2c349c3b415bdb6e632ebfb5e7e", 1549380587, 545259519],
				[54432, "7a47859985f959b896bbb395aa90abd31fc0b39318bf0008582950fc8a044248", 1549637254, 545259519],
				[56448, "7be1ed5be84c360a60bbe742bcbe267e32b7beadc01189759223365bbeb62823", 1549889972, 545242916],
				[58464, "ce9fc50902e64211e2c0d455a5906585ba0e3522542cdd2a0b6649afe9e8e6c5", 1550147031, 545259519],
				[60480, "a96712ccdcc2b22a65ee7276ed860901ffe0fd46e26fefaf6827189bb2d84e09", 1550401220, 545259519],
				[62496, "c38b1bfe4c3e094975ed607b395e8f7b31c3723dd24bfbd47aa574ecca1502a7", 1550682718, 545259519],
				[64512, "841adce0de01b403585a27040370058f96fe87681df1e53c724c3890ae81f3ce", 1550950944, 545259519],
				[66528, "28b8e264995934b9f42fe145e19f3afffeb8374f9fa603b78ef671763836562f", 1551280389, 545259519],
				[68544, "cd63541e5f864fe29d8778140e42dfe58dad46ae6343036c6e53c1f25de5f97f", 1551576878, 545259519],
				[70560, "86a52104b27dd1a41101326011daaea815083808faa6d066e7daa12a76ee34fd", 1551840630, 545259519]
			]
		}
	},
	"TokenChain": {
		"Index": 2,
		"MinFee": 10000,
		"FeePerKB": 10000,
		"DisconnectionTime": 300,
		"GenesisAddress": "XVfmhjxGxBKgzYxyXCJTb6YmaRfWPVunj4",
		"ChainParameters": {
			"Services": 0,
			"MagicNumber": 2019104,
			"StandardPort": 21618,
			"TargetTimeSpan": 86400,
			"TargetTimePerBlock": 120,
			"DNSSeeds": [
				"node-testnet-002.elastos.org",
				"node-testnet-003.elastos.org",
				"node-testnet-004.elastos.org",
				"node-testnet-005.elastos.org",
				"node-testnet-006.elastos.org",
				"node-testnet-007.elastos.org",
				"node-testnet-011.elastos.org"
			],
			"CheckPoints": [
				[0,      "b569111dfb5e12d40be5cf09e42f7301128e9ac7ab3c6a26f24e77872b9a730e", 1551744000, 486801407]
			]
		}
	}
}
		)"_json;
	}
}

#endif //__ELASTOS_SDK_TESTNET_H__
