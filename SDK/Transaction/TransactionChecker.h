// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONCHECKER_H__
#define __ELASTOS_SDK_TRANSACTIONCHECKER_H__

#include "Transaction.h"

namespace Elastos {
	namespace ElaWallet {

		class TransactionChecker {
		public:
			TransactionChecker(const TransactionPtr &transaction);

			virtual ~TransactionChecker();

			virtual void Check();

		protected:
			virtual bool checkTransactionOutput(const TransactionPtr &transaction);

			virtual bool checkTransactionAttribute(const TransactionPtr &transaction);

			virtual bool checkTransactionProgram(const TransactionPtr &transaction);

			virtual bool checkTransactionPayload(const TransactionPtr &transaction);

		protected:
			TransactionPtr _transaction;
		};

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONCHECKER_H__
