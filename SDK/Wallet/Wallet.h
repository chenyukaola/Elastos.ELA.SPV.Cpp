// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_WALLET_H__
#define __ELASTOS_SDK_WALLET_H__

#include "Database/DatabaseManager.h"
#include <Common/Lockable.h>
#include <Common/ElementSet.h>
#include <Account/SubAccount.h>
#include <Wallet/GroupedAsset.h>
#include <Plugin/Transaction/TransactionInput.h>

#include <boost/weak_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <map>

#define TX_FEE_PER_KB        1000ULL     // standard tx fee per kb of tx size, rounded up to nearest kb
#define TX_OUTPUT_SIZE       34          // estimated size for a typical transaction output
#define TX_INPUT_SIZE        148         // estimated size for a typical compact pubkey transaction input
#define TX_MAX_SIZE          100000      // no tx can be larger than this size in bytes
#define TX_UNCONFIRMED INT32_MAX
#define DEFAULT_FEE_PER_KB (10000)                  // 10 satoshis-per-byte
#define MIN_FEE_PER_KB     TX_FEE_PER_KB                       // bitcoind 0.12 default min-relay fee
#define MAX_FEE_PER_KB     ((TX_FEE_PER_KB*1000100 + 190)/191) // slightly higher than a 10,000bit fee on a 191byte tx
#define TX_MAX_LOCK_HEIGHT   500000000   // a lockTime below this value is a block height, otherwise a timestamp
#define TX_MIN_OUTPUT_AMOUNT (TX_FEE_PER_KB*3*(TX_OUTPUT_SIZE + TX_INPUT_SIZE)/1000) //no txout can be below this amount

namespace Elastos {
	namespace ElaWallet {

		class Transaction;
		class TransactionOutput;
		class TransactionInput;
		class Address;
		class Asset;
		class IPayload;
		class UTXO;
		class IOutputPayload;
		typedef boost::shared_ptr<Asset> AssetPtr;
		typedef boost::shared_ptr<Transaction> TransactionPtr;
		typedef boost::shared_ptr<IPayload> PayloadPtr;
		typedef boost::shared_ptr<UTXO> UTXOPtr;
		typedef std::vector<UTXOPtr> UTXOArray;
		typedef boost::shared_ptr<TransactionOutput> OutputPtr;
		typedef boost::shared_ptr<TransactionInput> InputPtr;
		typedef boost::shared_ptr<IOutputPayload> OutputPayloadPtr;

		enum TxnType {
			TXN_NORMAL,
			TXN_PENDING,
			TXN_COINBASE
		};

		class Wallet : public Lockable {
		public:
			class Listener {
			public:
				virtual void onUTXOUpdated(const UTXOArray &utxoAdded, const UTXOArray &utxoDeleted, bool replace) = 0;

				virtual void onBalanceChanged(const uint256 &asset, const BigInt &balance) = 0;

				virtual void onTxAdded(const TransactionPtr &tx) = 0;

				virtual void onTxUpdated(const std::vector<TransactionPtr> &txns) = 0;

				virtual void onTxDeleted(const TransactionPtr &tx, bool notifyUser, bool recommendRescan) = 0;

				virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) = 0;

				virtual void onUsedAddressSaved(const AddressSet &usedAddress, bool replace) = 0;

				virtual void onUsedAddressAdded(const AddressPtr &usedAddress) = 0;

				virtual void onTxnReplace(const std::vector<TransactionPtr> &txConfirmed,
										  const std::vector<TransactionPtr> &txPending,
										  const std::vector<TransactionPtr> &txCoinbase) = 0;

			public:
				virtual std::vector<TransactionPtr> onLoadTxn(const std::string &chainID, TxnType type) const = 0;

				virtual std::vector<TransactionPtr> onLoadTxnAfter(const std::string &chainID, uint32_t height) const = 0;

				virtual TransactionPtr onLoadTxn(const std::string &chainID, const uint256 &hash) const = 0;

				virtual bool onContainTxn(const uint256 &hash) const = 0;

				virtual std::vector<TransactionPtr> onLoadUTXOTxn(const std::string &chainID) const = 0;
			};

		public:
			Wallet(uint32_t lastBlockHeight,
				   bool existPendingTxnTable,
				   const std::string &walletID,
				   const std::string &chainID,
				   const AddressSet &usedAddress,
				   const std::vector<UTXOPtr> &utxo,
				   const std::vector<AssetPtr> &assetArray,
				   const SubAccountPtr &subAccount,
				   const boost::shared_ptr<Wallet::Listener> &listener,
				   const DatabaseManagerPtr &database);

			virtual ~Wallet();

			void GenerateCID();

			nlohmann::json GetBasicInfo() const;

			const std::string &GetWalletID() const;

			void SetBlockHeight(uint32_t height);

			uint32_t LastBlockHeight() const;

			nlohmann::json GetBalanceInfo();

			BigInt GetBalanceWithAddress(const uint256 &assetID, const std::string &addr) const;

			// returns the first unused external address
			AddressPtr GetReceiveAddress() const;

			size_t GetAllAddresses(AddressArray &addr, uint32_t start, size_t count, bool internal) const;

			size_t GetAllCID(AddressArray &cid, uint32_t start, size_t count) const;

			size_t GetAllPublickeys(std::vector<bytes_t> &pubkeys, uint32_t start, size_t count, bool containInternal);

			AddressPtr GetOwnerDepositAddress() const;

			AddressPtr GetCROwnerDepositAddress() const;

			AddressPtr GetOwnerAddress() const;

			AddressArray GetAllSpecialAddresses() const;

			bytes_t GetOwnerPublilcKey() const;

			bool IsDepositAddress(const AddressPtr &addr) const;

			// true if the address was previously generated by BRWalletUnusedAddrs() (even if it's now used)
			bool ContainsAddress(const AddressPtr &address);

			BigInt GetBalance(const uint256 &assetID) const;

			uint64_t GetFeePerKb() const;

			void SetFeePerKb(uint64_t fee);

			TransactionPtr Vote(const VoteContent &voteContent, const std::string &memo, bool max,
			                    VoteContentArray &dropedVotes);

			TransactionPtr Consolidate(const std::string &memo, const uint256 &asset);

			TransactionPtr CreateRetrieveTransaction(uint8_t type, const PayloadPtr &payload, const BigInt &amount,
													 const AddressPtr &fromAddress, const std::string &memo);

			TransactionPtr CreateTransaction(uint8_t type, const PayloadPtr &payload,
											 const AddressPtr &fromAddress, const OutputArray &outputs,
											 const std::string &memo, bool max = false);

			bool ContainsTransaction(const TransactionPtr &transaction);

			bool ContainsTransaction(const uint256 &txHash) const;

			bool RegisterTransaction(const TransactionPtr &tx);

			void RemoveTransaction(const uint256 &transactionHash);

			void UpdateTransactions(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp);

			TransactionPtr TransactionForHash(const uint256 &transactionHash) const;

			std::vector<TransactionPtr> GetTransactions(const bytes_t &types) const;

			std::map<uint256, TransactionPtr> TransactionsForInputs(const InputArray &inputs) const;

			bool TransactionIsValid(const TransactionPtr &transaction);

#if 0
			bool TransactionIsPending(const TransactionPtr &transaction);

			bool TransactionIsVerified(const TransactionPtr &transaction);
#endif

			BigInt AmountSentByTx(const TransactionPtr &tx);

			bool IsReceiveTransaction(const TransactionPtr &tx) const;

			bool StripTransaction(const TransactionPtr &tx) const;

			void SignTransaction(const TransactionPtr &tx, const std::string &payPassword) const;

			std::string SignWithDID(const AddressPtr &did, const std::string &msg, const std::string &payPasswd) const;

			std::string SignDigestWithDID(const AddressPtr &did, const uint256 &digest,
			                              const std::string &payPasswd) const;

			bytes_t SignWithOwnerKey(const bytes_t &msg, const std::string &payPasswd);

			void UpdateLockedBalance();

			UTXOArray GetAllUTXO(const std::string &address) const;

			UTXOArray GetVoteUTXO() const;

			std::vector<TransactionPtr> TxUnconfirmedBefore(uint32_t blockHeight);

			void SetTxUnconfirmedAfter(uint32_t blockHeight);

			AddressArray UnusedAddresses(uint32_t gapLimit, bool internal);

			AssetPtr GetAsset(const uint256 &assetID) const;

			nlohmann::json GetAllAssets() const;

			bool AssetNameExist(const std::string &name) const;

		private:
			GroupedAssetPtr GetGroupedAsset(const uint256 &assetID) const;

			bool ContainsAsset(const uint256 &assetID) const;

			bool ContainsTx(const TransactionPtr &tx) const;

			bool ContainsInput(const InputPtr &in) const;

			std::vector<UTXOPtr> GetUTXO(const uint256 &assetID, const std::string &addr) const;

			bool IsAssetUnique(const std::vector<OutputPtr> &outputs) const;

			std::map<uint256, BigInt> BalanceAfterUpdatedTx(const TransactionPtr &tx, UTXOArray &deleted, UTXOArray &added);

			std::map<uint256, BigInt> BalanceAfterRemoveTx(const TransactionPtr &tx, UTXOArray &deleted, UTXOArray &added);

			void AddSpendingUTXO(const InputArray &inputs);

			void RemoveSpendingUTXO(const InputArray &inputs);

			void InstallAssets(const std::vector<AssetPtr> &assets);

			void InstallDefaultAsset();

			bool IsUTXOSpending(const UTXOPtr &utxo) const;

		protected:
			void UTXOUpdated(const UTXOArray &added, const UTXOArray &deleted, bool replace = false);

			void balanceChanged(const uint256 &asset, const BigInt &balance);

			void txnReplace(const std::vector<TransactionPtr> &txConfirmed,
							const std::vector<TransactionPtr> &txPending,
							const std::vector<TransactionPtr> &txCoinbase);

			void txAdded(const TransactionPtr &tx);

			void txUpdated(const std::vector<TransactionPtr> &txns);

			void txDeleted(const TransactionPtr &tx, bool notifyUser, bool recommendRescan);

			void assetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);

			std::vector<TransactionPtr> loadTxn(TxnType type) const;

			std::vector<TransactionPtr> loadTxnAfter(uint32_t height) const;

			TransactionPtr loadTxn(const uint256 &hash) const;

			bool containTxn(const uint256 &hash) const;

			void usedAddressSaved(const AddressSet &usedAddress, bool replace = false);

			void usedAddressAdded(const AddressPtr &usedaddress);

			std::vector<TransactionPtr> LoadUTXOTxn() const;
		protected:
			friend class GroupedAsset;

			std::string _walletID, _chainID;

			SubAccountPtr _subAccount;

			typedef std::map<uint256, GroupedAssetPtr> GroupedAssetMap;
			mutable GroupedAssetMap _groupedAssets;

//			typedef ElementSet<TransactionPtr> TransactionSet;
//			std::vector<TransactionPtr> _transactions, _coinbaseTransactions;
//			TransactionSet _allTx;

			DatabaseManagerWeakPtr _database;

			UTXOSet _spendingOutputs;

			uint64_t _feePerKb;

			uint32_t _blockHeight;
			boost::weak_ptr<Listener> _listener;
		};

		typedef boost::shared_ptr<Wallet> WalletPtr;

	}
}

#endif //__ELASTOS_SDK_WALLET_H__
