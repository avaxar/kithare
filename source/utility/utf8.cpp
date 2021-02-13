#include "utility/utf8.hpp"


std::string kh::encodeUtf8(const kh::String& str) {
	uint8* encoded_str = kh::_encodeToUTF8((uint32*)(&str[0]), -1);
	std::string estr((char*)encoded_str);

	delete encoded_str;
	return estr;
}

kh::String kh::decodeUtf8(const std::string& utf8_str) {
	uint32* decoded_str = kh::_decodeFromUtf8((uint8*)(&utf8_str[0]), -1);
	kh::String str(decoded_str);

	delete decoded_str;
	return str;
}

int64 kh::_getRealSizeUtf8(const uint8* src, const int64 size) {
	int64 ret = 0, i = 0;

	while (size == -1 || i < size) {
		if (!src[i] && size == -1)
			break;
		if ((uint8)src[i] < 0x80 || (uint8)src[i] >= 0xC0)
			ret++;
		i++;
	}
	return ret;
}

int64 kh::_getUint32SizeInUtf8(const uint32* src, const int64 size) {
	int64 ret = 0, i = 0;

	while (size == -1 || i < size) {
		if (!src[i] && size == -1)
			break;

		/* Handle different UTF-8 sizes */
		if (src[i] < 0x80)
			ret += 1;
		else if (src[i] < 0x800)
			ret += 2;
		else if (src[i] < 0x10000)
			ret += 3;
		else
			ret += 4;

		i++;
	}
	return ret;
}

uint32* kh::_decodeFromUtf8(const uint8* csrc, const int64 size) {
	int64 i = 0, realsize = kh::_getRealSizeUtf8(csrc, size);
	uint32* ret, * runner;
	/* Because char can be signed, and that can mess with the logic */
	const uint8* src = csrc;

	if (!realsize)
		return NULL; /* Empty string */

	ret = (uint32*)malloc((realsize + 1) * sizeof(uint32)); /* Do ``new`` stuff here */
	if (!ret)
		return NULL; /* malloc error */

	runner = ret;
	while (size == -1 || i < size) {
		if (!src[i] && size == -1)
			break;

		if (src[i] < 0x80)
			*runner = (uint32)src[i];
		else if (src[i] < 0xC0) {
			free(ret);
			return NULL; /* Unicode Error */
		}
		else {
			if (src[i] < 0xE0)
				*runner = ((uint32)(src[i] & 0x1F)) << 6;
			else {
				if (src[i] < 0xF0)
					*runner = ((uint32)(src[i] & 0x0F)) << 12;
				else {
					*runner = ((uint32)(src[i] & 0x07)) << 18;
					_SAFE_INC_I;
					*runner |= ((uint32)(src[i] & 0x3F)) << 12;
				}
				_SAFE_INC_I;
				*runner |= ((uint32)(src[i] & 0x3F)) << 6;
			}
			_SAFE_INC_I;
			*runner |= (uint32)(src[i] & 0x3F);
		}
		i++;
		runner++;
	}
	*runner = 0; /* Make last byte NULL */
	return ret;
}

uint8* kh::_encodeToUTF8(const uint32* src, const int64 size) {
	int64 i = 0, realsize = _getUint32SizeInUtf8(src, size);
	uint8* ret, * runner;

	if (!realsize)
		return NULL; /* Got empty string */

	ret = (uint8*)malloc((realsize + 1) * sizeof(uint8)); /* Do ``new`` stuff here */
	if (!ret)
		return NULL; /* malloc error */

	runner = ret;
	while (size == -1 || i < size) {
		if (!src[i] && size == -1)
			break;

		if (src[i] < 0x80)
			*runner = (char)src[i];
		else {
			if (src[i] < 0x800)
				*runner = 0xC0 | ((src[i] & 0x7C0) >> 6);
			else {
				if (src[i] < 0x10000)
					*runner = 0xE0 | ((src[i] & 0xF000) >> 12);
				else {
					*runner = 0xF0 | ((src[i] & 0x1F0000) >> 18);
					runner++;
					*runner = 0x80 | ((src[i] & 0x3F000) >> 12);
				}
				runner++;
				*runner = 0x80 | ((src[i] & 0xFC0) >> 6);
			}
			runner++;
			*runner = 0x80 | (src[i] & 0x3F);
		}
		runner++;
		i++;
	}
	*runner = 0; /* Make last byte NULL */
	return ret;
}
