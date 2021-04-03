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
	unsigned char* fileStr = nullptr;

public:
	~FilePointer();
	bool setFile(char* pass);
	void setCharArray(char* cArray, int size);//FILE�g��Ȃ��œǂݍ��ގ��g�p
	unsigned int getPos();
	void seekPointer(unsigned int ind);
	unsigned char getByte();
	void fRead(char* dst, int byteSize);
	unsigned int convertBYTEtoUINT();
	uint64_t convertBYTEtoUINT64();
};

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
	//��ChildID�ɑ΂���
	textureType texType = {};
};

class NodeRecord {

private:
	friend FbxLoader;
	friend FbxMeshNode;
	//�S�ă��g���G���f�B�A��
	unsigned char classNameLen = 0;
	char* className = nullptr;
	unsigned char* Property = nullptr;//(�^type, ����data�̏��ŕ���ł�) * �v���p�e�B�̐�
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

	textureType texType = {};
	char* nodeName[NUMNODENAME] = { nullptr };
	unsigned int NumChildren = 0;
	NodeRecord* nodeChildren = nullptr;//{}���̃m�[�h, NodeRecord���̔z��p

	int64_t thisConnectionID = -1;
	std::vector<NodeRecord*> connectionNode;//NodeRecord�|�C���^�z��p

	void searchName_Type(std::vector<ConnectionNo>& cn, uint64_t PropertyListLen);
	void createConnectionList(std::vector<ConnectionList>& cnLi, char* nodeName1);
	void set(bool version7500, FilePointer* fp, std::vector<ConnectionNo>& cn, std::vector<ConnectionList>& cnLi);
	~NodeRecord();
};

struct GlobalSettings {
	int UpAxis = 0;
	int UpAxisSign = 0;
	int FrontAxis = 0;
	int FrontAxisSign = 0;
	int CoordAxis = 0;
	int CoordAxisSign = 0;
	int OriginalUpAxis = 0;
	int OriginalUpAxisSign = 0;
	double UnitScaleFactor = 0.0;
	double OriginalUnitScaleFactor = 0.0;
	double AmbientColor[3] = {};
	int TimeMode = 0;
	int64_t TimeSpanStart = 0;
	int64_t TimeSpanStop = 0;
	double CustomFrameRate = 0.0;
};

class FbxLoader {

private:
	friend NodeRecord;
	unsigned int version = 0;//23����26�o�C�g�܂�4�o�C�g�������Ȃ�����,���g���G���f�B�A��(������ǂ�)
	bool convertByteRangeSwitch = false;//�ǂݍ��݃o�C�g���؂�ւ�, FBX7400�ȉ�4byte, FBX7500�ȏ��8byte
	NodeRecord FbxRecord;//�t�@�C�����̂܂�
	NodeRecord* rootNode = nullptr;//ConnectionID:0�̃|�C���^
	std::vector<ConnectionNo> cnNo;
	std::vector<ConnectionList> cnLi;
	unsigned int NumMesh = 0;
	FbxMeshNode* Mesh = nullptr;
	unsigned int NumDeformer = 0;
	Deformer* deformer[256] = { nullptr };//�f�t�H�[�}�[�݂̂̃t�@�C���Ή�
	Deformer* rootDeformer = nullptr;
	int numAnimation = 0;
	GlobalSettings Gset = {};

	bool fileCheck(FilePointer* fp);
	void searchVersion(FilePointer* fp);
	void readFBX(FilePointer* fp);
	bool Decompress(NodeRecord* node, unsigned char** output, unsigned int* outSize, unsigned int typeSize);
	void getLayerElementSub(NodeRecord* node, LayerElement* le);
	void getLayerElement(NodeRecord* node, FbxMeshNode* mesh);
	bool nameComparison(char* name1, char* name2);
	void setParentPointerOfSubDeformer(FbxMeshNode* mesh);
	void getSubDeformer(NodeRecord* node, FbxMeshNode* mesh);
	void getDeformer(NodeRecord* node, FbxMeshNode* mesh);
	void getGeometry(NodeRecord* node, FbxMeshNode* mesh);
	void getMaterial(NodeRecord* node, FbxMeshNode* mesh, unsigned int* materialIndex);
	void checkGeometry(NodeRecord* node, bool check[2]);
	void checkMaterial(NodeRecord* node, bool* check);
	bool checkMeshNodeRecord(NodeRecord* node);
	void getMesh();
	void setParentPointerOfNoneMeshSubDeformer();
	void getNoneMeshSubDeformer(NodeRecord* node);
	void getNoneMeshDeformer();
	void getCol(NodeRecord* pro70Child, double Col[3], char* ColStr);
	void getAnimationCurve(unsigned int& animInd, NodeRecord* animNode, AnimationCurve* anim, char* Lcl);
	void getAnimation(NodeRecord* model, Deformer* defo);
	void ConvertUCHARtoDouble(unsigned char* arr, double* outArr, unsigned int outsize);
	void ConvertUCHARtoINT32(unsigned char* arr, int* outArr, unsigned int outsize);
	void ConvertUCHARtoint64_t(unsigned char* arr, int64_t* outArr, unsigned int outsize);
	void ConvertUCHARtofloat(unsigned char* arr, float* outArr, unsigned int outsize);
	void drawname(NodeRecord* node, bool cnNode);
	void getPropertiesInt(NodeRecord* pro70Child, int& pi, char* pName);
	void getPropertiesint64(NodeRecord* pro70Child, int64_t& pi, char* pName);
	void getPropertiesDouble(NodeRecord* pro70Child, double* piArr, int num, char* pName);
	void getLcl(NodeRecord* model, Lcl& lcl);

public:
	~FbxLoader();
	bool setFbxFile(char* pass);
	bool setBinaryInFbxFile(char* strArray, int size);
	NodeRecord* getFbxRecord();
	NodeRecord* getRootNode();
	unsigned int getNumFbxMeshNode();
	FbxMeshNode* getFbxMeshNode(unsigned int index);
	unsigned int getNumNoneMeshDeformer();
	Deformer* getNoneMeshDeformer(unsigned int index);
	int getNumAnimation() { return numAnimation; }
	GlobalSettings getGlobalSettings() { return Gset; }
	int getVersion();
	void drawRecord();
	void drawNode();
};

#endif
