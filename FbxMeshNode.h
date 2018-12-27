//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　       FbxMeshNode                                          **//
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
	double getKeyValue(int64_t time);
};

class Deformer {

private:
	friend FbxMeshNode;
	friend FbxLoader;
	//ツリー情報
	char *name = nullptr;//自身の名前
	UINT NumChild = 0;
	char *childName[100] = { nullptr };//子DeformerName
	Deformer *parentNode = nullptr;//EvaluateGlobalTransformの計算に使う

	int IndicesCount = 0;//このボーンに影響を受ける頂点インデックス数
	int *Indices = nullptr;//このボーンに影響を受ける頂点のインデックス配列
	double *Weights = nullptr;//このボーンに影響を受ける頂点のウエイト配列
	double TransformMatrix[16] = { 0 };//中心位置
	double TransformLinkMatrix[16] = { 0 };//初期姿勢行列(絶対位置)
	double Pose[16] = { 0 };//姿勢行列(使わないかも)
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
	double *SubEvaluateGlobalTransform(int64_t time);

public:
	~Deformer() {
		aDELETE(name);
		aDELETE(Indices);
		aDELETE(Weights);
		for (int i = 0; i < 100; i++)aDELETE(childName[i]);
	}
	char *getName();
	int getIndicesCount();
	int *getIndices();
	double *getWeights();
	double getTransformLinkMatrix(UINT y, UINT x);
	int64_t getTimeFRAMES60(int frame);
	int64_t getTimeFRAMES30(int frame);
	void EvaluateLocalTransform(int64_t time);
	void EvaluateGlobalTransform(int64_t time);
	double getEvaluateLocalTransform(UINT y, UINT x);
	double getEvaluateGlobalTransform(UINT y, UINT x);
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
	UINT NumDeformer = 0;
	Deformer *deformer[100] = { nullptr };
	Deformer *rootDeformer = nullptr;

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

	//Deformer
	UINT getNumDeformer();
	Deformer *getDeformer(UINT index);
};

#endif
