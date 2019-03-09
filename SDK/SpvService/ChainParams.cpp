// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ChainParams.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>

#include <Core/BRMerkleBlock.h>

namespace Elastos {
	namespace ElaWallet {

		ChainParams::ChainParams(const ChainParams &params) {
			_dnsSeeds.insert(_dnsSeeds.begin(), params._dnsSeeds.begin(), params._dnsSeeds.end());
			_checkpoints.insert(_checkpoints.begin(), params._checkpoints.begin(), params._checkpoints.end());

			_standardPort = params._standardPort;
			_magicNumber = params._magicNumber;
			_services = params._services;
			_targetTimeSpan = params._targetTimeSpan;
			_targetTimePerBlock = params._targetTimePerBlock;
		}

		ChainParams::ChainParams(const CoinConfig &config) {
			if ("MainNet" == config.NetType) {
				MainNetParamsInit(config.Type);
			} else if ("TestNet" == config.NetType) {
				TestNetParamsInit(config.Type);
			} else if ("RegNet" == config.NetType || "RegTest" == config.NetType) {
				RegNetParamsInit(config.Type);
			} else {
				ErrorChecker::CheckCondition(true, Error::WrongNetType,
											 "Invalid net type " + config.NetType + " in coin config");
			}

		}

		ChainParams &ChainParams::operator=(const ChainParams &params) {
			_dnsSeeds.clear();
			_dnsSeeds.insert(_dnsSeeds.begin(), params._dnsSeeds.begin(), params._dnsSeeds.end());
			_checkpoints.clear();
			_checkpoints.insert(_checkpoints.begin(), params._checkpoints.begin(), params._checkpoints.end());

			_standardPort = params._standardPort;
			_magicNumber = params._magicNumber;
			_services = params._services;
			_targetTimeSpan = params._targetTimeSpan;
			_targetTimePerBlock = params._targetTimePerBlock;

			return *this;
		}

		const std::vector<std::string> &ChainParams::GetDNSSeeds() const {
			return _dnsSeeds;
		}

		const CheckPoint &ChainParams::GetLastCheckpoint() const {
			return _checkpoints.back();
		}

		const CheckPoint &ChainParams::GetFirstCheckpoint() const {
			return _checkpoints.front();
		}

		const std::vector<CheckPoint> &ChainParams::GetCheckpoints() const {
			return _checkpoints;
		}

		const uint32_t &ChainParams::GetMagicNumber() const {
			return _magicNumber;
		}

		const uint16_t &ChainParams::GetStandardPort() const {
			return _standardPort;
		}

		const uint64_t &ChainParams::GetServices() const {
			return _services;
		}

		const uint32_t &ChainParams::GetTargetTimeSpan() const {
			return _targetTimeSpan;
		}

		const uint32_t &ChainParams::GetTargetTimePerBlock() const {
			return _targetTimePerBlock;
		}

		// main net
		void ChainParams::MainNetMainChainParamsInit() {
			_dnsSeeds.emplace_back("node-mainnet-002.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-006.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-014.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-016.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-021.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-003.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-007.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-015.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-017.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-022.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-004.elastos.org");
			_dnsSeeds.emplace_back("node-mainnet-023.elastos.org");
			_checkpoints.emplace_back(0,      "05f458a5522851622cae2bb138498dec60a8f0b233802c97a1ca41f9f214708d", 1513936800, 486801407);
			_checkpoints.emplace_back(2016,   "333d9a0e874cf1b165a998c061c2f8be8e03ce31712046d001823d528e1fec84", 1513999567, 503353328);
			_checkpoints.emplace_back(12096,  "6347d62e227dbf74c8a4c478fa4f8d351f24a3db3f5a01aad32ea262b6e6f6aa", 1515218030, 492349017);
			_checkpoints.emplace_back(22176,  "a32a6a82af8eb8e25543ed09671e52f7a54b4adf2cc703134bc122010f1f73af", 1516694131, 503433043);
			_checkpoints.emplace_back(32256,  "3c6f75a2a6f9d37918036b912cb341ccd687d04ff354df4afe4aa83a3a97f0eb", 1517916962, 503438901);
			_checkpoints.emplace_back(42336,  "5512e09b1cc98893969a3801f5f02de7b5146748b99142162803d3fc65b094a1", 1519125604, 503440501);
			_checkpoints.emplace_back(52416,  "2c3c8ebd558387a59880046b1f7dea9f55ec2d2525811806ab673411de078896", 1520340390, 503442853);
			_checkpoints.emplace_back(62496,  "8c6d9086e3944cc56bef87eaafb6d2cca23e7e7b8ae637bc390e6ae61bcd0fa8", 1521478059, 493673302);
			_checkpoints.emplace_back(72576,  "a25db21f09d01ac77bfa3c88eb4ca25d87b357333721a07c1d4d568bc7dddaa4", 1522696898, 493968530);
			_checkpoints.emplace_back(82656,  "d3238d44fcaa42ec51f9314185eb57950f00184152ccc38ff94c99eaba27ac32", 1523909845, 494238365);
			_checkpoints.emplace_back(92736,  "9f2e7f0c8668cb37e7e249123ba5436239245c3a582aac2cfa00e63f84fd7259", 1525118590, 494040753);
			_checkpoints.emplace_back(102816, "21fbf33a8ac97d4da0660fc8cf1d4b42ced6cd39cb6ff9f81aa4ed898b4adfb2", 1526317611, 492534043);
			_checkpoints.emplace_back(112896, "db22a1202b815fbc483a6a34f7faca80286351599082b1beb1c9155a6fb35b03", 1527524832, 492355105);
			_checkpoints.emplace_back(122976, "4755a56fa7a9bfa12125d573bf4c47af1caaf3b749de890bf1881d9de04778e2", 1528725950, 491428429);
			_checkpoints.emplace_back(133056, "668fcd9479fff7bbd267746046ad9fe1875be00d34fa67e300d83d199e9e5624", 1530001812, 503350194);
			_checkpoints.emplace_back(143136, "c7890d31f73b85c03593f7c6dace0a7604e6709519cff75cf945b7c83b0f7aee", 1531153562, 490135368);
			_checkpoints.emplace_back(153216, "1bc095d2c47131e1a4e1d08ca533b9caac30008e319441b7d202ddfe75e263cb", 1532368202, 490129493);
			_checkpoints.emplace_back(163296, "dc35e7815ba7f2def7fb656cca1bde709ebe99cd4f66c39ac573a2763be559d9", 1533576903, 490408901);
			_checkpoints.emplace_back(173376, "ce52eca3db14476ae7b01faa0258bc21b4b93fe0b0364342785cf479f213f851", 1534787121, 489990925);
			_checkpoints.emplace_back(183456, "03665026718f02ccb9c764aafe20ab5c1f4963cf7eac6931a300d40b28343f59", 1535433865, 457157190);
			_checkpoints.emplace_back(193536, "ad48cf72bbee8634044ac24dd5ea207c0ba8d6e0dec9b97a64b3eea55c73aa79", 1535895095, 407453180);
			_checkpoints.emplace_back(203616, "5e230d36db48b3951bd7d4979d25aa18a83c24941b083d63bfef286489acce36", 1537105427, 407173769);
			_checkpoints.emplace_back(213696, "d01abf397fd29c2ba66da728c35834aa0c8a21a7d1c9e6f2e7db4c90059ddd26", 1538328848, 407607682);
			_checkpoints.emplace_back(223776, "ffc80b94b160752fcb89e3d021f65040f5c0861d23625d7437930215ecc11b48", 1539537072, 407725582);
			_checkpoints.emplace_back(233856, "5ace1133b6a861b20a4b3885c686b3f6d54829504105019d28b51358721e12e8", 1540749706, 407616602);
			_checkpoints.emplace_back(243936, "e14e533e6d6640ef026f29cc1ab14eaeaa4cd999072ca11d8d15e066665c688f", 1541951161, 407273974);
			_checkpoints.emplace_back(254016, "6803d7f909508a3d36704edffc5acfbf87e02d1fc2250257d862f173aca9ed0d", 1543162984, 407276433);
			_checkpoints.emplace_back(258048, "02fa157f43a6fae7f464c025ebbadc778dcda972df712918b410e3d136a14e12", 1543653867, 407635758);
			_checkpoints.emplace_back(262080, "51d8b2092a2a3b56a29190f09c56056a8165fb2dd185549e4541f902031557a3", 1544131384, 407053338);
			_checkpoints.emplace_back(266112, "cf2cb62e7b461ac35b06392041767a8159e32db9e035ad5d8e1fa95ee5e19681", 1544620959, 407339006);
			_checkpoints.emplace_back(270144, "23d54cde315f656c924686302b4ac5bbd4e9c56715093421ae10a0685be12f9a", 1545105908, 407303582);
			_checkpoints.emplace_back(274176, "9d80d127af6138ccc7ff5de638e15dbfbf9253d5e6c3041263e2fd634ed7a8d0", 1545585364, 407176107);
			_checkpoints.emplace_back(278208, "281b84c7faaf810c3907867a3cf6bc01e857bf74951e3332e53fa71e1852690a", 1546072809, 407016279);
			_checkpoints.emplace_back(282240, "0e7a54742d0e1d7a23c7b0c0cea8ed4bc227bb09914bbd7577371650935196b3", 1546557064, 407056627);
			_checkpoints.emplace_back(286272, "ed0077cd99df8cccecaeb42fc86404dda8625c282ddfd1f0365726ecd32e56a6", 1547045132, 407171830);
			_checkpoints.emplace_back(290304, "4112fc59859c358733906c70184baf9c1e5146c35b04e7e282b045ef660ae21a", 1547533574, 407443760);
			_checkpoints.emplace_back(294336, "7dd4e7c122b4879a8a1daaab0b85d00f017ceded606c66fe3a4efb558ddd10b6", 1548016365, 407078205);
			_checkpoints.emplace_back(298368, "e5a2559e09a60cffdf1149e8811add9ffe10eaa0ff81461fa6466e03723b9cdf", 1548503892, 407430198);
			_checkpoints.emplace_back(302400, "570941723956204c77b07ac6771cc9bd0fe754872fdc36a38c10fb926fe93101", 1548990307, 407560029);
			_checkpoints.emplace_back(306432, "293dade529079148068dc18a154743d5b076099f9102b9d7cbd79c289038339f", 1549474118, 407435849);
			_checkpoints.emplace_back(310464, "7162d31f3a397f0404c379c3394485e97ca7c88f80dbdec98e9787bed953a20e", 1549957439, 407342345);
			_checkpoints.emplace_back(314496, "bba5bc0a3bbf3d714e8f414d096440eb18586ce9bed9a006e91e79745ea12566", 1550434616, 407071079);
			_checkpoints.emplace_back(318528, "8b75b943b49d711b82427644b769da5847122bb16b6041811b0ce7928565d87b", 1550923897, 407321316);
			_checkpoints.emplace_back(322560, "8ca8aa2c69bcc45727396ffc6eee0ed2019ea2b2fc4ff17ced85796a18669cff", 1551388640, 406145876);
			_checkpoints.emplace_back(324576, "aea8458dfafe6e6ce7a325220cd9c2ef422a41cc8a90aa0364699dbda62183d7", 1551631041, 406048162);
			_checkpoints.emplace_back(326592, "ebb2ba952f8bb5b1d005f6c5355970ba3351b23f48e6965b7007f7b4e29ee92c", 1551876151, 406029313);
			_standardPort = 20866;
			_magicNumber = 2017001;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		void ChainParams::MainNetIDChainParamsInit() {
			_dnsSeeds.emplace_back("did-mainnet-001.elastos.org");
			_dnsSeeds.emplace_back("did-mainnet-002.elastos.org");
			_dnsSeeds.emplace_back("did-mainnet-003.elastos.org");
			_dnsSeeds.emplace_back("did-mainnet-004.elastos.org");
			_dnsSeeds.emplace_back("did-mainnet-005.elastos.org");
			_checkpoints.emplace_back(0,      "56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3", 1530360000, 486801407);
			_checkpoints.emplace_back(2016,   "df060a6475ace78657e1ec945cebb37f2ffb6367b185f9ba5edfa987f16b84f4", 1531426582, 520126740);
			_checkpoints.emplace_back(6048,   "4e6a53d3e3e61d4883a10d62fb42afb62ad6f10807bcc3791db284f43b063671", 1532395676, 522028475);
			_checkpoints.emplace_back(10080,  "00ac65927dcf741e87b0560b9676a35abc5d1ab74afc6144f4159315f15561d2", 1535069960, 539089007);
			_checkpoints.emplace_back(14112,  "d342ee0817e30a57a515ccfaaf836e7314aac1235d0659d5e6ffc46671ae4979", 1535367638, 527395580);
			_checkpoints.emplace_back(18144,  "fc4e7ba460e38964faeb33873418c9fcd3ae2648a543cc1e8f5c47806b593ac8", 1535503457, 504235070);
			_checkpoints.emplace_back(22176,  "9ef0a2d21af1f1ebba5b092ce0f672b221ea7d1f2e765790c0741aaff7667da2", 1535640920, 486964831);
			_checkpoints.emplace_back(26208,  "a9eb520903f5f1ec1a11a3f5040791d59ae63b8e16846a994bbd303ba43b50db", 1536008684, 475415774);
			_checkpoints.emplace_back(30240,  "470d858db326df0e738248ac5f5811f1b31a0c1ca1892f17a93673efefde3a45", 1536500538, 476055185);
			_checkpoints.emplace_back(34272,  "f78fc41c0d2ba1c0c133ebcb351ecfedc42324a31201b6c207d0165016430a74", 1536991969, 476548897);
			_checkpoints.emplace_back(38304,  "9592ad50c220ea2900d0c3659df470b4e8e459ac6ee2d43ddc9b6ddd15166645", 1537492347, 477909524);
			_checkpoints.emplace_back(42336,  "548c15f8daede3d85aed4a6ad8bf013c6cb2b3c7ea7aba0134ffb2adc450850a", 1537976718, 478079428);
			_checkpoints.emplace_back(46368,  "f9e8780e4df7eeecf80f8cb8fcdd2f08f2357da9f50557235373c20123e9fa45", 1538494105, 486580306);
			_checkpoints.emplace_back(50400,  "b877aa70a40ec714b211c19d0f00060f86e659acf2766bd2a3d6a2fc24f29a6d", 1539105182, 486700362);
			_checkpoints.emplace_back(54432,  "e40aab4302d385b0ec4cc85ca8a1c1a8c338e2b01d0a67c70e7f9bc0b7e4995b", 1539618297, 486739142);
			_checkpoints.emplace_back(58464,  "800c33ab6faf802559f94106f9012f734ff08a2f183f6eccebc3016735557602", 1540197515, 487108954);
			_checkpoints.emplace_back(62496,  "760a11232f9fcf83dd30a17f9feac767c3a1158371fa0e051b1f2956e8b96ecc", 1540686763, 487128669);
			_checkpoints.emplace_back(66528,  "886813b522b8ed25b5ed49a466a60dabc9ec873505f6febf650013d0de8ff760", 1541180178, 487200624);
			_checkpoints.emplace_back(70560,  "a8efba9d67b13d05b1037b8a53ee917c19e1f0505808b8ba345ee6dd29fb2f5e", 1541773076, 488572069);
			_checkpoints.emplace_back(74592,  "63ba136d741cdc27fa7c03ce438c1d2b9caf7540a3be801c967c09697f0458dc", 1542924090, 503654166);
			_checkpoints.emplace_back(76608,  "5936928083508074454609d054f1da99b1b9342b5df46dcf1af87347382f9d3e", 1543749293, 520134229);
			_checkpoints.emplace_back(78624,  "5ca5f44be2515483c55db47b905a440565475c63327dfab2d105833f4720f593", 1544008452, 520140813);
			_checkpoints.emplace_back(80640,  "4e6d93b893696d7682c2e05c1c625525a59d39a9ad7f192fdfd21401f052262e", 1544288813, 520166012);
			_checkpoints.emplace_back(82656,  "0e38cfc8af645a171591105cde90640287d606fcb891c10c6c1e1204b8577dc6", 1544541545, 520173465);
			_checkpoints.emplace_back(84672,  "2e6bcea30987d6adfced5eaef28fe18ad1dff8924bd42fb7d5ffc6373736f3c0", 1544789780, 520192062);
			_checkpoints.emplace_back(86688,  "441d3794627823a55cf8ec3c300bb10c8f63651dbc76db8c22739ac8a5336b3e", 1544974030, 520127197);
			_checkpoints.emplace_back(88704,  "7d5048c2f1639ad1743cbf340f0006e03cc194b9e1bc732e4c3e71a1aa599160", 1545220162, 520128038);
			_checkpoints.emplace_back(90720,  "5155b5bdb51127669b7f7e456f60aa0ae4c69611d3ebeb51e4fada9f61aeca9f", 1545465318, 520129181);
			_checkpoints.emplace_back(92736,  "96ad3d3f88e1b9fc5b78f917cd7cdfd410100463918274d9c3c3b8e493fa4bcf", 1545716545, 520130779);
			_checkpoints.emplace_back(96768,  "f09ee323bfb0a22264dcb8751d446c2255f0d6ee607f3237221a8596c2a96778", 1546212379, 520137936);
			_checkpoints.emplace_back(100800, "9d8f322841672c45f46ebd699551764a7d349e8f002982d21e84063e3f82bad3", 1546700197, 520140135);
			_checkpoints.emplace_back(104832, "120f1c4ef974eb2b9e5e52ce8cdb50947c5f3f0c621817aaa7c4b3cffaef7611", 1547192867, 520144354);
			_checkpoints.emplace_back(108864, "172ec9dd0f25048287a3814c1fc76631f0089a1b510e52db28d7c205cb0e42f5", 1548506898, 520897403);
			_checkpoints.emplace_back(112896, "1da20f89b914016e44e4d09bc30e85e7eb7aa5b2e6116a34aebcf764adc81556", 1548999909, 520999282);
			_checkpoints.emplace_back(116928, "66f2cddd1043eb61ccc5175fd95e20139d1c0d6a5000cff33a51ab9d36cf2551", 1549525236, 521471623);
			_checkpoints.emplace_back(120960, "9f9901ba95043d9840c177aa71dd91c6a8bc85613e364b2c14a21e6b17561d3f", 1550053282, 522277229);
			_checkpoints.emplace_back(124992, "e04faca7a4f4c39154f13c4afb5a39c0ec92c29810e230ca5cf088c54afdf730", 1550551272, 522606384);
			_checkpoints.emplace_back(129024, "6b5bc4a25e1a7d6e32e77d0ff6117798cda5a209bfb26063c9335a8fc035f40a", 1551070409, 523617671);
			_checkpoints.emplace_back(131040, "d82242161807d283f0cf7b23d958e8d976a042ba4f76be9690d8daf5b3d72749", 1551286214, 522744768);
			_checkpoints.emplace_back(133056, "0c8700398106cf79a90fdff85aaf49ed91a2247582b0ede35d19de5e3771359c", 1551537603, 523094962);
			_standardPort = 20608;
			_magicNumber = 2017002;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		// test net
		void ChainParams::TestNetMainChainParamsInit() {
			_dnsSeeds.emplace_back("node-testnet-002.elastos.org");
			_dnsSeeds.emplace_back("node-testnet-003.elastos.org");
			_dnsSeeds.emplace_back("node-testnet-004.elastos.org");
			_checkpoints.emplace_back(0,      "6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3", 1513936800, 486801407);
			_checkpoints.emplace_back(2016,   "99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767", 1517940844, 503906048);
			_checkpoints.emplace_back(8064,   "fa9b768a5a670a8c58d8b411a8f856f73d6d4652261530b330dee49a8319a158", 1522645593, 494006531);
			_checkpoints.emplace_back(14112,  "20931a5184a88116d76b8669fae9fe038ca3e5209d74a64ec78947142c16a95e", 1523380720, 494547269);
			_checkpoints.emplace_back(20160,  "0fd0ecfabdd3f3405b9808e4f67749232d23404fd1c90a4e2c755ead8651e759", 1524792298, 503351573);
			_checkpoints.emplace_back(26208,  "d3ab4cdccac516981d90b4eed4956d23a5c6cda76b033386754a7493f3e2e54c", 1525542345, 503358411);
			_checkpoints.emplace_back(32256,  "0c03acf34f4cd5df02062e658f91dfe884788831b940b18a136f7d33adae00b0", 1526270878, 503361032);
			_checkpoints.emplace_back(38304,  "ea313df1089b8b001c40565aa5e90d17da51885a29459b966d0019ac7dd90002", 1526997348, 503361573);
			_checkpoints.emplace_back(44352,  "77fcd9c9e3a875b46d0029805cfe3bdb06f7beef49ea40eaadb20368cd0bea2e", 1527704178, 503349609);
			_checkpoints.emplace_back(50400,  "176751ba71545428b0a1740abcd0f1846b97b1455fbbcb27a5c778a575fa8b71", 1528433088, 503351979);
			_checkpoints.emplace_back(56448,  "d6faaa1526829d58cbd2c7afbabdb7bffefcc5494d808a79ad79d9d6d6fb17af", 1529153218, 494866089);
			_checkpoints.emplace_back(62496,  "8cb6da886b6258e8a6daefce9d5566575bd2ba95b4bd684105954ab9e69be042", 1529864598, 493529298);
			_checkpoints.emplace_back(68544,  "c7595156595f1b925f5dd3aeeaabc9c3b3605e5d9ba934c396a31126398a10fe", 1530583548, 492729858);
			_checkpoints.emplace_back(74592,  "47ea897ceb1ad471b5a7277e348ad43456b08d149db39b419174cb295be4fed8", 1531345056, 503351198);
			_checkpoints.emplace_back(80640,  "db4f79022b23ffada435eb3d0681deb11a1591428044cf4625188f1c6277f9b1", 1532146578, 503381165);
			_checkpoints.emplace_back(86688,  "a25b7207c98cbbede93db67ca41a0ac308d198c556eea955ff63f52b0b70131a", 1532870279, 503379835);
			_checkpoints.emplace_back(92736,  "c5134b4d7b275a6109cf2f1e7c4f31ad96a4c6c19d2bfd98e7b655d9d49d5723", 1533598732, 503377199);
			_checkpoints.emplace_back(98784,  "552732e51a44b3cf99992447b9b3f79226fa90f0f9f1b47e15836abc2c334322", 1534328247, 503380456);
			_checkpoints.emplace_back(104832, "ec5c35a62347241f33949ac5b072af32a3fe0f3e9cc1ccbabe7598629c94940a", 1535057104, 503376374);
			_checkpoints.emplace_back(110880, "743823d8e5d1e74aa7aa8de9a20ca3b42dad4c1115b1631d6071d66d68f6a4f9", 1535780560, 503376620);
			_checkpoints.emplace_back(116928, "7151ea9c0d36479c9f6e92776b88f9f925bcbcddcff44deb7463205dee8f06d3", 1536517600, 503382324);
			_checkpoints.emplace_back(122976, "e8e5c0ca094d156fb5ce864c3a3ec68925de51a187060920b1252936b21356cd", 1537239070, 503382468);
			_checkpoints.emplace_back(129024, "1e08db102db105f1d58703da1153322001533a3d4fe31a794dfb118600f1a615", 1537969555, 503380928);
			_checkpoints.emplace_back(135072, "681fee1c6ec452b1c38b5aadb26b9b60371b8104136bcef628c5b828643aebf9", 1538701705, 503383651);
			_checkpoints.emplace_back(141120, "8c1451e25bc148f16c99121945fcc97d03ca4ef480fa55b84611cd6b52672889", 1539676171, 503378272);
			_checkpoints.emplace_back(147168, "4a0ffb895b654fc46d068eb8110d715c176119ad13e9cffdda5bf88db412590c", 1540414770, 503373743);
			_checkpoints.emplace_back(153216, "e648b4fcafe6e408d84114a44b4e9e53d194bf5df187d00481eeb1a30e5f566f", 1541250942, 503380880);
			_checkpoints.emplace_back(159264, "a72b0aab079818986cca9fbee3155fe9d20c7cfb64a1aed3edfa603d6d688c6e", 1541946004, 503356074);
			_checkpoints.emplace_back(165312, "c4aea704d135021876cebe2f7801af6d2f9e1258ab0f53814989c7631cbf890b", 1542741008, 503404457);
			_checkpoints.emplace_back(167328, "0e81022030c88a719b5d60b638a9d9e2ae0b0688eb309ff3ab47c82975936daf", 1542969353, 503382380);
			_checkpoints.emplace_back(173376, "20c8f646071b2d9a41abcecb55bf09e2af7caa1c692f571e8f6e2273cb547738", 1543844436, 503357414);
			_checkpoints.emplace_back(177408, "3e3d6a71daa032e5e6674f140cdf8c2766d7da0ef8d69fe77e5111512fa23f49", 1544330910, 503357435);
			_checkpoints.emplace_back(181440, "3254db7fef1b6590f1a33abf88d2b40a6a49bddc7d688339fa61770271b78cc3", 1544807137, 503353944);
			_checkpoints.emplace_back(185472, "111049f6eb751601ce15aa288e7e813baf3794fed70eb812394dc72ef4cb2c4c", 1545309920, 503355950);
			_checkpoints.emplace_back(189504, "edbd2e2f4d7d02ef602b9d52ba7baaebd72f39388c57d915f08e02a2d71f8a1b", 1545895133, 503441377);
			_checkpoints.emplace_back(193536, "55a3ab4a6683d4b4795dfa01383596521cfc5828931e0e57dc7cda96d35d9611", 1546496103, 503678530);
			_checkpoints.emplace_back(197568, "480c6edab668ad1b9086a7cc14b868d6e457eeb98e016e1c41760f04606917c1", 1547086808, 504513304);
			_checkpoints.emplace_back(201600, "c0a47ea9657556d1e558bc6e2503b90a4f6b5d1a80925e26ce291fd0d377fabb", 1547680847, 507371501);
			_checkpoints.emplace_back(205632, "92dc4c71364962f41e0cee240bb6415fcdb580806c255addd0f417ee77505715", 1548258881, 520133124);
			_checkpoints.emplace_back(209664, "37af774031cca5f819079eab9c9d11e4b5306480555910a02cebc89995d3e995", 1548771633, 520151345);
			_checkpoints.emplace_back(213696, "a7b79bfe30651f1b58abb5d4088d3373f99fb3aa651caccf0173cd1bffc5aa06", 1549290927, 520176927);
			_checkpoints.emplace_back(217728, "c9e12fcb1ae475b941efe6621f661830103a813521f496f426495a538f3786e0", 1549798122, 520204153);
			_checkpoints.emplace_back(221760, "80b9c71e6a913821b47d21bd1396996be62d940197960091b22cd7cf152d520b", 1550289969, 520213088);
			_checkpoints.emplace_back(225792, "f79f7efa84b78907a04ebccbaa9b47a2aa7ffa3f8e2fdee5ab6487fe2f0bd652", 1550800956, 520254139);
			_checkpoints.emplace_back(229824, "8ed32800d95e6703a871726ceb68faed1d5a8b047dd625c7ea01d76c5bce84a6", 1551313167, 520311485);
			_checkpoints.emplace_back(231840, "6ca4aafd19df42379062f677ad0b7147e63d42cf115d32d41e5fd201986426f1", 1551559204, 520321991);
			_checkpoints.emplace_back(233856, "7c8b242b3366e1ebf1a99b92cb20067df7a451bb2ee20bbeeb6083273e4e2048", 1551813488, 520346523);
			_standardPort = 21866;
			_magicNumber = 2018001;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		void ChainParams::TestNetIDChainParamsInit() {
			_dnsSeeds.emplace_back("did-testnet-001.elastos.org");
			_dnsSeeds.emplace_back("did-testnet-002.elastos.org");
			_dnsSeeds.emplace_back("did-testnet-003.elastos.org");
			_dnsSeeds.emplace_back("did-testnet-004.elastos.org");
			_dnsSeeds.emplace_back("did-testnet-005.elastos.org");
			_checkpoints.emplace_back(0,     "56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3", 1513936800, 486801407);
			_checkpoints.emplace_back(2016,  "9b3069a05478988d4c9d2d4b941af048680dbe92353bc8f0cf282766aa935edb", 1532276131, 505977014);
			_checkpoints.emplace_back(4032,  "6e8f5e21ddd736bb62dcd3ae445444702dc0b0ee560b4161877d0c8f0d9ad448", 1532628770, 510283913);
			_checkpoints.emplace_back(6048,  "8cef466bdc7c9ed79fdd6b36babb5b6670c916c07d87b473327460e57d21799b", 1533004749, 520187129);
			_checkpoints.emplace_back(8064,  "46a9dd847b62f278a26a93e3f7a4cc135b30754e07f2754d6ac0baf6870c8060", 1533346700, 520376318);
			_checkpoints.emplace_back(10080, "09cdb3d720a7095c7023241361ca8a317c75273fbdd0e5dc268667683e360780", 1533694521, 520890958);
			_checkpoints.emplace_back(12096, "2790468186eb341dfe06fc6b8a66ad48d2bfb597d0f612e2a917d703c951aee1", 1534057013, 521637984);
			_checkpoints.emplace_back(14112, "f5582a7bab4ec851172dd96b9c18d8a046081de65ed05f316e4dc14a5a8fe176", 1534506459, 536912421);
			_checkpoints.emplace_back(16128, "8b78d2fee6751c82d419e5eb98bbf83a1514a046ae9ffd1462aceaeed3f189ec", 1534771533, 536935584);
			_checkpoints.emplace_back(18144, "ba98bdbeb358fc2ac565dd3a6f706cef7dfdc60063f1f3517311744c3a908de4", 1535034489, 536952176);
			_checkpoints.emplace_back(20160, "b4d0dcec130dccd6cfa61880cdfe30259e90c69d1f83056f0495910cca05246b", 1535300757, 536980894);
			_checkpoints.emplace_back(22176, "9738541d86b7843be83926be997164615a3102e33edece26da351fdae70d2d6a", 1535557289, 536997059);
			_checkpoints.emplace_back(24192, "e9198d6fdd89d292f7e967266c22c59e245d015d93c0428626211965b8349d05", 1535828463, 537035498);
			_checkpoints.emplace_back(26208, "8636093320982e3707224cb19252c2f2de124bd1c480c3e4ba35f6d564e2d713", 1536086668, 537085786);
			_checkpoints.emplace_back(28224, "b98d6c3e7f424d44bf4f21f9093be7b2d29f2134137949769718c33ae22bd205", 1536361613, 537169686);
			_checkpoints.emplace_back(30240, "8c43de5c79cd48da6754b2098fcb230d525b7b0f1e44fc7c87b2b38064269991", 1536626736, 537255791);
			_checkpoints.emplace_back(32256, "a8000672bff80dcf7117062073df6f3cb511be0e49c2862f91432a00eeef053e", 1536886078, 537301386);
			_checkpoints.emplace_back(34272, "640298c39b33c1298d87e482115053177ae60b721e23801db0061bb4e24d393b", 1537146343, 537429158);
			_checkpoints.emplace_back(36288, "5d2a3ae65b42c7233e2fcbfc2a4f86eb24a693ab7e3b47c38830e57c956ff49c", 1537593491, 539041984);
			_checkpoints.emplace_back(38304, "3763dde6fa684f31615b4237a17116777bf807a19701c3d5d02604e9bdcea1a6", 1538035204, 545259519);
			_checkpoints.emplace_back(40320, "25ad2ec3bd7531301d367b4f7b1357e567665213c776744775fed7f80b9b0349", 1538398560, 545259519);
			_checkpoints.emplace_back(42336, "18086d3158974bbc1a331b194e0580504ed76058d88da84fc446b0020b63c329", 1538663566, 545259519);
			_checkpoints.emplace_back(44352, "fed3a4a23a70eb6aebe0db46d509b9f239c31f48c3d9e8c8a516ce4690fbc0b0", 1545058592, 545259519);
			_checkpoints.emplace_back(46368, "d177b074f16f0affcaade7c1da0484d7f10f6faf02e421b8b915496d6679bd82", 1548462469, 545259519);
			_checkpoints.emplace_back(48384, "99fa8265769bc8bb752c1b5614e4ac6c3114ecefb7de3484d77359a51e20b754", 1548730334, 545259519);
			_checkpoints.emplace_back(50400, "ed572ae38820c513227570edacc6aea88968776160fade9e291bcb5a0d877311", 1549118884, 545259519);
			_checkpoints.emplace_back(52416, "5295b35d8ebcedd0ae482416b28491441a2de2c349c3b415bdb6e632ebfb5e7e", 1549380587, 545259519);
			_checkpoints.emplace_back(54432, "7a47859985f959b896bbb395aa90abd31fc0b39318bf0008582950fc8a044248", 1549637254, 545259519);
			_checkpoints.emplace_back(56448, "7be1ed5be84c360a60bbe742bcbe267e32b7beadc01189759223365bbeb62823", 1549889972, 545242916);
			_checkpoints.emplace_back(58464, "ce9fc50902e64211e2c0d455a5906585ba0e3522542cdd2a0b6649afe9e8e6c5", 1550147031, 545259519);
			_checkpoints.emplace_back(60480, "a96712ccdcc2b22a65ee7276ed860901ffe0fd46e26fefaf6827189bb2d84e09", 1550401220, 545259519);
			_checkpoints.emplace_back(62496, "c38b1bfe4c3e094975ed607b395e8f7b31c3723dd24bfbd47aa574ecca1502a7", 1550682718, 545259519);
			_checkpoints.emplace_back(64512, "841adce0de01b403585a27040370058f96fe87681df1e53c724c3890ae81f3ce", 1550950944, 545259519);
			_checkpoints.emplace_back(66528, "28b8e264995934b9f42fe145e19f3afffeb8374f9fa603b78ef671763836562f", 1551280389, 545259519);
			_checkpoints.emplace_back(68544, "cd63541e5f864fe29d8778140e42dfe58dad46ae6343036c6e53c1f25de5f97f", 1551576878, 545259519);
			_checkpoints.emplace_back(70560, "86a52104b27dd1a41101326011daaea815083808faa6d066e7daa12a76ee34fd", 1551840630, 545259519);
			_standardPort = 21608;
			_magicNumber = 201901251;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		// reg net (reg test)
		void ChainParams::RegNetMainChainParamsInit() {
			_dnsSeeds.emplace_back("node-regtest-002.elastos.org");
			_dnsSeeds.emplace_back("node-regtest-003.elastos.org");
			_dnsSeeds.emplace_back("node-regtest-004.elastos.org");
			_checkpoints.emplace_back(0,      "6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3", 1513936800, 486801407);
			_checkpoints.emplace_back(2016,   "99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767", 1517940844, 503906048);
			_checkpoints.emplace_back(8064,   "fa9b768a5a670a8c58d8b411a8f856f73d6d4652261530b330dee49a8319a158", 1522645593, 494006531);
			_checkpoints.emplace_back(14112,  "20931a5184a88116d76b8669fae9fe038ca3e5209d74a64ec78947142c16a95e", 1523380720, 494547269);
			_checkpoints.emplace_back(20160,  "40eec8f7f3c7f4d9ea4c7dbe5eb85456c72ee79f7e5940a5e7d5f4c80bc1e7a7", 1524736650, 503356298);
			_checkpoints.emplace_back(26208,  "3d2269ae53697c648a40aabaf6841cd2a43bc38699264a64bcd166e000cd452a", 1525472220, 503357792);
			_checkpoints.emplace_back(32256,  "9ba5fec5acaf23302110e333b022a074de88b4e36b9264e98297cbaec09f591f", 1526225370, 503370323);
			_checkpoints.emplace_back(38304,  "9307bcfe0fe867a824e3a9139d626a607aa1c09396c027df8fa243b4110f499d", 1527042222, 503415621);
			_checkpoints.emplace_back(44352,  "cf8516eb910306862bcc206519b0e6eb187dc20ce384ca525dd1c0ec751c92f2", 1528221733, 503374579);
			_checkpoints.emplace_back(50400,  "8e34fdc18ef27996022cc36d85f856690990faa077d92b78989c04f113205cd0", 1528926400, 493749763);
			_checkpoints.emplace_back(56448,  "74d8765907eb63ba286d2d8373de916c25c5daf5250c25d652597c2422e0bc9c", 1530223515, 503358059);
			_checkpoints.emplace_back(62496,  "398e154da8acf7565af3a8cae969b2aa8cdf0abb5817bcb0e2b64c8155557416", 1531091111, 503350455);
			_checkpoints.emplace_back(68544,  "2b24892cda519c15de01dd18298853c5dcd5f278c8a92ccc7e1060ca2f206c71", 1531796891, 503349562);
			_checkpoints.emplace_back(74592,  "f7afa994b23a5628f109e1de8563f4ae5963a814b1c972425d921d581bcec7a0", 1532636667, 503374250);
			_checkpoints.emplace_back(80640,  "bcadac789eec2262f099cc0f57ef53937520ddbc298cb2341d00bce1cbabd7ec", 1533423380, 503380311);
			_checkpoints.emplace_back(86688,  "7de2bfb514c7c466aa4903d47d9e5ea89d8cd13e60fc134b1ac7a6337499fcab", 1534151990, 503377735);
			_checkpoints.emplace_back(92736,  "d19883bc202ef4802f73bd18ce2456c8c933e9a2793bb566efc21cf3a48fe52c", 1536149363, 503375367);
			_checkpoints.emplace_back(98784,  "a8efe800d2a18bf7dca867e307561d11e33abe20fc8a6c20e82f05efcaa3f6cd", 1536880958, 503376323);
			_checkpoints.emplace_back(104832, "bfe6f03363b445f7c6d6acbb8b8ab71641178e0d1f612fc459a622b19c741377", 1537577738, 503359020);
			_checkpoints.emplace_back(110880, "3f2ea95ca953b97a638a24627a41826973a485fc7c0e661ad7c6cb5870910e5f", 1538309387, 503358893);
			_checkpoints.emplace_back(112896, "8ac3137dfb231d3f89878f28342791e457efb029c6710ead8ed0d759ae6dc8e2", 1538558993, 503359813);
			_checkpoints.emplace_back(114912, "4d2197b83c20edac4125a9414b56d0a9a0f0252858c7eb95b50805626f1c7256", 1538794988, 503359557);
			_checkpoints.emplace_back(116928, "4be50ea1cba8d75933f388969a15802b7273d9dc557a667265921b712b0e208d", 1539040448, 503361516);
			_checkpoints.emplace_back(118944, "103d8bca96229bc76d7c1df5f203a3d4a6cdb378041477a74639c00a530bab13", 1539284093, 503361471);
			_checkpoints.emplace_back(120960, "291a553a23883ccf0792e0505f79596e1252d7c2cc825de8f96fca40ea26d0be", 1539522368, 503359408);
			_checkpoints.emplace_back(122976, "ab5c2a7b606ade0623951c7126400e661f10c0ab82bfda956e443990f8c49a7f", 1539767258, 503361461);
			_checkpoints.emplace_back(124992, "5420b6b629e4c5341da3695d795465426eff4326a09be83063cb452b26ca8635", 1540008863, 503359169);
			_checkpoints.emplace_back(127008, "71fdd19e1bd779d6f11e127de7782f39ca0d5476a9158ead02419ca4bddbe8fc", 1540253228, 503360931);
			_checkpoints.emplace_back(129024, "d2a8414d797b581e255c02972b2505e3e828d6142135b7fd119efed2b2da2917", 1540490963, 503358925);
			_checkpoints.emplace_back(131040, "580e961870022c0579148b41944d6a2b80d03c6b4cdd585ba399c67349280838", 1540733044, 503358919);
			_checkpoints.emplace_back(133056, "9aef3347ed7af0e4546ecf9b342f3c209c37825eaab796f4b4cc0ec1687e6160", 1540976708, 503362036);
			_checkpoints.emplace_back(135072, "1be111acd7ede75f84ebe43a7e1241646d38097bd48b4d7feb39911bdb4ff27e", 1541191558, 493484297);
			_checkpoints.emplace_back(137088, "e77f78b3c6adb8143575fc551d5056f9f797fe705277e1fa87c42682a100600f", 1541589779, 503371235);
			_checkpoints.emplace_back(139104, "a6d2f7414828a267ff1bcee404bd5c676777021e8b15714669c920db98f1831f", 1541890309, 503374077);
			_checkpoints.emplace_back(141120, "eb95cff77582810bb43c029dcccb8f7fefef3962ea561a289ea06fb290c92ceb", 1542184026, 503354742);
			_checkpoints.emplace_back(143136, "3f9095f4d7e7819ec4e1687e982c491957870e32c02325d2a50703d73cb1db27", 1542437751, 503366694);
			_checkpoints.emplace_back(145152, "72433991bf9cf92b722bbade88a04c3429c31af0510949664a230c679b334276", 1542676270, 503354551);
			_checkpoints.emplace_back(147168, "a723418d93697fc2393cdd585d402f73a7afec40eb7487461f60a6074f9a3d28", 1543204596, 503610078);
			_checkpoints.emplace_back(149184, "612c08e7c956d77c755499f51b7edf3dafd86def890d0721588af5a472829d64", 1543402000, 494830249);
			_checkpoints.emplace_back(151200, "86403be084b80919cbc5583694019bc4a88e2a122e192ddf7c0de1c2b3cbbdbd", 1543879438, 503472236);
			_checkpoints.emplace_back(153216, "01331560094a35d1116585be45f084dcb0799a9b1bad515616196dc5ea981792", 1544445272, 504178829);
			_checkpoints.emplace_back(155232, "4dfaffbba8e43c9543d3fe57e5e495b2437c2cee0679c8ec37a529cbf13e8d3c", 1545018237, 520137200);
			_checkpoints.emplace_back(157248, "992bc7cd00977741ca1f762359331a92ae8a0e6520d12d1d506b22cf9ab5db77", 1545541751, 520571185);
			_checkpoints.emplace_back(159264, "18a003ed7c6973d3b86948f3f5955e6c10bd28fd7b92fb782e84db1c3d560d6f", 1545865242, 521404356);
			_checkpoints.emplace_back(161280, "864eb79f7527e24fcecdcf7b99c28458f5d6ac5f3f965acaef34d91e05c1e0f4", 1546104316, 521336930);
			_checkpoints.emplace_back(163296, "ab8004f64fdabf4592c81aa6335044e4b29390c2f94923024be5471ca321403f", 1546326011, 521126789);
			_checkpoints.emplace_back(165312, "e1ea9c5ea4fc9c8d58fe74f788525b638261e94276956f9fa8a2fe9deb6a1f9c", 1546677050, 522266848);
			_checkpoints.emplace_back(167328, "6e2f2e1c1d1bcee1cda1cf231548f298395d1ff3fe799a7448ce72ece4be142a", 1547022886, 525706056);
			_checkpoints.emplace_back(169344, "a86b24ebfd01c6db2cb79043ff9edbd16edfc92f4f93dbfab1e579fa63286e1f", 1547372501, 536931941);
			_checkpoints.emplace_back(171360, "9aaa94df9967ab17ebdeaa092dc7a7bdd8bce560374312881d9ad076cee95e01", 1547677822, 536991628);
			_standardPort = 22866;
			_magicNumber = 20180627;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		void ChainParams::RegNetIDChainParamsInit() {
			_dnsSeeds.emplace_back("did-regtest-001.elastos.org");
			_dnsSeeds.emplace_back("did-regtest-002.elastos.org");
			_dnsSeeds.emplace_back("did-regtest-003.elastos.org");
			_dnsSeeds.emplace_back("did-regtest-004.elastos.org");
			_dnsSeeds.emplace_back("did-regtest-005.elastos.org");
			_checkpoints.emplace_back(0,     "56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3", 1513936800, 486801407);
			_standardPort = 22608;
			_magicNumber = 201901171;
			_services = 0;
			_targetTimeSpan = 86400;
			_targetTimePerBlock = 120;
		}

		void ChainParams::MainNetParamsInit(SubWalletType type) {
			if (type == Mainchain || type == Normal) {
				MainNetMainChainParamsInit();
			} else if (type == Idchain) {
				MainNetIDChainParamsInit();
			} else {
				ErrorChecker::CheckCondition(true, Error::InvalidCoinType,
											 "Unsupport coin type in coin config");
			}
		}

		void ChainParams::TestNetParamsInit(SubWalletType type) {
			if (type == Mainchain || type == Normal) {
				TestNetMainChainParamsInit();
			} else if (type == Idchain) {
				TestNetIDChainParamsInit();
			} else {
				ErrorChecker::CheckCondition(true, Error::InvalidCoinType,
											 "Unsupport coin type in coin config");
			}
		}

		void ChainParams::RegNetParamsInit(SubWalletType type) {
			if (type == Mainchain || type == Normal) {
				RegNetMainChainParamsInit();
			} else if (type == Idchain) {
				RegNetIDChainParamsInit();
			} else {
				ErrorChecker::CheckCondition(true, Error::InvalidCoinType,
											 "Unsupport coin type in coin config");
			}
		}

	}
}

