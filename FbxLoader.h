//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　         FbxLoader                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_FbxLoader_Header
#define Class_FbxLoader_Header

#include <stdio.h>
#include <vector>
#include "FbxMeshNode.h"

#define NUMNODENAME 10

class FilePointer {

private:
	unsigned int pointer = 0;
	unsigned char *fileStr = nullptr;

public:
	~FilePointer();
	bool setFile(char *pass);
	void setCharArray(char *cArray, int size);//FILE使わないで読み込む時使用
	unsigned int getPos();
	void seekPointer(unsigned int ind);
	unsigned char getByte();
	void fRead(char *dst, int byteSize);
	unsigned int convertBYTEtoUINT();
};

unsigned int convertUCHARtoUINT(unsigned char *arr);
int convertUCHARtoINT32(unsigned char *arr);
int64_t convertUCHARtoint64(unsigned char *arr);
unsigned long long convertUCHARtoUINT64(unsigned char *arr);
double convertUCHARtoDouble(unsigned char *arr);

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
	unsigned int EndOffset = 0;//次のファイルの先頭バイト数
	unsigned int NumProperties = 0;//プロパティの数
	unsigned int PropertyListLen = 0;//プロパティリストの大きさ(byte)
	unsigned char classNameLen = 0;
	char *className = nullptr;
	unsigned char *Property = nullptr;//(型type, そのdataの順で並んでる) * プロパティの数
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
	unsigned int NumChildren = 0;
	NodeRecord *nodeChildren = nullptr;//{}内のノード, NodeRecord実体配列用

	int64_t thisConnectionID = -1;
	std::vector<NodeRecord*> connectionNode;//NodeRecordポインタ配列用

	void searchName_Type(std::vector<ConnectionNo>& cn);
	void createConnectionList(std::vector<ConnectionList>& cnLi);
	void set(FilePointer *fp, std::vector<ConnectionNo>& cn, std::vector<ConnectionList>& cnLi);
	~NodeRecord();
};

class FbxLoader {

private:
	friend NodeRecord;
	unsigned int version = 0;//23から26バイトまで4バイト分符号なし整数,リトルエンディアン(下から読む)
	NodeRecord FbxRecord;//ファイルそのまま
	NodeRecord *rootNode = nullptr;//ConnectionID:0のポインタ
	std::vector<ConnectionNo> cnNo;
	std::vector<ConnectionList> cnLi;
	unsigned int NumMesh = 0;
	FbxMeshNode *Mesh = nullptr;
	unsigned int NumDeformer = 0;
	Deformer *deformer[256] = { nullptr };//デフォーマーのみのファイル対応
	Deformer *rootDeformer = nullptr;

	bool fileCheck(FilePointer *fp);
	void searchVersion(FilePointer *fp);
	void readFBX(FilePointer *fp);
	bool Decompress(NodeRecord *node, unsigned char **output, unsigned int *outSize, unsigned int typeSize);
	void getLayerElementSub(NodeRecord *node, LayerElement *le);
	void getLayerElement(NodeRecord *node, FbxMeshNode *mesh);
	bool nameComparison(char *name1, char *name2);
	void setParentPointerOfSubDeformer(FbxMeshNode *mesh);
	void getSubDeformer(NodeRecord *node, FbxMeshNode *mesh);
	void getDeformer(NodeRecord *node, FbxMeshNode *mesh);
	void getGeometry(NodeRecord *node, FbxMeshNode *mesh);
	void getMaterial(NodeRecord *node, FbxMeshNode *mesh, unsigned int *materialIndex);
	void getMesh();
	void setParentPointerOfNoneMeshSubDeformer();
	void getNoneMeshSubDeformer(NodeRecord *node);
	void getNoneMeshDeformer();
	void getCol(NodeRecord *pro70Child, double Col[3], char *ColStr);
	void getLcl(NodeRecord *pro70Child, AnimationCurve anim[3], char *LclStr);
	void getAnimationCurve(NodeRecord *animNode, AnimationCurve anim[3], char *Lcl);
	void getAnimation(NodeRecord *model, Deformer *defo);
	void ConvertUCHARtoDouble(unsigned char *arr, double *outArr, unsigned int outsize);
	void ConvertUCHARtoINT32(unsigned char *arr, int *outArr, unsigned int outsize);
	void ConvertUCHARtoint64_t(unsigned char *arr, int64_t *outArr, unsigned int outsize);
	void ConvertUCHARtofloat(unsigned char *arr, float *outArr, unsigned int outsize);
	void drawname(NodeRecord *node, bool cnNode);

public:
	~FbxLoader();
	bool setFbxFile(char *pass);
	bool setBinaryInFbxFile(char *strArray, int size);
	NodeRecord *getFbxRecord();
	NodeRecord *getRootNode();
	unsigned int getNumFbxMeshNode();
	FbxMeshNode *getFbxMeshNode(unsigned int index);
	unsigned int getNumNoneMeshDeformer();
	Deformer *getNoneMeshDeformer(unsigned int index);
	int getVersion();
	void drawRecord();
	void drawNode();
};

#endif
