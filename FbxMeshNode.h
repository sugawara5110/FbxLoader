//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　       FbxMeshNode                                          **//
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
class FbxMaterialNode;

class textureType {
public:
	bool NormalMap = false;
	bool DiffuseColor = false;
	bool SpecularColor = false;
	bool SpecularFactor = false;
	bool EmissiveFactor = false;
};

class LayerElement {

private:
	friend FbxMeshNode;
	friend FbxLoader;
	char* MappingInformationType = nullptr;
	char* ReferenceInformationType = nullptr;
	char* name = nullptr;

	unsigned int Nummaterialarr = 0;
	int* materials = nullptr;

	unsigned int Numnormals = 0;
	double* normals = nullptr;
	unsigned int NumNormalsIndex = 0;
	int* NormalsIndex = nullptr;
	double* AlignedNormals = nullptr;

	unsigned int NumUV = 0;
	double* UV = nullptr;
	unsigned int NumUVindex = 0;
	int* UVindex = nullptr;
	double* AlignedUV = nullptr;

	~LayerElement() {
		aDELETE(MappingInformationType);
		aDELETE(ReferenceInformationType);
		aDELETE(name);

		aDELETE(materials);

		aDELETE(normals);
		aDELETE(NormalsIndex);
		aDELETE(AlignedNormals);

		aDELETE(UV);
		aDELETE(UVindex);
		aDELETE(AlignedUV);
	}
};

class AnimationCurve {

private:
	friend Deformer;
	friend FbxLoader;
	unsigned int NumKey = 0;
	double Default = 0.0;
	bool def = false;
	int64_t* KeyTime = nullptr;
	float* KeyValueFloat = nullptr;
	double nullret = 0.0;

	~AnimationCurve() {
		aDELETE(KeyTime);
		aDELETE(KeyValueFloat);
	}
	double getKeyValue(int64_t time);
};

struct Lcl {
	double Translation[3] = {};
	double Rotation[3] = {};
	double Scaling[3] = { 1.0,1.0,1.0 };
};

class Deformer {

private:
	friend FbxMeshNode;
	friend FbxLoader;
	//ツリー情報
	char* name = nullptr;//自身の名前
	unsigned int NumChild = 0;
	char* childName[100] = { nullptr };//子DeformerName
	Deformer* parentNode = nullptr;//EvaluateGlobalTransformの計算に使う

	int IndicesCount = 0;//このボーンに影響を受ける頂点インデックス数
	int* Indices = nullptr;//このボーンに影響を受ける頂点のインデックス配列
	double* Weights = nullptr;//このボーンに影響を受ける頂点のウエイト配列
	double TransformMatrix[16] = {};//中心位置
	double TransformLinkMatrix[16] = {};//初期姿勢行列(絶対位置)
	double LocalPose[16] = {};
	double GlobalPose[16] = {};
	static int numAnimation;

	AnimationCurve* Translation = nullptr;
	AnimationCurve* Rotation = nullptr;
	AnimationCurve* Scaling = nullptr;

	Lcl lcl = {};

	void MatrixScaling(double mat[16], double sx, double sy, double sz);
	void MatrixRotationX(double mat[16], double theta);
	void MatrixRotationY(double mat[16], double theta);
	void MatrixRotationZ(double mat[16], double theta);
	void MatrixTranslation(double mat[16], double movx, double movy, double movz);
	void MatrixMultiply(double outmat[16], double mat1[16], double mat2[16]);
	double CalDetMat4x4(double mat[16]);
	void MatrixInverse(double outmat[16], double mat[16]);
	void VectorMatrixMultiply(double inoutvec[3], double mat[16]);
	double* SubEvaluateGlobalTransform(int64_t time, int animationIndex);

public:
	Deformer() {
		Translation = new AnimationCurve[numAnimation * 3];
		Rotation = new AnimationCurve[numAnimation * 3];
		Scaling = new AnimationCurve[numAnimation * 3];
		for (int i = 0; i < numAnimation * 3; i++)
			Scaling[i].nullret = 1.0;
	}
	~Deformer() {
		aDELETE(Translation);
		aDELETE(Rotation);
		aDELETE(Scaling);
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
	void EvaluateLocalTransform(int64_t time, int animationIndex = 0);
	void EvaluateGlobalTransform(int64_t time, int animationIndex = 0);
	double getEvaluateLocalTransform(unsigned int y, unsigned int x);
	double getEvaluateGlobalTransform(unsigned int y, unsigned int x);
	double* getLocalPose() { return LocalPose; }
	double* getGlobalPose() { return GlobalPose; }
	Deformer* getParentNode() { return parentNode; }
	Lcl getLcl() { return lcl; }
};

class TextureName {

private:
	friend FbxMeshNode;
	friend FbxLoader;
	friend FbxMaterialNode;

	char* name = nullptr;
	textureType type = {};
	char* UVname = nullptr;

	~TextureName() {
		aDELETE(name);
		aDELETE(UVname);
	}
};

class FbxMaterialNode {

private:
	friend FbxMeshNode;
	friend FbxLoader;
	double Diffuse[3] = {};
	double Specular[3] = {};
	double Ambient[3] = {};
	char* MaterialName = nullptr;
	const static int numTex = 10;
	int NumDifTexture = 0;
	TextureName textureDifName[numTex] = {};
	int NumNorTexture = 0;
	TextureName textureNorName[numTex] = {};

public:
	~FbxMaterialNode() {
		aDELETE(MaterialName);
	}
};

class FbxMeshNode {

private:
	friend FbxLoader;
	char* name = nullptr;
	double* vertices = nullptr;//頂点
	unsigned int NumVertices = 0;//頂点数, xyzで1組
	int* polygonVertices = nullptr;//頂点インデックス
	unsigned int NumPolygonVertices = 0;//頂点インデックス数
	unsigned int NumPolygon = 0;//ポリゴン数
	unsigned int* PolygonSize = nullptr;//各ポリゴン頂点インデックス数
	const static int NumLayerElement = 10;
	int NumMaterial = 0;//マテリアル数
	FbxMaterialNode** material = nullptr;
	LayerElement* Material[NumLayerElement] = { nullptr };//ポリゴン毎の使用マテリアル番号配列
	int NumNormalsObj = 0;
	LayerElement* Normals[NumLayerElement] = { nullptr };
	int NumUVObj = 0;
	LayerElement** UV = nullptr;
	unsigned int NumDeformer = 0;
	Deformer** deformer = nullptr;
	Deformer* rootDeformer = nullptr;
	Lcl lcl = {};

public:
	~FbxMeshNode();
	char* getName();

	//頂点
	unsigned int getNumVertices();
	double* getVertices();

	//頂点インデックス
	unsigned int getNumPolygonVertices();
	int* getPolygonVertices();

	//ポリゴン
	unsigned int getNumPolygon();
	unsigned int getPolygonSize(unsigned int pind);
	unsigned int getNumMaterial();

	//Material
	char* getMaterialName(unsigned int Index = 0);
	int getNumDiffuseTexture(unsigned int Index);
	char* getDiffuseTextureName(unsigned int Index, unsigned int texNo = 0);
	textureType getDiffuseTextureType(unsigned int Index, unsigned int texNo);
	char* getDiffuseTextureUVName(unsigned int Index, unsigned int texNo = 0);
	int getNumNormalTexture(unsigned int Index);
	char* getNormalTextureName(unsigned int Index, unsigned int texNo = 0);
	char* getNormalTextureUVName(unsigned int Index, unsigned int texNo = 0);
	double getDiffuseColor(unsigned int Index, unsigned int ColIndex);
	double getSpecularColor(unsigned int Index, unsigned int ColIndex);
	double getAmbientColor(unsigned int Index, unsigned int ColIndex);

	char* getMaterialMappingInformationType(unsigned int layerIndex = 0);
	char* getMaterialReferenceInformationType(unsigned int layerIndex = 0);
	int getMaterialNoOfPolygon(unsigned int polygonNo, unsigned int layerIndex = 0);

	//Normal
	int getNumNormalObj();
	unsigned int getNumNormal(unsigned int layerIndex = 0);
	char* getNormalName(unsigned int layerIndex = 0);
	char* getNormalMappingInformationType(unsigned int layerIndex = 0);
	char* getNormalReferenceInformationType(unsigned int layerIndex = 0);
	double* getNormal(unsigned int layerIndex = 0);
	unsigned int getNumNormalIndex(unsigned int layerIndex = 0);
	int* getNormalIndex(unsigned int layerIndex = 0);
	double* getAlignedNormal(unsigned int layerIndex = 0);

	//UV
	int getNumUVObj();
	unsigned int getNumUV(unsigned int layerIndex = 0);
	char* getUVName(unsigned int layerIndex = 0);
	char* getUVMappingInformationType(unsigned int layerIndex = 0);
	char* getUVReferenceInformationType(unsigned int layerIndex = 0);
	double* getUV(unsigned int layerIndex = 0);
	unsigned int getNumUVindex(unsigned int layerIndex = 0);
	int* getUVindex(unsigned int layerIndex = 0);
	double* getAlignedUV(unsigned int layerIndex = 0);

	//Deformer
	unsigned int getNumDeformer();
	Deformer* getDeformer(unsigned int index);

	Lcl getLcl() { return lcl; }
};

#endif
