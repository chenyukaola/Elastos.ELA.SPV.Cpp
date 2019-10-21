// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDChainSubWallet.h"
#include "MasterWallet.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Base64.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>
#include <SDK/WalletCore/BIPs/Key.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/Plugin/Transaction/Payload/DIDInfo.h>
#include <SDK/Plugin/Transaction/Program.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/IDTransaction.h>

#include <set>
#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace Elastos {
	namespace ElaWallet {

		IDChainSubWallet::IDChainSubWallet(const CoinInfoPtr &info,
		                                   const ChainConfigPtr &config,
		                                   MasterWallet *parent) :
				SidechainSubWallet(info, config, parent) {

		}

		IDChainSubWallet::~IDChainSubWallet() {

		}

		nlohmann::json
		IDChainSubWallet::CreateIDTransaction(const nlohmann::json &payloadJson, const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("memo: {}", memo);

			Address receiveAddr;
			PayloadPtr payload = nullptr;
			try {
				payload = PayloadPtr(new DIDInfo());
				payload->FromJson(payloadJson, 0);
				DIDInfo *didInfo = static_cast<DIDInfo *>(payload.get());
				ErrorChecker::CheckParam(!didInfo->IsValid(), Error::InvalidArgument, "verify did signature failed");
				std::string id = didInfo->DIDPayload().ID();
				std::vector<std::string> idSplited;
				boost::algorithm::split(idSplited, id, boost::is_any_of(":"), boost::token_compress_on);
				ErrorChecker::CheckParam(idSplited.size() != 3, Error::InvalidArgument,
										 "invalid id format in payload JSON");
				receiveAddr = Address(idSplited[2]);
				ErrorChecker::CheckParam(!receiveAddr.Valid(), Error::InvalidArgument,
										 "invalid receive addr(id) in payload JSON");
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Create id tx param error: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(0, receiveAddr, Asset::GetELAAssetID())));

			TransactionPtr tx = CreateTx(IDTransaction::didTransaction, payload, "", outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());

			return result;
		}

		std::vector<std::string>
		IDChainSubWallet::getVerifiableCredentialTypes(const CredentialSubject &subject) {
			std::vector<std::string> types = {
					"SelfProclaimedCredential",
					"BasicProfileCredential",
			};

			if (subject.GetPhone().size() > 0) {
				types.push_back("PhoneCredential");
			}

			if (subject.GetAlipay().size() > 0 || subject.GetWechat().size() > 0 || subject.GetWeibo().size() > 0
			    || subject.GetTwitter().size() > 0 || subject.GetFacebook().size() > 0
			    || subject.GetMicrosoftPassport().size() > 0 || subject.GetGoogleAccount().size() > 0
			    || subject.GetHomePage().size() > 0 || subject.GetEmail().size() > 0) {
				types.push_back("InternetAccountCredential");
			}
			return types;
		}

		nlohmann::json IDChainSubWallet::GetAllDID(uint32_t start, uint32_t count) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);

			nlohmann::json j;
			std::vector<Address> did;
			size_t maxCount = _walletManager->GetWallet()->GetAllDID(did, start, count);

			nlohmann::json didJson;
			for (size_t i = 0; i < did.size(); ++i) {
				didJson.push_back(did[i].String());
			}

			j["DID"] = didJson;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		std::string IDChainSubWallet::Sign(const std::string &did, const std::string &message,
		                                   const std::string &payPassword) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("did: {}", did);
			ArgInfo("message: {}", message);
			ArgInfo("payPasswd: *");

			std::string signature = _walletManager->GetWallet()->SignWithDID(Address(did), message, payPassword);

			ArgInfo("r => {}", signature);

			return signature;
		}

		bool IDChainSubWallet::VerifySignature(const std::string &publicKey, const std::string &message,
											   const std::string &signature) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("pubkey: {}", publicKey);
			ArgInfo("message: {}", message);
			ArgInfo("signature: {}", signature);

			bytes_t pubkey(publicKey), sign(signature);
			Key key(pubkey);
			bool r = key.Verify(message, sign);

			ArgInfo("r => {}", r);
			return r;
		}

		std::string IDChainSubWallet::GetDIDPublicKey(const std::string &pubkey) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("pubkey:{}", pubkey);

			ErrorChecker::CheckParamNotEmpty(pubkey, "public key");

			std::string did = Address(PrefixIDChain, pubkey).String();

			ArgInfo("r => {}", did);
			return did;
		}

		nlohmann::json
		IDChainSubWallet::GenerateDIDInfoPayload(const nlohmann::json &didInfo, const std::string &payPasswd) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("didInfo: {}", didInfo.dump());
			ArgInfo("paypassword: *");

			ErrorChecker::CheckParam(didInfo.empty() || !didInfo.is_object(), Error::InvalidArgument,
									 "invalid credentialSubject JSON");

			std::string did = didInfo["id"].get<std::string>();
			std::vector<std::string> idSplited;
			boost::algorithm::split(idSplited, did, boost::is_any_of(":"), boost::token_compress_on);
			ErrorChecker::CheckParam(idSplited.size() != 3, Error::InvalidArgument, "invalid id format");

			std::string operation = didInfo["operation"].get<std::string>();
			ErrorChecker::CheckParam(operation != "create" && operation != "update" && operation != "deactivate",
									 Error::InvalidArgument, "invalid operation");

			nlohmann::json pubKeyInfoArray = didInfo["publicKey"];
			ErrorChecker::CheckJsonArray(pubKeyInfoArray, 1, "pubKeyInfoArray");

			nlohmann::json credentialSubject= didInfo["credentialSubject"];
			ErrorChecker::CheckParam(!credentialSubject.is_object(), Error::InvalidArgument,
									 "invalid credentialSubject JSON");

			std::string expirationDate = didInfo["expires"].get<std::string>();
			ErrorChecker::CheckInternetDate(expirationDate);

			DIDHeaderInfo headerInfo("elastos/did/1.0", operation);

			DIDPubKeyInfoArray didPubKeyInfoArray;
			for (nlohmann::json::iterator it = pubKeyInfoArray.begin(); it != pubKeyInfoArray.end(); ++it) {
				DIDPubKeyInfo pubKeyInfo;
				if ((*it).find("publicKey") != (*it).end()) {
					std::string pbk = (*it)["publicKey"].get<std::string>();
					bytes_t pubkey;
					pubkey.setHex(pbk);
					(*it)["publicKeyBase58"] = Base58::Encode(pubkey);
				}
				pubKeyInfo.FromJson(*it, 0);
				didPubKeyInfoArray.push_back(pubKeyInfo);
			}

			CredentialSubject subject;
			try {
				subject.FromJson(credentialSubject, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "CredentialSubject format err: " + std::string(e.what()));
			}

			VerifiableCredential verifiableCredential;
			verifiableCredential.SetID(did);

			std::vector<std::string> types = getVerifiableCredentialTypes(subject);
			verifiableCredential.SetTypes(types);
			verifiableCredential.SetCredentialSubject(subject);

			std::stringstream issuerDate;
			time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			issuerDate << std::put_time(std::localtime(&t), "%FT%TZ");
			verifiableCredential.SetIssuerDate(issuerDate.str());

			VerifiableCredentialArray verifiableCredentials;
			verifiableCredentials.push_back(verifiableCredential);

			DIDPayloadInfo payloadInfo;
			payloadInfo.SetID(did);
			payloadInfo.SetExpires(expirationDate);
			payloadInfo.SetPublickKey(didPubKeyInfoArray);
			payloadInfo.SetVerifiableCredential(verifiableCredentials);

			DIDInfo didInfoPayload;
			didInfoPayload.SetDIDHeader(headerInfo);
			didInfoPayload.SetDIDPlayloadInfo(payloadInfo);

			std::string sourceData = headerInfo.Specification() + headerInfo.Operation() + didInfoPayload.DIDPayloadString();

			std::string signature = Sign(idSplited[2], sourceData, payPasswd);
			DIDProofInfo didProofInfo("#primary", Base64::Encode(signature));
			didInfoPayload.SetDIDProof(didProofInfo);

			nlohmann::json result = didInfoPayload.ToJson(0);
			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json IDChainSubWallet::GetDIDInfo(const std::string &did) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("did:{}", did);

			std::vector<std::string> idSplited;
			boost::algorithm::split(idSplited, did, boost::is_any_of(":"), boost::token_compress_on);
			ErrorChecker::CheckParam(idSplited.size() != 3, Error::InvalidArgument, "invalid id format");

			std::vector<TransactionPtr> allTxs = _walletManager->GetWallet()->GetAllTransactions();
			size_t count = allTxs.size();
			DIDInfo *didInfo = nullptr;

			for (size_t i = count; i > 0; --i) {
				TransactionPtr tx = allTxs[i - 1];
				if (tx->GetTransactionType() == IDTransaction::didTransaction) {
					DIDInfo *payload = dynamic_cast<DIDInfo *>(tx->GetPayload());
					if (payload && payload->DIDPayload().ID() == did) {
						didInfo = payload;
						break;
					}
				}
			}

			nlohmann::json j;
			if (didInfo) {
				j = toOutJsonInfo(didInfo);
			}

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json IDChainSubWallet::toOutJsonInfo(const DIDInfo *didInfo) const {
			nlohmann::json summary;

			const DIDHeaderInfo &header = didInfo->DIDHeader();
			const DIDPayloadInfo &payloadInfo = didInfo->DIDPayload();

			summary["operation"] = header.Operation();
			summary["id"] = payloadInfo.ID();
			summary["expires"] =  payloadInfo.Expires();

			nlohmann::json jPublicKeys;
			const DIDPubKeyInfoArray &publicKeys = payloadInfo.PublicKeyInfo();
			for (size_t i = 0; i < publicKeys.size(); ++i) {
				nlohmann::json pbk;
				pbk["id"]= publicKeys[i].ID();
				bytes_t pubkey = Base58::Decode(publicKeys[i].PublicKeyBase58());
				pbk["publicKey"] = pubkey.getHex();

				if (publicKeys[i].Controller().size() > 0){
					pbk["controller"] = publicKeys[i].Controller();
				}
				jPublicKeys.push_back(pbk);
			}
			summary["publicKey"] = jPublicKeys;

			const VerifiableCredentialArray &verifiableCredentialArray = payloadInfo.GetVerifiableCredential();
			for (size_t i = 0; i < verifiableCredentialArray.size(); ++i) {
				summary["credentialSubject"] = verifiableCredentialArray[i].GetCredentialSubject().ToJson(0);
			}

			return summary;
		}
	}
}
