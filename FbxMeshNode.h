//*****************************************************************************************//
//**                                                                                     **//
//**                   �@�@�@       FbxMeshNode                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#ifndef Class_FbxMeshNode_Header
#define Class_FbxMeshNode_Header

#include <windows.h>

#define sDELETE(p) if(p){delete p;      p=nullptr;}
#define aDELETE(p) if(p){delete[] p;    p=nullptr;}

class FbxLoader;
class FbxMeshNode;

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

class Deformer {

private:
	friend FbxMeshNode;
	friend FbxLoader;
	char *name = nullptr;
	int IndicesCount = 0;//���̃{�[���ɉe�����󂯂钸�_�C���f�b�N�X��
	int *Indices = nullptr;//���̃{�[���ɉe�����󂯂钸�_�̃C���f�b�N�X�z��
	double *Weights = nullptr;//���̃{�[���ɉe�����󂯂钸�_�̃E�G�C�g�z��
	double TransformLinkMatrix[16];
	double Pose[16];

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
	double getEvaluateGlobalTransform(UINT y, UINT x, double time);
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
