// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BRPeerMessages_h
#define BRPeerMessages_h

#include <pthread.h>
#include <netinet/in.h>

#include "BRPeer.h"
#include "BRSet.h"

#ifdef __cplusplus
extern "C" {
#endif


//todo change this to somewhere correctly
#define HEADER_LENGTH      24
#define MAX_MSG_LENGTH     0x02000000
#define MAX_GETDATA_HASHES 50000
#define ENABLED_SERVICES   0ULL  // we don't provide full blocks to remote nodes
#define PROTOCOL_VERSION   70013
#define MIN_PROTO_VERSION  70002 // peers earlier than this protocol version not supported (need v0.9 txFee relay rules)
#define LOCAL_HOST         ((UInt128) { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01 })
#define CONNECT_TIMEOUT    3.0
#define MESSAGE_TIMEOUT    10.0
#define PTHREAD_STACK_SIZE  (512 * 1024)

typedef enum {
	inv_undefined = 0,
	inv_tx = 1,
	inv_block = 2,
	inv_filtered_block = 3
} inv_type;

typedef struct BRPeerManagerStruct BRPeerManager;

typedef struct {
	BRPeer peer; // superstruct on top of BRPeer
	uint32_t magicNumber;
	char host[INET6_ADDRSTRLEN];
	BRPeerStatus status;
	int waitingForNetwork;
	volatile int needsFilterUpdate;
	uint64_t nonce, feePerKb;
	char *useragent;
	uint32_t version, lastblock, earliestKeyTime, currentBlockHeight;
	double startTime, pingTime;
	volatile double disconnectTime, mempoolTime;
	int sentVerack, gotVerack, sentGetaddr, sentFilter, sentGetdata, sentMempool, sentGetblocks;
	UInt256 lastBlockHash;
	BRMerkleBlock *currentBlock;
	UInt256 *currentBlockTxHashes, *knownBlockHashes, *knownTxHashes;
	BRSet *knownTxHashSet;
	volatile int socket;
	void *info;
	void (*connected)(void *info);
	void (*disconnected)(void *info, int error);
	void (*relayedPeers)(void *info, const BRPeer peers[], size_t peersCount);
	void (*relayedTx)(void *info, BRTransaction *tx);
	void (*hasTx)(void *info, UInt256 txHash);
	void (*rejectedTx)(void *info, UInt256 txHash, uint8_t code);
	void (*relayedBlock)(void *info, BRMerkleBlock *block);
	void (*notfound)(void *info, const UInt256 txHashes[], size_t txCount, const UInt256 blockHashes[],
					 size_t blockCount);
	void (*setFeePerKb)(void *info, uint64_t feePerKb);
	BRTransaction *(*requestedTx)(void *info, UInt256 txHash);
	int (*networkIsReachable)(void *info);
	void (*threadCleanup)(void *info);
	void **volatile pongInfo;
	void (**volatile pongCallback)(void *info, int success);
	void *volatile mempoolInfo;
	void (*volatile mempoolCallback)(void *info, int success);
	pthread_t thread;

	BRPeerManager *manager;
} BRPeerContext;

typedef struct {
	void (*BRPeerSendVerackMessage)(BRPeer *peer);
	int (*BRPeerAcceptVerackMessage)(BRPeer *peer, const uint8_t *msg, size_t msgLen);

	void (*BRPeerSendVersionMessage)(BRPeer *peer);
	int (*BRPeerAcceptVersionMessage)(BRPeer *peer, const uint8_t *msg, size_t msgLen);

	void (*BRPeerSendAddressMessage)(BRPeer *peer);
	int (*BRPeerAcceptAddressMessage)(BRPeer *peer, const uint8_t *msg, size_t msgLen);

	void (*BRPeerSendInventoryMessage)(BRPeer *peer, const UInt256 txHashes[], size_t txCount);
	int (*BRPeerAcceptInventoryMessage)(BRPeer *peer, const uint8_t *msg, size_t msgLen);

	void (*BRPeerSendGetAddressMessage)(BRPeer *peer);
	int (*BRPeerAcceptGetAddressMessage)(BRPeer *peer, const uint8_t *msg, size_t msgLen);

	void (*BRPeerSendTxMessage)(BRPeer *peer, BRTransaction *tx);
	int (*BRPeerAcceptTxMessage)(BRPeer *peer, const uint8_t *msg, size_t msgLen);

	int (*BRPeerAcceptMerkleblockMessage)(BRPeer *peer, const uint8_t *msg, size_t msgLen);

	int (*BRPeerAcceptNotFoundMessage)(BRPeer *peer, const uint8_t *msg, size_t msgLen);

} BRPeerMessages;

BRPeerMessages *BRPeerMessageNew(void);

void BRPeerMessageFree(BRPeerMessages *peerMessages);
void BRPeerAddKnownTxHashes(const BRPeer *peer, const UInt256 txHashes[], size_t txCount);


#ifdef __cplusplus
}
#endif

#endif //BRPeerMessages_h
