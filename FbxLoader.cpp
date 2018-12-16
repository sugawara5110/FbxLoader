//*****************************************************************************************//
//**                                                                                     **//
//**                   �@�@�@         FbxLoader                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "FbxLoader.h"
#include "iostream"
#define Kaydata_FBX_binary 18

UINT convertBYTEtoUINT(FILE *fp) {
	char ver[4];
	fread(ver, sizeof(char), 4, fp);

	return ((UCHAR)ver[3] << 24) | ((UCHAR)ver[2] << 16) |
		((UCHAR)ver[1] << 8) | ((UCHAR)ver[0]);
}

UINT convertUCHARtoUINT(UCHAR *arr) {
	return ((UINT)arr[3] << 24) | ((UINT)arr[2] << 16) |
		((UINT)arr[1] << 8) | ((UINT)arr[0]);
}

INT32 convertUCHARtoINT32(UCHAR *arr) {
	return ((INT32)arr[3] << 24) | ((INT32)arr[2] << 16) |
		((INT32)arr[1] << 8) | ((INT32)arr[0]);
}

int64_t convertUCHARtoint64(UCHAR *arr) {
	return ((int64_t)arr[7] << 56) | ((int64_t)arr[6] << 48) |
		((int64_t)arr[5] << 40) | ((int64_t)arr[4] << 32) |
		((int64_t)arr[3] << 24) | ((int64_t)arr[2] << 16) |
		((int64_t)arr[1] << 8) | ((int64_t)arr[0]);
}

UINT64 convertUCHARtoUINT64(UCHAR *arr) {
	return ((UINT64)arr[7] << 56) | ((UINT64)arr[6] << 48) |
		((UINT64)arr[5] << 40) | ((UINT64)arr[4] << 32) |
		((UINT64)arr[3] << 24) | ((UINT64)arr[2] << 16) |
		((UINT64)arr[1] << 8) | ((UINT64)arr[0]);
}

void NodeRecord::searchName_Type(std::vector<ConnectionNo>& cn) {
	int swt = 0;
	UINT ln = 0;
	int nameNo = 0;
	int addInd = 0;
	UCHAR *pr = nullptr;
	UCHAR sw = 0;
	for (UINT i = 0; i < PropertyListLen; i++) {
		switch (swt) {
		case 3:
			//L�̏���
			if (!strcmp("Time", &this->className[this->classNameLen - 4])) {
				i += 7;
				swt = 0;
				continue;
			}
			if (thisConnectionID == -1 && i == 1) {//ConnectionID�͈�ڂ�Property
				thisConnectionID = convertUCHARtoint64(&Property[i]);
			}
			i += 7;
			swt = 0;
			continue;
			break;
		case 2:
			pr = &Property[i];
			memcpy(nodeName[nameNo], (char*)pr, ln);
			nodeName[nameNo][ln] = '\0';
			nameNo++;
			if (nameNo >= NUMNODENAME)return;
			i += ln - 1;
			swt = 0;
			continue;
			break;
		case 1:
			ln = (Property[i + 3] << 24) | (Property[i + 2] << 16) |
				(Property[i + 1] << 8) | (Property[i]);
			if (ln > 0) {
				nodeName[nameNo] = new char[ln + 1];
				i += 3;
				swt = 2;
			}
			else {
				i += 3;
				swt = 0;
				continue;
			}
			break;
		case 0:
			if (Property[i] == 'S') {
				swt = 1;
				break;
			}
			if (Property[i] == 'L') {
				swt = 3;
				break;
			}
			sw = Property[i];
			if (sw == 'C')i++;
			if (sw == 'Y')i += 2;
			if (sw == 'I' || sw == 'F')i += 4;
			if (sw == 'D')i += 8;
			//����^�̏ꍇ��
			//Lenght:  4byte
			//Data:    Length byte
			if (sw == 'R') {
				i += 4 + ((Property[i + 4] << 24) | (Property[i + 3] << 16) |
					(Property[i + 2] << 8) | (Property[i + 1]));
			}
			/*
			ArrayLenght:     4byte
			Encoding:        4byte
			CompressedLenght:4byte
			Contents         ��  CompressedLenght�ɃT�C�Y�������Ă�

			�z��̏ꍇ������byte�񕪃X�L�b�v����
			Property�z���9byte���v�f�����炵CompressedLenght�ɃA�N�Z�X��Contents�T�C�Y�����o��
			���o�����T�C�Y+ 4 * 3�o�C�g��"i"��i�߂�
			*/
			if (sw == 'f' || sw == 'd' || sw == 'l' || sw == 'i' || sw == 'b') {
				i += 12 + ((Property[i + 3 + 9] << 24) | (Property[i + 2 + 9] << 16) |
					(Property[i + 1 + 9] << 8) | (Property[i + 9]));
			}
			break;
		}
	}
	if (thisConnectionID != -1) {
		ConnectionNo tmp;
		tmp.ConnectionID = thisConnectionID;
		tmp.ConnectionIDPointer = this;
		cn.push_back(tmp);
	}
}

void NodeRecord::createConnectionList(std::vector<ConnectionList>& cnLi) {
	ConnectionList cl;
	//S len "OO" L �v8byte�̎���ChildID
	cl.ChildID = convertUCHARtoint64(&Property[8]);
	//L 1byte�̎���ParentID
	cl.ParentID = convertUCHARtoint64(&Property[17]);
	cnLi.push_back(cl);
}

void NodeRecord::set(FILE *fp, std::vector<ConnectionNo>& cn, std::vector<ConnectionList>& cnLi) {
	EndOffset = convertBYTEtoUINT(fp);
	NumProperties = convertBYTEtoUINT(fp);
	PropertyListLen = convertBYTEtoUINT(fp);
	classNameLen = fgetc(fp);
	className = new char[classNameLen + 1];
	fread(className, sizeof(char), classNameLen, fp);
	className[classNameLen] = '\0';
	if (PropertyListLen > 0) {
		Property = new UCHAR[PropertyListLen];
		fread(Property, sizeof(UCHAR), PropertyListLen, fp);
		searchName_Type(cn);
		if (!strcmp(className, "C") && (!strcmp(nodeName[0], "OO") || !strcmp(nodeName[0], "OP"))) {
			createConnectionList(cnLi);
		}
	}

	fpos_t curpos = 0;
	fgetpos(fp, &curpos);
	//���݂̃t�@�C���|�C���^��EndOffset����O,����
	//���t�@�C���|�C���^����4byte���S��0�ł͂Ȃ��ꍇ, �q�m�[�h�L��
	if (EndOffset > UINT(curpos) && convertBYTEtoUINT(fp) != 0) {
		UINT topChildPointer = UINT(curpos);
		UINT childEndOffset = 0;
		//�q�m�[�hEndOffset�����ǂ�,���J�E���g
		do {
			fseek(fp, sizeof(char) * -4, SEEK_CUR);//"convertBYTEtoUINT(fp) != 0"�̕��߂�
			NumChildren++;
			childEndOffset = convertBYTEtoUINT(fp);
			fseek(fp, sizeof(char) * childEndOffset, SEEK_SET);
		} while (EndOffset > childEndOffset && convertBYTEtoUINT(fp) != 0);
		//�J�E���g���I������̂ōŏ��̎q�m�[�h�̃t�@�C���|�C���^�ɖ߂�
		fseek(fp, sizeof(char) * topChildPointer, SEEK_SET);
		nodeChildren = new NodeRecord[NumChildren];
		for (UINT i = 0; i < NumChildren; i++) {
			nodeChildren[i].set(fp, cn, cnLi);
		}
	}
	//�ǂݍ��݂��I�������̂�EndOffset�փ|�C���^�ړ�
	fseek(fp, sizeof(char) * EndOffset, SEEK_SET);
}

NodeRecord::~NodeRecord() {
	aDELETE(className);
	aDELETE(Property);
	for (int i = 0; i < NUMNODENAME; i++) {
		aDELETE(nodeName[i]);
	}
	for (UINT i = 0; i < NumConnectionNode; i++)
		connectionNode[i] = nullptr;
	aDELETE(nodeChildren);
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

bool FbxLoader::fileCheck(FILE *fp) {

	char *str2 = "Kaydata FBX binary";

	int missCnt = 0;
	for (int i = 0; i < Kaydata_FBX_binary + 1; i++) {
		if ((char)fgetc(fp) != *str2) {
			missCnt++;
			if (missCnt > 3) {
				//�\�t�g�ɂ���ăX�y���~�X���L��̂łƂ肠����3�����ȏ�قȂ�ꍇfalse�ɂ���
				//�ʃt�@�C���̏ꍇ�قڑS�����Ă͂܂�Ȃ��͂�
				MessageBoxA(0, "�o�C�i���t�@�C���ł͂���܂���", 0, MB_OK);
				return false;
			}
		}
		str2++;
	}
	//0-20�o�C�g �wKaydata FBX binary  [null]�x
	//21-22�o�C�g 0x1a, 0x00
	//23-26�o�C�g�܂�(4�o�C�g��): �����Ȃ�����,�o�[�W������\��
	fseek(fp, sizeof(char) * 23, SEEK_SET);//�o�C�i���ł�SEEK_END�͕s��

	return true;
}

void FbxLoader::searchVersion(FILE *fp) {
	//�o�[�W������23-26�o�C�g�܂�(4�o�C�g��)���g���G���f�B�A��(���̈ʂ���ǂ�ł���)
	version = convertBYTEtoUINT(fp);
}

void FbxLoader::readFBX(FILE *fp) {
	fpos_t curpos = 0;
	fgetpos(fp, &curpos);

	UINT nodeCount = 0;
	UINT endoffset = 0;

	while (convertBYTEtoUINT(fp) != 0) {
		fseek(fp, sizeof(char) * -4, SEEK_CUR);//"convertBYTEtoUINT(fp) != 0"�̕��߂�
		nodeCount++;
		endoffset = convertBYTEtoUINT(fp);
		fseek(fp, sizeof(char) * endoffset, SEEK_SET);
	}
	fseek(fp, sizeof(char) * curpos, SEEK_SET);

	FbxRecord.classNameLen = 9;
	FbxRecord.className = new char[FbxRecord.classNameLen + 1];
	strcpy(FbxRecord.className, "FbxRecord");
	FbxRecord.NumChildren = nodeCount;
	FbxRecord.nodeChildren = new NodeRecord[nodeCount];

	for (UINT i = 0; i < nodeCount; i++) {
		FbxRecord.nodeChildren[i].set(fp, cnNo, cnLi);
	}

	//�m�[�hID�̒ʂ�Ƀm�[�h���q����
	for (int i = 0; i < cnLi.size(); i++) {
		for (int j = 0; j < cnNo.size(); j++) {
			if (cnLi.data()[i].ParentID == cnNo.data()[j].ConnectionID) {//�e�m�[�h�̃|�C���^����
				for (int j1 = 0; j1 < cnNo.size(); j1++) {
					if (cnLi.data()[i].ChildID == cnNo.data()[j1].ConnectionID) {//�q�m�[�h�̃|�C���^����
						//�q�m�[�h�|�C���^���o
						NodeRecord *childP = cnNo.data()[j1].ConnectionIDPointer;
						//�e�m�[�h�|�C���^����e�m�[�h�փA�N�Z�X,
						//�e�m�[�h��connectionNode�֎q�m�[�h�|�C���^�ǉ�
						NodeRecord *cp = cnNo.data()[j].ConnectionIDPointer;
						cp->connectionNode[cp->NumConnectionNode++] = childP;
					}
				}
			}
		}
	}
	//rootNode�ݒ�
	for (int i = 0; i < cnNo.size(); i++) {
		if (cnNo.data()[i].ConnectionID == 0) {
			rootNode = cnNo.data()[i].ConnectionIDPointer;
			break;
		}
	}
}

void FbxLoader::Decompress(NodeRecord *node, UCHAR **output, UINT *outSize, UINT typeSize) {
	//�^1byte, �z��4byte, ���k�L��4byte, �T�C�Y4byte, ���^data2byte �v15byte��data
	UINT comp = convertUCHARtoUINT(&node->Property[5]);//���k�L��
	UINT meta = 0;
	if (comp == 1)meta = 2;
	UINT inSize = convertUCHARtoUINT(&node->Property[9]) - meta;//���^data������
	*outSize = convertUCHARtoUINT(&node->Property[1]);
	*output = new UCHAR[(*outSize) * typeSize];
	if (comp == 1) {
		dd.getDecompressArray(&node->Property[15], inSize, *output);//��
	}
	else {
		memcpy(*output, &node->Property[13], (*outSize) * typeSize);//�𓀖����̏ꍇ���^data2byte����
	}
}

void FbxLoader::getLayerElementSub(NodeRecord *node, LayerElement *le) {

	for (UINT i = 0; i < node->NumChildren; i++) {
		NodeRecord *n1 = &node->nodeChildren[i];

		if (!strcmp(n1->className, "Name")) {
			UINT size = convertUCHARtoUINT(&n1->Property[1]);
			if (size > 0) {
				le->name = new char[size + 1];
				UCHAR *pr = &n1->Property[5];
				memcpy(le->name, (char*)pr, size);
				le->name[size] = '\0';
			}
		}

		if (!strcmp(n1->className, "MappingInformationType")) {
			UINT size = convertUCHARtoUINT(&n1->Property[1]);
			if (size > 0) {
				le->MappingInformationType = new char[size + 1];
				UCHAR *pr = &n1->Property[5];
				memcpy(le->MappingInformationType, (char*)pr, size);
				le->MappingInformationType[size] = '\0';
			}
		}

		if (!strcmp(n1->className, "Materials")) {
			UCHAR *output = nullptr;
			UINT outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(INT32));
			le->Nummaterialarr = outSize;
			le->materials = new INT32[outSize];
			ConvertUCHARtoINT32(output, le->materials, outSize);
			aDELETE(output);
		}

		if (!strcmp(n1->className, "Normals")) {
			UCHAR *output = nullptr;
			UINT outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(double));
			le->Numnormals = outSize;
			le->normals = new double[outSize];
			ConvertUCHARtoDouble(output, le->normals, outSize);
			aDELETE(output);
		}

		if (!strcmp(n1->className, "UV")) {
			UCHAR *output = nullptr;
			UINT outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(double));
			le->NumUV = outSize;
			le->UV = new double[outSize];
			ConvertUCHARtoDouble(output, le->UV, outSize);
			aDELETE(output);
		}

		if (!strcmp(n1->className, "UVIndex")) {
			UCHAR *output = nullptr;
			UINT outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(INT32));
			le->NumUVindex = outSize;
			le->UVindex = new INT32[outSize];
			ConvertUCHARtoINT32(output, le->UVindex, outSize);
			aDELETE(output);
		}
	}
}

void FbxLoader::getLayerElement(NodeRecord *node, FbxMeshNode *mesh) {
	if (!strcmp(node->className, "LayerElementMaterial")) {
		INT32 No = convertUCHARtoINT32(&node->Property[1]);//���Ԃ񃌃C���[No�擾
		mesh->Material[No] = new LayerElement();
		LayerElement *mat = mesh->Material[No];
		INT32 Numl = No + 1;
		if (Numl > mesh->NumMaterial)mesh->NumMaterial = Numl;
		getLayerElementSub(node, mat);
	}
	if (!strcmp(node->className, "LayerElementNormal")) {
		INT32 No = convertUCHARtoINT32(&node->Property[1]);
		mesh->Normals[No] = new LayerElement();
		LayerElement *nor = mesh->Normals[No];
		getLayerElementSub(node, nor);
	}
	if (!strcmp(node->className, "LayerElementUV")) {
		INT32 No = convertUCHARtoINT32(&node->Property[1]);
		mesh->UV[No] = new LayerElement();
		LayerElement *uv = mesh->UV[No];
		getLayerElementSub(node, uv);
	}
}

void FbxLoader::getGeometry(NodeRecord *node, FbxMeshNode *mesh) {
	if (!strcmp(node->className, "Geometry")) {
		for (UINT i = 0; i < node->NumChildren; i++) {
			NodeRecord *n1 = &node->nodeChildren[i];

			if (!strcmp(n1->className, "Vertices") && mesh->vertices == nullptr) {
				UCHAR *output = nullptr;
				UINT outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(double));
				mesh->NumVertices = outSize / 3;
				mesh->vertices = new double[outSize];
				ConvertUCHARtoDouble(output, mesh->vertices, outSize);
				aDELETE(output);
			}

			if (!strcmp(n1->className, "PolygonVertexIndex") && mesh->polygonVertices == nullptr) {
				UCHAR *output = nullptr;
				UINT outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(INT32));
				mesh->NumPolygonVertices = outSize;
				mesh->polygonVertices = new INT32[outSize];
				ConvertUCHARtoINT32(output, mesh->polygonVertices, outSize);
				aDELETE(output);
				for (UINT i1 = 0; i1 < mesh->NumPolygonVertices; i1++) {
					if (mesh->polygonVertices[i1] < 0) {
						//�|���S�����̍ŏI�C���f�b�N�X��bit���]����Ă�̂�
						//�����Ń|���S�������J�E���g
						mesh->NumPolygon++;
					}
				}
				mesh->PolygonSize = new UINT[mesh->NumPolygon];
				UINT polCnt = 0;
				UINT PolygonSizeIndex = 0;
				for (UINT i1 = 0; i1 < mesh->NumPolygonVertices; i1++) {
					polCnt++;
					if (mesh->polygonVertices[i1] < 0) {
						mesh->polygonVertices[i1] = ~mesh->polygonVertices[i1];//bit���]
						mesh->PolygonSize[PolygonSizeIndex++] = polCnt;
						polCnt = 0;
					}
				}
			}

			getLayerElement(n1, mesh);

		}
	}
}

void FbxLoader::getMesh() {
	for (UINT i = 0; i < rootNode->NumConnectionNode; i++) {
		if (!strcmp(rootNode->connectionNode[i]->className, "Model") &&
			!strcmp(rootNode->connectionNode[i]->nodeName[1], "Mesh")) {
			NumMesh++;
		}
	}
	mesh = new FbxMeshNode[NumMesh];

	UINT cnt = 0;
	for (UINT i = 0; i < rootNode->NumConnectionNode; i++) {
		NodeRecord *n1 = rootNode->connectionNode[i];
		if (!strcmp(n1->className, "Model") &&
			!strcmp(n1->nodeName[1], "Mesh")) {
			int len = strlen(n1->nodeName[0]);
			mesh[cnt].name = new char[len + 1];
			strcpy_s(mesh[cnt].name, len + 1, n1->nodeName[0]);
			for (UINT i1 = 0; i1 < n1->NumConnectionNode; i1++) {
				NodeRecord *n2 = n1->connectionNode[i1];
				getGeometry(n2, &mesh[cnt]);
			}
			cnt++;
		}
	}
	//UV����
	for (UINT i = 0; i < NumMesh; i++) {
		for (INT32 i1 = 0; i1 < mesh[i].NumMaterial; i1++) {
			LayerElement *uv = mesh[i].UV[i1];
			uv->AlignedUV = new double[uv->NumUVindex * 2];
			UINT cnt = 0;
			for (UINT i2 = 0; i2 < uv->NumUVindex; i2++) {
				uv->AlignedUV[cnt++] = uv->UV[uv->UVindex[i2] * 2];//UVindex��UV��2�l����g�Ƃ��ẴC���f�b�N�X�Ȃ̂Ł~2�Ōv�Z
				uv->AlignedUV[cnt++] = uv->UV[uv->UVindex[i2] * 2 + 1];
			}
		}
	}
}

void FbxLoader::ConvertUCHARtoDouble(UCHAR *arr, double *outArr, UINT outsize) {
	for (UINT i = 0; i < outsize; i++) {
		int addInd = i * 8;
		UINT64 tmp = (((UINT64)arr[7 + addInd] << 56) | ((UINT64)arr[6 + addInd] << 48) |
			((UINT64)arr[5 + addInd] << 40) | ((UINT64)arr[4 + addInd] << 32) |
			((UINT64)arr[3 + addInd] << 24) | ((UINT64)arr[2 + addInd] << 16) |
			((UINT64)arr[1 + addInd] << 8) | ((UINT64)arr[addInd]));
		//byte�񂻂̂܂܂Ō^�ϊ�����
		double *dp = reinterpret_cast<double*>(&tmp);
		outArr[i] = *dp;
	}
}

void FbxLoader::ConvertUCHARtoINT32(UCHAR *arr, INT32 *outArr, UINT outsize) {
	for (UINT i = 0; i < outsize; i++) {
		int addInd = i * 4;
		outArr[i] = (((INT32)arr[3 + addInd] << 24) | ((INT32)arr[2 + addInd] << 16) |
			((INT32)arr[1 + addInd] << 8) | ((INT32)arr[addInd]));
	}
}

void FbxLoader::drawname(NodeRecord *node, bool cnNode) {
	static UINT level = 0;
	for (UINT j = 0; j < level; j++) {
		std::cout << "    " << std::flush;
	}
	std::cout << level << " " << node->className << ":" << std::flush;
	if (node->thisConnectionID != -1)std::cout << node->thisConnectionID << std::flush;
	std::cout << std::endl;
	for (int i = 0; i < NUMNODENAME; i++) {
		if (node->nodeName[i]) {
			for (UINT j = 0; j < level; j++) {
				std::cout << "    " << std::flush;
			}
			std::cout << " " << level << " " << node->nodeName[i] << std::endl;
		}
	}
	level++;
	for (UINT i = 0; i < node->NumChildren; i++) {
		drawname(&node->nodeChildren[i], cnNode);
	}
	if (cnNode) {
		for (UINT i = 0; i < node->NumConnectionNode; i++) {
			drawname(node->connectionNode[i], cnNode);
		}
	}
	level--;
	getchar();
}

FbxLoader::~FbxLoader() {
	aDELETE(mesh);
	std::vector<ConnectionNo>().swap(cnNo);//���
	std::vector<ConnectionList>().swap(cnLi);//���
	rootNode = nullptr;
}

bool FbxLoader::setFbxFile(char *pass) {
	FILE *fp;
	if (fopen_s(&fp, pass, "rb") != 0) {
		MessageBoxA(0, "�t�@�C���ǂݍ��݃G���[", 0, MB_OK);
		return false;
	}

	if (!fileCheck(fp))return false;
	searchVersion(fp);
	readFBX(fp);
	fclose(fp);
	getMesh();
	return true;
}

NodeRecord *FbxLoader::getFbxRecord() {
	return &FbxRecord;
}

NodeRecord *FbxLoader::getRootNode() {
	return rootNode;
}

UINT FbxLoader::getNumFbxMeshNode() {
	return NumMesh;
}

FbxMeshNode **FbxLoader::getFbxMeshNode() {
	return &mesh;
}

int FbxLoader::getVersion() {
	return version;
}

void FbxLoader::drawRecord() {
	drawname(&FbxRecord, false);
}

void FbxLoader::drawNode() {
	drawname(rootNode, true);
}