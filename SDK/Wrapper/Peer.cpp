// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <float.h>
#include <sys/time.h>
#include <tclDecls.h>
#include <SDK/Common/Log.h>
#include <arpa/inet.h>
#include <boost/thread.hpp>

#include "Peer.h"

namespace Elastos {
	namespace ElaWallet {

		const uint32_t DEFAULT_MAGICNUMBER = uint32_t(0);

		Peer::Peer(const UInt128 &addr, uint16_t port, uint64_t timestamp) :
				_status(Unknown),
				_magicNumber(DEFAULT_MAGICNUMBER) {
			_info.address = addr;
			_info.port = port;
			_info.timestamp = timestamp;
			_info.services = SERVICES_NODE_NETWORK;
			_info.flags = 0;

			initDefaultMessages();
		}

		Peer::Peer(uint32_t magicNumber) :
				_status(Unknown),
				_magicNumber(magicNumber),
				_pingTime(DBL_MAX),
				_mempoolTime(DBL_MAX),
				_disconnectTime(DBL_MAX),
				_socket(-1) {

			initDefaultMessages();
		}

		Peer::~Peer() {
		}

		Peer::Peer(const Peer &peer) {
			operator=(peer);
		}

		Peer &Peer::operator=(const Peer &peer) {
			_info.timestamp = peer._info.timestamp;
			_info.address = peer._info.address;
			_info.port = peer._info.port;
			_info.flags = peer._info.flags;
			_info.services = peer._info.services;

			return *this;
		}

		UInt128 Peer::getAddress() const {
			return _info.address;
		}

		uint16_t Peer::getPort() const {
			return _info.port;
		}

		uint64_t Peer::getTimestamp() const {
			return _info.timestamp;
		}

		void Peer::setEarliestKeyTime(uint32_t earliestKeyTime) {
			_earliestKeyTime = earliestKeyTime;
		}

		void Peer::setCurrentBlockHeight(uint32_t currentBlockHeight) {
			_currentBlockHeight = currentBlockHeight;
		}

		Peer::ConnectStatus Peer::getConnectStatusValue() const {
			return _status;
		}

		void Peer::connect() {
			struct timeval tv;
			int error = 0;
			pthread_attr_t attr;

			if (_status == Peer::Disconnected || _waitingForNetwork) {
				_status = Peer::Connecting;

				if (0 && networkIsReachable()) { // delay until network is reachable
					if (!_waitingForNetwork) Log::traceWithTime("waiting for network reachability");
					_waitingForNetwork = 1;
				} else {
					Log::traceWithTime("connecting");
					_waitingForNetwork = 0;
					gettimeofday(&tv, NULL);
					_disconnectTime = tv.tv_sec + (double) tv.tv_usec / 1000000 + CONNECT_TIMEOUT;

//					if (pthread_attr_init(&attr) != 0) {
//						error = ENOMEM;
//						Log::traceWithTime("error creating thread");
//						status = BRPeerStatusDisconnected;
//						if (_listener) _listener->OnDisconnected(error);
//					} else if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0 ||
//							   pthread_attr_setstacksize(&attr, PTHREAD_STACK_SIZE) != 0 ||
//							   pthread_create(&_thread, &attr, &Peer::peerThreadRoutine, this) != 0) {
//						error = EAGAIN;
//						Log::traceWithTime("error creating thread");
//						pthread_attr_destroy(&attr);
//						status = BRPeerStatusDisconnected;
//						if (_listener) _listener->OnDisconnected(error);
//					}
					boost::thread workThread(boost::bind(&Peer::peerThreadRoutine, this));
				}
			}
		}

		void Peer::disconnect() {
			int socket = _socket;

			if (socket >= 0) {
				_socket = -1;
				if (shutdown(socket, SHUT_RDWR) < 0) {
					//fixme [refactor]
//					Log::traceWithTime("peer shutdown error: %s", strerror(errno));
				}
				close(socket);
			}
		}

		void Peer::scheduleDisconnect(double seconds) {
			struct timeval tv;

			gettimeofday(&tv, NULL);
			_disconnectTime = (seconds < 0) ? DBL_MAX : tv.tv_sec + (double) tv.tv_usec / 1000000 + seconds;
		}

		void Peer::setNeedsFilterUpdate(bool needsFilterUpdate) {
			_needsFilterUpdate = needsFilterUpdate;
		}

		const std::string &Peer::getHost() const {
			if (_host.empty()) {
				char temp[INET6_ADDRSTRLEN];
				if (isIPv4()) {
					inet_ntop(AF_INET, &_info.address.u32[3], temp, sizeof(temp));
				} else inet_ntop(AF_INET6, &_info.address, temp, sizeof(temp));

				_host = temp;
			}

			return _host;
		}

		uint32_t Peer::getVersion() const {
			return _version;
		}

		const std::string &Peer::getUserAgent() const {
			return _useragent;
		}

		uint32_t Peer::getLastBlock() const {
			return _lastblock;
		}

		uint64_t Peer::getFeePerKb() const {
			return _feePerKb;
		}

		double Peer::getPingTime() const {
			return _pingTime;
		}

		bool Peer::IsEqual(const Peer *otherPeer) const {
			return (this == otherPeer ||
					(UInt128Eq(&_info.address, &otherPeer->_info.address)) &&
					(_info.port == otherPeer->_info.port));
		}

		bool Peer::networkIsReachable() const {
			//fixme [refactor]
			return false;
		}

		bool Peer::isIPv4() const {
			return (_info.address.u64[0] == 0 && _info.address.u16[4] == 0 && _info.address.u16[5] == 0xffff);
		}

		void Peer::peerThreadRoutine() {

			int socket, error = 0;

			//fixme [refactor]
//			pthread_cleanup_push(threadCleanup, info);

			if (openSocket(PF_INET6, CONNECT_TIMEOUT, &error)) {
				struct timeval tv;
				double time = 0, msgTimeout;
				uint8_t header[HEADER_LENGTH];
				std::string payload;
				payload.resize(0x1000);
				size_t len = 0, payloadLen = 0x1000;
				ssize_t n = 0;

				gettimeofday(&tv, NULL);
				_startTime = tv.tv_sec + (double) tv.tv_usec / 1000000;
				_messages[MSG_VERSION]->Send(Message::DefaultParam);

				while (_socket >= 0 && !error) {
					len = 0;
					socket = _socket;

					while (socket >= 0 && !error && len < HEADER_LENGTH) {
						n = read(socket, &header[len], sizeof(header) - len);
						if (n > 0) len += n;
						if (n == 0)
							error = ECONNRESET;
						if (n < 0 && errno != EWOULDBLOCK) {
							error = errno;
							if (error == EINTR) {
								error = 0;
								continue;
							}
						}

						gettimeofday(&tv, NULL);
						time = tv.tv_sec + (double) tv.tv_usec / 1000000;
						if (!error && time >= _disconnectTime) error = ETIMEDOUT;

						if (!error && time >= _mempoolTime) {
							Log::traceWithTime("done waiting for mempool response");
							//fixme [refactor]
//							manager->peerMessages->BRPeerSendPingMessage(peer, mempoolInfo,
//																			   mempoolCallback);
//							mempoolCallback = NULL;
							_mempoolTime = DBL_MAX;
						}

						while (sizeof(uint32_t) <= len && UInt32GetLE(header) != _magicNumber) {
							memmove(header, &header[1],
									--len); // consume one byte at a time until we find the magic number
						}

						socket = _socket;
					}

					if (error) {
						//fixme [refactor]
//						Log::traceWithTime(peer, "read socket error: %s", strerror(error));
					} else if (header[15] != 0) { // verify header type field is NULL terminated
						Log::traceWithTime("malformed message header: type not NULL terminated");
						error = EPROTO;
					} else if (len == HEADER_LENGTH) {
						std::string type = (const char *) (&header[4]);
						uint32_t msgLen = UInt32GetLE(&header[16]);
						uint32_t checksum = UInt32GetLE(&header[20]);
						UInt256 hash;

						if (msgLen > MAX_MSG_LENGTH) { // check message length
							//fixme [refactor]
//							peer_log(peer, "error reading %s, message length %"PRIu32" is too long", type, msgLen);
							error = EPROTO;
						} else {
//                    peer_dbg(peer, "start read head: port %d", (int)port);
							if (msgLen > payloadLen) {
								payloadLen = msgLen;
								payload.resize(payloadLen);
							}
							len = 0;
							socket = _socket;
							msgTimeout = time + MESSAGE_TIMEOUT;

							while (socket >= 0 && !error && len < msgLen) {
								n = read(socket, &payload[len], msgLen - len);
								//peer_log(peer, "read socket n %ld", n);
								if (n > 0) len += n;
								if (n == 0)
									error = ECONNRESET;
								if (n < 0 && errno != EWOULDBLOCK) {
									error = errno;
									if (error == EINTR) {
										error = 0;
										continue;
									}
								}

								gettimeofday(&tv, NULL);
								time = tv.tv_sec + (double) tv.tv_usec / 1000000;
								if (n > 0) msgTimeout = time + MESSAGE_TIMEOUT;
								if (!error && time >= msgTimeout)
									error = ETIMEDOUT;
								socket = _socket;
							}

							if (error) {
								//fixme [refactor]
//								peer_log(peer, "read socket error: %s", strerror(error));
							} else if (len == msgLen) {
								BRSHA256_2(&hash, payload.data(), msgLen);

								if (UInt32GetLE(&hash) != checksum) { // verify checksum
									//fixme [refactor]
//									peer_log(peer,
//											 "error reading %s, invalid checksum %x, expected %x, payload length:%"PRIu32
//													 ", SHA256_2:%s", type, UInt32GetLE(&hash), checksum, msgLen,
//											 u256hex(hash));
									error = EPROTO;
								} else if (!acceptMessage(payload, type)) error = EPROTO;
							}
						}
					}
				}
			}

			socket = _socket;
			_socket = -1;
			_status = Peer::Disconnected;
			if (socket >= 0) close(socket);
			Log::traceWithTime("disconnected");

			//fixme [refactor]
//			while (array_count(pongCallback) > 0) {
//				void (*pongCallback)(void *, int) = pongCallback[0];
//				void *pongInfo = pongInfo[0];
//
//				array_rm(pongCallback, 0);
//				array_rm(pongInfo, 0);
//				if (pongCallback) pongCallback(pongInfo, 0);
//			}

			//fixme [refactor]
//			if (mempoolCallback) mempoolCallback(mempoolInfo, 0);
//			mempoolCallback = NULL;
			if (_listener) _listener->OnDisconnected(error);
			//fixme [refactor]
//		pthread_cleanup_pop(1)
		}

		void Peer::RegisterListner(Peer::Listener *listener) {
			_listener = listener;
		}

		void Peer::UnRegisterListener() {
			_listener = nullptr;
		}

		void Peer::initDefaultMessages() {
			//fixme [refactor]
		}

		bool Peer::acceptMessage(const std::string &msg, const std::string &type) {
			bool r = false;

			if (_currentBlock != nullptr && MSG_TX == type) { // if we receive a non-tx message, merkleblock is done
				//fixme [refactor]
//        peer_log(peer, "incomplete merkleblock %s, expected %zu more tx, got %s", u256hex(ctx->currentBlock->blockHash),
//                 array_count(ctx->currentBlockTxHashes), type);
				_currentBlockTxHashes.clear();
				_currentBlock.reset();
				r = 0;
			} else if (_messages.find(type) != _messages.end())
				_messages[type]->Accept(msg);
			//fixme [refactor]
//    else peer_log(peer, "dropping %s, length %zu, not implemented", type, msgLen);

			return r;
		}

	}
}
