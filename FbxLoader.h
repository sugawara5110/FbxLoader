//*****************************************************************************************//
//**                                                                                     **//
//**                   �@�@�@         FbxLoader                                          **//
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
	//�S�ă��g���G���f�B�A��
	UINT EndOffset = 0;//���̃t�@�C���̐擪�o�C�g��
	UINT NumProperties = 0;//�v���p�e�B�̐�
	UINT PropertyListLen = 0;//�v���p�e�B���X�g�̑傫��(byte)
	UCHAR classNameLen = 0;
	char *className = nullptr;
	UCHAR *Property = nullptr;//(�^type, ����data�̏��ŕ���ł�) * �v���p�e�B�̐�
	//�^type��ޕʂ�data���e (�^type��1byte)
	//�v���~�e�B�u�^
	//Y:2byte �����t�������^
	//C:1bit  (1byte�g��)bool
	//I:4byte �����t�������^
	//F:4byte �P���x���������_
	//D:8byte �{���x���������_
	//L:8byte �����t�������^

	//�z��^
	//f: F�̔z��
	//d: D�̔z��
	//l: L�̔z��
	//i: I�̔z��
	//b: C�̔z��
	//�z��^�̏ꍇ��
	//ArrayLenght:     4byte  (�z��, �����k�̏ꍇ�͔z�񐔁~�f�[�^�^��Contents�T�C�Y�ƂȂ�)
	//Encoding:        4byte  (0:�����k 1:���k�ς�)
	//CompressedLenght:4byte  (���k�ς݂̏ꍇ��Contents�̃T�C�Y)
	//Contents:        ��   (�f�[�^)
	//�̏��Ԃŕ���ł�

	//����^
	//S: string
	//R: data(���f�[�^)
	//����^�̏ꍇ��
	//Lenght:  4byte
	//Data:    Length byte
	//�̏��ŕ���ł�

	char *nodeName[NUMNODENAME] = { nullptr };
	UINT NumChildren = 0;
	NodeRecord *nodeChildren = nullptr;//{}���̃m�[�h, NodeRecord���̔z��p

	int64_t thisConnectionID = -1;
	UINT NumConnectionNode = 0;
	NodeRecord *connectionNode[100] = { nullptr };//NodeRecord�|�C���^�z��p

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
	double *vertices = nullptr;//���_
	UINT NumVertices = 0;//���_��, xyz��1�g
	INT32 *polygonVertices = nullptr;//���_�C���f�b�N�X
	UINT NumPolygonVertices = 0;//���_�C���f�b�N�X��
	UINT NumPolygon = 0;//�|���S����
	UINT *PolygonSize = nullptr;//�e�|���S�����_�C���f�b�N�X��
	INT32 NumMaterial = 0;
	LayerElement *Material[5] = { nullptr };
	LayerElement *Normals[5] = { nullptr };
	LayerElement *UV[5] = { nullptr };

public:
	~FbxMeshNode();
	char *getName();
	//���_
	UINT getNumVertices();
	double *getVertices();
	//���_�C���f�b�N�X
	UINT getNumPolygonVertices();
	INT32 *getPolygonVertices();
	//�|���S��
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
	UINT version = 0;//23����26�o�C�g�܂�4�o�C�g�������Ȃ�����,���g���G���f�B�A��(������ǂ�)
	NodeRecord FbxRecord;
	NodeRecord *rootNode = nullptr;//ConnectionID:0�̃|�C���^
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
