//*****************************************************************************************//
//**                                                                                     **//
//**                   �@�@�@       FbxMeshNode                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_FbxMeshNode_Header
#define Class_FbxMeshNode_Header

#include <stdint.h>

#define sDELETE(p) if(p){delete p;      p=nullptr;}
#define aDELETE(p) if(p){delete[] p;    p=nullptr;}

class FbxLoader;
class FbxMeshNode;
class Deformer;

class LayerElement {

private:
	friend FbxMeshNode;
	friend FbxLoader;
	char *MappingInformationType = nullptr;
	char *name = nullptr;
	unsigned int Nummaterialarr = 0;
	int *materials = nullptr;
	unsigned int Numnormals = 0;
	double *normals = nullptr;
	unsigned int NumUV = 0;
	double *UV = nullptr;
	unsigned int NumUVindex = 0;
	int *UVindex = nullptr;
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

class AnimationCurve {

private:
	friend Deformer;
	friend FbxLoader;
	double Lcl = 0;
	unsigned int NumKey = 0;
	double Default = 0.0;
	bool def = false;
	int64_t *KeyTime = nullptr;
	float *KeyValueFloat = nullptr;

	~AnimationCurve() {
		aDELETE(KeyTime);
		aDELETE(KeyValueFloat);
	}
	double getKeyValue(int64_t time);
};

class Deformer {

private:
	friend FbxMeshNode;
	friend FbxLoader;
	//�c���[���
	char* name = nullptr;//���g�̖��O
	unsigned int NumChild = 0;
	char* childName[100] = { nullptr };//�qDeformerName
	Deformer* parentNode = nullptr;//EvaluateGlobalTransform�̌v�Z�Ɏg��

	int IndicesCount = 0;//���̃{�[���ɉe�����󂯂钸�_�C���f�b�N�X��
	int* Indices = nullptr;//���̃{�[���ɉe�����󂯂钸�_�̃C���f�b�N�X�z��
	double* Weights = nullptr;//���̃{�[���ɉe�����󂯂钸�_�̃E�G�C�g�z��
	double TransformMatrix[16] = { 0 };//���S�ʒu
	double TransformLinkMatrix[16] = { 0 };//�����p���s��(��Έʒu)
	double LocalPose[16] = { 0 };
	double GlobalPose[16] = { 0 };

	AnimationCurve Translation[3];
	AnimationCurve Rotation[3];
	AnimationCurve Scaling[3];

	void MatrixScaling(double mat[16], double sx, double sy, double sz);
	void MatrixRotationX(double mat[16], double theta);
	void MatrixRotationY(double mat[16], double theta);
	void MatrixRotationZ(double mat[16], double theta);
	void MatrixTranslation(double mat[16], double movx, double movy, double movz);
	void MatrixMultiply(double outmat[16], double mat1[16], double mat2[16]);
	double CalDetMat4x4(double mat[16]);
	void MatrixInverse(double outmat[16], double mat[16]);
	void VectorMatrixMultiply(double inoutvec[3], double mat[16]);
	double* SubEvaluateGlobalTransform(int64_t time);

public:
	~Deformer() {
		aDELETE(name);
		aDELETE(Indices);
		aDELETE(Weights);
		for (int i = 0; i < 100; i++)aDELETE(childName[i]);
	}
	char* getName();
	int getIndicesCount();
	int* getIndices();
	double* getWeights();
	double getTransformLinkMatrix(unsigned int y, unsigned int x);
	double* getTransformLinkMatrixDirect() { return TransformLinkMatrix; }
	int64_t getTimeFRAMES60(int frame);
	int64_t getTimeFRAMES30(int frame);
	void EvaluateLocalTransform(int64_t time);
	void EvaluateGlobalTransform(int64_t time);
	double getEvaluateLocalTransform(unsigned int y, unsigned int x);
	double getEvaluateGlobalTransform(unsigned int y, unsigned int x);
	double* getLocalPose() { return LocalPose; }
	double* getGlobalPose() { return GlobalPose; }
	Deformer* getParentNode() { return parentNode; }
};

class FbxMaterialNode {

private:
	friend FbxMeshNode;
	friend FbxLoader;
	double Diffuse[3] = { 0 };
	double Specular[3] = { 0 };
	double Ambient[3] = { 0 };
	char *MaterialName = nullptr;
	char *textureDifName = nullptr;
	char *textureNorName = nullptr;

public:
	~FbxMaterialNode() {
		aDELETE(MaterialName);
		aDELETE(textureDifName);
		aDELETE(textureNorName);
	}
};

class FbxMeshNode {

private:
	friend FbxLoader;
	char *name = nullptr;
	double *vertices = nullptr;//���_
	unsigned int NumVertices = 0;//���_��, xyz��1�g
	int *polygonVertices = nullptr;//���_�C���f�b�N�X
	unsigned int NumPolygonVertices = 0;//���_�C���f�b�N�X��
	unsigned int NumPolygon = 0;//�|���S����
	unsigned int *PolygonSize = nullptr;//�e�|���S�����_�C���f�b�N�X��
	int NumMaterial = 0;//�}�e���A����
	FbxMaterialNode *material[5] = { nullptr };
	LayerElement *Material[5] = { nullptr };
	LayerElement *Normals[5] = { nullptr };
	LayerElement *UV[5] = { nullptr };
	unsigned int NumDeformer = 0;
	Deformer *deformer[256] = { nullptr };
	Deformer *rootDeformer = nullptr;

public:
	~FbxMeshNode();
	char *getName();

	//���_
	unsigned int getNumVertices();
	double *getVertices();

	//���_�C���f�b�N�X
	unsigned int getNumPolygonVertices();
	int *getPolygonVertices();

	//�|���S��
	unsigned int getNumPolygon();
	unsigned int getPolygonSize(unsigned int pind);
	unsigned int getNumMaterial();

	//Material
	char *getMaterialName(unsigned int layerIndex = 0);
	char *getMaterialMappingInformationType(unsigned int layerIndex = 0);
	int getMaterialNoOfPolygon(unsigned int polygonNo, unsigned int layerIndex = 0);
	char *getDiffuseTextureName(unsigned int Index);
	char *getNormalTextureName(unsigned int Index);
	double getDiffuseColor(unsigned int Index, unsigned int ColIndex);
	double getSpecularColor(unsigned int Index, unsigned int ColIndex);
	double getAmbientColor(unsigned int Index, unsigned int ColIndex);

	//Normal
	unsigned int getNumNormal(unsigned int layerIndex = 0);
	char *getNormalName(unsigned int layerIndex = 0);
	char *getNormalMappingInformationType(unsigned int layerIndex = 0);
	double *getNormal(unsigned int layerIndex = 0);

	//UV
	unsigned int getNumUV(unsigned int layerIndex = 0);
	char *getUVName(unsigned int layerIndex = 0);
	char *getUVMappingInformationType(unsigned int layerIndex = 0);
	double *getUV(unsigned int layerIndex = 0);
	unsigned int getNumUVindex(unsigned int layerIndex = 0);
	int *getUVindex(unsigned int layerIndex = 0);
	double *getAlignedUV(unsigned int layerIndex = 0);

	//Deformer
	unsigned int getNumDeformer();
	Deformer *getDeformer(unsigned int index);
};

#endif
