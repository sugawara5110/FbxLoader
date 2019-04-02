//*****************************************************************************************//
//**                                                                                     **//
//**                   �@�@�@         FbxLoader                                          **//
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
	void setCharArray(char *cArray, int size);//FILE�g��Ȃ��œǂݍ��ގ��g�p
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
	//�S�ă��g���G���f�B�A��
	unsigned int EndOffset = 0;//���̃t�@�C���̐擪�o�C�g��
	unsigned int NumProperties = 0;//�v���p�e�B�̐�
	unsigned int PropertyListLen = 0;//�v���p�e�B���X�g�̑傫��(byte)
	unsigned char classNameLen = 0;
	char *className = nullptr;
	unsigned char *Property = nullptr;//(�^type, ����data�̏��ŕ���ł�) * �v���p�e�B�̐�
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
	unsigned int NumChildren = 0;
	NodeRecord *nodeChildren = nullptr;//{}���̃m�[�h, NodeRecord���̔z��p

	int64_t thisConnectionID = -1;
	std::vector<NodeRecord*> connectionNode;//NodeRecord�|�C���^�z��p

	void searchName_Type(std::vector<ConnectionNo>& cn);
	void createConnectionList(std::vector<ConnectionList>& cnLi);
	void set(FilePointer *fp, std::vector<ConnectionNo>& cn, std::vector<ConnectionList>& cnLi);
	~NodeRecord();
};

class FbxLoader {

private:
	friend NodeRecord;
	unsigned int version = 0;//23����26�o�C�g�܂�4�o�C�g�������Ȃ�����,���g���G���f�B�A��(������ǂ�)
	NodeRecord FbxRecord;//�t�@�C�����̂܂�
	NodeRecord *rootNode = nullptr;//ConnectionID:0�̃|�C���^
	std::vector<ConnectionNo> cnNo;
	std::vector<ConnectionList> cnLi;
	unsigned int NumMesh = 0;
	FbxMeshNode *Mesh = nullptr;
	unsigned int NumDeformer = 0;
	Deformer *deformer[256] = { nullptr };//�f�t�H�[�}�[�݂̂̃t�@�C���Ή�
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
