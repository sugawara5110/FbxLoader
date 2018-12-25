//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　         FbxLoader                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_FbxLoader_Header
#define Class_FbxLoader_Header

#include <stdio.h>
#include <vector>
#include "DecompressDeflate.h"
#include "FbxMeshNode.h"

#define NUMNODENAME 10

UINT convertBYTEtoUINT(FILE *fp);
UINT convertUCHARtoUINT(UCHAR *arr);
INT32 convertUCHARtoINT32(UCHAR *arr);
int64_t convertUCHARtoint64(UCHAR *arr);
UINT64 convertUCHARtoUINT64(UCHAR *arr);
double convertUCHARtoDouble(UCHAR *arr);

class FbxLoader;
class FbxMeshNode;
class NodeRecord;

class ConnectionNo {

public:
	int64_t ConnectionID = -1;
	NodeRecord *ConnectionIDPointer = nullptr;
};

class ConnectionList {

public:
	int64_t ChildID = -1;
	int64_t ParentID = -1;
};

class NodeRecord {

private:
	friend FbxLoader;
	friend FbxMeshNode;
	//全てリトルエンディアン
	UINT EndOffset = 0;//次のファイルの先頭バイト数
	UINT NumProperties = 0;//プロパティの数
	UINT PropertyListLen = 0;//プロパティリストの大きさ(byte)
	UCHAR classNameLen = 0;
	char *className = nullptr;
	UCHAR *Property = nullptr;//(型type, そのdataの順で並んでる) * プロパティの数
	//型type種類別のdata内容 (型typeは1byte)
	//プリミティブ型
	//Y:2byte 符号付き整数型
	//C:1bit  (1byte使う)bool
	//I:4byte 符号付き整数型
	//F:4byte 単精度浮動小数点
	//D:8byte 倍精度浮動小数点
	//L:8byte 符号付き整数型

	//配列型
	//f: Fの配列
	//d: Dの配列
	//l: Lの配列
	//i: Iの配列
	//b: Cの配列
	//配列型の場合は
	//ArrayLenght:     4byte  (配列数, 無圧縮の場合は配列数×データ型がContentsサイズとなる)
	//Encoding:        4byte  (0:無圧縮 1:圧縮済み)
	//CompressedLenght:4byte  (圧縮済みの場合のContentsのサイズ)
	//Contents:        可変   (データ)
	//の順番で並んでる

	//特殊型
	//S: string
	//R: data(生データ)
	//特殊型の場合は
	//Lenght:  4byte
	//Data:    Length byte
	//の順で並んでる

	char *nodeName[NUMNODENAME] = { nullptr };
	UINT NumChildren = 0;
	NodeRecord *nodeChildren = nullptr;//{}内のノード, NodeRecord実体配列用

	int64_t thisConnectionID = -1;
	UINT NumConnectionNode = 0;
	NodeRecord *connectionNode[100] = { nullptr };//NodeRecordポインタ配列用

	void searchName_Type(std::vector<ConnectionNo>& cn);
	void createConnectionList(std::vector<ConnectionList>& cnLi);
	void set(FILE *fp, std::vector<ConnectionNo>& cn, std::vector<ConnectionList>& cnLi);
	~NodeRecord();
};

class FbxLoader {

private:
	friend NodeRecord;
	UINT version = 0;//23から26バイトまで4バイト分符号なし整数,リトルエンディアン(下から読む)
	NodeRecord FbxRecord;//ファイルそのまま
	NodeRecord *rootNode = nullptr;//ConnectionID:0のポインタ
	NodeRecord *Skeleton = nullptr;//Deformer最上位ノードポインタ
	DecompressDeflate dd;
	std::vector<ConnectionNo> cnNo;
	std::vector<ConnectionList> cnLi;
	UINT NumMesh = 0;
	FbxMeshNode *Mesh = nullptr;

	bool fileCheck(FILE *fp);
	void searchVersion(FILE *fp);
	void readFBX(FILE *fp);
	void Decompress(NodeRecord *node, UCHAR **output, UINT *outSize, UINT typeSize);
	void getLayerElementSub(NodeRecord *node, LayerElement *le);
	void getLayerElement(NodeRecord *node, FbxMeshNode *mesh);
	bool nameComparison(char *name1, char *name2);
	void setParentPointerOfSubDeformer(FbxMeshNode *mesh);
	void getSubDeformer(NodeRecord *node, FbxMeshNode *mesh);
	void getDeformer(NodeRecord *node, FbxMeshNode *mesh);
	void getGeometry(NodeRecord *node, FbxMeshNode *mesh);
	void getMesh();
	void getLcl(NodeRecord *pro70Child, AnimationCurve anim[3], char *LclStr);
	void getAnimationCurve(NodeRecord *animNode, AnimationCurve anim[3], char *Lcl);
	void getPoseSub2(int64_t cnId, NodeRecord *node, FbxMeshNode *mesh);
	void getPoseSub(NodeRecord *node, FbxMeshNode *mesh);
	void getPose();
	void ConvertUCHARtoDouble(UCHAR *arr, double *outArr, UINT outsize);
	void ConvertUCHARtoINT32(UCHAR *arr, INT32 *outArr, UINT outsize);
	void ConvertUCHARtoint64_t(UCHAR *arr, int64_t *outArr, UINT outsize);
	void ConvertUCHARtofloat(UCHAR *arr, float *outArr, UINT outsize);
	void drawname(NodeRecord *node, bool cnNode);

public:
	~FbxLoader();
	bool setFbxFile(char *pass);
	NodeRecord *getFbxRecord();
	NodeRecord *getRootNode();
	UINT getNumFbxMeshNode();
	FbxMeshNode *getFbxMeshNode(UINT index);
	int getVersion();
	void drawRecord();
	void drawNode();
};

#endif
