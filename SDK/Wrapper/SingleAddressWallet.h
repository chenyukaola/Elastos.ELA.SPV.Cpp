// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SINGLEADDRESSWALLET_H__
#define __ELASTOS_SDK_SINGLEADDRESSWALLET_H__

#include "Wallet.h"

namespace Elastos {
	namespace SDK {

		class SingleAddressWallet : public Wallet {
		public:
			SingleAddressWallet(const SharedWrapperList<Transaction, BRTransaction *> &transactions,
								const MasterPubKeyPtr &masterPubKey,
								const boost::shared_ptr<Listener> &listener);

			virtual ~SingleAddressWallet();

		private:
			BRWallet *createSingleWallet(BRTransaction *transactions[], size_t txCount, const BRMasterPubKey &mpk);
		};

	}
}

#endif //__ELASTOS_SDK_SINGLEADDRESSWALLET_H__
