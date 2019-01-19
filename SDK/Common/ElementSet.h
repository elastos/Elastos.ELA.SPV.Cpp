// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONSET_H__
#define __ELASTOS_SDK_TRANSACTIONSET_H__

#include <set>

namespace Elastos {
	namespace ElaWallet {

		template<class T>
		class ElementSet {
		public:

			T Get(const UInt256 &hash) const {
				if (!Contains(hash)) return nullptr;

				typename std::set<T>::const_iterator it;
				it = std::find_if(_elements.begin(), _elements.end(), [&hash](const T &e) {
					return UInt256Eq(&hash, &e->getHash()) == 1;
				});

				if (it == _elements.end())
					return nullptr;

				return *it;
			}

			bool Contains(const T &tx) const {
				if (_elements.find(tx) != _elements.end()) {
					return true;
				}

				return Contains(tx->getHash());
			}

			bool Contains(const UInt256 &hash) const {
				typename std::set<T>::const_iterator it;
				it = std::find_if(_elements.begin(), _elements.end(), [&hash](const T &e) {
					return UInt256Eq(&hash, &e->getHash()) == 1;
				});
				return it != _elements.end();
			}

			void Insert(const T &tx) {
				_elements.insert(tx);
			}

			size_t Size() {
				return _elements.size();
			}

			void Remove(const T &tx) {
				typename std::set<T>::const_iterator it;
				for(it = _elements.cbegin(); it != _elements.cend();) {
					if (UInt256Eq(&tx->getHash(), &(*it)->getHash())) {
						it = _elements.erase(it);
					} else {
						++it;
					}
				}
			}

			void Clear() {
				_elements.clear();
			}

		private:
			std::set<T> _elements;
		};

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONSET_H__
