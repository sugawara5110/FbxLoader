//*****************************************************************************************//
//**                                                                                     **//
//**                   Å@Å@Å@       FbxMeshNode                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#include "FbxMeshNode.h"

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

double Deformer::getEvaluateGlobalTransform(UINT y, UINT x, double time) {
	return Pose[y * 4 + x];
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
