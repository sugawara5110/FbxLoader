//*****************************************************************************************//
//**                                                                                     **//
//**                   �@�@�@       FbxMeshNode                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_FbxMeshNode_Header
#define Class_FbxMeshNode_Header

#include <windows.h>
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

class AnimationCurve {

private:
	friend Deformer;
	friend FbxLoader;
	double Lcl = 0;
	UINT NumKey = 0;
	double Default = 0.0;
	bool def = false;
	int64_t *KeyTime = nullptr;
	float *KeyValueFloat = nullptr;

	~AnimationCurve() {
		aDELETE(KeyTime);
		aDELETE(KeyValueFloat);
	}
	float getKeyValue(double time);
};

class Deformer {

private:
	friend FbxMeshNode;
	friend FbxLoader;
	char *name = nullptr;
	int IndicesCount = 0;//���̃{�[���ɉe�����󂯂钸�_�C���f�b�N�X��
	int *Indices = nullptr;//���̃{�[���ɉe�����󂯂钸�_�̃C���f�b�N�X�z��
	double *Weights = nullptr;//���̃{�[���ɉe�����󂯂钸�_�̃E�G�C�g�z��
	double TransformLinkMatrix[16] = { 0 };//�����p���s��
	double Pose[16] = { 0 };//�p���s��(�g��Ȃ�����)
	float outPose[16] = { 0 };//�o�͗p

	AnimationCurve Translation[3];
	AnimationCurve Rotation[3];
	AnimationCurve Scaling[3];

	void MatrixScaling(float mat[16], float sx, float sy, float sz);
	void MatrixRotationX(float mat[16], float theta);
	void MatrixRotationY(float mat[16], float theta);
	void MatrixRotationZ(float mat[16], float theta);
	void MatrixTranslation(float mat[16], float movx, float movy, float movz);
	void MatrixMultiply(float outmat[16], float mat1[16], float mat2[16]);
	float CalDetMat4x4(float mat[16]);
	void MatrixInverse(float outmat[16], float mat[16]);

public:
	~Deformer() {
		aDELETE(name);
		aDELETE(Indices);
		aDELETE(Weights);
	}
	char *getName();
	int getIndicesCount();
	int *getIndices();
	double *getWeights();
	double getTransformLinkMatrix(UINT y, UINT x);
	void EvaluateLocalTransform(double time);
	void EvaluateGlobalTransform(double time);
	double getEvaluateTransform(UINT y, UINT x);
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
	UINT NumDeformer = 0;
	Deformer *deformer[100] = { nullptr };

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

	//Deformer
	UINT getNumDeformer();
	Deformer *getDeformer(UINT index);
};

#endif
