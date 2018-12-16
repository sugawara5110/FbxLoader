//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　         FbxLoader                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_FbxLoader_Header
#define Class_FbxLoader_Header

#include <stdio.h>
#include <windows.h>
#include <vector>
#include "DecompressDeflate.h"
#define sDELETE(p) if(p){delete p;      p=nullptr;}
#define aDELETE(p) if(p){delete[] p;    p=nullptr;}
#define NUMNODENAME 10

UINT convertBYTEtoUINT(FILE *fp);
UINT convertUCHARtoUINT(UCHAR *arr);
INT32 convertUCHARtoINT32(UCHAR *arr);
int64_t convertUCHARtoint64(UCHAR *arr);
UINT64 convertUCHARtoUINT64(UCHAR *arr);

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

class LayerElement {

private:
	friend FbxMeshNode;
	friend FbxLoader;
	char *MappingInformationType = nullptr;
	char *name = nullptr;
	UINT Nummaterialarr = 0;
	INT32 *materials = nullptr;
	UINT Numnormals = 0;
	double *normals = nullptr;
	UINT NumUV = 0;
	double *UV = nullptr;
	UINT NumUVindex = 0;
	INT32 *UVindex = nullptr;
	double *AlignedUV = nullptr;

	~LayerElement() {
		aDELETE(MappingInformationType);
		aDELETE(name);
		aDELETE(materials);
		aDELETE(normals);
		aDELETE(UV);
		aDELETE(UVindex);
		aDELETE(AlignedUV);
	}
};

class FbxMeshNode {

private:
	friend FbxLoader;
	char *name = nullptr;
	double *vertices = nullptr;//頂点
	UINT NumVertices = 0;//頂点数, xyzで1組
	INT32 *polygonVertices = nullptr;//頂点インデックス
	UINT NumPolygonVertices = 0;//頂点インデックス数
	UINT NumPolygon = 0;//ポリゴン数
	UINT *PolygonSize = nullptr;//各ポリゴン頂点インデックス数
	INT32 NumMaterial = 0;
	LayerElement *Material[5] = { nullptr };
	LayerElement *Normals[5] = { nullptr };
	LayerElement *UV[5] = { nullptr };

public:
	~FbxMeshNode();
	char *getName();
	//頂点
	UINT getNumVertices();
	double *getVertices();
	//頂点インデックス
	UINT getNumPolygonVertices();
	INT32 *getPolygonVertices();
	//ポリゴン
	UINT getNumPolygon();
	UINT getPolygonSize(UINT pind);
	UINT getNumMaterial();
	//Material
	char *getMaterialName(UINT layerIndex = 0);
	char *getMaterialMappingInformationType(UINT layerIndex = 0);
	INT32 getMaterialNoOfPolygon(UINT polygonNo, UINT layerIndex = 0);
	//Normal
	UINT getNumNormal(UINT layerIndex = 0);
	char *getNormalName(UINT layerIndex = 0);
	char *getNormalMappingInformationType(UINT layerIndex = 0);
	double *getNormal(UINT layerIndex = 0);
	//UV
	UINT getNumUV(UINT layerIndex = 0);
	char *getUVName(UINT layerIndex = 0);
	char *getUVMappingInformationType(UINT layerIndex = 0);
	double *getUV(UINT layerIndex = 0);
	UINT getNumUVindex(UINT layerIndex = 0);
	INT32 *getUVindex(UINT layerIndex = 0);
	double *getAlignedUV(UINT layerIndex = 0);
};

class FbxLoader {

private:
	friend NodeRecord;
	UINT version = 0;//23から26バイトまで4バイト分符号なし整数,リトルエンディアン(下から読む)
	NodeRecord FbxRecord;
	NodeRecord *rootNode = nullptr;//ConnectionID:0のポインタ
	DecompressDeflate dd;
	std::vector<ConnectionNo> cnNo;
	std::vector<ConnectionList> cnLi;
	UINT NumMesh = 0;
	FbxMeshNode *mesh = nullptr;

	bool fileCheck(FILE *fp);
	void searchVersion(FILE *fp);
	void readFBX(FILE *fp);
	void Decompress(NodeRecord *node, UCHAR **output, UINT *outSize, UINT typeSize);
	void getLayerElementSub(NodeRecord *node, LayerElement *le);
	void getLayerElement(NodeRecord *node, FbxMeshNode *mesh);
	void getGeometry(NodeRecord *node, FbxMeshNode *mesh);
	void getMesh();
	void ConvertUCHARtoDouble(UCHAR *arr, double *outArr, UINT outsize);
	void ConvertUCHARtoINT32(UCHAR *arr, INT32 *outArr, UINT outsize);
	void drawname(NodeRecord *node, bool cnNode);

public:
	~FbxLoader();
	bool setFbxFile(char *pass);
	NodeRecord *getFbxRecord();
	NodeRecord *getRootNode();
	UINT getNumFbxMeshNode();
	FbxMeshNode **getFbxMeshNode();
	int getVersion();
	void drawRecord();
	void drawNode();
};

#endif
