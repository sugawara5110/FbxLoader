//*****************************************************************************************//
//**                                                                                     **//
//**                   �@�@�@         FbxLoader                                          **//
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

class FbxLoader {

private:
	friend NodeRecord;
	UINT version = 0;//23����26�o�C�g�܂�4�o�C�g�������Ȃ�����,���g���G���f�B�A��(������ǂ�)
	NodeRecord FbxRecord;//�t�@�C�����̂܂�
	NodeRecord *rootNode = nullptr;//ConnectionID:0�̃|�C���^
	NodeRecord *Skeleton = nullptr;//Deformer�ŏ�ʃm�[�h�|�C���^
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
