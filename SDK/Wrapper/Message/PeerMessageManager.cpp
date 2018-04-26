// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <BRPeerMessages.h>

#include "BRPeerMessages.h"

#include "PeerMessageManager.h"
#include "TransactionMessage.h"
#include "MerkleBlockMessage.h"
#include "VersionMessage.h"
#include "AddressMessage.h"
#include "InventoryMessage.h"
#include "AddressMessage.h"
#include "BloomFilterMessage.h"
#include "NotFoundMessage.h"
#include "GetBlocksMessage.h"
#include "GetDataMessage.h"

namespace Elastos {
	namespace SDK {

		namespace {
			int BRPeerAcceptTxMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				TransactionMessage *message = static_cast<TransactionMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_TX).get());
				boost::function<int(BRPeer *, const uint8_t *, size_t)> fun =
						boost::bind(&TransactionMessage::Accept, message, _1, _2, _3);
				return fun(peer, msg, msgLen);
			}

			void BRPeerSendTxMessage(BRPeer *peer, BRTransaction *tx) {
				TransactionMessage *message = static_cast<TransactionMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_TX).get());
				boost::function<void(BRPeer *, void *)> fun =
						boost::bind(&TransactionMessage::Send, message, _1, _2);

				fun(peer, tx);
			}

			int BRPeerAcceptMerkleblockMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				MerkleBlockMessage *message = static_cast<MerkleBlockMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_MERKLEBLOCK).get());
				boost::function<void(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&MerkleBlockMessage::Accept, message, _1, _2, _3);

				fun(peer, msg, msgLen);
			}

			int BRPeerAcceptVersionMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				VersionMessage *message = static_cast<VersionMessage *>(
						PeerMessageManager::instance().getMessage(MSG_VERSION).get());
				boost::function<int(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&VersionMessage::Accept, message, _1, _2, _3);

				return fun(peer, msg, msgLen);
			}

			void BRPeerSendVersionMessage(BRPeer *peer) {
				VersionMessage *message = static_cast<VersionMessage *>(
						PeerMessageManager::instance().getMessage(MSG_VERSION).get());
				boost::function<void(BRPeer *peer)> fun =
						boost::bind(&VersionMessage::Send, message, _1);

				fun(peer);
			}

			int BRPeerAcceptAddressMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				AddressMessage *message = static_cast<AddressMessage *>(
						PeerMessageManager::instance().getMessage(MSG_ADDR).get());
				boost::function<int(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&AddressMessage::Accept, message, _1, _2, _3);

				return fun(peer, msg, msgLen);
			}

			void BRPeerSendAddressMessage(BRPeer *peer) {
				AddressMessage *message = static_cast<AddressMessage *>(
						PeerMessageManager::instance().getMessage(MSG_ADDR).get());
				boost::function<void(BRPeer *peer)> fun =
						boost::bind(&AddressMessage::Send, message, _1);

				fun(peer);
			}

			int BRPeerAcceptInventoryMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				InventoryMessage *message = static_cast<InventoryMessage *>(
						PeerMessageManager::instance().getMessage(MSG_INV).get());
				boost::function<int(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&InventoryMessage::Accept, message, _1, _2, _3);

				return fun(peer, msg, msgLen);
			}

			void BRPeerSendInventoryMessage(BRPeer *peer, const UInt256 txHashes[], size_t txCount) {
				InventoryMessage *message = static_cast<InventoryMessage *>(
						PeerMessageManager::instance().getMessage(MSG_INV).get());
				boost::function<void(BRPeer *peer, const UInt256 txHashes[], size_t txCount)> fun =
						boost::bind(&InventoryMessage::Send, message, _1, _2, _3);

				fun(peer, txHashes, txCount);
			}

			int BRPeerAcceptNotFoundMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				NotFoundMessage *message = static_cast<NotFoundMessage *>(
						PeerMessageManager::instance().getMessage(MSG_GETADDR).get());
				boost::function<int(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&NotFoundMessage::Accept, message, _1, _2, _3);

				return fun(peer, msg, msgLen);
			}

			void BRPeerSendFilterload(BRPeer *peer, BRBloomFilter *filter) {
				BloomFilterMessage *message = static_cast<BloomFilterMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_FILTERLOAD).get());
				boost::function<void(BRPeer *, void *)> fun =
						boost::bind(&BloomFilterMessage::Send, message, _1, _2);

				fun(peer, filter);
			}

			void BRPeerSendGetblocks(BRPeer *peer, const UInt256 locators[], size_t locatorsCount, UInt256 hashStop) {
				GetBlocksMessage *message = static_cast<GetBlocksMessage *>(
						PeerMessageManager::instance().getMessage(MSG_GETBLOCKS).get());
				boost::function<void(BRPeer *, const UInt256 locators[], size_t locatorsCount, UInt256 hashStop)> fun =
						boost::bind(&GetBlocksMessage::SendGetBlocks, message, _1, _2, _3, _4);

				fun(peer, locators, locatorsCount, hashStop);
			}

			void BRPeerSendGetdata(BRPeer *peer, const UInt256 txHashes[], size_t txCount,
								   const UInt256 blockHashes[], size_t blockCount) {
				GetDataMessage *message = static_cast<GetDataMessage *>(
						PeerMessageManager::instance().getMessage(MSG_GETDATA).get());
				boost::function<void(BRPeer *peer, const UInt256 txHashes[], size_t txCount,
									 const UInt256 blockHashes[], size_t blockCount)> fun =
						boost::bind(&GetDataMessage::SendGetData, message, _1, _2, _3, _4, _5);

				fun(peer, txHashes, txCount, blockHashes, blockCount);
			}
		}

		PeerMessageManager PeerMessageManager::_instance = PeerMessageManager();

		PeerMessageManager::PeerMessageManager() {
		}

		PeerMessageManager::~PeerMessageManager() {
		}

		void PeerMessageManager::initMessages(BRPeerManager *peerManager) {

			peerManager->peerMessages = BRPeerMessageNew();

			peerManager->peerMessages->BRPeerAcceptTxMessage = BRPeerAcceptTxMessage;
			peerManager->peerMessages->BRPeerSendTxMessage = BRPeerSendTxMessage;
			_wrapperMessages[MSG_TX] = WrapperMessagePtr(new TransactionMessage);

			peerManager->peerMessages->BRPeerAcceptMerkleblockMessage = BRPeerAcceptMerkleblockMessage;
			_wrapperMessages[MSG_MERKLEBLOCK] = WrapperMessagePtr(new MerkleBlockMessage);

			peerManager->peerMessages->BRPeerAcceptVersionMessage = BRPeerAcceptVersionMessage;
			peerManager->peerMessages->BRPeerSendVersionMessage = BRPeerSendVersionMessage;
			_messages[MSG_VERSION] = MessagePtr(new VersionMessage);

			peerManager->peerMessages->BRPeerAcceptInventoryMessage = BRPeerAcceptInventoryMessage;
			peerManager->peerMessages->BRPeerSendInventoryMessage = BRPeerSendInventoryMessage;
			_messages[MSG_INV] = MessagePtr(new InventoryMessage);

			peerManager->peerMessages->BRPeerAcceptAddressMessage = BRPeerAcceptAddressMessage;
			peerManager->peerMessages->BRPeerSendAddressMessage = BRPeerSendAddressMessage;
			_messages[MSG_ADDR] = MessagePtr(new AddressMessage);

			peerManager->peerMessages->BRPeerAcceptNotFoundMessage = BRPeerAcceptNotFoundMessage;
			_messages[MSG_NOTFOUND] = MessagePtr(new NotFoundMessage);

			peerManager->peerMessages->BRPeerSendFilterloadMessage = BRPeerSendFilterload;
			_wrapperMessages[MSG_FILTERLOAD] = WrapperMessagePtr(new BloomFilterMessage);

			peerManager->peerMessages->BRPeerSendGetblocksMessage = BRPeerSendGetblocks;
			_messages[MSG_GETBLOCKS] = MessagePtr(new GetBlocksMessage);

			//use same message with getblocks
			peerManager->peerMessages->BRPeerSendGetheadersMessage = BRPeerSendGetblocks;

			peerManager->peerMessages->BRPeerSendGetdataMessage = BRPeerSendGetdata;
			_messages[MSG_GETDATA] = MessagePtr(new GetDataMessage);
		}

		PeerMessageManager &PeerMessageManager::instance() {
			return _instance;
		}

		const PeerMessageManager::WrapperMessagePtr &PeerMessageManager::getWrapperMessage(
				const std::string &message) {
			return _wrapperMessages[message];
		}

		const PeerMessageManager::MessagePtr &PeerMessageManager::getMessage(const std::string &message) {
			return _messages[message];
		}
	}
}