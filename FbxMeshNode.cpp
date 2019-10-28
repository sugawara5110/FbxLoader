//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　       FbxMeshNode                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#include "FbxMeshNode.h"
#include <math.h>

double AnimationCurve::getKeyValue(int64_t time) {
	unsigned int ind = 0;
	int64_t ti = time;
	if (!KeyValueFloat)return 0.0;
	if (NumKey <= 1)return (double)KeyValueFloat[0];
	for (ind = 1; ind < NumKey; ind++) {
		if (KeyTime[ind] > ti)break;//tiがKeyTime[ind]未満, KeyTime[ind-1]以上
	}
	if (ind >= NumKey)return (double)KeyValueFloat[NumKey - 1];
	int64_t differenceTime = KeyTime[ind] - KeyTime[ind - 1];
	int64_t tmp1 = ti - KeyTime[ind - 1];
	double mag = (double)tmp1 / (double)differenceTime;
	double differenceVal = (double)KeyValueFloat[ind] - (double)KeyValueFloat[ind - 1];
	double addVal = differenceVal * mag;
	return (double)KeyValueFloat[ind - 1] + addVal;
}

void Deformer::MatrixScaling(double mat[16], double sx, double sy, double sz) {
	mat[0] = sx; mat[1] = 0.0; mat[2] = 0.0; mat[3] = 0.0;
	mat[4] = 0.0; mat[5] = sy; mat[6] = 0.0; mat[7] = 0.0;
	mat[8] = 0.0; mat[9] = 0.0; mat[10] = sz; mat[11] = 0.0;
	mat[12] = 0.0; mat[13] = 0.0; mat[14] = 0.0; mat[15] = 1.0;
}

void Deformer::MatrixRotationX(double mat[16], double theta) {
	double the = theta * 3.14 / 180.0;
	mat[0] = 1.0; mat[1] = 0.0; mat[2] = 0.0; mat[3] = 0.0;
	mat[4] = 0.0; mat[5] = cos(the); mat[6] = sin(the); mat[7] = 0.0;
	mat[8] = 0.0; mat[9] = -sin(the); mat[10] = cos(the); mat[11] = 0.0;
	mat[12] = 0.0; mat[13] = 0.0; mat[14] = 0.0; mat[15] = 1.0;
}

void Deformer::MatrixRotationY(double mat[16], double theta) {
	double the = theta * 3.14 / 180.0;
	mat[0] = cos(the); mat[1] = 0.0; mat[2] = -sin(the); mat[3] = 0.0;
	mat[4] = 0.0; mat[5] = 1.0; mat[6] = 0.0; mat[7] = 0.0;
	mat[8] = sin(the); mat[9] = 0.0; mat[10] = cos(the); mat[11] = 0.0;
	mat[12] = 0.0; mat[13] = 0.0; mat[14] = 0.0; mat[15] = 1.0;
}

void Deformer::MatrixRotationZ(double mat[16], double theta) {
	double the = theta * 3.14 / 180.0;
	mat[0] = cos(the); mat[1] = sin(the); mat[2] = 0.0; mat[3] = 0.0;
	mat[4] = -sin(the); mat[5] = cos(the); mat[6] = 0.0; mat[7] = 0.0;
	mat[8] = 0.0; mat[9] = 0.0; mat[10] = 1.0; mat[11] = 0.0;
	mat[12] = 0.0; mat[13] = 0.0; mat[14] = 0.0; mat[15] = 1.0;
}

void Deformer::MatrixTranslation(double mat[16], double movx, double movy, double movz) {
	mat[0] = 1.0; mat[1] = 0.0; mat[2] = 0.0; mat[3] = 0.0;
	mat[4] = 0.0; mat[5] = 1.0; mat[6] = 0.0; mat[7] = 0.0;
	mat[8] = 0.0; mat[9] = 0.0; mat[10] = 1.0; mat[11] = 0.0;
	mat[12] = movx; mat[13] = movy; mat[14] = movz; mat[15] = 1.0;
}

void Deformer::MatrixMultiply(double outmat[16], double mat1[16], double mat2[16]) {
	outmat[0] = mat1[0] * mat2[0] + mat1[1] * mat2[4] + mat1[2] * mat2[8] + mat1[3] * mat2[12];
	outmat[1] = mat1[0] * mat2[1] + mat1[1] * mat2[5] + mat1[2] * mat2[9] + mat1[3] * mat2[13];
	outmat[2] = mat1[0] * mat2[2] + mat1[1] * mat2[6] + mat1[2] * mat2[10] + mat1[3] * mat2[14];
	outmat[3] = mat1[0] * mat2[3] + mat1[1] * mat2[7] + mat1[2] * mat2[11] + mat1[3] * mat2[15];

	outmat[4] = mat1[4] * mat2[0] + mat1[5] * mat2[4] + mat1[6] * mat2[8] + mat1[7] * mat2[12];
	outmat[5] = mat1[4] * mat2[1] + mat1[5] * mat2[5] + mat1[6] * mat2[9] + mat1[7] * mat2[13];
	outmat[6] = mat1[4] * mat2[2] + mat1[5] * mat2[6] + mat1[6] * mat2[10] + mat1[7] * mat2[14];
	outmat[7] = mat1[4] * mat2[3] + mat1[5] * mat2[7] + mat1[6] * mat2[11] + mat1[7] * mat2[15];

	outmat[8] = mat1[8] * mat2[0] + mat1[9] * mat2[4] + mat1[10] * mat2[8] + mat1[11] * mat2[12];
	outmat[9] = mat1[8] * mat2[1] + mat1[9] * mat2[5] + mat1[10] * mat2[9] + mat1[11] * mat2[13];
	outmat[10] = mat1[8] * mat2[2] + mat1[9] * mat2[6] + mat1[10] * mat2[10] + mat1[11] * mat2[14];
	outmat[11] = mat1[8] * mat2[3] + mat1[9] * mat2[7] + mat1[10] * mat2[11] + mat1[11] * mat2[15];

	outmat[12] = mat1[12] * mat2[0] + mat1[13] * mat2[4] + mat1[14] * mat2[8] + mat1[15] * mat2[12];
	outmat[13] = mat1[12] * mat2[1] + mat1[13] * mat2[5] + mat1[14] * mat2[9] + mat1[15] * mat2[13];
	outmat[14] = mat1[12] * mat2[2] + mat1[13] * mat2[6] + mat1[14] * mat2[10] + mat1[15] * mat2[14];
	outmat[15] = mat1[12] * mat2[3] + mat1[13] * mat2[7] + mat1[14] * mat2[11] + mat1[15] * mat2[15];
}

double Deformer::CalDetMat4x4(double mat[16]) {
	return mat[0] * mat[5] * mat[10] * mat[15] + mat[0] * mat[6] * mat[11] * mat[13] + mat[0] * mat[7] * mat[9] * mat[14]
		+ mat[1] * mat[4] * mat[11] * mat[14] + mat[1] * mat[6] * mat[8] * mat[15] + mat[1] * mat[7] * mat[10] * mat[12]
		+ mat[2] * mat[4] * mat[9] * mat[15] + mat[2] * mat[5] * mat[11] * mat[12] + mat[2] * mat[7] * mat[8] * mat[13]
		+ mat[3] * mat[4] * mat[10] * mat[13] + mat[3] * mat[5] * mat[8] * mat[14] + mat[3] * mat[6] * mat[9] * mat[12]
		- mat[0] * mat[5] * mat[11] * mat[14] - mat[0] * mat[6] * mat[9] * mat[15] - mat[0] * mat[7] * mat[10] * mat[13]
		- mat[1] * mat[4] * mat[10] * mat[15] - mat[1] * mat[6] * mat[11] * mat[12] - mat[1] * mat[7] * mat[8] * mat[14]
		- mat[2] * mat[4] * mat[11] * mat[13] - mat[2] * mat[5] * mat[8] * mat[15] - mat[2] * mat[7] * mat[9] * mat[12]
		- mat[3] * mat[4] * mat[9] * mat[14] - mat[3] * mat[5] * mat[10] * mat[12] - mat[3] * mat[6] * mat[8] * mat[13];
}

void Deformer::MatrixInverse(double outmat[16], double mat[16]) {
	double det = CalDetMat4x4(mat);
	double inv_det = (1.0 / det);

	outmat[0] = inv_det * (mat[5] * mat[10] * mat[15] + mat[6] * mat[11] * mat[13] + mat[7] * mat[9] * mat[14] - mat[5] * mat[11] * mat[14] - mat[6] * mat[9] * mat[15] - mat[7] * mat[10] * mat[13]);
	outmat[1] = inv_det * (mat[1] * mat[11] * mat[14] + mat[2] * mat[9] * mat[15] + mat[3] * mat[10] * mat[13] - mat[1] * mat[10] * mat[15] - mat[2] * mat[11] * mat[13] - mat[3] * mat[9] * mat[14]);
	outmat[2] = inv_det * (mat[1] * mat[6] * mat[15] + mat[2] * mat[7] * mat[13] + mat[3] * mat[5] * mat[14] - mat[1] * mat[7] * mat[14] - mat[2] * mat[5] * mat[15] - mat[3] * mat[6] * mat[13]);
	outmat[3] = inv_det * (mat[1] * mat[7] * mat[10] + mat[2] * mat[5] * mat[11] + mat[3] * mat[6] * mat[9] - mat[1] * mat[6] * mat[11] - mat[2] * mat[7] * mat[9] - mat[3] * mat[5] * mat[10]);

	outmat[4] = inv_det * (mat[4] * mat[11] * mat[14] + mat[6] * mat[8] * mat[15] + mat[7] * mat[10] * mat[12] - mat[4] * mat[10] * mat[15] - mat[6] * mat[11] * mat[12] - mat[7] * mat[8] * mat[14]);
	outmat[5] = inv_det * (mat[0] * mat[10] * mat[15] + mat[2] * mat[11] * mat[12] + mat[3] * mat[8] * mat[14] - mat[0] * mat[11] * mat[14] - mat[2] * mat[8] * mat[15] - mat[3] * mat[10] * mat[12]);
	outmat[6] = inv_det * (mat[0] * mat[7] * mat[14] + mat[2] * mat[4] * mat[15] + mat[3] * mat[6] * mat[12] - mat[0] * mat[6] * mat[15] - mat[2] * mat[7] * mat[12] - mat[3] * mat[4] * mat[14]);
	outmat[7] = inv_det * (mat[0] * mat[6] * mat[11] + mat[2] * mat[7] * mat[8] + mat[3] * mat[4] * mat[10] - mat[0] * mat[7] * mat[10] - mat[2] * mat[4] * mat[11] - mat[3] * mat[6] * mat[8]);

	outmat[8] = inv_det * (mat[4] * mat[9] * mat[15] + mat[5] * mat[11] * mat[12] + mat[7] * mat[8] * mat[13] - mat[4] * mat[11] * mat[13] - mat[5] * mat[8] * mat[15] - mat[7] * mat[9] * mat[12]);
	outmat[9] = inv_det * (mat[0] * mat[11] * mat[13] + mat[1] * mat[8] * mat[15] + mat[3] * mat[9] * mat[12] - mat[0] * mat[9] * mat[15] - mat[1] * mat[11] * mat[12] - mat[3] * mat[8] * mat[13]);
	outmat[10] = inv_det * (mat[0] * mat[5] * mat[15] + mat[1] * mat[7] * mat[12] + mat[3] * mat[4] * mat[13] - mat[0] * mat[7] * mat[13] - mat[1] * mat[4] * mat[15] - mat[3] * mat[5] * mat[12]);
	outmat[11] = inv_det * (mat[0] * mat[7] * mat[9] + mat[1] * mat[4] * mat[11] + mat[3] * mat[5] * mat[8] - mat[0] * mat[5] * mat[11] - mat[1] * mat[7] * mat[8] - mat[3] * mat[4] * mat[9]);

	outmat[12] = inv_det * (mat[4] * mat[10] * mat[13] + mat[5] * mat[8] * mat[14] + mat[6] * mat[9] * mat[12] - mat[4] * mat[9] * mat[14] - mat[5] * mat[10] * mat[12] - mat[6] * mat[8] * mat[13]);
	outmat[13] = inv_det * (mat[0] * mat[9] * mat[14] + mat[1] * mat[10] * mat[12] + mat[2] * mat[8] * mat[13] - mat[0] * mat[10] * mat[13] - mat[1] * mat[8] * mat[14] - mat[2] * mat[9] * mat[12]);
	outmat[14] = inv_det * (mat[0] * mat[6] * mat[13] + mat[1] * mat[4] * mat[14] + mat[2] * mat[5] * mat[12] - mat[0] * mat[5] * mat[14] - mat[1] * mat[6] * mat[12] - mat[2] * mat[4] * mat[13]);
	outmat[15] = inv_det * (mat[0] * mat[5] * mat[10] + mat[1] * mat[6] * mat[8] + mat[2] * mat[4] * mat[9] - mat[0] * mat[6] * mat[9] - mat[1] * mat[4] * mat[10] - mat[2] * mat[5] * mat[8]);
}

void Deformer::VectorMatrixMultiply(double inoutvec[3], double mat[16]) {
	double x = inoutvec[0];
	double y = inoutvec[1];
	double z = inoutvec[2];
	double w = 0;

	inoutvec[0] = x * mat[0] + y * mat[4] + z * mat[8] + mat[12];
	inoutvec[1] = x * mat[1] + y * mat[5] + z * mat[9] + mat[13];
	inoutvec[2] = x * mat[2] + y * mat[6] + z * mat[10] + mat[14];
	w = x * mat[3] + y * mat[7] + z * mat[11] + mat[15];
	inoutvec[0] /= w;
	inoutvec[1] /= w;
	inoutvec[2] /= w;
}

char *Deformer::getName() {
	return name;
}

int Deformer::getIndicesCount() {
	return IndicesCount;
}

int *Deformer::getIndices() {
	return Indices;
}

double *Deformer::getWeights() {
	return Weights;
}

double Deformer::getTransformLinkMatrix(unsigned int y, unsigned int x) {
	return TransformLinkMatrix[y * 4 + x];
}

int64_t Deformer::getTimeFRAMES60(int frame) {
	return frame * 769769300ll;
}

int64_t Deformer::getTimeFRAMES30(int frame) {
	return frame * 1539538600ll;
}

void Deformer::EvaluateLocalTransform(int64_t time) {
	double sca[16] = {};
	MatrixScaling(sca, Scaling[0].getKeyValue(time), Scaling[1].getKeyValue(time), Scaling[2].getKeyValue(time));
	double rotx[16] = {};
	MatrixRotationX(rotx, Rotation[0].getKeyValue(time));
	double roty[16] = {};
	MatrixRotationY(roty, Rotation[1].getKeyValue(time));
	double rotz[16] = {};
	MatrixRotationZ(rotz, Rotation[2].getKeyValue(time));
	double mov[16] = {};
	MatrixTranslation(mov, Translation[0].getKeyValue(time), Translation[1].getKeyValue(time), Translation[2].getKeyValue(time));

	double rotxy[16] = {};
	MatrixMultiply(rotxy, rotx, roty);
	double rotxyz[16] = {};
	MatrixMultiply(rotxyz, rotxy, rotz);

	double scrot[16] = {};
	MatrixMultiply(scrot, sca, rotxyz);
	MatrixMultiply(LocalPose, scrot, mov);
}

double *Deformer::SubEvaluateGlobalTransform(int64_t time) {
	EvaluateLocalTransform(time);
	if (parentNode) {
		//ルートノード以外
		double *GlobalPosePare = parentNode->SubEvaluateGlobalTransform(time);
		MatrixMultiply(GlobalPose, LocalPose, GlobalPosePare);
		return GlobalPose;
	}
	//ルートノード
	return LocalPose;
}

void Deformer::EvaluateGlobalTransform(int64_t time) {
	SubEvaluateGlobalTransform(time);
}

double Deformer::getEvaluateLocalTransform(unsigned int y, unsigned int x) {
	return LocalPose[y * 4 + x];
}

double Deformer::getEvaluateGlobalTransform(unsigned int y, unsigned int x) {
	return GlobalPose[y * 4 + x];
}

FbxMeshNode::~FbxMeshNode() {
	aDELETE(name);
	aDELETE(vertices);
	aDELETE(polygonVertices);
	aDELETE(PolygonSize);
	for (int i = 0; i < NumLayerElement; i++) {
		sDELETE(material[i]);
		sDELETE(Material[i]);
		sDELETE(Normals[i]);
		sDELETE(UV[i]);
	}
	for (unsigned int i = 0; i < NumDeformer; i++)sDELETE(deformer[i]);
	sDELETE(rootDeformer);
}

char *FbxMeshNode::getName() {
	return name;
}

unsigned int FbxMeshNode::getNumVertices() {
	return NumVertices;
}

double *FbxMeshNode::getVertices() {
	return vertices;
}

unsigned int FbxMeshNode::getNumPolygonVertices() {
	return NumPolygonVertices;
}

int *FbxMeshNode::getPolygonVertices() {
	return polygonVertices;
}

unsigned int FbxMeshNode::getNumPolygon() {
	return NumPolygon;
}

unsigned int FbxMeshNode::getPolygonSize(unsigned int pind) {
	return PolygonSize[pind];
}

unsigned int FbxMeshNode::getNumMaterial() {
	return NumMaterial;
}

//Material
char* FbxMeshNode::getMaterialName(unsigned int Index) {
	return material[Index]->MaterialName;
}

int FbxMeshNode::getNumDiffuseTexture(unsigned int Index) {
	return material[Index]->NumDifTexture;
}

char* FbxMeshNode::getDiffuseTextureName(unsigned int Index, unsigned int texNo) {
	return material[Index]->textureDifName[texNo].name;
}

textureType FbxMeshNode::getDiffuseTextureType(unsigned int Index, unsigned int texNo) {
	return material[Index]->textureDifName[texNo].type;
}

char* FbxMeshNode::getDiffuseTextureUVName(unsigned int Index, unsigned int texNo) {
	return material[Index]->textureDifName[texNo].UVname;
}

int FbxMeshNode::getNumNormalTexture(unsigned int Index) {
	return material[Index]->NumNorTexture;
}

char* FbxMeshNode::getNormalTextureName(unsigned int Index, unsigned int texNo) {
	return material[Index]->textureNorName[texNo].name;
}

char* FbxMeshNode::getNormalTextureUVName(unsigned int Index, unsigned int texNo) {
	return material[Index]->textureNorName[texNo].UVname;
}

double FbxMeshNode::getDiffuseColor(unsigned int Index, unsigned int ColIndex) {
	return material[Index]->Diffuse[ColIndex];
}

double FbxMeshNode::getSpecularColor(unsigned int Index, unsigned int ColIndex) {
	return material[Index]->Specular[ColIndex];
}

double FbxMeshNode::getAmbientColor(unsigned int Index, unsigned int ColIndex) {
	return material[Index]->Ambient[ColIndex];
}

char* FbxMeshNode::getMaterialMappingInformationType(unsigned int layerIndex) {
	return Material[layerIndex]->MappingInformationType;
}

char* FbxMeshNode::getMaterialReferenceInformationType(unsigned int layerIndex) {
	return Material[layerIndex]->ReferenceInformationType;
}

int FbxMeshNode::getMaterialNoOfPolygon(unsigned int polygonNo, unsigned int layerIndex) {
	if (Material[layerIndex]->Nummaterialarr <= polygonNo)return 0;
	return Material[layerIndex]->materials[polygonNo];
}

//Normal
int FbxMeshNode::getNumNormalObj() {
	return NumNormalsObj;
}

unsigned int FbxMeshNode::getNumNormal(unsigned int layerIndex) {
	return Normals[layerIndex]->Numnormals;
}

char *FbxMeshNode::getNormalName(unsigned int layerIndex) {
	return Normals[layerIndex]->name;
}

char *FbxMeshNode::getNormalMappingInformationType(unsigned int layerIndex) {
	return Normals[layerIndex]->MappingInformationType;
}

char* FbxMeshNode::getNormalReferenceInformationType(unsigned int layerIndex) {
	return Normals[layerIndex]->ReferenceInformationType;
}

double *FbxMeshNode::getNormal(unsigned int layerIndex) {
	return Normals[layerIndex]->normals;
}

//UV
int FbxMeshNode::getNumUVObj() {
	return NumUVObj;
}

unsigned int FbxMeshNode::getNumUV(unsigned int layerIndex) {
	return UV[layerIndex]->NumUV;
}

char *FbxMeshNode::getUVName(unsigned int layerIndex) {
	return UV[layerIndex]->name;
}

char *FbxMeshNode::getUVMappingInformationType(unsigned int layerIndex) {
	return UV[layerIndex]->MappingInformationType;
}

char* FbxMeshNode::getUVReferenceInformationType(unsigned int layerIndex) {
	return UV[layerIndex]->ReferenceInformationType;
}

double *FbxMeshNode::getUV(unsigned int layerIndex) {
	return UV[layerIndex]->UV;
}

unsigned int FbxMeshNode::getNumUVindex(unsigned int layerIndex) {
	return UV[layerIndex]->NumUVindex;
}

int *FbxMeshNode::getUVindex(unsigned int layerIndex) {
	return UV[layerIndex]->UVindex;
}

double *FbxMeshNode::getAlignedUV(unsigned int layerIndex) {
	return UV[layerIndex]->AlignedUV;
}

//Deformer
unsigned int FbxMeshNode::getNumDeformer() {
	return NumDeformer;
}

Deformer *FbxMeshNode::getDeformer(unsigned int index) {
	return deformer[index];
}
