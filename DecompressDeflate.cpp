//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　    DecompressDeflate                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#include "DecompressDeflate.h"
#include <string.h>

static const int bitMask[]{
	0b0000000000000000,
	0b0000000000000001,
	0b0000000000000011,
	0b0000000000000111,
	0b0000000000001111,
	0b0000000000011111,
	0b0000000000111111,
	0b0000000001111111,
	0b0000000011111111,
	0b0000000111111111,
	0b0000001111111111,
	0b0000011111111111,
	0b0000111111111111,
	0b0001111111111111,
	0b0011111111111111,
	0b0111111111111111,
	0b1111111111111111
};

static const int byteArrayNumbit = 8;
static const int LEFT_INV_BIT1 = 0b0101010101010101;
static const int RIGHT_INV_BIT1 = 0b1010101010101010;
static const int LEFT_INV_BIT2 = 0b0011001100110011;
static const int RIGHT_INV_BIT2 = 0b1100110011001100;
static const int LEFT_INV_BIT4 = 0b0000111100001111;
static const int RIGHT_INV_BIT4 = 0b1111000011110000;

static unsigned short intInversion(unsigned short ba, int numBit) {
	unsigned short oBa = 0;
	const int baseNum = 16;
	oBa = ((ba & LEFT_INV_BIT1) << 1) | ((ba & RIGHT_INV_BIT1) >> 1);
	oBa = ((oBa & LEFT_INV_BIT2) << 2) | ((oBa & RIGHT_INV_BIT2) >> 2);
	oBa = ((oBa & LEFT_INV_BIT4) << 4) | ((oBa & RIGHT_INV_BIT4) >> 4);
	return ((oBa << 8) | (oBa >> 8)) >> (baseNum - numBit);
}

static void bitInversion(unsigned char* ba, unsigned int size) {
	for (unsigned int i = 0; i < size; i++) {
		ba[i] = (unsigned char)intInversion((unsigned short)ba[i], 8);
	}
}

static void getBit(unsigned long long* CurSearchBit, unsigned char* byteArray, unsigned char NumBit, unsigned short* outBinArr, bool firstRight) {
	unsigned int baind = (unsigned int)(*CurSearchBit / byteArrayNumbit);//配列インデックス
	unsigned int bitPos = (unsigned int)(*CurSearchBit % byteArrayNumbit);//要素内bit位置インデックス
	unsigned char shiftBit = byteArrayNumbit * 3 - NumBit - bitPos;
	*outBinArr = (((byteArray[baind] << 16) | (byteArray[baind + 1] << 8) |
		byteArray[baind + 2]) >> shiftBit) & bitMask[NumBit];
	if (firstRight) * outBinArr = intInversion(*outBinArr, NumBit);
	*CurSearchBit += NumBit;
}

void HuffmanNode::setVal(unsigned short val, unsigned short bitArr, unsigned char numBit) {
	if (numBit == 0) {//葉まで到達したら値を格納
		Val = val;
		return;
	}
	if (bitArr >> (numBit - 1) == 0) {
		if (!bit0)bit0 = new HuffmanNode();
		bit0->setVal(val, bitArr & bitMask[numBit - 1], numBit - 1);
	}
	else {
		if (!bit1)bit1 = new HuffmanNode();
		bit1->setVal(val, bitArr & bitMask[numBit - 1], numBit - 1);
	}
}

unsigned short HuffmanNode::getVal(unsigned long long* curSearchBit, unsigned char* byteArray) {
	if (!bit0 && !bit1) {//葉まで到達したら値を返す
		return Val;
	}
	unsigned short outBin = 0;
	getBit(curSearchBit, byteArray, 1, &outBin, false);
	if (outBin == 0) {
		return bit0->getVal(curSearchBit, byteArray);
	}
	else {
		return bit1->getVal(curSearchBit, byteArray);
	}
}

HuffmanNode::~HuffmanNode() {
	s_DELETE(bit0);
	s_DELETE(bit1);
}

void HuffmanTree::createTree(unsigned short* signArr, unsigned char* numSignArr, unsigned int arraySize) {
	for (unsigned int i = 0; i < arraySize; i++) {
		if (numSignArr[i] == 0)continue;//符号長0(符号無し)はスキップ
		hn.setVal(i, signArr[i], numSignArr[i]);
	}
}

unsigned short HuffmanTree::getVal(unsigned long long* curSearchBit, unsigned char* byteArray) {
	return hn.getVal(curSearchBit, byteArray);
}

const unsigned short DecompressDeflate::dest[] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577 };

const unsigned char DecompressDeflate::NumBit[] = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,
7,7,8,8,9,9,10,10,11,11,12,12,13,13 };

void DecompressDeflate::getLength(unsigned short val, unsigned short *len, unsigned char *bitlen) {
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

void DecompressDeflate::getDestLength(unsigned short val, unsigned short *len, unsigned char *bitlen) {
	*len = dest[val];
	*bitlen = NumBit[val];
}

void DecompressDeflate::DecompressLZSS(unsigned char *outArray, unsigned int *outIndex, unsigned short MatchLen, unsigned short destLen) {
	unsigned int srcInd = *outIndex - destLen;
	unsigned char *src = &outArray[srcInd];
	unsigned char *dst = &outArray[*outIndex];
	//一致長MatchLen > 自分からの距離destLenの場合一致長に達するまで同じとこを繰り返す
	for (unsigned short i = 0; i < MatchLen; i++) {
		unsigned int sind = i % destLen;
		dst[i] = src[sind];
	}
	*outIndex += MatchLen;
}

void DecompressDeflate::createFixedHuffmanSign() {
	for (unsigned int i = 0; i < NUMSTR; i++) {
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
	strTree = new HuffmanTree();
	strTree->createTree(strSign, strNumSign, NUMSTR);

	for (unsigned int i = 0; i < NUMLEN; i++) {
		lenSign[i] = i;
		lenNumSign[i] = 5;
	}
	lenTree = new HuffmanTree();
	lenTree->createTree(lenSign, lenNumSign, NUMLEN);
}

void DecompressDeflate::SortIndex(unsigned short *sortedIndex, unsigned char *hclens, unsigned int size) {
	unsigned int topSize = (unsigned int)(size * 0.5);
	unsigned int halfSize = size - topSize;
	unsigned short *topSortedIndex = new unsigned short[topSize];
	unsigned short *halfSortedIndex = new unsigned short[halfSize];
	unsigned char *tophclens = new unsigned char[topSize];
	unsigned char *halfhclens = new unsigned char[halfSize];
	memcpy(topSortedIndex, sortedIndex, topSize * sizeof(unsigned short));
	memcpy(halfSortedIndex, &sortedIndex[topSize], halfSize * sizeof(unsigned short));
	memcpy(tophclens, hclens, topSize * sizeof(unsigned char));
	memcpy(halfhclens, &hclens[topSize], halfSize * sizeof(unsigned char));

	if (topSize > 1)SortIndex(topSortedIndex, tophclens, topSize);
	if (halfSize > 1)SortIndex(halfSortedIndex, halfhclens, halfSize);

	unsigned int topIndex = 0;
	unsigned int halfIndex = 0;
	unsigned int iInd = 0;
	for (unsigned int i = 0; i < size; i++) {
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
		for (unsigned int i = iInd; i < size; i++) {
			hclens[i] = tophclens[topIndex];
			sortedIndex[i] = topSortedIndex[topIndex];
			topIndex++;
		}
	}
	if (halfSize > halfIndex) {
		for (unsigned int i = iInd; i < size; i++) {
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

void DecompressDeflate::CreateSign(unsigned short *clens, unsigned char *hclens, unsigned short *SortedIndex, unsigned int size) {
	unsigned int firstIndex = 0;
	while (hclens[SortedIndex[firstIndex++]] == 0);
	firstIndex--;
	unsigned short clensVal = 0;
	unsigned char NumBit = hclens[SortedIndex[firstIndex]];
	for (unsigned int i = firstIndex; i < size; i++) {
		clensVal = clensVal << (hclens[SortedIndex[i]] - NumBit);
		clens[SortedIndex[i]] = clensVal++;
		NumBit = hclens[SortedIndex[i]];
	}
}

void DecompressDeflate::createCustomHuffmanSign(unsigned long long* curSearchBit, unsigned char* byteArray) {
	unsigned short HLIT = 0;//文字/一致長符号の個数5bit
	unsigned short HDIST = 0;//距離符号の個数5bit
	unsigned short HCLEN = 0;//符号長表の符号長表のサイズ4bit
	unsigned char MaxBit[3] = { 5,5,4 };
	unsigned short tmp[3] = {};
	//HLIT、HDIST、HCLEN、符号長表の符号長表、拡張ビットは値なので右詰め,
	//符号は左詰め
	for (unsigned int i = 0; i < 3; i++) {
		getBit(curSearchBit, byteArray, MaxBit[i], &tmp[i], true);
	}
	HLIT = tmp[0];
	HDIST = tmp[1];
	HCLEN = tmp[2];
	//(HCLEN + 4) * 3 のビット読み込み
	const unsigned char NumSign = 19;
	//符号長表の符号長表作成
	unsigned char hclens[NumSign] = { 0 };//符号長配列
	unsigned short clens[NumSign] = { 0 };//符号配列
	//16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15の順番で要素データが並んでる,符号長を表している
	const unsigned char SignInd[NumSign] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
	unsigned char numroop = HCLEN + 4;
	//読み込んだ順に上記のインデックス順に配列に格納, 上記の符号長の符号の長さを表している
	for (unsigned char i = 0; i < numroop; i++) {
		unsigned short sl = 0;
		getBit(curSearchBit, byteArray, 3, &sl, true);//符号長数を読み込む
		hclens[SignInd[i]] = (unsigned char)sl;
	}
	//符号長が小さい順に並べる, (0個以外)
	unsigned short SortedIndex[NumSign] = {};//ソート後のインデックス配列
	unsigned char hclensCopy[NumSign] = {};
	memcpy(hclensCopy, hclens, NumSign * sizeof(unsigned char));
	for (int i = 0; i < NumSign; i++)SortedIndex[i] = i;//ソートインデックス初期化
	//インデックスソート
	//符号生成をする前に符号長の短い順に整列する為のインデックス配列を生成
	//長さの短い符号から順に連番で符号を付与する 
	//短い符号から長い符号へは短い符号→長い符号の上位『短い符号のビット数』ビットだけ短い符号の次の番号+残りのビットは0
	//例: 000 → 001 → 01000 → 01001
	SortIndex(SortedIndex, hclensCopy, NumSign);

	//文字/一致長符号長表,距離符号長表生成の為の符号表生成
	CreateSign(clens, hclens, SortedIndex, NumSign);
	HuffmanTree clensTree;
	clensTree.createTree(clens, hclens, NumSign);

	//文字/一致長符号長表,距離符号長表生成
	unsigned short strSigLen = HLIT + 257;
	unsigned short destSigLen = HDIST + 1;
	unsigned char* sigLenList = new unsigned char[strSigLen + destSigLen];
	unsigned short prevBit = 0;
	unsigned int sigLenInd = 0;
	while (sigLenInd < (unsigned int)(strSigLen + destSigLen)) {
		unsigned short val = clensTree.getVal(curSearchBit, byteArray);
		if (val == 16) {
			//直前に取り出された符号長を3~6回繰り返す。16の後の2bit 00=3回 11=6回繰り返す
			unsigned short obit = 0;
			getBit(curSearchBit, byteArray, 2, &obit, true);//反復長を取り出す符号ではない為,通常順序
			for (unsigned short i16 = 0; i16 < obit + 3; i16++)sigLenList[sigLenInd++] = (unsigned char)prevBit;
		}
		if (val == 17) {
			//0符号を3~10回繰り返す17の後3bit
			unsigned short obit = 0;
			getBit(curSearchBit, byteArray, 3, &obit, true);
			for (unsigned short i17 = 0; i17 < obit + 3; i17++)sigLenList[sigLenInd++] = 0;
		}
		if (val == 18) {
			//0符号を11~138回繰り返す18の後7bit
			unsigned short obit = 0;
			getBit(curSearchBit, byteArray, 7, &obit, true);
			for (unsigned short i18 = 0; i18 < obit + 11; i18++)sigLenList[sigLenInd++] = 0;
		}
		if (val < 16) {
			//符号長の場合
			sigLenList[sigLenInd++] = (unsigned char)val;
			prevBit = val;
		}
	}

	//文字/一致長符号長表,距離符号長表からそれぞれの符号表を生成する
	//文字/一致長符号長表,距離符号長表に分割する
	unsigned char* strSigLenList = sigLenList;
	unsigned char* destSigLenList = &sigLenList[strSigLen];

	unsigned char* strSigLenListCopy = new unsigned char[strSigLen];
	unsigned char* destSigLenListCopy = new unsigned char[destSigLen];
	memcpy(strSigLenListCopy, strSigLenList, strSigLen * sizeof(unsigned char));
	memcpy(destSigLenListCopy, destSigLenList, destSigLen * sizeof(unsigned char));

	unsigned short* strSigLenListSortedIndex = new unsigned short[strSigLen];
	unsigned short* destSigLenListSortedIndex = new unsigned short[destSigLen];
	unsigned short* strSigList = new unsigned short[strSigLen];
	unsigned short* destSigList = new unsigned short[destSigLen];
	//初期化
	for (unsigned short i = 0; i < strSigLen; i++)strSigLenListSortedIndex[i] = i;
	for (unsigned short i = 0; i < destSigLen; i++)destSigLenListSortedIndex[i] = i;
	//符号生成前に小さい順番にソート
	SortIndex(strSigLenListSortedIndex, strSigLenListCopy, strSigLen);
	SortIndex(destSigLenListSortedIndex, destSigLenListCopy, destSigLen);
	//文字/一致長符号表,距離符号表生成
	CreateSign(strSigList, strSigLenList, strSigLenListSortedIndex, strSigLen);
	CreateSign(destSigList, destSigLenList, destSigLenListSortedIndex, destSigLen);

	memcpy(strSign, strSigList, strSigLen * sizeof(unsigned short));
	memcpy(strNumSign, strSigLenList, strSigLen * sizeof(unsigned char));
	memcpy(lenSign, destSigList, destSigLen * sizeof(unsigned short));
	memcpy(lenNumSign, destSigLenList, destSigLen * sizeof(unsigned char));

	strTree = new HuffmanTree();
	strTree->createTree(strSign, strNumSign, strSigLen);
	lenTree = new HuffmanTree();
	lenTree->createTree(lenSign, lenNumSign, destSigLen);

	a_DELETE(sigLenList);
	a_DELETE(strSigLenListSortedIndex);
	a_DELETE(destSigLenListSortedIndex);
	a_DELETE(strSigList);
	a_DELETE(destSigList);
	a_DELETE(strSigLenListCopy);
	a_DELETE(destSigLenListCopy);
}

void DecompressDeflate::DecompressHuffman(unsigned long long* curSearchBit, unsigned char* byteArray, unsigned int* outIndex, unsigned char* outArray) {
	//符号範囲探索
	bool roop = true;
	while (roop) {
		unsigned int val = strTree->getVal(curSearchBit, byteArray);
		if (val <= 255) {
			//0~255の値はそのまま置換無しでoutArray代入
			outArray[(*outIndex)++] = val;//置換無しで出力(1byte)
		}
		if (val == 256) {
			roop = false;
		}
		if (256 < val) {
			//文字一致長取得//257~264は拡張ビット無し
			unsigned char bitlen = 0;
			unsigned short MatchLen = 0; //取り出した一致長
			getLength(val, &MatchLen, &bitlen);
			unsigned short outExpansionBit = 0;
			getBit(curSearchBit, byteArray, bitlen, &outExpansionBit, true);//拡張ビット読み込み,数値なのでリトルエンディアン
			MatchLen += outExpansionBit;//拡張ビット有った場合, 一致長に足す
			//距離値処理
			unsigned int destVal = lenTree->getVal(curSearchBit, byteArray);
			unsigned short destLen = 0;
			unsigned char destbitlen = 0;
			getDestLength(destVal, &destLen, &destbitlen);
			unsigned short outExpansionlenBit = 0;
			getBit(curSearchBit, byteArray, destbitlen, &outExpansionlenBit, true);
			destLen += outExpansionlenBit;
			//取り出した一致長, 距離から値を読み出す
			DecompressLZSS(outArray, outIndex, MatchLen, destLen);
		}
	}
	s_DELETE(strTree);
	s_DELETE(lenTree);
}

void DecompressDeflate::Uncompress(unsigned long long *curSearchBit, unsigned char *byteArray, unsigned int *outIndex, unsigned char *outArray) {
	while ((*curSearchBit) % byteArrayNumbit != 0) { (*curSearchBit)++; }//次のbyte境界までdata無しなので飛ばす
	unsigned short LEN = 0;//2byte NLENの後から続くdataのbyte数
	getBit(curSearchBit, byteArray, 16, &LEN, true);
	unsigned short NLEN = 0;//2byte LENの1の補数
	getBit(curSearchBit, byteArray, 16, &NLEN, true);
	//ここからデータ
	for (unsigned short i = 0; i < LEN; i++) {
		unsigned short val = 0;
		getBit(curSearchBit, byteArray, 8, &val, true);
		outArray[(*outIndex)++] = (unsigned char)val;
	}
}

unsigned short DecompressDeflate::blockFinal(unsigned long long *curSearchBit, unsigned char *byteArray) {
	unsigned short bf = 0;
	getBit(curSearchBit, byteArray, 1, &bf, true);
	return bf;
}

unsigned short DecompressDeflate::blockType(unsigned long long *curSearchBit, unsigned char *byteArray) {
	unsigned short bt = 0;
	getBit(curSearchBit, byteArray, 2, &bt, true);
	return bt;
}

bool DecompressDeflate::getDecompressArray(unsigned char *bA, unsigned int size, unsigned char *outArray) {
	unsigned long long curSearchBit = 0;//現bit位置
	unsigned int outIndex = 0;//outArrayはBYTE単位で書き込み
	bitInversion(bA, size);
	bool roop = true;
	while (roop) {
		if (blockFinal(&curSearchBit, bA) == 1)roop = false;
		unsigned short sw = blockType(&curSearchBit, bA);
		switch (sw) {
		case 0:
			Uncompress(&curSearchBit, bA, &outIndex, outArray);
			break;
		case 1:
			createFixedHuffmanSign();
			break;
		case 2:
			createCustomHuffmanSign(&curSearchBit, bA);
			break;
		case 3:
			return false;
		}
		if (sw == 1 || sw == 2)
			DecompressHuffman(&curSearchBit, bA, &outIndex, outArray);
	}
	return true;
}