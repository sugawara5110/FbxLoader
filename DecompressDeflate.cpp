//*****************************************************************************************//
//**                                                                                     **//
//**                   �@�@�@    DecompressDeflate                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#include "DecompressDeflate.h"

const UINT16 DecompressDeflate::dest[] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577 };

const UCHAR DecompressDeflate::NumBit[] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,
7,7,8,8,9,9,10,10,11,11,12,12,13,13 };

void DecompressDeflate::bitInversion(UCHAR *ba, UINT size) {
	for (UINT i = 0; i < size; i++) {
		ba[i] = ((ba[i] & 0x55) << 1) | ((ba[i] & 0xAA) >> 1);//0x55:01010101, 0xAA:10101010
		ba[i] = ((ba[i] & 0x33) << 2) | ((ba[i] & 0xCC) >> 2);//0x33:00110011, 0xCC:11001100
		ba[i] = (ba[i] << 4) | (ba[i] >> 4);
	}
}

void DecompressDeflate::getLength(UINT16 val, UINT16 *len, UCHAR *bitlen) {
	*len = 0;
	*bitlen = 0;
	if (257 <= val && val <= 264) {
		*len = val - 254;
	}
	if (265 <= val && val <= 268) {
		*len = (val - 265) * 2 + 11;
		*bitlen = 1;
	}
	if (269 <= val && val <= 272) {
		*len = (val - 269) * 4 + 19;
		*bitlen = 2;
	}
	if (273 <= val && val <= 276) {
		*len = (val - 273) * 8 + 35;
		*bitlen = 3;
	}
	if (277 <= val && val <= 280) {
		*len = (val - 277) * 16 + 67;
		*bitlen = 4;
	}
	if (281 <= val && val <= 284) {
		*len = (val - 281) * 32 + 131;
		*bitlen = 5;
	}
	if (285 == val) {
		*len = 258;
		*bitlen = 0;
	}
}

void DecompressDeflate::getDestLength(UINT16 val, UINT16 *len, UCHAR *bitlen) {
	*len = dest[val];
	*bitlen = NumBit[val];
}

void DecompressDeflate::DecompressLZSS(UCHAR *outArray, UINT *outIndex, UINT16 MatchLen, UINT16 destLen) {
	UINT srcInd = *outIndex - destLen;
	UCHAR *src = &outArray[srcInd];
	UCHAR *dst = &outArray[*outIndex];
	//��v��MatchLen > ��������̋���destLen�̏ꍇ��v���ɒB����܂œ����Ƃ����J��Ԃ�
	for (UINT16 i = 0; i < MatchLen; i++) {
		UINT sind = i % destLen;
		dst[i] = src[sind];
	}
	*outIndex += MatchLen;
}

void DecompressDeflate::getBit(UINT64 *curSearchBit, UCHAR *byteArray, UCHAR NumBit, UINT16 *outBinArr, bool firstRight) {
	for (UCHAR i = 0; i < NumBit; i++) {
		UINT baind = *curSearchBit / byteArrayNumbit;//�z��C���f�b�N�X
		UINT searBit = *curSearchBit % byteArrayNumbit;//�v�f��bit�ʒu�C���f�b�N�X
		UCHAR NumShift = byteArrayNumbit - 1 - searBit;
		UCHAR popbit = (byteArray[baind] >> NumShift) & 0x01;//�ړIbit���o��, bit�ʒu�ŉE
		UCHAR NumShift16 = NumBit - 1 - i;//�����p������l�߂�
		if (firstRight)NumShift16 = i;//�E����l�߂�
		UINT16 posbit16 = (UINT16(popbit) << NumShift16);
		(*outBinArr) |= posbit16; //bit�ǉ�
		(*curSearchBit)++;
	}
}

void DecompressDeflate::createFixedHuffmanSign() {
	for (UINT i = 0; i < 286; i++) {
		if (0 <= i && i <= 143) {
			strSign[i] = i + 48;
			strNumSign[i] = 8;
		}
		if (144 <= i && i <= 255) {
			strSign[i] = i + 256;
			strNumSign[i] = 9;
		}
		if (256 <= i && i <= 279) {
			strSign[i] = i - 256;
			strNumSign[i] = 7;
		}
		if (280 <= i && i <= 287) {
			strSign[i] = i - 88;
			strNumSign[i] = 8;
		}
	}
	for (UINT i = 0; i < 30; i++) {
		lenSign[i] = i;
		lenNumSign[i] = 5;
	}
}

void DecompressDeflate::SortIndex(UINT16 *sortedIndex, UCHAR *hclens, UINT size) {
	UINT topSize = size * 0.5f;
	UINT halfSize = size - topSize;
	UINT16 *topSortedIndex = new UINT16[topSize];
	UINT16 *halfSortedIndex = new UINT16[halfSize];
	UCHAR *tophclens = new UCHAR[topSize];
	UCHAR *halfhclens = new UCHAR[halfSize];
	memcpy(topSortedIndex, sortedIndex, topSize * sizeof(UINT16));
	memcpy(halfSortedIndex, &sortedIndex[topSize], halfSize * sizeof(UINT16));
	memcpy(tophclens, hclens, topSize * sizeof(UCHAR));
	memcpy(halfhclens, &hclens[topSize], halfSize * sizeof(UCHAR));

	if (topSize > 1)SortIndex(topSortedIndex, tophclens, topSize);
	if (halfSize > 1)SortIndex(halfSortedIndex, halfhclens, halfSize);

	UINT topIndex = 0;
	UINT halfIndex = 0;
	UINT iInd = 0;
	for (UINT i = 0; i < size; i++) {
		if (tophclens[topIndex] <= halfhclens[halfIndex]) {
			hclens[i] = tophclens[topIndex];
			sortedIndex[i] = topSortedIndex[topIndex];
			topIndex++;
			if (topSize <= topIndex) {
				iInd = i + 1;
				break;
			}
		}
		else {
			hclens[i] = halfhclens[halfIndex];
			sortedIndex[i] = halfSortedIndex[halfIndex];
			halfIndex++;
			if (halfSize <= halfIndex) {
				iInd = i + 1;
				break;
			}
		}
	}
	if (topSize > topIndex) {
		for (UINT i = iInd; i < size; i++) {
			hclens[i] = tophclens[topIndex];
			sortedIndex[i] = topSortedIndex[topIndex];
			topIndex++;
		}
	}
	if (halfSize > halfIndex) {
		for (UINT i = iInd; i < size; i++) {
			hclens[i] = halfhclens[halfIndex];
			sortedIndex[i] = halfSortedIndex[halfIndex];
			halfIndex++;
		}
	}
	a_DELETE(topSortedIndex);
	a_DELETE(halfSortedIndex);
	a_DELETE(tophclens);
	a_DELETE(halfhclens);
}

void DecompressDeflate::CreateSign(UINT16 *clens, UCHAR *hclens, UINT16 *SortedIndex, UINT size) {
	UINT firstIndex = 0;
	while (hclens[SortedIndex[firstIndex++]] == 0);
	firstIndex--;
	UINT16 clensVal = 0;
	UCHAR NumBit = hclens[SortedIndex[firstIndex]];
	for (UINT i = firstIndex; i < size; i++) {
		clensVal = clensVal << (hclens[SortedIndex[i]] - NumBit);
		clens[SortedIndex[i]] = clensVal++;
		NumBit = hclens[SortedIndex[i]];
	}
}

void DecompressDeflate::createCustomHuffmanSign(UINT64 *curSearchBit, UCHAR *byteArray) {
	UINT16 HLIT = 0;//����/��v�������̌�5bit
	UINT16 HDIST = 0;//���������̌�5bit
	UINT16 HCLEN = 0;//�������\�̕������\�̃T�C�Y4bit
	UCHAR MaxBit[3] = { 5,5,4 };
	UINT16 tmp[3] = { 0 };
	//HLIT�AHDIST�AHCLEN�A�������\�̕������\�A�g���r�b�g�͒l�Ȃ̂ŉE�l��,
	//�����͍��l��
	for (UINT i = 0; i < 3; i++) {
		getBit(curSearchBit, byteArray, MaxBit[i], &tmp[i], true);
	}
	HLIT = tmp[0];
	HDIST = tmp[1];
	HCLEN = tmp[2];
	//(HCLEN + 4) * 3 �̃r�b�g�ǂݍ���
	const UCHAR NumSign = 19;
	//�������\�̕������\�쐬
	UCHAR hclens[NumSign] = { 0 };//�������z��
	UINT16 clens[NumSign] = { 0 };//�����z��
	//16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15�̏��Ԃŗv�f�f�[�^������ł�,��������\���Ă���
	const UCHAR SignInd[NumSign] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
	UCHAR numroop = HCLEN + 4;
	//�ǂݍ��񂾏��ɏ�L�̃C���f�b�N�X���ɔz��Ɋi�[, ��L�̕������̕����̒�����\���Ă���
	for (UCHAR i = 0; i < numroop; i++) {
		UINT16 sl = 0;
		getBit(curSearchBit, byteArray, 3, &sl, true);//����������ǂݍ���
		hclens[SignInd[i]] = (UCHAR)sl;
	}
	//�����������������ɕ��ׂ�, (0�ȊO)
	UINT16 SortedIndex[NumSign] = { 0 };//�\�[�g��̃C���f�b�N�X�z��
	UCHAR hclensCopy[NumSign] = { 0 };
	memcpy(hclensCopy, hclens, NumSign * sizeof(UCHAR));
	for (int i = 0; i < NumSign; i++)SortedIndex[i] = i;//�\�[�g�C���f�b�N�X������
	//�C���f�b�N�X�\�[�g
	//��������������O�ɕ������̒Z�����ɐ��񂷂�ׂ̃C���f�b�N�X�z��𐶐�
	//�����̒Z���������珇�ɘA�Ԃŕ�����t�^���� 
	//�Z���������璷�������ւ͒Z�����������������̏�ʁw�Z�������̃r�b�g���x�r�b�g�����Z�������̎��̔ԍ�+�c��̃r�b�g��0
	//��: 000 �� 001 �� 01000 �� 01001
	SortIndex(SortedIndex, hclensCopy, NumSign);

	//����/��v���������\,�����������\�����ׂ̈̕����\����
	CreateSign(clens, hclens, SortedIndex, NumSign);

	//����/��v���������\,�����������\����
	UINT16 strSigLen = HLIT + 257;
	UINT16 destSigLen = HDIST + 1;
	UCHAR *sigLenList = new UCHAR[strSigLen + destSigLen];
	UINT16 firstIndex = 0;
	while (hclens[SortedIndex[firstIndex++]] == 0);
	firstIndex--;
	UINT16 prevBit = 0;
	UINT sigLenInd = 0;
	while (sigLenInd < strSigLen + destSigLen) {
		for (UINT i1 = firstIndex; i1 < NumSign; i1++) {//bit�T��
			UINT16 outBin = 0;
			getBit(curSearchBit, byteArray, hclens[SortedIndex[i1]], &outBin, false);//������ǂݍ���
			if (clens[SortedIndex[i1]] == outBin) {//��v�����ꍇ
				if (SortedIndex[i1] == 16) {
					//���O�Ɏ��o���ꂽ��������3~6��J��Ԃ��B16�̌��2bit 00=3�� 11=6��J��Ԃ�
					UINT16 obit = 0;
					getBit(curSearchBit, byteArray, 2, &obit, true);//�����������o�������ł͂Ȃ���,�ʏ폇��
					for (UINT16 i16 = 0; i16 < obit + 3; i16++)sigLenList[sigLenInd++] = prevBit;
					break;
				}
				if (SortedIndex[i1] == 17) {
					//0������3~10��J��Ԃ�17�̌�3bit
					UINT16 obit = 0;
					getBit(curSearchBit, byteArray, 3, &obit, true);
					for (UINT16 i17 = 0; i17 < obit + 3; i17++)sigLenList[sigLenInd++] = 0;
					break;
				}
				if (SortedIndex[i1] == 18) {
					//0������11~138��J��Ԃ�18�̌�7bit
					UINT16 obit = 0;
					getBit(curSearchBit, byteArray, 7, &obit, true);
					for (UINT16 i18 = 0; i18 < obit + 11; i18++)sigLenList[sigLenInd++] = 0;
					break;
				}
				//�������̏ꍇ
				sigLenList[sigLenInd++] = SortedIndex[i1];
				prevBit = SortedIndex[i1];
				break;
			}
			(*curSearchBit) -= hclens[SortedIndex[i1]];//��v���Ȃ������ꍇ�ǂݍ��񂾕���bit�ʒu��߂�
		}
	}

	//����/��v���������\,�����������\���炻�ꂼ��̕����\�𐶐�����
	//����/��v���������\,�����������\�ɕ�������
	UCHAR *strSigLenList = sigLenList;
	UCHAR *destSigLenList = &sigLenList[strSigLen];

	UCHAR *strSigLenListCopy = new UCHAR[strSigLen];
	UCHAR *destSigLenListCopy = new UCHAR[destSigLen];
	memcpy(strSigLenListCopy, strSigLenList, strSigLen * sizeof(UCHAR));
	memcpy(destSigLenListCopy, destSigLenList, destSigLen * sizeof(UCHAR));

	UINT16 *strSigLenListSortedIndex = new UINT16[strSigLen];
	UINT16 *destSigLenListSortedIndex = new UINT16[destSigLen];
	UINT16 *strSigList = new UINT16[strSigLen];
	UINT16 *destSigList = new UINT16[destSigLen];
	//������
	for (UINT16 i = 0; i < strSigLen; i++)strSigLenListSortedIndex[i] = i;
	for (UINT16 i = 0; i < destSigLen; i++)destSigLenListSortedIndex[i] = i;
	//���������O�ɏ��������ԂɃ\�[�g
	SortIndex(strSigLenListSortedIndex, strSigLenListCopy, strSigLen);
	SortIndex(destSigLenListSortedIndex, destSigLenListCopy, destSigLen);
	//����/��v�������\,���������\����
	CreateSign(strSigList, strSigLenList, strSigLenListSortedIndex, strSigLen);
	CreateSign(destSigList, destSigLenList, destSigLenListSortedIndex, destSigLen);

	memcpy(strSign, strSigList, strSigLen * sizeof(UINT16));
	memcpy(strNumSign, strSigLenList, strSigLen * sizeof(UCHAR));
	memcpy(lenSign, destSigList, destSigLen * sizeof(UINT16));
	memcpy(lenNumSign, destSigLenList, destSigLen * sizeof(UCHAR));

	a_DELETE(sigLenList);
	a_DELETE(strSigLenListSortedIndex);
	a_DELETE(destSigLenListSortedIndex);
	a_DELETE(strSigList);
	a_DELETE(destSigList);
	a_DELETE(strSigLenListCopy);
	a_DELETE(destSigLenListCopy);
}

void DecompressDeflate::DecompressHuffman(UINT64 *curSearchBit, UCHAR *byteArray, UINT *outIndex, UCHAR *outArray) {
	//�����͈͒T��
	bool roop = true;
	while (roop) {
		for (UINT val = 0; val < 286; val++) {
			UINT16 outBin = 0;
			getBit(curSearchBit, byteArray, strNumSign[val], &outBin, false);//������ǂݍ���,�����Ȃ̂Ńr�b�O�G���f�B�A��
			if (strNumSign[val] > 0 && strSign[val] == outBin) {//��v�����ꍇ
				if (val <= 255) {
					//0~255�̒l�͂��̂܂ܒu��������outArray���
					outArray[(*outIndex)++] = val;//�u�������ŏo��(1byte)
					break;
				}
				if (val == 256) {
					roop = false;
					break;
				}
				if (256 < val) {
					//������v���擾//257~264�͊g���r�b�g����
					UCHAR bitlen = 0;
					UINT16 MatchLen = 0; //���o������v��
					getLength(val, &MatchLen, &bitlen);
					UINT16 outExpansionBit = 0;
					getBit(curSearchBit, byteArray, bitlen, &outExpansionBit, true);//�g���r�b�g�ǂݍ���,���l�Ȃ̂Ń��g���G���f�B�A��
					MatchLen += outExpansionBit;//�g���r�b�g�L�����ꍇ, ��v���ɑ���
					//�����l����
					for (UINT destVal = 0; destVal < 30; destVal++) {
						UINT16 lenBin = 0;
						getBit(curSearchBit, byteArray, lenNumSign[destVal], &lenBin, false);
						if (lenNumSign[destVal] > 0 && lenSign[destVal] == lenBin) {
							UINT16 destLen = 0;
							UCHAR destbitlen = 0;
							getDestLength(destVal, &destLen, &destbitlen);
							UINT16 outExpansionlenBit = 0;
							getBit(curSearchBit, byteArray, destbitlen, &outExpansionlenBit, true);
							destLen += outExpansionlenBit;
							//���o������v��, ��������l��ǂݏo��
							DecompressLZSS(outArray, outIndex, MatchLen, destLen);
							break;
						}
						(*curSearchBit) -= lenNumSign[destVal];//��v���Ȃ������ꍇ�ǂݍ��񂾕���bit�ʒu��߂�
					}
					break;
				}
			}
			(*curSearchBit) -= strNumSign[val];//��v���Ȃ������ꍇ�ǂݍ��񂾕���bit�ʒu��߂�
		}
	}
}

UINT16 DecompressDeflate::blockFinal(UINT64 *curSearchBit, UCHAR *byteArray) {
	UINT16 bf = 0;
	getBit(curSearchBit, byteArray, 1, &bf, true);
	return bf;
}

UINT16 DecompressDeflate::blockType(UINT64 *curSearchBit, UCHAR *byteArray) {
	UINT16 bt = 0;
	getBit(curSearchBit, byteArray, 2, &bt, true);
	return bt;
}

void DecompressDeflate::getDecompressArray(UCHAR *bA, UINT size, UCHAR *outArray) {
	UINT64 curSearchBit = 0;//��bit�ʒu
	UINT outIndex = 0;//outArray��BYTE�P�ʂŏ�������
	bitInversion(bA, size);
	bool roop = true;
	while (roop) {
		if (blockFinal(&curSearchBit, bA) == 1)roop = false;
		UINT16 sw = blockType(&curSearchBit, bA);
		switch (sw) {
		case 0:
			MessageBoxA(0, "�񈳏k�͑Ή����Ă܂���", 0, MB_OK);
			break;
		case 1:
			createFixedHuffmanSign();
			break;
		case 2:
			createCustomHuffmanSign(&curSearchBit, bA);
			break;
		case 3:
			MessageBoxA(0, "�u���b�N�^�C�v�G���[", 0, MB_OK);
			break;
		}
		DecompressHuffman(&curSearchBit, bA, &outIndex, outArray);
	}
}