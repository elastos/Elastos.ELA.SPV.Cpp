#include "CMemBlock.h"
#include "ByteStream.h"
#include "BRAddress.h"

namespace Elastos {
	namespace ElaWallet {


		ByteStream::ByteStream(bool isBe)
				: _pos(0), _count(0), _size(0), _buf(nullptr), _autorelease(true) ,_isBe(isBe) {
		}

		ByteStream::ByteStream(uint64_t size, bool isBe)
				: _pos(0), _count(0), _size(size), _buf(new uint8_t[size]), _autorelease(true) ,_isBe(isBe) {
			memset(_buf, 0, sizeof(uint8_t) * size);
		}

		ByteStream::ByteStream(uint8_t *buf, uint64_t size, bool autorelease, bool isBe)
				: _pos(0), _count(size), _size(size), _buf(buf), _autorelease(autorelease) ,_isBe(isBe) {}

		ByteStream::~ByteStream() {
			if (_autorelease) {
				delete[]_buf;
				_buf = nullptr;
			}

		}

		void ByteStream::ensureCapacity(uint64_t newsize) {
			if ((int64_t)(newsize - _size) > 0) {
				uint64_t oldCapacity = _size;
				uint64_t newCapacity = oldCapacity << 1;
				int64_t diff = newCapacity - newsize;
				if (diff < 0)
					newCapacity = newsize;
				if (newCapacity <= 0) {
					if (newsize <= 0) // overflow
						return;
					newCapacity = UINT64_MAX;
				}
				uint8_t *newBuf = new uint8_t[newCapacity];
				memset(newBuf, 0, newCapacity);
				memcpy(newBuf, _buf, oldCapacity);
				delete[] _buf;
				_buf = newBuf;
				_size = newCapacity;
			}
		}

		bool ByteStream::checkSize(uint64_t readSize) {
			if (_pos + readSize > _count)
				return false;
			return true;
		}

		void ByteStream::setPosition(uint64_t position) {
			_pos = position;
		}

		uint64_t ByteStream::position() {
			return _pos;
		}

		uint64_t ByteStream::length() {
			return _count;
		}

		void ByteStream::put(uint8_t byte) {
			ensureCapacity(position() + sizeof(uint8_t));
			_buf[_pos++] = byte;
			_count = _pos;
		}

		uint8_t ByteStream::get() {
			if (!checkSize(sizeof(uint8_t)))
				return 0;
			return _buf[_pos++];
		}

		uint8_t ByteStream::getUByte() {
			return get();
		}

		int8_t ByteStream::getByte() {
			if (!checkSize(sizeof(uint8_t)))
				return 0;
			return _buf[_pos++];
		}

		void ByteStream::putShort(int16_t v) {
			ensureCapacity(position() + sizeof(int16_t));
			if (_isBe) {
				_buf[_pos++] = (v >> 8) & 0xff;
				_buf[_pos++] = v & 0xff;
			}
			else {
				_buf[_pos++] = (v) & 0xff;
				_buf[_pos++] = (v >> 8) & 0xff;
			}
			_count = _pos;
		}

		void ByteStream::putUint16(uint16_t v) {
			ensureCapacity(position() + sizeof(uint16_t));
			if (_isBe) {
				_buf[_pos++] = (v >> 8) & 0xff;
				_buf[_pos++] = v & 0xff;
			}
			else {
				_buf[_pos++] = v & 0xff;
				_buf[_pos++] = (v >> 8) & 0xff;
			}
			_count = _pos;
		}

		int16_t ByteStream::getShort() {
			if (!checkSize(sizeof(int16_t)))
				return 0;
			if (_isBe) {
				int16_t r1 = (int16_t) _buf[_pos++] << 8;
				int16_t r2 = (int16_t) _buf[_pos++];
				int16_t ret = r1 | r2;
				return ret;
			}
			else {
				int16_t r1 = (int16_t) _buf[_pos++];
				int16_t r2 = (int16_t) _buf[_pos++] << 8;
				int16_t ret = r1 | r2;
				return ret;
			}
		}

		uint16_t ByteStream::getUint16() {
			if (!checkSize(sizeof(uint16_t)))
				return 0;
			if (_isBe) {
				uint16_t r1 = (uint16_t) _buf[_pos++] << 8;
				uint16_t r2 = (uint16_t) _buf[_pos++];
				uint16_t ret = r1 | r2;
				return ret;
			}
			else {
				uint16_t r1 = (uint16_t) _buf[_pos++];
				uint16_t r2 = (uint16_t) _buf[_pos++] << 8;
				uint16_t ret = r1 | r2;
				return ret;
			}
		}

		void ByteStream::putInt(int32_t v) {
			ensureCapacity(position() + sizeof(int32_t));
			if (_isBe) {
				_buf[_pos++] = (v >> 24) & 0xff;
				_buf[_pos++] = (v >> 16) & 0xff;
				_buf[_pos++] = (v >> 8) & 0xff;
				_buf[_pos++] = v & 0xff;
			}
			else {
				_buf[_pos++] = v & 0xff;
				_buf[_pos++] = (v >> 8) & 0xff;
				_buf[_pos++] = (v >> 16) & 0xff;
				_buf[_pos++] = (v >> 24) & 0xff;
			}
			_count = _pos;
		}

		void ByteStream::putUint32(uint32_t v) {
			ensureCapacity(position() + sizeof(uint32_t));
			if (_isBe) {
				_buf[_pos++] = (v >> 24) & 0xff;
				_buf[_pos++] = (v >> 16) & 0xff;
				_buf[_pos++] = (v >> 8) & 0xff;
				_buf[_pos++] = v & 0xff;
			}
			else {
				_buf[_pos++] = v & 0xff;
				_buf[_pos++] = (v >> 8) & 0xff;
				_buf[_pos++] = (v >> 16) & 0xff;
				_buf[_pos++] = (v >> 24) & 0xff;
			}
			_count = _pos;
		}

		int32_t ByteStream::getInt() {
			if (!checkSize(sizeof(int32_t)))
				return 0;
			if (_isBe) {
				int32_t ret = (int32_t) _buf[_pos++] << 24;
				ret |= (int32_t) _buf[_pos++] << 16;
				ret |= (int32_t) _buf[_pos++] << 8;
				ret |= (int32_t) _buf[_pos++];
				return ret;
			}
			else {
				int32_t ret = (int32_t) _buf[_pos++];
				ret |= (int32_t) _buf[_pos++] << 8;
				ret |= (int32_t) _buf[_pos++] << 16;
				ret |= (int32_t) _buf[_pos++] << 24;
				return ret;
			}
		}

		uint32_t ByteStream::getUint32() {
			if (!checkSize(sizeof(uint32_t)))
				return 0;
			if (_isBe) {
				uint32_t ret = (uint32_t) _buf[_pos++] << 24;
				ret |= (int32_t) _buf[_pos++] << 16;
				ret |= (int32_t) _buf[_pos++] << 8;
				ret |= (int32_t) _buf[_pos++];
				return ret;
			}
			else {
				uint32_t ret = (uint32_t) _buf[_pos++];
				ret |= (int32_t) _buf[_pos++] << 8;
				ret |= (int32_t) _buf[_pos++] << 16;
				ret |= (int32_t) _buf[_pos++] << 24;
				return ret;
			}
		}

		void ByteStream::getInts(int32_t *buf, int32_t len) {
			if (!checkSize(sizeof(int32_t) * len))
				return;
			if (_isBe) {
				for (int32_t i = 0; i < len; i++) {
					buf[i] = (int32_t) _buf[_pos++] << 24;
					buf[i] |= (int32_t) _buf[_pos++] << 16;
					buf[i] |= (int32_t) _buf[_pos++] << 8;
					buf[i] |= (int32_t) _buf[_pos++];
				}
			}
			else {
				for (int32_t i = 0; i < len; i++) {
					buf[i] = (int32_t) _buf[_pos++];
					buf[i] |= (int32_t) _buf[_pos++] << 8;
					buf[i] |= (int32_t) _buf[_pos++] << 16;
					buf[i] |= (int32_t) _buf[_pos++] << 24;
				}
			}
		}

		void ByteStream::putLong(int64_t v) {
			ensureCapacity(position() + sizeof(int64_t));
			if (_isBe) {
				_buf[_pos++] = (v >> 56) & 0xff;
				_buf[_pos++] = (v >> 48) & 0xff;
				_buf[_pos++] = (v >> 40) & 0xff;
				_buf[_pos++] = (v >> 32) & 0xff;
				_buf[_pos++] = (v >> 24) & 0xff;
				_buf[_pos++] = (v >> 16) & 0xff;
				_buf[_pos++] = (v >> 8) & 0xff;
				_buf[_pos++] = v & 0xff;
			}
			else {
				_buf[_pos++] = v & 0xff;
				_buf[_pos++] = (v >> 8) & 0xff;
				_buf[_pos++] = (v >> 16) & 0xff;
				_buf[_pos++] = (v >> 24) & 0xff;
				_buf[_pos++] = (v >> 32) & 0xff;
				_buf[_pos++] = (v >> 40) & 0xff;
				_buf[_pos++] = (v >> 48) & 0xff;
				_buf[_pos++] = (v >> 56) & 0xff;
			}
			_count = _pos;
		}

		void ByteStream::putUint64(uint64_t v) {
			ensureCapacity(position() + sizeof(uint64_t));
			if (_isBe) {
				_buf[_pos++] = (v >> 56) & 0xff;
				_buf[_pos++] = (v >> 48) & 0xff;
				_buf[_pos++] = (v >> 40) & 0xff;
				_buf[_pos++] = (v >> 32) & 0xff;
				_buf[_pos++] = (v >> 24) & 0xff;
				_buf[_pos++] = (v >> 16) & 0xff;
				_buf[_pos++] = (v >> 8) & 0xff;
				_buf[_pos++] = v & 0xff;
			}
			else {
				_buf[_pos++] = v & 0xff;
				_buf[_pos++] = (v >> 8) & 0xff;
				_buf[_pos++] = (v >> 16) & 0xff;
				_buf[_pos++] = (v >> 24) & 0xff;
				_buf[_pos++] = (v >> 32) & 0xff;
				_buf[_pos++] = (v >> 40) & 0xff;
				_buf[_pos++] = (v >> 48) & 0xff;
				_buf[_pos++] = (v >> 56) & 0xff;
			}
			_count = _pos;
		}

		int64_t ByteStream::getLong() {

			if (!checkSize(sizeof(int64_t)))
				return 0;
			uint64_t ret = 0;
			if (_isBe) {
				ret = (int64_t) _buf[_pos++] << 56;
				ret |= (int64_t) _buf[_pos++] << 48;
				ret |= (int64_t) _buf[_pos++] << 40;
				ret |= (int64_t) _buf[_pos++] << 32;
				ret |= (int64_t) _buf[_pos++] << 24;
				ret |= (int64_t) _buf[_pos++] << 16;
				ret |= (int64_t) _buf[_pos++] << 8;
				ret |= (int64_t) _buf[_pos++];
			}
			else {
				ret = (int64_t) _buf[_pos++];
				ret |= (int64_t) _buf[_pos++] << 8;
				ret |= (int64_t) _buf[_pos++] << 16;
				ret |= (int64_t) _buf[_pos++] << 24;
				ret |= (int64_t) _buf[_pos++] << 32;
				ret |= (int64_t) _buf[_pos++] << 40;
				ret |= (int64_t) _buf[_pos++] << 48;
				ret |= (int64_t) _buf[_pos++] << 56;
			}
			return ret;
		}

		uint64_t ByteStream::getUint64() {
			if (!checkSize(sizeof(uint64_t)))
				return 0;
			uint64_t ret = 0;
			if (_isBe) {
				ret = (int64_t) _buf[_pos++] << 56;
				ret |= (int64_t) _buf[_pos++] << 48;
				ret |= (int64_t) _buf[_pos++] << 40;
				ret |= (int64_t) _buf[_pos++] << 32;
				ret |= (int64_t) _buf[_pos++] << 24;
				ret |= (int64_t) _buf[_pos++] << 16;
				ret |= (int64_t) _buf[_pos++] << 8;
				ret |= (int64_t) _buf[_pos++];
			}
			else {
				ret = (int64_t) _buf[_pos++];
				ret |= (int64_t) _buf[_pos++] << 8;
				ret |= (int64_t) _buf[_pos++] << 16;
				ret |= (int64_t) _buf[_pos++] << 24;
				ret |= (int64_t) _buf[_pos++] << 32;
				ret |= (int64_t) _buf[_pos++] << 40;
				ret |= (int64_t) _buf[_pos++] << 48;
				ret |= (int64_t) _buf[_pos++] << 56;
			}
			return ret;
		}

		void ByteStream::putBytes(const uint8_t *byte, uint64_t len) {
			ensureCapacity(position() + sizeof(uint8_t) * len);
			memcpy(&_buf[position()], byte, len);
			_pos += len;
			_count = _pos;
		}

		void ByteStream::getBytes(uint8_t *buf, uint64_t len) {
			if (!checkSize(sizeof(uint8_t) * len))
				return;
			memcpy(buf, &_buf[_pos], len);
			_pos += len;
		}

		uint64_t ByteStream::getVarUint() {
			size_t len = 0;
			uint8_t tbuff[sizeof(uint8_t)*9];
			memcpy(tbuff, &_buf[_pos], sizeof(tbuff));
			uint64_t value = BRVarInt(tbuff, sizeof(tbuff), &len);
			getBytes(tbuff, len);
			return value;
		}

		void ByteStream::putUTF8(const char *str) {
			int len = 0;
			int idx = 0;
			while (str[idx] != '\0') {
				len++;
				idx++;
			}
			putShort(len);
			putBytes((uint8_t *) str, len);
		}

		void ByteStream::putVarUint(uint64_t value) {
			uint8_t tbuff[sizeof(uint8_t)*9];
			uint64_t len = BRVarIntSet(tbuff, sizeof(tbuff), value);
			putBytes(tbuff, len);
		}

		char *ByteStream::getUTF8(int32_t &len) {
			short utfLen = getShort();
			char *utfBuffer = new char[utfLen + 1];
			if (!checkSize(utfLen)) {
				utfBuffer[0] = '\0';
			} else {
				getBytes((uint8_t *) utfBuffer, utfLen);
				utfBuffer[utfLen] = '\0';
			}

			len = utfLen + 2;

			return utfBuffer;
		}

		uint64_t ByteStream::availableSize() {
			uint64_t ret = _size - _count;

			return ((int64_t)ret) < 0 ? 0 : ret;
		}

		char *ByteStream::getUTF8() {
			int len = 0;
			return getUTF8(len);
		}

		CMBlock ByteStream::getBuffer() {
			if (_count <= 0) {
				return CMBlock();
			}

			CMBlock buff((size_t)_count);
			memcpy(buff, _buf, _count);
			return buff;
		}

		void ByteStream::skip(int bytes) {
			if (checkSize(bytes))
				_pos += bytes;
		}

		void ByteStream::reset() {
			this->setPosition(0);
			this->_size = 0;
			if (this->_buf != nullptr) {
				delete[] this->_buf;
				this->_buf = nullptr;
			}
			this->_count = 0;
		}

		void ByteStream::increasePosition(size_t len) {
			_pos += len;
		}

		bool ByteStream::readUint8(uint8_t &val) {
			return readBytes(&val, 1);
		}

		void ByteStream::writeUint8(uint8_t val) {
			writeBytes(&val, 1);
		}

		bool ByteStream::readUint16(uint16_t &val, ByteOrder byteOrder) {
			return readBytes(&val, sizeof(uint16_t), byteOrder);
		}

		void ByteStream::writeUint16(uint16_t val, ByteOrder byteOrder) {
			writeBytes(&val, sizeof(uint16_t), byteOrder);
		}

		bool ByteStream::readUint32(uint32_t &val, ByteOrder byteOrder) {
			return readBytes(&val, sizeof(uint32_t), byteOrder);
		}

		void ByteStream::writeUint32(uint32_t val, ByteOrder byteOrder) {
			writeBytes(&val, sizeof(uint32_t), byteOrder);
		}

		bool ByteStream::readUint64(uint64_t &val, ByteOrder byteOrder) {
			return readBytes(&val, sizeof(uint64_t), byteOrder);
		}

		void ByteStream::writeUint64(uint64_t val, ByteOrder byteOrder) {
			writeBytes(&val, sizeof(uint64_t), byteOrder);
		}

		bool ByteStream::readBytes(void *buf, size_t len, ByteOrder byteOrder) {
			if (!checkSize(len))
				return false;

			size_t pos = position();

			if (buf != nullptr) {
				if (byteOrder == LittleEndian) {
					memcpy(buf, &_buf[pos], len);
				} else {
					for (size_t i = 0; i < len; ++i) {
						((uint8_t *)buf)[i] = _buf[len - 1 - i + pos];
					}
				}
			}

			increasePosition(len);

			return true;
		}

		void ByteStream::writeBytes(const void *buf, size_t len, ByteOrder byteOrder) {
			ensureCapacity(position() + len);

			size_t pos = position();

			if (byteOrder == LittleEndian) {
				memcpy(&_buf[pos], buf, len);
			} else {
				for (size_t i = 0; i < len; ++i) {
					_buf[pos + i] = ((uint8_t *)buf)[len - i - 1];
				}
			}

			increasePosition(len);
			_count = position();
		}

		bool ByteStream::readVarBytes(CMBlock &bytes) {
			uint64_t length = 0;
			if (!readVarUint(length)) {
				return false;
			}

			bytes.Resize((size_t)length);
			return readBytes(bytes, bytes.GetSize());
		}

		void ByteStream::writeVarBytes(const void *bytes, size_t len) {
			writeVarUint((uint64_t)len);
			writeBytes(bytes, len);
		}

		void ByteStream::writeVarBytes(const CMBlock &bytes) {
			writeVarUint((uint64_t)bytes.GetSize());
			writeBytes(bytes, bytes.GetSize());
		}

		bool ByteStream::readVarUint(uint64_t &value) {
			size_t len = 0;
			value = BRVarInt(&_buf[_pos], 9, &len);
			return readBytes(nullptr, len);
		}

		void ByteStream::writeVarUint(uint64_t value) {
			size_t len = BRVarIntSet(nullptr, 0, value);

			ensureCapacity(position() + len);
			BRVarIntSet(&_buf[position()], len, value);

			increasePosition(len);
			_count = position();
		}

		bool ByteStream::readVarString(char *str, size_t strSize) {
			CMBlock bytes;
			if (!readVarBytes(bytes)) {
				return false;
			}
			size_t len = bytes.GetSize() > strSize - 1 ? strSize - 1 : bytes.GetSize();
			strncpy(str, (const char *)(const void *)bytes, len);
			str[len] = '\0';

			return true;
		}

		bool ByteStream::readVarString(std::string &str) {
			CMBlock bytes;
			if (!readVarBytes(bytes)) {
				return false;
			}
			str = std::string((const char *)(const void *)bytes, (size_t)bytes.GetSize());

			return true;
		}

		void ByteStream::writeVarString(const char *str) {
			writeVarBytes(str, strlen(str));
		}

		void ByteStream::writeVarString(const std::string &str) {
			writeVarBytes(str.c_str(), str.length());
		}

	}
}