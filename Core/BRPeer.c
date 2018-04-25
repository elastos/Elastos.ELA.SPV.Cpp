//
//  BRPeer.c
//
//  Created by Aaron Voisine on 9/2/15.
//  Copyright (c) 2015 breadwallet LLC.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include "BRPeer.h"
#include "BRMerkleBlock.h"
#include "BRAddress.h"
#include "BRSet.h"
#include "BRArray.h"
#include "BRCrypto.h"
#include "BRInt.h"
#include "BRPeerMessages.h"
#include "BRPeerManager.h"
#include <stdlib.h>
#include <float.h>
#include <inttypes.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

// the standard blockchain download protocol works as follows (for SPV mode):
// - local peer sends getblocks
// - remote peer reponds with inv containing up to 500 block hashes
// - local peer sends getdata with the block hashes
// - remote peer responds with multiple merkleblock and tx messages
// - remote peer sends inv containg 1 hash, of the most recent block
// - local peer sends getdata with the most recent block hash
// - remote peer responds with merkleblock
// - if local peer can't connect the most recent block to the chain (because it started more than 500 blocks behind), go
//   back to first step and repeat until entire chain is downloaded
//
// we modify this sequence to improve sync performance and handle adding bip32 addresses to the bloom filter as needed:
// - local peer sends getheaders
// - remote peer responds with up to 2000 headers
// - local peer immediately sends getheaders again and then processes the headers
// - previous two steps repeat until a header within a week of earliestKeyTime is reached (further headers are ignored)
// - local peer sends getblocks
// - remote peer responds with inv containing up to 500 block hashes
// - local peer sends getdata with the block hashes
// - if there were 500 hashes, local peer sends getblocks again without waiting for remote peer
// - remote peer responds with multiple merkleblock and tx messages, followed by inv containing up to 500 block hashes
// - previous two steps repeat until an inv with fewer than 500 block hashes is received
// - local peer sends just getdata for the final set of fewer than 500 block hashes
// - remote peer responds with multiple merkleblock and tx messages
// - if at any point tx messages consume enough wallet addresses to drop below the bip32 chain gap limit, more addresses
//   are generated and local peer sends filterload with an updated bloom filter
// - after filterload is sent, getdata is sent to re-request recent blocks that may contain new tx matching the filter

inline static int _BRPeerIsIPv4(const BRPeer *peer)
{
    return (peer->address.u64[0] == 0 && peer->address.u16[4] == 0 && peer->address.u16[5] == 0xffff);
}

static int _BRPeerAcceptHeadersMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    size_t off = 0, count = (size_t)BRVarInt(msg, msgLen, &off);
    int r = 1;

    if (off == 0 || off + 81*count > msgLen) {
        peer_log(peer, "malformed headers message, length is %zu, should be %zu for %zu header(s)", msgLen,
                 BRVarIntSize(count) + 81*count, count);
        r = 0;
    }
    else {
        peer_log(peer, "got %zu header(s)", count);

        // To improve chain download performance, if this message contains 2000 headers then request the next 2000
        // headers immediately, and switch to requesting blocks when we receive a header newer than earliestKeyTime
        uint32_t timestamp = (count > 0) ? UInt32GetLE(&msg[off + 81*(count - 1) + 68]) : 0;

        if (count >= 2000 || (timestamp > 0 && timestamp + 7*24*60*60 + BLOCK_MAX_TIME_DRIFT >= ctx->earliestKeyTime)) {
            size_t last = 0;
            time_t now = time(NULL);
            UInt256 locators[2];

            BRSHA256_2(&locators[0], &msg[off + 81*(count - 1)], 80);
            BRSHA256_2(&locators[1], &msg[off], 80);

            if (timestamp > 0 && timestamp + 7*24*60*60 + BLOCK_MAX_TIME_DRIFT >= ctx->earliestKeyTime) {
                // request blocks for the remainder of the chain
                timestamp = (++last < count) ? UInt32GetLE(&msg[off + 81*last + 68]) : 0;

                while (timestamp > 0 && timestamp + 7*24*60*60 + BLOCK_MAX_TIME_DRIFT < ctx->earliestKeyTime) {
                    timestamp = (++last < count) ? UInt32GetLE(&msg[off + 81*last + 68]) : 0;
                }

                BRSHA256_2(&locators[0], &msg[off + 81*(last - 1)], 80);
                BRPeerSendGetblocks(peer, locators, 2, UINT256_ZERO);
            }
            else BRPeerSendGetheaders(peer, locators, 2, UINT256_ZERO);

            for (size_t i = 0; r && i < count; i++) {
                BRMerkleBlock *block = BRMerkleBlockParse(&msg[off + 81*i], 81);

                if (! BRMerkleBlockIsValid(block, (uint32_t)now)) {
                    peer_log(peer, "invalid block header: %s", u256hex(block->blockHash));
                    BRMerkleBlockFree(block);
                    r = 0;
                }
                else if (ctx->relayedBlock) {
                    ctx->relayedBlock(ctx->info, block);
                }
                else BRMerkleBlockFree(block);
            }
        }
        else {
            peer_log(peer, "non-standard headers message, %zu is fewer header(s) than expected", count);
            r = 0;
        }
    }

    return r;
}

static int _BRPeerAcceptGetdataMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    size_t off = 0, count = (size_t)BRVarInt(msg, msgLen, &off);
    int r = 1;

    if (off == 0 || off + 36*count > msgLen) {
        peer_log(peer, "malformed getdata message, length is %zu, should %zu for %zu item(s)", msgLen,
                 BRVarIntSize(count) + 36*count, count);
        r = 0;
    }
    else if (count > MAX_GETDATA_HASHES) {
        peer_log(peer, "dropping getdata message, %zu is too many items, max is %d", count, MAX_GETDATA_HASHES);
    }
    else {
        struct inv_item { uint8_t item[36]; } *notfound = NULL;
        BRTransaction *tx = NULL;

        peer_log(peer, "got getdata with %zu item(s)", count);

        for (size_t i = 0; i < count; i++) {
            inv_type type = UInt32GetLE(&msg[off]);
            UInt256 hash;
            UInt256Get(&hash, &msg[off + sizeof(uint32_t)]);

            switch (type) {
                case inv_tx:
                    if (ctx->requestedTx) tx = ctx->requestedTx(ctx->info, hash);

                    if (tx && BRTransactionSize(tx) < TX_MAX_SIZE) {
                        ctx->manager->peerMessages->BRPeerSendTxMessage(peer, tx);
                        break;
                    }

                    // fall through
                default:
                    if (! notfound) array_new(notfound, 1);
                    array_add(notfound, *(struct inv_item *)&msg[off]);
                    break;
            }

            off += 36;
        }

        if (notfound) {
            size_t bufLen = BRVarIntSize(array_count(notfound)) + 36*array_count(notfound), o = 0;
            uint8_t *buf = malloc(bufLen);

            assert(buf != NULL);
            o += BRVarIntSet(&buf[o], (o <= bufLen ? bufLen - o : 0), array_count(notfound));
            memcpy(&buf[o], notfound, 36*array_count(notfound));
            o += 36*array_count(notfound);
            array_free(notfound);
            BRPeerSendMessage(peer, buf, o, MSG_NOTFOUND);
            free(buf);
        }
    }

    return r;
}

static int _BRPeerAcceptPingMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
    int r = 1;

    if (sizeof(uint64_t) > msgLen) {
        peer_log(peer, "malformed ping message, length is %zu, should be %zu", msgLen, sizeof(uint64_t));
        r = 0;
    }
    else {
        peer_log(peer, "got ping");
        BRPeerSendMessage(peer, msg, msgLen, MSG_PONG);
    }

    return r;
}

static int _BRPeerAcceptPongMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    struct timeval tv;
    double pingTime;
    int r = 1;

    if (sizeof(uint64_t) > msgLen) {
        peer_log(peer, "malformed pong message, length is %zu, should be %zu", msgLen, sizeof(uint64_t));
        r = 0;
    }
    else if (UInt64GetLE(msg) != ctx->nonce) {
        peer_log(peer, "pong message has wrong nonce: %"PRIu64", expected: %"PRIu64, UInt64GetLE(msg), ctx->nonce);
        r = 0;
    }
    else if (array_count(ctx->pongCallback) == 0) {
        peer_log(peer, "got unexpected pong");
        r = 0;
    }
    else {
        if (ctx->startTime > 1) {
            gettimeofday(&tv, NULL);
            pingTime = tv.tv_sec + (double)tv.tv_usec/1000000 - ctx->startTime;

            // 50% low pass filter on current ping time
            ctx->pingTime = ctx->pingTime*0.5 + pingTime*0.5;
            ctx->startTime = 0;
            peer_log(peer, "got pong in %fs", pingTime);
        }
        else peer_log(peer, "got pong");

        if (array_count(ctx->pongCallback) > 0) {
            void (*pongCallback)(void *, int) = ctx->pongCallback[0];
            void *pongInfo = ctx->pongInfo[0];

            array_rm(ctx->pongCallback, 0);
            array_rm(ctx->pongInfo, 0);
            if (pongCallback) pongCallback(pongInfo, 1);
        }
    }

    return r;
}

// described in BIP61: https://github.com/bitcoin/bips/blob/master/bip-0061.mediawiki
static int _BRPeerAcceptRejectMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    size_t off = 0, strLen = (size_t)BRVarInt(msg, msgLen, &off);
    int r = 1;

    if (off + strLen + sizeof(uint8_t) > msgLen) {
        peer_log(peer, "malformed reject message, length is %zu, should be >= %zu", msgLen,
                 off + strLen + sizeof(uint8_t));
        r = 0;
    }
    else {
        char type[(strLen < 0x1000) ? strLen + 1 : 0x1000];
        uint8_t code;
        size_t len = 0, hashLen = 0;

        strncpy(type, (const char *)&msg[off], sizeof(type) - 1);
        type[sizeof(type) - 1] = '\0';
        off += strLen;
        code = msg[off++];
        strLen = (size_t)BRVarInt(&msg[off], (off <= msgLen ? msgLen - off : 0), &len);
        off += len;
        if (strncmp(type, MSG_TX, sizeof(type)) == 0) hashLen = sizeof(UInt256);

        if (off + strLen + hashLen > msgLen) {
            peer_log(peer, "malformed reject message, length is %zu, should be >= %zu", msgLen, off + strLen + hashLen);
            r = 0;
        }
        else {
            char reason[(strLen < 0x1000) ? strLen + 1 : 0x1000];
            UInt256 txHash = UINT256_ZERO;

            strncpy(reason, (const char *)&msg[off], sizeof(reason) - 1);
            reason[sizeof(reason) - 1] = '\0';
            off += strLen;
            if (hashLen == sizeof(UInt256)) UInt256Get(&txHash, &msg[off]);
            off += hashLen;

            if (! UInt256IsZero(&txHash)) {
                peer_log(peer, "rejected %s code: 0x%x reason: \"%s\" txid: %s", type, code, reason, u256hex(txHash));
                if (ctx->rejectedTx) ctx->rejectedTx(ctx->info, txHash, code);
            }
            else peer_log(peer, "rejected %s code: 0x%x reason: \"%s\"", type, code, reason);
        }
    }

    return r;
}

// BIP133: https://github.com/bitcoin/bips/blob/master/bip-0133.mediawiki
static int _BRPeerAcceptFeeFilterMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    int r = 1;

    if (sizeof(uint64_t) > msgLen) {
        peer_log(peer, "malformed feefilter message, length is %zu, should be >= %zu", msgLen, sizeof(uint64_t));
        r = 0;
    }
    else {
        ctx->feePerKb = UInt64GetLE(msg);
        peer_log(peer, "got feefilter with rate %"PRIu64, ctx->feePerKb);
        if (ctx->setFeePerKb) ctx->setFeePerKb(ctx->info, ctx->feePerKb);
    }

    return r;
}

static int _BRPeerAcceptMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen, const char *type)
{
    peer_log(peer, "------start _BRPeerAcceptMessage: type %s --------", type);

    BRPeerContext *ctx = (BRPeerContext *)peer;
    int r = 1;

    if (ctx->currentBlock && strncmp(MSG_TX, type, 12) != 0) { // if we receive a non-tx message, merkleblock is done
        peer_log(peer, "incomplete merkleblock %s, expected %zu more tx, got %s", u256hex(ctx->currentBlock->blockHash),
                 array_count(ctx->currentBlockTxHashes), type);
        array_clear(ctx->currentBlockTxHashes);
        ctx->currentBlock = NULL;
        r = 0;
    }
    else if (strncmp(MSG_VERSION, type, 12) == 0) r = ctx->manager->peerMessages->BRPeerAcceptVersionMessage(peer, msg, msgLen);
    else if (strncmp(MSG_VERACK, type, 12) == 0) r = ctx->manager->peerMessages->BRPeerAcceptVerackMessage(peer, msg, msgLen);
    else if (strncmp(MSG_ADDR, type, 12) == 0) r = ctx->manager->peerMessages->BRPeerAcceptAddressMessage(peer, msg, msgLen);
    else if (strncmp(MSG_INV, type, 12) == 0) r = ctx->manager->peerMessages->BRPeerAcceptInventoryMessage(peer, msg, msgLen);
    else if (strncmp(MSG_TX, type, 12) == 0) r = ctx->manager->peerMessages->BRPeerAcceptTxMessage(peer, msg, msgLen);
    else if (strncmp(MSG_HEADERS, type, 12) == 0) r = _BRPeerAcceptHeadersMessage(peer, msg, msgLen);
    else if (strncmp(MSG_GETADDR, type, 12) == 0) r = ctx->manager->peerMessages->BRPeerAcceptGetAddressMessage(peer, msg, msgLen);
    else if (strncmp(MSG_GETDATA, type, 12) == 0) r = _BRPeerAcceptGetdataMessage(peer, msg, msgLen);
    else if (strncmp(MSG_NOTFOUND, type, 12) == 0)r = ctx->manager->peerMessages->BRPeerAcceptNotFoundMessage(peer, msg, msgLen);
    else if (strncmp(MSG_PING, type, 12) == 0) r = _BRPeerAcceptPingMessage(peer, msg, msgLen);
    else if (strncmp(MSG_PONG, type, 12) == 0)
        r = _BRPeerAcceptPongMessage(peer, msg, msgLen);
    else if (strncmp(MSG_MERKLEBLOCK, type, 12) == 0) r = ctx->manager->peerMessages->BRPeerAcceptMerkleblockMessage(peer, msg, msgLen);
    else if (strncmp(MSG_REJECT, type, 12) == 0) r = _BRPeerAcceptRejectMessage(peer, msg, msgLen);
    else if (strncmp(MSG_FEEFILTER, type, 12) == 0) r = _BRPeerAcceptFeeFilterMessage(peer, msg, msgLen);
    else peer_log(peer, "dropping %s, length %zu, not implemented", type, msgLen);

    return r;
}

static int _BRPeerOpenSocket(BRPeer *peer, int domain, double timeout, int *error)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    struct sockaddr_storage addr;
    struct timeval tv;
    fd_set fds;
    socklen_t addrLen, optLen;
    int count, arg = 0, err = 0, on = 1, r = 1;

    ctx->socket = socket(domain, SOCK_STREAM, 0);

    if (ctx->socket < 0) {
        err = errno;
        r = 0;
    }
    else {
        tv.tv_sec = 1; // one second timeout for send/receive, so thread doesn't block for too long
        tv.tv_usec = 0;
        setsockopt(ctx->socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(ctx->socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        setsockopt(ctx->socket, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
#ifdef SO_NOSIGPIPE // BSD based systems have a SO_NOSIGPIPE socket option to supress SIGPIPE signals
        setsockopt(ctx->socket, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
#endif
        arg = fcntl(ctx->socket, F_GETFL, NULL);
        if (arg < 0 || fcntl(ctx->socket, F_SETFL, arg | O_NONBLOCK) < 0) r = 0; // temporarily set socket non-blocking
        if (! r) err = errno;
    }

    if (r) {
        memset(&addr, 0, sizeof(addr));

        if (domain == PF_INET6) {
            ((struct sockaddr_in6 *)&addr)->sin6_family = AF_INET6;
            ((struct sockaddr_in6 *)&addr)->sin6_addr = *(struct in6_addr *)&peer->address;
            ((struct sockaddr_in6 *)&addr)->sin6_port = htons(peer->port);
            addrLen = sizeof(struct sockaddr_in6);
        }
        else {
            ((struct sockaddr_in *)&addr)->sin_family = AF_INET;
            ((struct sockaddr_in *)&addr)->sin_addr = *(struct in_addr *)&peer->address.u32[3];
            ((struct sockaddr_in *)&addr)->sin_port = htons(peer->port);
            addrLen = sizeof(struct sockaddr_in);
        }

        if (connect(ctx->socket, (struct sockaddr *)&addr, addrLen) < 0) err = errno;

        if (err == EINPROGRESS) {
            err = 0;
            optLen = sizeof(err);
            tv.tv_sec = timeout;
            tv.tv_usec = (long)(timeout*1000000) % 1000000;
            FD_ZERO(&fds);
            FD_SET(ctx->socket, &fds);
            count = select(ctx->socket + 1, NULL, &fds, NULL, &tv);

            if (count <= 0 || getsockopt(ctx->socket, SOL_SOCKET, SO_ERROR, &err, &optLen) < 0 || err) {
                if (count == 0) err = ETIMEDOUT;
                if (count < 0 || ! err) err = errno;
                r = 0;
            }
        }
        else if (err && domain == PF_INET6 && _BRPeerIsIPv4(peer)) {
            return _BRPeerOpenSocket(peer, PF_INET, timeout, error); // fallback to IPv4
        }
        else if (err) r = 0;

        if (r) peer_log(peer, "socket connected");
        fcntl(ctx->socket, F_SETFL, arg); // restore socket non-blocking status
    }

    if (! r && err) peer_log(peer, "connect error: %s", strerror(err));
    if (error && err) *error = err;
    return r;
}

static void *_peerThreadRoutine(void *arg)
{
    BRPeer *peer = arg;
    BRPeerContext *ctx = arg;
    int socket, error = 0;

    pthread_cleanup_push(ctx->threadCleanup, ctx->info);

    if (_BRPeerOpenSocket(peer, PF_INET6, CONNECT_TIMEOUT, &error)) {
        struct timeval tv;
        double time = 0, msgTimeout;
        uint8_t header[HEADER_LENGTH], *payload = malloc(0x1000);
        size_t len = 0, payloadLen = 0x1000;
        ssize_t n = 0;

        assert(payload != NULL);
        gettimeofday(&tv, NULL);
        ctx->startTime = tv.tv_sec + (double)tv.tv_usec/1000000;
        ctx->manager->peerMessages->BRPeerSendVersionMessage(peer);

        while (ctx->socket >= 0 && ! error) {
            len = 0;
            socket = ctx->socket;

            while (socket >= 0 && ! error && len < HEADER_LENGTH) {
                n = read(socket, &header[len], sizeof(header) - len);
                if (n > 0) len += n;
                if (n == 0) error = ECONNRESET;
                if (n < 0 && errno != EWOULDBLOCK) error = errno;
                gettimeofday(&tv, NULL);
                time = tv.tv_sec + (double)tv.tv_usec/1000000;
                if (! error && time >= ctx->disconnectTime) error = ETIMEDOUT;

                if (! error && time >= ctx->mempoolTime) {
                    peer_log(peer, "done waiting for mempool response");
                    BRPeerSendPing(peer, ctx->mempoolInfo, ctx->mempoolCallback);
                    ctx->mempoolCallback = NULL;
                    ctx->mempoolTime = DBL_MAX;
                }

                while (sizeof(uint32_t) <= len && UInt32GetLE(header) != ctx->magicNumber) {
                    memmove(header, &header[1], --len); // consume one byte at a time until we find the magic number
                }

                socket = ctx->socket;
            }

            if (error) {
                peer_log(peer, "%s", strerror(error));
            }
            else if (header[15] != 0) { // verify header type field is NULL terminated
                peer_log(peer, "malformed message header: type not NULL terminated");
                error = EPROTO;
            }
            else if (len == HEADER_LENGTH) {
                const char *type = (const char *)(&header[4]);
                uint32_t msgLen = UInt32GetLE(&header[16]);
                uint32_t checksum = UInt32GetLE(&header[20]);
                UInt256 hash;

                if (msgLen > MAX_MSG_LENGTH) { // check message length
                    peer_log(peer, "error reading %s, message length %"PRIu32" is too long", type, msgLen);
                    error = EPROTO;
                }
                else {
                    peer_log(peer, "start read head: prot %d", (int)peer->port);
                    if (msgLen > payloadLen) payload = realloc(payload, (payloadLen = msgLen));
                    assert(payload != NULL);
                    len = 0;
                    socket = ctx->socket;
                    msgTimeout = time + MESSAGE_TIMEOUT;

                    while (socket >= 0 && ! error && len < msgLen) {
                        n = read(socket, &payload[len], msgLen - len);
                        peer_log(peer, "read socket n %ld", n);
                        if (n > 0) len += n;
                        if (n == 0) error = ECONNRESET;
                        if (n < 0 && errno != EWOULDBLOCK) error = errno;
                        gettimeofday(&tv, NULL);
                        time = tv.tv_sec + (double)tv.tv_usec/1000000;
                        if (n > 0) msgTimeout = time + MESSAGE_TIMEOUT;
                        if (! error && time >= msgTimeout) error = ETIMEDOUT;
                        socket = ctx->socket;
                    }

                    if (error) {
                        peer_log(peer, "%s", strerror(error));
                    }
                    else if (len == msgLen) {
                        BRSHA256_2(&hash, payload, msgLen);

                        if (UInt32GetLE(&hash) != checksum) { // verify checksum
                            peer_log(peer, "error reading %s, invalid checksum %x, expected %x, payload length:%"PRIu32
                                     ", SHA256_2:%s", type, UInt32GetLE(&hash), checksum, msgLen, u256hex(hash));
                            error = EPROTO;
                        }
                        else if (! _BRPeerAcceptMessage(peer, payload, msgLen, type)) error = EPROTO;
                    }
                }
            }
        }

        free(payload);
    }

    socket = ctx->socket;
    ctx->socket = -1;
    ctx->status = BRPeerStatusDisconnected;
    if (socket >= 0) close(socket);
    peer_log(peer, "disconnected");

    while (array_count(ctx->pongCallback) > 0) {
        void (*pongCallback)(void *, int) = ctx->pongCallback[0];
        void *pongInfo = ctx->pongInfo[0];

        array_rm(ctx->pongCallback, 0);
        array_rm(ctx->pongInfo, 0);
        if (pongCallback) pongCallback(pongInfo, 0);
    }

    if (ctx->mempoolCallback) ctx->mempoolCallback(ctx->mempoolInfo, 0);
    ctx->mempoolCallback = NULL;
    if (ctx->disconnected) ctx->disconnected(ctx->info, error);
    pthread_cleanup_pop(1);
    return NULL; // detached threads don't need to return a value
}

static void _dummyThreadCleanup(void *info)
{
}

BRPeer *BRPeerCopy(const BRPeer *peer)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    BRPeer *newPeer = BRPeerNew(ctx->magicNumber);
    newPeer->timestamp = peer->timestamp;
    newPeer->address = peer->address;
    newPeer->port = peer->port;
    newPeer->flags = peer->flags;
    newPeer->services = peer->services;

    return newPeer;
}

// returns a newly allocated BRPeer struct that must be freed by calling BRPeerFree()
BRPeer *BRPeerNew(uint32_t magicNumber)
{
    BRPeerContext *ctx = calloc(1, sizeof(*ctx));

    assert(ctx != NULL);
	memset(ctx, 0, sizeof(*ctx));

    ctx->magicNumber = magicNumber;
    array_new(ctx->useragent, 40);
    array_new(ctx->knownBlockHashes, 10);
    array_new(ctx->currentBlockTxHashes, 10);
    array_new(ctx->knownTxHashes, 10);
    ctx->knownTxHashSet = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
    array_new(ctx->pongInfo, 10);
    array_new(ctx->pongCallback, 10);
    ctx->pingTime = DBL_MAX;
    ctx->mempoolTime = DBL_MAX;
    ctx->disconnectTime = DBL_MAX;
    ctx->socket = -1;
    ctx->threadCleanup = _dummyThreadCleanup;
    return &ctx->peer;
}

// info is a void pointer that will be passed along with each callback call
// void connected(void *) - called when peer handshake completes successfully
// void disconnected(void *, int) - called when peer connection is closed, error is an errno.h code
// void relayedPeers(void *, const BRPeer[], size_t) - called when an "addr" message is received from peer
// void relayedTx(void *, BRTransaction *) - called when a "tx" message is received from peer
// void hasTx(void *, UInt256 txHash) - called when an "inv" message with an already-known tx hash is received from peer
// void rejectedTx(void *, UInt256 txHash, uint8_t) - called when a "reject" message is received from peer
// void relayedBlock(void *, BRMerkleBlock *) - called when a "merkleblock" or "headers" message is received from peer
// void notfound(void *, const UInt256[], size_t, const UInt256[], size_t) - called when "notfound" message is received
// BRTransaction *requestedTx(void *, UInt256) - called when "getdata" message with a tx hash is received from peer
// int networkIsReachable(void *) - must return true when networking is available, false otherwise
// void threadCleanup(void *) - called before a thread terminates to faciliate any needed cleanup
void BRPeerSetCallbacks(BRPeer *peer, void *info,
                        void (*connected)(void *info),
                        void (*disconnected)(void *info, int error),
                        void (*relayedPeers)(void *info, const BRPeer peers[], size_t peersCount),
                        void (*relayedTx)(void *info, BRTransaction *tx),
                        void (*hasTx)(void *info, UInt256 txHash),
                        void (*rejectedTx)(void *info, UInt256 txHash, uint8_t code),
                        void (*relayedBlock)(void *info, BRMerkleBlock *block),
                        void (*notfound)(void *info, const UInt256 txHashes[], size_t txCount,
                                         const UInt256 blockHashes[], size_t blockCount),
                        void (*setFeePerKb)(void *info, uint64_t feePerKb),
                        BRTransaction *(*requestedTx)(void *info, UInt256 txHash),
                        int (*networkIsReachable)(void *info),
                        void (*threadCleanup)(void *info))
{
    BRPeerContext *ctx = (BRPeerContext *)peer;

    ctx->info = info;
    ctx->connected = connected;
    ctx->disconnected = disconnected;
    ctx->relayedPeers = relayedPeers;
    ctx->relayedTx = relayedTx;
    ctx->hasTx = hasTx;
    ctx->rejectedTx = rejectedTx;
    ctx->relayedBlock = relayedBlock;
    ctx->notfound = notfound;
    ctx->setFeePerKb = setFeePerKb;
    ctx->requestedTx = requestedTx;
    ctx->networkIsReachable = networkIsReachable;
    ctx->threadCleanup = (threadCleanup) ? threadCleanup : _dummyThreadCleanup;
}

// set earliestKeyTime to wallet creation time in order to speed up initial sync
void BRPeerSetEarliestKeyTime(BRPeer *peer, uint32_t earliestKeyTime)
{
    ((BRPeerContext *)peer)->earliestKeyTime = earliestKeyTime;
}

// call this when local block height changes (helps detect tarpit nodes)
void BRPeerSetCurrentBlockHeight(BRPeer *peer, uint32_t currentBlockHeight)
{
    ((BRPeerContext *)peer)->currentBlockHeight = currentBlockHeight;
}

// current connection status
BRPeerStatus BRPeerConnectStatus(BRPeer *peer)
{
    return ((BRPeerContext *)peer)->status;
}

// open connection to peer and perform handshake
void BRPeerConnect(BRPeer *peer)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    struct timeval tv;
    int error = 0;
    pthread_attr_t attr;

    if (ctx->status == BRPeerStatusDisconnected || ctx->waitingForNetwork) {
        ctx->status = BRPeerStatusConnecting;

        if (ctx->networkIsReachable && ! ctx->networkIsReachable(ctx->info)) { // delay until network is reachable
            if (! ctx->waitingForNetwork) peer_log(peer, "waiting for network reachability");
            ctx->waitingForNetwork = 1;
        }
        else {
            peer_log(peer, "connecting");
            ctx->waitingForNetwork = 0;
            gettimeofday(&tv, NULL);
            ctx->disconnectTime = tv.tv_sec + (double)tv.tv_usec/1000000 + CONNECT_TIMEOUT;

            if (pthread_attr_init(&attr) != 0) {
                error = ENOMEM;
                peer_log(peer, "error creating thread");
                ctx->status = BRPeerStatusDisconnected;
                //if (ctx->disconnected) ctx->disconnected(ctx->info, error);
            }
            else if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0 ||
                     pthread_attr_setstacksize(&attr, PTHREAD_STACK_SIZE) != 0 ||
                     pthread_create(&ctx->thread, &attr, _peerThreadRoutine, peer) != 0) {
                error = EAGAIN;
                peer_log(peer, "error creating thread");
                pthread_attr_destroy(&attr);
                ctx->status = BRPeerStatusDisconnected;
                //if (ctx->disconnected) ctx->disconnected(ctx->info, error);
            }
        }
    }
}

// close connection to peer
void BRPeerDisconnect(BRPeer *peer)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    int socket = ctx->socket;

    if (socket >= 0) {
        ctx->socket = -1;
        if (shutdown(socket, SHUT_RDWR) < 0) peer_log(peer, "%s", strerror(errno));
        close(socket);
    }
}

// call this to (re)schedule a disconnect in the given number of seconds, or < 0 to cancel (useful for sync timeout)
void BRPeerScheduleDisconnect(BRPeer *peer, double seconds)
{
    BRPeerContext *ctx = ((BRPeerContext *)peer);
    struct timeval tv;

    gettimeofday(&tv, NULL);
    ctx->disconnectTime = (seconds < 0) ? DBL_MAX : tv.tv_sec + (double)tv.tv_usec/1000000 + seconds;
}

// call this when wallet addresses need to be added to bloom filter
void BRPeerSetNeedsFilterUpdate(BRPeer *peer, int needsFilterUpdate)
{
    ((BRPeerContext *)peer)->needsFilterUpdate = needsFilterUpdate;
}

// display name of peer address
const char *BRPeerHost(BRPeer *peer)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;

    if (ctx->host[0] == '\0') {
        if (_BRPeerIsIPv4(peer)) {
            inet_ntop(AF_INET, &peer->address.u32[3], ctx->host, sizeof(ctx->host));
        }
        else inet_ntop(AF_INET6, &peer->address, ctx->host, sizeof(ctx->host));
    }

    return ctx->host;
}

// connected peer version number
uint32_t BRPeerVersion(BRPeer *peer)
{
    return ((BRPeerContext *)peer)->version;
}

// connected peer user agent string
const char *BRPeerUserAgent(BRPeer *peer)
{
    return ((BRPeerContext *)peer)->useragent;
}

// best block height reported by connected peer
uint32_t BRPeerLastBlock(BRPeer *peer)
{
    return ((BRPeerContext *)peer)->lastblock;
}

// average ping time for connected peer
double BRPeerPingTime(BRPeer *peer)
{
    return ((BRPeerContext *)peer)->pingTime;
}

// minimum tx fee rate peer will accept
uint64_t BRPeerFeePerKb(BRPeer *peer)
{
    return ((BRPeerContext *)peer)->feePerKb;
}

#ifndef MSG_NOSIGNAL   // linux based systems have a MSG_NOSIGNAL send flag, useful for supressing SIGPIPE signals
#define MSG_NOSIGNAL 0 // set to 0 if undefined (BSD has the SO_NOSIGPIPE sockopt, and windows has no signals at all)
#endif

// sends a bitcoin protocol message to peer
void BRPeerSendMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen, const char *type)
{
    if (msgLen > MAX_MSG_LENGTH) {
        peer_log(peer, "failed to send %s, length %zu is too long", type, msgLen);
    }
    else {
        BRPeerContext *ctx = (BRPeerContext *)peer;
        uint8_t buf[HEADER_LENGTH + msgLen], hash[32];
        size_t off = 0;
        ssize_t n = 0;
        struct timeval tv;
        int socket, error = 0;

        UInt32SetLE(&buf[off], ctx->magicNumber);
        off += sizeof(uint32_t);
        strncpy((char *)&buf[off], type, 12);
        off += 12;
        UInt32SetLE(&buf[off], (uint32_t)msgLen);
        off += sizeof(uint32_t);
        BRSHA256_2(hash, msg, msgLen);
        memcpy(&buf[off], hash, sizeof(uint32_t));
        off += sizeof(uint32_t);
        memcpy(&buf[off], msg, msgLen);
        peer_log(peer, "sending %s", type);
        msgLen = 0;
        socket = ctx->socket;
        if (socket < 0) error = ENOTCONN;

        while (socket >= 0 && ! error && msgLen < sizeof(buf)) {
            n = send(socket, &buf[msgLen], sizeof(buf) - msgLen, MSG_NOSIGNAL);
            if (n >= 0) msgLen += n;
            if (n < 0 && errno != EWOULDBLOCK) error = errno;
            gettimeofday(&tv, NULL);
            if (! error && tv.tv_sec + (double)tv.tv_usec/1000000 >= ctx->disconnectTime) error = ETIMEDOUT;
            socket = ctx->socket;
        }

        if (error) {
            peer_log(peer, "%s", strerror(error));
            BRPeerDisconnect(peer);
        }
    }
}

void BRPeerSendFilterload(BRPeer *peer, const uint8_t *filter, size_t filterLen)
{
    ((BRPeerContext *)peer)->sentFilter = 1;
    ((BRPeerContext *)peer)->sentMempool = 0;
    BRPeerSendMessage(peer, filter, filterLen, MSG_FILTERLOAD);
}

void BRPeerSendMempool(BRPeer *peer, const UInt256 knownTxHashes[], size_t knownTxCount, void *info,
                       void (*completionCallback)(void *info, int success))
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    struct timeval tv;
    int sentMempool = ctx->sentMempool;

    ctx->sentMempool = 1;

    if (! sentMempool && ! ctx->mempoolCallback) {
        BRPeerAddKnownTxHashes(peer, knownTxHashes, knownTxCount);

        if (completionCallback) {
            gettimeofday(&tv, NULL);
            ctx->mempoolTime = tv.tv_sec + (double)tv.tv_usec/1000000 + 10.0;
            ctx->mempoolInfo = info;
            ctx->mempoolCallback = completionCallback;
        }

        BRPeerSendMessage(peer, NULL, 0, MSG_MEMPOOL);
    }
    else {
        peer_log(peer, "mempool request already sent");
        if (completionCallback) completionCallback(info, 0);
    }
}

void BRPeerSendGetheaders(BRPeer *peer, const UInt256 locators[], size_t locatorsCount, UInt256 hashStop)
{
    peer_log(peer, "*********BRPeerSendGetheaders*************");

    size_t i, off = 0;
    size_t msgLen = sizeof(uint32_t) + BRVarIntSize(locatorsCount) + sizeof(*locators)*locatorsCount + sizeof(hashStop);
    uint8_t msg[msgLen];

    UInt32SetLE(&msg[off], PROTOCOL_VERSION);
    off += sizeof(uint32_t);
    off += BRVarIntSet(&msg[off], (off <= msgLen ? msgLen - off : 0), locatorsCount);

    for (i = 0; i < locatorsCount; i++) {
        UInt256Set(&msg[off], locators[i]);
        off += sizeof(UInt256);
    }

    UInt256Set(&msg[off], hashStop);
    off += sizeof(UInt256);

    if (locatorsCount > 0) {
        peer_log(peer, "calling getheaders with %zu locators: [%s,%s %s]", locatorsCount, u256hex(locators[0]),
                 (locatorsCount > 2 ? " ...," : ""), (locatorsCount > 1 ? u256hex(locators[locatorsCount - 1]) : ""));
        BRPeerSendMessage(peer, msg, off, MSG_GETHEADERS);
    }
}

void BRPeerSendGetblocks(BRPeer *peer, const UInt256 locators[], size_t locatorsCount, UInt256 hashStop)
{
    peer_log(peer, "*********BRPeerSendGetblocks*************");

    size_t i, off = 0;
    size_t msgLen = sizeof(uint32_t) + BRVarIntSize(locatorsCount) + sizeof(*locators)*locatorsCount + sizeof(hashStop);
    uint8_t msg[msgLen];

    UInt32SetLE(&msg[off], PROTOCOL_VERSION);
    off += sizeof(uint32_t);
    off += BRVarIntSet(&msg[off], (off <= msgLen ? msgLen - off : 0), locatorsCount);

    for (i = 0; i < locatorsCount; i++) {
        UInt256Set(&msg[off], locators[i]);
        off += sizeof(UInt256);
    }

    UInt256Set(&msg[off], hashStop);
    off += sizeof(UInt256);

    if (locatorsCount > 0) {
        peer_log(peer, "calling getblocks with %zu locators: [%s,%s %s]", locatorsCount, u256hex(locators[0]),
                 (locatorsCount > 2 ? " ...," : ""), (locatorsCount > 1 ? u256hex(locators[locatorsCount - 1]) : ""));
        BRPeerSendMessage(peer, msg, off, MSG_GETBLOCKS);
    }
}

void BRPeerSendGetdata(BRPeer *peer, const UInt256 txHashes[], size_t txCount, const UInt256 blockHashes[],
                       size_t blockCount)
{
    size_t i, off = 0, count = txCount + blockCount;

    if (count > MAX_GETDATA_HASHES) { // limit total hash count to MAX_GETDATA_HASHES
        peer_log(peer, "couldn't send getdata, %zu is too many items, max is %d", count, MAX_GETDATA_HASHES);
    }
    else if (count > 0) {
        size_t msgLen = BRVarIntSize(count) + (sizeof(uint32_t) + sizeof(UInt256))*(count);
        uint8_t msg[msgLen];

        off += BRVarIntSet(&msg[off], (off <= msgLen ? msgLen - off : 0), count);

        for (i = 0; i < txCount; i++) {
            UInt32SetLE(&msg[off], inv_tx);
            off += sizeof(uint32_t);
            UInt256Set(&msg[off], txHashes[i]);
            off += sizeof(UInt256);
        }

        for (i = 0; i < blockCount; i++) {
            UInt32SetLE(&msg[off], inv_filtered_block);
            off += sizeof(uint32_t);
            UInt256Set(&msg[off], blockHashes[i]);
            off += sizeof(UInt256);
        }

        ((BRPeerContext *)peer)->sentGetdata = 1;
        BRPeerSendMessage(peer, msg, off, MSG_GETDATA);
    }
}

void BRPeerSendPing(BRPeer *peer, void *info, void (*pongCallback)(void *info, int success))
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    uint8_t msg[sizeof(uint64_t)];
    struct timeval tv;

    gettimeofday(&tv, NULL);
    ctx->startTime = tv.tv_sec + (double)tv.tv_usec/1000000;
    array_add(ctx->pongInfo, info);
    array_add(ctx->pongCallback, pongCallback);
    UInt64SetLE(msg, ctx->nonce);
    BRPeerSendMessage(peer, msg, sizeof(msg), MSG_PING);
}

// useful to get additional tx after a bloom filter update
void BRPeerRerequestBlocks(BRPeer *peer, UInt256 fromBlock)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;
    size_t i = array_count(ctx->knownBlockHashes);

    while (i > 0 && ! UInt256Eq(&(ctx->knownBlockHashes[i - 1]), &fromBlock)) i--;

    if (i > 0) {
        array_rm_range(ctx->knownBlockHashes, 0, i - 1);
        peer_log(peer, "re-requesting %zu block(s)", array_count(ctx->knownBlockHashes));
        BRPeerSendGetdata(peer, NULL, 0, ctx->knownBlockHashes, array_count(ctx->knownBlockHashes));
    }
}

void BRPeerFree(BRPeer *peer)
{
    BRPeerContext *ctx = (BRPeerContext *)peer;

    if (ctx->useragent) array_free(ctx->useragent);
    if (ctx->currentBlockTxHashes) array_free(ctx->currentBlockTxHashes);
    if (ctx->knownBlockHashes) array_free(ctx->knownBlockHashes);
    if (ctx->knownTxHashes) array_free(ctx->knownTxHashes);
    if (ctx->knownTxHashSet) BRSetFree(ctx->knownTxHashSet);
    if (ctx->pongCallback) array_free(ctx->pongCallback);
    if (ctx->pongInfo) array_free(ctx->pongInfo);
    free(ctx);
}

void BRPeerAcceptMessageTest(BRPeer *peer, const uint8_t *msg, size_t msgLen, const char *type)
{
    _BRPeerAcceptMessage(peer, msg, msgLen, type);
}
