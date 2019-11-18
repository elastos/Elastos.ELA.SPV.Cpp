// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_REGTEST_H__
#define __ELASTOS_SDK_REGTEST_H__

namespace Elastos {
	namespace ElaWallet {
		const nlohmann::json DefaultRegTestConfig = R"(
{
	"NetType": "RegTest",
	"ELA": {
		"Index": 0,
		"MinFee": 10000,
		"FeePerKB": 10000,
		"DisconnectionTime": 300,
		"GenesisAddress": "",
		"ChainParameters": {
			"Services": 0,
			"MagicNumber": 2018211,
			"StandardPort": 22338,
			"TargetTimeSpan": 86400,
			"TargetTimePerBlock": 120,
			"DNSSeeds": [
				"node-regtest-002.eadd.co",
				"node-regtest-003.eadd.co",
				"node-regtest-004.eadd.co"
			],
			"CheckPoints": [
				[0,      "6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3", 1513936800, 486801407],
				[2016,   "99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767", 1517940844, 503906048],
				[10080,  "4dbf87bdf287319958c480fbc9249cbc93847a54f05ccb13f8283de1cf365158", 1522878764, 493221476],
				[18144,  "a299a6fde95bde058996253fb090658ca7b19b6366c48c053ac43302728d380d", 1523866303, 494684787],
				[26208,  "3d2269ae53697c648a40aabaf6841cd2a43bc38699264a64bcd166e000cd452a", 1525472220, 503357792],
				[34272,  "8dfe3ec6125c06c40bcd9a72963b6e60dc214b7a61ad2e805b138f60ac7e92de", 1526448032, 503363437],
				[42336,  "8b638a328e1f1930fba8cb5ee0703a2c16aa362a52dc8b8729a33fb487706441", 1527512109, 503361968],
				[50400,  "8e34fdc18ef27996022cc36d85f856690990faa077d92b78989c04f113205cd0", 1528926400, 493749763],
				[58464,  "5fadb0eb085f90998857f18b0524db988d323650b50ce8ad2289320c1a9b9a07", 1530458426, 503351139],
				[66528,  "2fec85c9bc0a9f9b8f6ac106bfae0c957485d456eec86f21eafddff1ace2fffe", 1531553227, 503351485],
				[74592,  "f7afa994b23a5628f109e1de8563f4ae5963a814b1c972425d921d581bcec7a0", 1532636667, 503374250],
				[82656,  "3389fef7151029d9dca13cf109e668c0d6b96af8d0f56422c9c776b58313af62", 1533667029, 503380217],
				[90720,  "0d3022fe8d93e8dd0610a37d33648c1cfffcb6016633d0303de2d177fe79287b", 1535848298, 503352685],
				[98784,  "a8efe800d2a18bf7dca867e307561d11e33abe20fc8a6c20e82f05efcaa3f6cd", 1536880958, 503376323],
				[106848, "b4e0669d215d7861534e7b7c09fe645da9ea170e3e730dbb33a6e47180942b19", 1537826633, 503359584],
				[114912, "4d2197b83c20edac4125a9414b56d0a9a0f0252858c7eb95b50805626f1c7256", 1538794988, 503359557],
				[122976, "ab5c2a7b606ade0623951c7126400e661f10c0ab82bfda956e443990f8c49a7f", 1539767258, 503361461],
				[131040, "580e961870022c0579148b41944d6a2b80d03c6b4cdd585ba399c67349280838", 1540733044, 503358919],
				[139104, "a6d2f7414828a267ff1bcee404bd5c676777021e8b15714669c920db98f1831f", 1541890309, 503374077],
				[147168, "a723418d93697fc2393cdd585d402f73a7afec40eb7487461f60a6074f9a3d28", 1543204596, 503610078],
				[155232, "4dfaffbba8e43c9543d3fe57e5e495b2437c2cee0679c8ec37a529cbf13e8d3c", 1545018237, 520137200],
				[163296, "ab8004f64fdabf4592c81aa6335044e4b29390c2f94923024be5471ca321403f", 1546326011, 521126789],
				[171360, "9aaa94df9967ab17ebdeaa092dc7a7bdd8bce560374312881d9ad076cee95e01", 1547677822, 536991628],
				[179424, "5bc5fbf4cd5c0abaecc4956ab7d1e141c97608d9cd4a482cdde8218e405903cf", 1548762492, 537278208],
				[187488, "b09d8db40d497e67d9c75a6cb0a78331053851a1a9b39c9f28bee8e5856837c8", 1549784847, 537593934],
				[193536, "9b73014a04f79ee6ffb11fa0e9e34e9eb969cebe5be82c05e81fdd8803e77ccc", 1550625933, 539075664],
				[199584, "6bdfcf51a9512a61fe48c75591e733f716be679ea13bbb9d9ad65912dd78d30e", 1551401692, 540766560],
				[205632, "bf1d83c6e3072f1860d3a7a5753f194d118c89aab71c099b5623912aee589ffc", 1552349806, 545259519],
				[211680, "6022294c7d1f1babe8617acbd101abc6fb0d9cb597a54f6bc3c7d247a32f1a45", 1553540283, 540868898],
				[217728, "d19f948c74a65a29928ea7d67b94ac9431b3cc31b6739ae2f0110def84029bec", 1554887667, 525050151],
				[223776, "d7c69027048a086237e20ebbf48f90ddcbac341100640be56041c7fd5439109e", 1555097627, 503379668],
				[225792, "8b79e7ec261f3919f6053169d1fbdbb8243cc43b66e47ee3502cf865e1f2ccbf", 1555229310, 487440005],
				[227808, "abae406f799290e4a8b7c5977d0678c7238b95a08f17c95bc1bdd013a79083b2", 1555543973, 488147127],
				[229824, "eebef41b99e799260944cb1a35a1f580d0d11ed9cda80cb584ab143a73c65fbc", 1555796273, 488411671],
				[233856, "5c25ed7053dd05ade5e27ebc344d439520c659fde6f3fecc0f17502b2358ed97", 1560574729, 487934909],
				[237888, "45e3002e8d4fef8f8abbce6e21bb5da55b3ac275bf2a13abb26f8a49238c5aed", 1561054303, 487831384],
				[241920, "9358c32f737c0a8357f05b78eba1b530dfc027a20b9fa437886a5c0273239c02", 1561527449, 487659825],
				[245952, "08e7f3dd03dcab5a9ecc938c02d1722af7ef35254931fc9225422c07574f7ca2", 1562028121, 487170887],
				[249984, "ef567121ca72bae678c5eea12ea8a5d0666639e8235c7d267b3d2d672588a9f5", 1562573537, 487128559],
				[254016, "96e9adc67649e8bb0dacdd6c33bd8926044147e22308846660e30cf3dd1880d0", 1563493390, 487764115],
				[258048, "e6b2db4329d1f3ef170ac1f0ef12d7c94abc4e0618a833d701c156624b28e32d", 1563897272, 487827974],
				[262080, "287387f82927740c1bf7f9618de686fb35e251c87cd21b918c77c3a74133dae0", 1564372543, 487662550],
				[266112, "6067180934bef764eb8951afe7faaa04d0e5afe4df0dee9c8f607c91df7db5c3", 1564867457, 487804286],
				[270144, "c50e677383dd5b18af90034ce521785e32de3b02b10dcda3f7d28be7884fdc89", 1565660507, 488411827],
				[274176, "5eec7d907e52c34d3b89b16269f80105a2c733b30e67b930f60be179b3117bdc", 1566115969, 487726135],
				[278208, "7a10672679f48118e7d2f821d10ae3b0ae11a34f8f80426e53d86aa017c490b6", 1566627044, 487816417],
				[282240, "022d16ebafc5f7052798ff6ff5a55a0ccfe08329c187a1b6b609c8d5d6df1356", 1567109767, 487780865],
				[286272, "140d1d03a65e652a3866116207497d100b61d00a699a7cd3ed126951c1f24948", 1567600115, 487802136],
				[288288, "097f7424cde0de4e1a2fd8c1cedea25bea349d9ed9db55134681ac486ec06075", 1567833306, 487685307],
				[290304, "f029ca9849bcfc6cef8da24794d69bd302bba16787cff64ac09632d2a8da12f4", 1568074582, 487724432],
				[292320, "547878acac5ed6dd61cba550e8cd20267239888e73027feaf93075da9b928878", 1568790003, 491281700],
				[294336, "f5a56b7a82a1f7527e946db25d1418bc57d567044ed11305727e5b27ec762d5c", 1568950473, 487818487]
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
			"StandardPort": 22608,
			"MagicNumber": 2018202,
			"Services": 0,
			"TargetTimeSpan": 86400,
			"TargetTimePerBlock": 120,
			"DNSSeeds": [
				"node-regtest-002.eadd.co",
				"node-regtest-003.eadd.co",
				"node-regtest-004.eadd.co"
			],
			"CheckPoints": [
				[0,     "56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3", 1513936800, 486801407],
				[2016,  "fb29c3de03ab8dbdc9e50ee882efc9a8115697a7681593ec1c587dd8ddc0da70", 1548438160, 505759638]
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
			"MagicNumber": 2019204,
			"StandardPort": 22618,
			"TargetTimeSpan": 86400,
			"TargetTimePerBlock": 120,
			"DNSSeeds": [
				"node-regtest-102.eadd.co",
				"node-regtest-103.eadd.co",
				"node-regtest-104.eadd.co",
				"node-regtest-105.eadd.co",
				"node-regtest-106.eadd.co",
				"node-regtest-107.eadd.co"
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

#endif //__ELASTOS_SDK_REGTEST_H__