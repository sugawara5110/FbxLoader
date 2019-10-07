//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　    DecompressDeflate                                       **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_DecompressDeflate_Header
#define Class_DecompressDeflate_Header

#define s_DELETE(p) if(p){delete p;    p=nullptr;}
#define a_DELETE(p) if(p){delete[] p;    p=nullptr;}
#define NUMSTR 286
#define NUMLEN 30

class DecompressDeflate;
class HuffmanTree;

class HuffmanNode {

private:
	friend HuffmanTree;
	HuffmanNode* bit0 = nullptr;
	HuffmanNode* bit1 = nullptr;
	unsigned short Val = 0;

	void setVal(unsigned short val, unsigned short bitArr, unsigned char numBit);
	unsigned short getVal(unsigned long long* curSearchBit, unsigned char* byteArray);
	~HuffmanNode();
};

class HuffmanTree {

private:
	friend DecompressDeflate;
	HuffmanNode hn;

	void createTree(unsigned short* signArr, unsigned char* numSignArr, unsigned int arraySize);
	unsigned short getVal(unsigned long long* curSearchBit, unsigned char* byteArray);
};

class DecompressDeflate {

private:
	static const unsigned short dest[];
	static const unsigned char NumBit[];
	unsigned short strSign[NUMSTR] = {};
	unsigned char strNumSign[NUMSTR] = {};
	unsigned short lenSign[NUMLEN] = {};
	unsigned char lenNumSign[NUMLEN] = {};
	HuffmanTree* strTree = nullptr;
	HuffmanTree* lenTree = nullptr;

	void getLength(unsigned short DecryptionVal, unsigned short* len, unsigned char* bitlen);
	void getDestLength(unsigned short Val, unsigned short* len, unsigned char* bitlen);
	void DecompressLZSS(unsigned char* outArray, unsigned int* outIndex, unsigned short MatchLen, unsigned short destLen);
	void createFixedHuffmanSign();
	void SortIndex(unsigned short* sortedIndex, unsigned char* hclens, unsigned int size);
	void CreateSign(unsigned short* clens, unsigned char* hclens, unsigned short* SortedIndex, unsigned int size);
	void createCustomHuffmanSign(unsigned long long* curSearchBit, unsigned char* byteArray);
	void DecompressHuffman(unsigned long long* curSearchBit, unsigned char* byteArray, unsigned int* outIndex, unsigned char* outArray);
	void Uncompress(unsigned long long* curSearchBit, unsigned char* byteArray, unsigned int* outIndex, unsigned char* outArray);
	unsigned short blockFinal(unsigned long long* curSearchBit, unsigned char* byteArray);
	unsigned short blockType(unsigned long long* curSearchBit, unsigned char* byteArray);

public:
	bool getDecompressArray(unsigned char* byteArray, unsigned int size, unsigned char* outArray);
};

#endif

/*
ハフマンDecompress後, LZSSのDecompress

最初の1bit  0:まだ後にブロック有り, 1:最後のブロック
次の2bit   01:固定ハフマン, 10:カスタムハフマン
256がそのブロック終端

固定ハフマン符号表
文字(変換無し)と一致した長さの符号表
   値	     ビット数	     符号
0 - 143	     8	   00110000 - 10111111
144 - 255	 9	   110010000 - 111111111
256 - 279    7	   0000000 - 0010111
280 - 287    8	   11000000 - 11000111

距離  の符号表
値	ビット数	符号
0 - 31	5	00000 - 11111

LZSS

0~255    一致長さ, 距離に置換されなかった0x00~0xffまでの文字
256      ブロック終端
257~285  一致した長さ, 一致した文字までの距離で使用する数字

一致した長さのベース, 拡張ビット数対応表
値	一致した長さ	拡張ビット数	表現できる範囲
257	3	    0	3
258	4	    0	4
259	5	    0	5
260	6	    0	6
261	7	    0	7
262	8	    0	8
263	9	    0	9
264	10	    0	10
265	11	    1	11 - 12
266	13	    1	13 - 14
267	15	    1	15 - 16
268	17	    1	17 - 18
269	19	    2	19 - 22
270	23	    2	23 - 27
271	27	    2	27 - 30
272	31	    2	31 - 34
273	35	    3	35 - 42
274	43	    3	43 - 50
275	51	    3	51 - 58
276	59	    3	59 - 66
277	67	    4	67 - 82
278	83	    4	83 - 98
279	99	    4	99 - 114
280	115	    4	115 - 130
281	131	    5	131 - 162
282	163	    5	163 - 194
283	195  	5	195 - 226
284	227	    5	227 - 257
285	258 	0	258

距離" のベース値, 拡張ビット数対応表
値    距離   拡張ビット数	 表現できる範囲
0	1	    0	1
1	2	    0	2
2	3	    0	3
3	4	    0	4
4	5	    1	5 - 6
5	7	    1	7 - 8
6	9	    2	9 - 12
7	13	    2	13 - 16
8	17	    3	17 - 24
9	25	    3	25 - 32
10	33	    4	33 - 48
11	49	    4	49 - 64
12	65	    5	65 - 96
13	97	    5	97 - 128
14	129	    6	129 - 192
15	193	    6	193 - 256
16	257	    7	257 - 384
17	385	    7	385 - 512
18	513	    8	513 - 768
19	769 	8	769 - 1024
20	1025	9	1025 - 1536
21	1537	9	1537 - 2048
22	2049	10	2049 - 3072
23	3073	10	3073 - 4096
24	4097	11	4097 - 6144
25	6145	11	6145 - 8192
26	8193	12	8193 - 12288
27	12289	12	12289 - 16384
28	16385	13	16385 - 24576
29	24577	13	24577 - 32768

カスタムハフマン配置
ヘッダー情報3bit
HLIT：文字/一致長符号の個数 (5ビット)
HDIST：距離符号の個数 (5ビット)
HCLEN：符号長表の符号長表のサイズ (4ビット)
符号長表の符号長表
符号化された文字/一致長の符号長表
符号化された距離の符号長表
圧縮データ
*/