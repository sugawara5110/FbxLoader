//*****************************************************************************************//
//**                                                                                     **//
//**                   Å@Å@Å@       FbxMeshNode                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#include "FbxMeshNode.h"
#include <math.h>

float AnimationCurve::getKeyValue(double time) {
	UINT ind = 0;
	int64_t ti = (int64_t)time;
	if (NumKey <= 1)return KeyValueFloat[0];
	for (ind = 1; ind < NumKey; ind++) {
		if (KeyTime[ind] > ti)break;//tiÇ™KeyTime[ind]ñ¢ñû, KeyTime[ind-1]à»è„
	}
	if (ind >= NumKey)return KeyValueFloat[NumKey - 1];
	int64_t differenceTime = KeyTime[ind] - KeyTime[ind - 1];
	int64_t tmp1 = ti - KeyTime[ind - 1];
	float mag = (float)tmp1 / (float)differenceTime;
	float differenceVal = KeyValueFloat[ind] - KeyValueFloat[ind - 1];
	float addVal = differenceVal * mag;
	return KeyValueFloat[ind - 1] + addVal;
}

void Deformer::MatrixScaling(float mat[16], float sx, float sy, float sz) {
	mat[0] = sx; mat[1] = 0.0f; mat[2] = 0.0f; mat[3] = 0.0f;
	mat[4] = 0.0f; mat[5] = sy; mat[6] = 0.0f; mat[7] = 0.0f;
	mat[8] = 0.0f; mat[9] = 0.0f; mat[10] = sz; mat[11] = 0.0f;
	mat[12] = 0.0f; mat[13] = 0.0f; mat[14] = 0.0f; mat[15] = 1.0f;
}

void Deformer::MatrixRotationX(float mat[16], float theta) {
	float the = theta * 3.14f / 180.0f;
	mat[0] = 1.0f; mat[1] = 0.0f; mat[2] = 0.0f; mat[3] = 0.0f;
	mat[4] = 0.0f; mat[5] = (float)cos(the); mat[6] = (float)sin(the); mat[7] = 0.0f;
	mat[8] = 0.0f; mat[9] = (float)-sin(the); mat[10] = (float)cos(the); mat[11] = 0.0f;
	mat[12] = 0.0f; mat[13] = 0.0f; mat[14] = 0.0f; mat[15] = 1.0f;
}

void Deformer::MatrixRotationY(float mat[16], float theta) {
	float the = theta * 3.14f / 180.0f;
	mat[0] = (float)cos(the); mat[1] = 0.0f; mat[2] = (float)-sin(the); mat[3] = 0.0f;
	mat[4] = 0.0f; mat[5] = 1.0f; mat[6] = 0.0f; mat[7] = 0.0f;
	mat[8] = (float)sin(the); mat[9] = 0.0f; mat[10] = (float)cos(the); mat[11] = 0.0f;
	mat[12] = 0.0f; mat[13] = 0.0f; mat[14] = 0.0f; mat[15] = 1.0f;
}

void Deformer::MatrixRotationZ(float mat[16], float theta) {
	float the = theta * 3.14f / 180.0f;
	mat[0] = (float)cos(the); mat[1] = (float)sin(the); mat[2] = 0.0f; mat[3] = 0.0f;
	mat[4] = (float)-sin(the); mat[5] = (float)cos(the); mat[6] = 0.0f; mat[7] = 0.0f;
	mat[8] = 0.0f; mat[9] = 0.0f; mat[10] = 1.0f; mat[11] = 0.0f;
	mat[12] = 0.0f; mat[13] = 0.0f; mat[14] = 0.0f; mat[15] = 1.0f;
}

void Deformer::MatrixTranslation(float mat[16], float movx, float movy, float movz) {
	mat[0] = 1.0f; mat[1] = 0.0f; mat[2] = 0.0f; mat[3] = 0.0f;
	mat[4] = 0.0f; mat[5] = 1.0f; mat[6] = 0.0f; mat[7] = 0.0f;
	mat[8] = 0.0f; mat[9] = 0.0f; mat[10] = 1.0f; mat[11] = 0.0f;
	mat[12] = movx; mat[13] = movy; mat[14] = movz; mat[15] = 1.0f;
}

void Deformer::MatrixMultiply(float outmat[16], float mat1[16], float mat2[16]) {
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

float Deformer::CalDetMat4x4(float mat[16]) {
	return mat[0] * mat[5] * mat[10] * mat[15] + mat[0] * mat[6] * mat[11] * mat[13] + mat[0] * mat[7] * mat[9] * mat[14]
		+ mat[1] * mat[4] * mat[11] * mat[14] + mat[1] * mat[6] * mat[8] * mat[15] + mat[1] * mat[7] * mat[10] * mat[12]
		+ mat[2] * mat[4] * mat[9] * mat[15] + mat[2] * mat[5] * mat[11] * mat[12] + mat[2] * mat[7] * mat[8] * mat[13]
		+ mat[3] * mat[4] * mat[10] * mat[13] + mat[3] * mat[5] * mat[8] * mat[14] + mat[3] * mat[6] * mat[9] * mat[12]
		- mat[0] * mat[5] * mat[11] * mat[14] - mat[0] * mat[6] * mat[9] * mat[15] - mat[0] * mat[7] * mat[10] * mat[13]
		- mat[1] * mat[4] * mat[10] * mat[15] - mat[1] * mat[6] * mat[11] * mat[12] - mat[1] * mat[7] * mat[8] * mat[14]
		- mat[2] * mat[4] * mat[11] * mat[13] - mat[2] * mat[5] * mat[8] * mat[15] - mat[2] * mat[7] * mat[9] * mat[12]
		- mat[3] * mat[4] * mat[9] * mat[14] - mat[3] * mat[5] * mat[10] * mat[12] - mat[3] * mat[6] * mat[8] * mat[13];
}

void Deformer::MatrixInverse(float outmat[16], float mat[16]) {
	float det = CalDetMat4x4(mat);
	float inv_det = (1.0f / det);

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

double Deformer::getTransformLinkMatrix(UINT y, UINT x) {
	return TransformLinkMatrix[y * 4 + x];
}

void Deformer::EvaluateLocalTransform(double time) {
	float sca[16] = { 0 };
	MatrixScaling(sca, Scaling[0].getKeyValue(time), Scaling[1].getKeyValue(time), Scaling[2].getKeyValue(time));
	float rotx[16] = { 0 };
	MatrixRotationX(rotx, Rotation[0].getKeyValue(time));
	float roty[16] = { 0 };
	MatrixRotationY(roty, Rotation[1].getKeyValue(time));
	float rotz[16] = { 0 };
	MatrixRotationZ(rotz, Rotation[2].getKeyValue(time));
	float mov[16] = { 0 };
	MatrixTranslation(mov, Translation[0].getKeyValue(time), Translation[1].getKeyValue(time), Translation[2].getKeyValue(time));

	float rotxy[16] = { 0 };
	MatrixMultiply(rotxy, rotx, roty);
	float rotxyz[16] = { 0 };
	MatrixMultiply(rotxyz, rotxy, rotz);

	float scrot[16] = { 0 };
	MatrixMultiply(scrot, sca, rotxyz);
	MatrixMultiply(outPose, scrot, mov);
}

void Deformer::EvaluateGlobalTransform(double time) {

	float aa = Scaling[0].getKeyValue(time);
	float aa1 = Scaling[1].getKeyValue(time);
	float aa2 = Scaling[2].getKeyValue(time);
	float aaa = Translation[0].getKeyValue(time);
	float aaa1 = Translation[1].getKeyValue(time);
	float aaa2 = Translation[2].getKeyValue(time);
	float aaaa = Rotation[0].getKeyValue(time);
	float aaaa1 = Rotation[1].getKeyValue(time);
	float aaaa2 = Rotation[2].getKeyValue(time);

	float sca[16] = { 0 };
	MatrixScaling(sca, Scaling[0].getKeyValue(time), Scaling[1].getKeyValue(time), Scaling[2].getKeyValue(time));
	float rotx[16] = { 0 };
	MatrixRotationX(rotx, Rotation[0].getKeyValue(time));
	float roty[16] = { 0 };
	MatrixRotationY(roty, Rotation[1].getKeyValue(time));
	float rotz[16] = { 0 };
	MatrixRotationZ(rotz, Rotation[2].getKeyValue(time));
	float mov[16] = { 0 };
	MatrixTranslation(mov, Translation[0].getKeyValue(time), Translation[1].getKeyValue(time), Translation[2].getKeyValue(time));

	float rotxy[16] = { 0 };
	MatrixMultiply(rotxy, rotx, roty);
	float rotxyz[16] = { 0 };
	MatrixMultiply(rotxyz, rotxy, rotz);

	float scrot[16] = { 0 };
	MatrixMultiply(scrot, sca, rotxyz);
	MatrixMultiply(outPose, scrot, mov);
}

double Deformer::getEvaluateTransform(UINT y, UINT x) {
	return (double)outPose[y * 4 + x];
}

FbxMeshNode::~FbxMeshNode() {
	aDELETE(name);
	aDELETE(vertices);
	aDELETE(polygonVertices);
	aDELETE(PolygonSize);
	for (int i = 0; i < 5; i++) {
		sDELETE(Material[i]);
		sDELETE(Normals[i]);
		sDELETE(UV[i]);
	}
	for (int i = 0; i < 100; i++)sDELETE(deformer[i]);
}

char *FbxMeshNode::getName() {
	return name;
}

UINT FbxMeshNode::getNumVertices() {
	return NumVertices;
}

double *FbxMeshNode::getVertices() {
	return vertices;
}

UINT FbxMeshNode::getNumPolygonVertices() {
	return NumPolygonVertices;
}

INT32 *FbxMeshNode::getPolygonVertices() {
	return polygonVertices;
}

UINT FbxMeshNode::getNumPolygon() {
	return NumPolygon;
}

UINT FbxMeshNode::getPolygonSize(UINT pind) {
	return PolygonSize[pind];
}

UINT FbxMeshNode::getNumMaterial() {
	return NumMaterial;
}
//Material
char *FbxMeshNode::getMaterialName(UINT layerIndex) {
	return Material[layerIndex]->name;
}

char *FbxMeshNode::getMaterialMappingInformationType(UINT layerIndex) {
	return Material[layerIndex]->MappingInformationType;
}

INT32 FbxMeshNode::getMaterialNoOfPolygon(UINT polygonNo, UINT layerIndex) {
	if (Material[layerIndex]->Nummaterialarr <= polygonNo)return 0;
	return Material[layerIndex]->materials[polygonNo];
}
//Normal
UINT FbxMeshNode::getNumNormal(UINT layerIndex) {
	return Normals[layerIndex]->Numnormals;
}

char *FbxMeshNode::getNormalName(UINT layerIndex) {
	return Normals[layerIndex]->name;
}

char *FbxMeshNode::getNormalMappingInformationType(UINT layerIndex) {
	return Normals[layerIndex]->MappingInformationType;
}

double *FbxMeshNode::getNormal(UINT layerIndex) {
	return Normals[layerIndex]->normals;
}
//UV
UINT FbxMeshNode::getNumUV(UINT layerIndex) {
	return UV[layerIndex]->NumUV;
}

char *FbxMeshNode::getUVName(UINT layerIndex) {
	return UV[layerIndex]->name;
}

char *FbxMeshNode::getUVMappingInformationType(UINT layerIndex) {
	return UV[layerIndex]->MappingInformationType;
}

double *FbxMeshNode::getUV(UINT layerIndex) {
	return UV[layerIndex]->UV;
}

UINT FbxMeshNode::getNumUVindex(UINT layerIndex) {
	return UV[layerIndex]->NumUVindex;
}

INT32 *FbxMeshNode::getUVindex(UINT layerIndex) {
	return UV[layerIndex]->UVindex;
}

double *FbxMeshNode::getAlignedUV(UINT layerIndex) {
	return UV[layerIndex]->AlignedUV;
}
//Deformer
UINT FbxMeshNode::getNumDeformer() {
	return NumDeformer;
}

Deformer *FbxMeshNode::getDeformer(UINT index) {
	return deformer[index];
}
