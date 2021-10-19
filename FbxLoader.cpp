//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　         FbxLoader                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "FbxLoader.h"
#include "iostream"
#include "../DecompressDeflate/DecompressDeflate.h"
#define Kaydara_FBX_binary 18

FilePointer::~FilePointer() {
	if (fileStr) {
		delete[] fileStr;
		fileStr = nullptr;
	}
}

bool FilePointer::setFile(char* pass) {
	FILE* fp = fopen(pass, "rb");
	if (fp == NULL) {
		return false;
	}
	unsigned int count = 0;
	while (fgetc(fp) != EOF) {
		count++;
	}
	count++;//EOFの分
	fseek(fp, 0, SEEK_SET);//最初に戻す
	fileStr = new unsigned char[count];
	fread((char*)fileStr, sizeof(unsigned char), count, fp);
	fclose(fp);
	return true;
}

void FilePointer::setCharArray(char *cArray, int size) {
	fileStr = new unsigned char[size];
	memcpy(fileStr, cArray, size * sizeof(unsigned char));
}

unsigned int FilePointer::getPos() {
	return pointer;
}

void FilePointer::seekPointer(unsigned int ind) {
	pointer = ind;
}

unsigned char FilePointer::getByte() {
	unsigned int ret = fileStr[pointer];
	pointer++;
	return ret;
}

void FilePointer::fRead(char *dst, int byteSize) {
	memcpy(dst, &fileStr[pointer], byteSize * sizeof(unsigned char));
	pointer += byteSize;
}

unsigned int FilePointer::convertBYTEtoUINT() {
	unsigned int ret = (fileStr[3 + pointer] << 24) | (fileStr[2 + pointer] << 16) |
		(fileStr[1 + pointer] << 8) | (fileStr[0 + pointer]);
	pointer += 4;
	return ret;
}

uint64_t FilePointer::convertBYTEtoUINT64() {
	uint64_t ret = ((uint64_t)fileStr[7 + pointer] << 56) | ((uint64_t)fileStr[6 + pointer] << 48) |
		((uint64_t)fileStr[5 + pointer] << 40) | ((uint64_t)fileStr[4 + pointer] << 32) |
		((uint64_t)fileStr[3 + pointer] << 24) | ((uint64_t)fileStr[2 + pointer] << 16) |
		((uint64_t)fileStr[1 + pointer] << 8) | ((uint64_t)fileStr[0 + pointer]);
	pointer += 8;
	return ret;
}

unsigned int FilePointer::convertBYTEtoUINT32or64() {
	if (versionSw)return (unsigned int)convertBYTEtoUINT64();
	return convertBYTEtoUINT();
}

void FilePointer::backPointer4or8() {
	if (versionSw)
		pointer -= 8;
	else
		pointer -= 4;
}

static unsigned int convertUCHARtoUINT(unsigned char* arr) {
	return ((unsigned int)arr[3] << 24) | ((unsigned int)arr[2] << 16) |
		((unsigned int)arr[1] << 8) | ((unsigned int)arr[0]);
}

static int convertUCHARtoINT32(unsigned char* arr) {
	return ((int)arr[3] << 24) | ((int)arr[2] << 16) |
		((int)arr[1] << 8) | ((int)arr[0]);
}

static int64_t convertUCHARtoint64(unsigned char* arr) {
	return ((int64_t)arr[7] << 56) | ((int64_t)arr[6] << 48) |
		((int64_t)arr[5] << 40) | ((int64_t)arr[4] << 32) |
		((int64_t)arr[3] << 24) | ((int64_t)arr[2] << 16) |
		((int64_t)arr[1] << 8) | ((int64_t)arr[0]);
}

static unsigned long long convertUCHARtoUINT64(unsigned char* arr) {
	return ((unsigned long long)arr[7] << 56) | ((unsigned long long)arr[6] << 48) |
		((unsigned long long)arr[5] << 40) | ((unsigned long long)arr[4] << 32) |
		((unsigned long long)arr[3] << 24) | ((unsigned long long)arr[2] << 16) |
		((unsigned long long)arr[1] << 8) | ((unsigned long long)arr[0]);
}

static double convertUCHARtoDouble(unsigned char* arr) {
	unsigned long long tmp = ((unsigned long long)arr[7] << 56) | ((unsigned long long)arr[6] << 48) |
		((unsigned long long)arr[5] << 40) | ((unsigned long long)arr[4] << 32) |
		((unsigned long long)arr[3] << 24) | ((unsigned long long)arr[2] << 16) |
		((unsigned long long)arr[1] << 8) | ((unsigned long long)arr[0]);
	//byte列そのままで型変換する
	double* dp = reinterpret_cast<double*>(&tmp);
	return *dp;
}

void NodeRecord::searchName_Type(std::vector<ConnectionNo>& cn, uint64_t PropertyListLen) {
	int swt = 0;
	unsigned int ln = 0;
	int nameNo = 0;
	int addInd = 0;
	unsigned char* pr = nullptr;
	unsigned char sw = 0;
	for (unsigned int i = 0; i < PropertyListLen; i++) {
		switch (swt) {
		case 3:
			//Lの処理
			if (!strcmp("Time", &this->className[this->classNameLen - 4])) {
				i += 7;
				swt = 0;
				continue;
			}
			if (thisConnectionID == -1 && i == 1) {//ConnectionIDは一個目のProperty
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
			//特殊型の場合は
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
			Contents         可変  CompressedLenghtにサイズが入ってる

			配列の場合これらのbyte列分スキップする
			Property配列は9byte分要素をずらしCompressedLenghtにアクセスしContentsサイズを取り出す
			取り出したサイズ+ 4 * 3バイト分"i"を進める
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

void NodeRecord::createConnectionList(std::vector<ConnectionList>& cnLi, char* nodeName1) {
	ConnectionList cl;
	//S len "OO" L 計8byteの次にChildID
	cl.ChildID = convertUCHARtoint64(&Property[8]);
	//L 1byteの次にParentID
	cl.ParentID = convertUCHARtoint64(&Property[17]);
	if (nodeName1) {
		if (!strcmp("NormalMap", nodeName1))cl.texType.NormalMap = true;
		if (!strcmp("DiffuseColor", nodeName1))cl.texType.DiffuseColor = true;
		if (!strcmp("SpecularColor", nodeName1))cl.texType.SpecularColor = true;
		if (!strcmp("SpecularFactor", nodeName1))cl.texType.SpecularFactor = true;
		if (!strcmp("EmissiveFactor", nodeName1))cl.texType.EmissiveFactor = true;
	}
	cnLi.push_back(cl);
}

void NodeRecord::set(FilePointer* fp, std::vector<ConnectionNo>& cn, std::vector<ConnectionList>& cnLi) {
	unsigned int EndOffset = 0;//次のファイルの先頭バイト数 
	unsigned int NumProperties = 0;//プロパティの数
	unsigned int PropertyListLen = 0;//プロパティリストの大きさ(byte)
	//上記3パラメーターはFBX7400以下4byte, FBX7500以上は8byte
	EndOffset = fp->convertBYTEtoUINT32or64();
	NumProperties = fp->convertBYTEtoUINT32or64();
	PropertyListLen = fp->convertBYTEtoUINT32or64();

	classNameLen = fp->getByte();
	className = new char[classNameLen + 1];
	fp->fRead(className, classNameLen);
	className[classNameLen] = '\0';
	if (PropertyListLen > 0) {
		Property = new unsigned char[PropertyListLen];
		fp->fRead((char*)Property, PropertyListLen);
		searchName_Type(cn, PropertyListLen);
		if (!strcmp(className, "C") && (!strcmp(nodeName[0], "OO") || !strcmp(nodeName[0], "OP"))) {
			createConnectionList(cnLi, nodeName[1]);
		}
	}

	unsigned int curpos = fp->getPos();
	//現在のファイルポインタがEndOffsetより手前,かつ
	//現ファイルポインタから4byteが全て0ではない場合, 子ノード有り
	if (EndOffset > curpos && fp->convertBYTEtoUINT32or64() != 0) {
		unsigned int topChildPointer = curpos;
		unsigned int childEndOffset = 0;
		//子ノードEndOffsetをたどり,個数カウント
		do {
			fp->backPointer4or8();//"convertBYTEtoUINT() != 0"の分戻す
			NumChildren++;
			childEndOffset = fp->convertBYTEtoUINT32or64();
			fp->seekPointer(childEndOffset);
		} while (EndOffset > childEndOffset && fp->convertBYTEtoUINT32or64() != 0);
		//カウントが終わったので最初の子ノードのファイルポインタに戻す
		fp->seekPointer(topChildPointer);
		if (NumChildren > 0)nodeChildren = new NodeRecord[NumChildren];
		for (unsigned int i = 0; i < NumChildren; i++) {
			nodeChildren[i].set(fp, cn, cnLi);
		}
	}
	//読み込みが終了したのでEndOffsetへポインタ移動
	fp->seekPointer(EndOffset);
}

NodeRecord::~NodeRecord() {
	aDELETE(className);
	aDELETE(Property);
	for (int i = 0; i < NUMNODENAME; i++) {
		aDELETE(nodeName[i]);
	}
	std::vector<NodeRecord*>().swap(connectionNode);//解放
	aDELETE(nodeChildren);
}

bool FbxLoader::fileCheck(FilePointer *fp) {

	char *str2 = "Kaydara FBX binary";

	int missCnt = 0;
	for (int i = 0; i < Kaydara_FBX_binary + 1; i++) {
		if ((char)fp->getByte() != *str2) {
			missCnt++;
			if (missCnt > 3) {
				//ソフトによってスペルミスが有るのでとりあえず3文字以上異なる場合falseにする
				//別ファイルの場合ほぼ全数当てはまらないはず
				return false;
			}
		}
		str2++;
	}
	//0-20バイト 『Kaydara FBX binary  [null]』
	//21-22バイト 0x1a, 0x00
	//23-26バイトまで(4バイト分): 符号なし整数,バージョンを表す
	fp->seekPointer(23);//バイナリではSEEK_ENDは不定

	return true;
}

void FbxLoader::searchVersion(FilePointer* fp) {
	//バージョンは23-26バイトまで(4バイト分)リトルエンディアン(下の位から読んでいく)
	version = fp->convertBYTEtoUINT();
	if (version >= 7500)fp->versionSw = true;
}

void FbxLoader::readFBX(FilePointer* fp) {
	unsigned int curpos = fp->getPos();

	unsigned int nodeCount = 0;
	unsigned int endoffset = 0;

	while (fp->convertBYTEtoUINT32or64() != 0) {
		fp->backPointer4or8();//"convertBYTEtoUINT() != 0"の分戻す
		nodeCount++;
		endoffset = fp->convertBYTEtoUINT32or64();
		fp->seekPointer(endoffset);
	}

	fp->seekPointer(curpos);

	FbxRecord.classNameLen = 9;
	FbxRecord.className = new char[FbxRecord.classNameLen + 1];
	strcpy(FbxRecord.className, "FbxRecord");
	FbxRecord.NumChildren = nodeCount;
	FbxRecord.nodeChildren = new NodeRecord[nodeCount];

	for (unsigned int i = 0; i < nodeCount; i++) {
		FbxRecord.nodeChildren[i].set(fp, cnNo, cnLi);
		NodeRecord* n1 = &FbxRecord.nodeChildren[i];
		if (!strcmp(n1->className, "Definitions")) {
			for (unsigned int i1 = 0; i1 < n1->NumChildren; i1++) {
				NodeRecord* n2 = &n1->nodeChildren[i1];
				if (n2->nodeName[0] && !strcmp(n2->nodeName[0], "AnimationLayer")) {
					for (unsigned int i2 = 0; i2 < n2->NumChildren; i2++) {
						NodeRecord* n3 = &n2->nodeChildren[i2];
						if (!strcmp(n3->className, "Count")) {
							Deformer::numAnimation = convertUCHARtoINT32(&n3->Property[1]);
							numAnimation = Deformer::numAnimation;
						}
					}
				}
			}
		}
		if (!strcmp(n1->className, "GlobalSettings")) {
			for (unsigned int i1 = 0; i1 < n1->NumChildren; i1++) {
				NodeRecord* n2 = &n1->nodeChildren[i1];
				if (!strcmp(n2->className, "Properties70")) {
					for (unsigned int i2 = 0; i2 < n2->NumChildren; i2++) {
						NodeRecord* n3 = &n2->nodeChildren[i2];
						getPropertiesInt(n3, Gset.UpAxis, "UpAxis");
						getPropertiesInt(n3, Gset.UpAxisSign, "UpAxisSign");
						getPropertiesInt(n3, Gset.FrontAxis, "FrontAxis");
						getPropertiesInt(n3, Gset.FrontAxisSign, "FrontAxisSign");
						getPropertiesInt(n3, Gset.CoordAxis, "CoordAxis");
						getPropertiesInt(n3, Gset.CoordAxisSign, "CoordAxisSign");
						getPropertiesInt(n3, Gset.OriginalUpAxis, "OriginalUpAxis");
						getPropertiesInt(n3, Gset.OriginalUpAxisSign, "OriginalUpAxisSign");
						getPropertiesDouble(n3, &Gset.UnitScaleFactor, 1, "UnitScaleFactor");
						getPropertiesDouble(n3, &Gset.OriginalUnitScaleFactor, 1, "OriginalUnitScaleFactor");
						getPropertiesDouble(n3, Gset.AmbientColor, 3, "AmbientColor");
						getPropertiesInt(n3, Gset.TimeMode, "TimeMode");
						getPropertiesint64(n3, Gset.TimeSpanStart, "TimeSpanStart");
						getPropertiesint64(n3, Gset.TimeSpanStop, "TimeSpanStop");
						getPropertiesDouble(n3, &Gset.CustomFrameRate, 1, "CustomFrameRate");
					}
				}
			}
		}
	}

	//ノードIDの通りにノードを繋げる
	for (int i = 0; i < cnLi.size(); i++) {
		for (int j = 0; j < cnNo.size(); j++) {
			if (cnLi[i].ParentID == cnNo[j].ConnectionID) {//親ノードのポインタ検索
				for (int j1 = 0; j1 < cnNo.size(); j1++) {
					if (cnLi[i].ChildID == cnNo[j1].ConnectionID) {//子ノードのポインタ検索
						//子ノードポインタ抽出
						NodeRecord* childP = cnNo[j1].ConnectionIDPointer;
						//テクスチャType登録
						if (cnLi[i].texType.NormalMap)childP->texType.NormalMap = true;
						if (cnLi[i].texType.DiffuseColor)childP->texType.DiffuseColor = true;
						if (cnLi[i].texType.SpecularColor)childP->texType.SpecularColor = true;
						if (cnLi[i].texType.SpecularFactor)childP->texType.SpecularFactor = true;
						if (cnLi[i].texType.EmissiveFactor)childP->texType.EmissiveFactor = true;
						//親ノードポインタから親ノードへアクセス
						NodeRecord* cp = cnNo[j].ConnectionIDPointer;
						//既に登録済みでないかチェック
						bool add = true;
						for (int cnIndex = 0; cnIndex < cp->connectionNode.size(); cnIndex++) {
							if (cp->connectionNode[cnIndex]->thisConnectionID == childP->thisConnectionID) {
								add = false;
								break;
							}
						}
						//親ノード内connectionNodeへ子ノードポインタ追加
						if (add)cp->connectionNode.push_back(childP);
					}
				}
			}
		}
	}
	//rootNode設定
	for (int i = 0; i < cnNo.size(); i++) {
		if (cnNo[i].ConnectionID == 0) {
			rootNode = cnNo[i].ConnectionIDPointer;
			break;
		}
	}
}

bool FbxLoader::Decompress(NodeRecord *node, unsigned char **output, unsigned int *outSize, unsigned int typeSize) {
	//型1byte, 配列数4byte, 圧縮有無4byte, サイズ4byte, メタdata2byte 計15byte後data
	unsigned int comp = convertUCHARtoUINT(&node->Property[5]);//圧縮有無
	unsigned int meta = 0;
	if (comp == 1)meta = 2;
	unsigned int inSize = convertUCHARtoUINT(&node->Property[9]) - meta;//メタdata分引く
	if (inSize <= 0)return false;
	*outSize = convertUCHARtoUINT(&node->Property[1]);
	*output = new unsigned char[(*outSize) * typeSize];
	if (comp == 1) {
		unsigned char *propertyData = new unsigned char[inSize];
		memcpy(propertyData, &node->Property[15], inSize);
		DecompressDeflate dd;
		dd.getDecompressArray(propertyData, inSize, *output);//解凍
		a_DELETE(propertyData);
	}
	else {
		memcpy(*output, &node->Property[13], (*outSize) * typeSize);//解凍無しの場合メタdata2byte無し
	}
	return true;
}

void FbxLoader::getLayerElementSub(NodeRecord* node, LayerElement* le) {

	for (unsigned int i = 0; i < node->NumChildren; i++) {
		NodeRecord* n1 = &node->nodeChildren[i];

		if (!strcmp(n1->className, "Name")) {
			unsigned int size = convertUCHARtoUINT(&n1->Property[1]);
			if (size > 0) {
				le->name = new char[size + 1];
				unsigned char* pr = &n1->Property[5];
				memcpy(le->name, (char*)pr, size);
				le->name[size] = '\0';
			}
		}

		if (!strcmp(n1->className, "MappingInformationType")) {
			unsigned int size = convertUCHARtoUINT(&n1->Property[1]);
			if (size > 0) {
				le->MappingInformationType = new char[size + 1];
				unsigned char* pr = &n1->Property[5];
				memcpy(le->MappingInformationType, (char*)pr, size);
				le->MappingInformationType[size] = '\0';
			}
		}

		if (!strcmp(n1->className, "ReferenceInformationType")) {
			unsigned int size = convertUCHARtoUINT(&n1->Property[1]);
			if (size > 0) {
				le->ReferenceInformationType = new char[size + 1];
				unsigned char* pr = &n1->Property[5];
				memcpy(le->ReferenceInformationType, (char*)pr, size);
				le->ReferenceInformationType[size] = '\0';
			}
		}

		if (!strcmp(n1->className, "Materials")) {
			unsigned char* output = nullptr;
			unsigned int outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(int));
			le->Nummaterialarr = outSize;
			le->materials = new int[outSize];
			ConvertUCHARtoINT32(output, le->materials, outSize);
			aDELETE(output);
		}

		if (!strcmp(n1->className, "Normals")) {
			unsigned char* output = nullptr;
			unsigned int outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(double));
			le->Numnormals = outSize;
			le->normals = new double[outSize];
			ConvertUCHARtoDouble(output, le->normals, outSize);
			aDELETE(output);
		}

		if (!strcmp(n1->className, "NormalsIndex")) {
			unsigned char* output = nullptr;
			unsigned int outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(int));
			le->NumNormalsIndex = outSize;
			le->NormalsIndex = new int[outSize];
			ConvertUCHARtoINT32(output, le->NormalsIndex, outSize);
			aDELETE(output);
		}

		if (!strcmp(n1->className, "UV")) {
			unsigned char* output = nullptr;
			unsigned int outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(double));
			le->NumUV = outSize;
			le->UV = new double[outSize];
			ConvertUCHARtoDouble(output, le->UV, outSize);
			aDELETE(output);
		}

		if (!strcmp(n1->className, "UVIndex")) {
			unsigned char* output = nullptr;
			unsigned int outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(int));
			le->NumUVindex = outSize;
			le->UVindex = new int[outSize];
			ConvertUCHARtoINT32(output, le->UVindex, outSize);
			aDELETE(output);
		}
	}
}

void FbxLoader::getLayerElementCounter(NodeRecord* node, FbxMeshNode* mesh) {
	if (!strcmp(node->className, "LayerElementUV")) {
		mesh->NumUVObj++;
	}
}

void FbxLoader::getLayerElement(NodeRecord* node, FbxMeshNode* mesh) {
	if (!strcmp(node->className, "LayerElementMaterial")) {
		int No = convertUCHARtoINT32(&node->Property[1]);//たぶんレイヤーNo取得
		mesh->Material[No] = new LayerElement();
		LayerElement* mat = mesh->Material[No];
		getLayerElementSub(node, mat);
	}
	if (!strcmp(node->className, "LayerElementNormal")) {
		int No = convertUCHARtoINT32(&node->Property[1]);
		mesh->Normals[No] = new LayerElement();
		LayerElement* nor = mesh->Normals[No];
		getLayerElementSub(node, nor);
		mesh->NumNormalsObj++;
	}
	if (!strcmp(node->className, "LayerElementUV")) {
		int No = convertUCHARtoINT32(&node->Property[1]);
		mesh->UV[No] = new LayerElement();
		LayerElement* uv = mesh->UV[No];
		getLayerElementSub(node, uv);
	}
}

bool FbxLoader::nameComparison(char *name1, char *name2) {
	//名前文字列に空白文字が有る場合,空白文字以前を取り除く
	char *name1Tmp = name1;
	do {
		while (*name1Tmp != ' ' && *name1Tmp != '\0') {
			name1Tmp++;
		}
		if (*name1Tmp == '\0') {
			break;
		}
		else {
			name1Tmp++;
			name1 = name1Tmp;
		}
	} while (1);

	char *name2Tmp = name2;
	do {
		while (*name2Tmp != ' ' && *name2Tmp != '\0') {
			name2Tmp++;
		}
		if (*name2Tmp == '\0') {
			break;
		}
		else {
			name2Tmp++;
			name2 = name2Tmp;
		}
	} while (1);

	//名前が一致してるか
	int len1 = (int)strlen(name1);
	int len2 = (int)strlen(name2);
	if (len1 == len2 && !strcmp(name1, name2))return true;
	return false;
}

void FbxLoader::setParentPointerOfSubDeformer(FbxMeshNode* mesh) {
	if (mesh->NumDeformer <= 0)return;
	for (unsigned int i = 0; i < mesh->NumDeformer + 1; i++) {
		Deformer* defo = nullptr;
		if (i < mesh->NumDeformer)
			defo = mesh->deformer[i];
		else
			defo = mesh->rootDeformer;

		for (unsigned int i1 = 0; i1 < defo->NumChild; i1++) {
			for (unsigned int i2 = 0; i2 < mesh->NumDeformer; i2++) {
				//登録した子Deformer名と一致するDeformerに自身のポインタを登録
				if (nameComparison(defo->childName[i1], mesh->deformer[i2]->name)) {
					mesh->deformer[i2]->parentNode = defo;
				}
			}
		}
	}
}

void FbxLoader::getSubDeformer(NodeRecord* node, FbxMeshNode* mesh) {
	//各Deformer情報取得
	if (!strcmp(node->className, "Deformer")) {
		mesh->deformer[mesh->NumDeformer] = new Deformer();
		Deformer* defo = mesh->deformer[mesh->NumDeformer];

		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			NodeRecord* n1 = node->connectionNode[i];
			if (!strcmp(n1->className, "Model")) {//自身のModel
				getLcl(n1, defo->lcl);
				getAnimation(n1, defo);
			}
		}

		//Name登録
		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			NodeRecord* n1 = node->connectionNode[i];
			if (!strcmp(n1->className, "Model")) {//自身のModel
				//自身のName
				int len = (int)strlen(n1->nodeName[0]);
				defo->name = new char[len + 1];
				strcpy(defo->name, n1->nodeName[0]);
				for (unsigned int i1 = 0; i1 < n1->connectionNode.size(); i1++) {
					NodeRecord* n2 = n1->connectionNode[i1];
					if (!strcmp(n2->className, "Model")) {//子ノードのModel
						int ln = (int)strlen(n2->nodeName[0]);
						defo->childName[defo->NumChild] = new char[ln + 1];
						strcpy(defo->childName[defo->NumChild++], n2->nodeName[0]);
					}
				}
			}
		}

		for (unsigned int i = 0; i < node->NumChildren; i++) {
			NodeRecord* n1 = &node->nodeChildren[i];

			//インデックス配列,数
			if (!strcmp(n1->className, "Indexes")) {
				unsigned char* output = nullptr;
				unsigned int outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(int));
				defo->IndicesCount = outSize;
				defo->Indices = new int[outSize];
				ConvertUCHARtoINT32(output, defo->Indices, outSize);
				aDELETE(output);
			}

			//ウエイト
			if (!strcmp(n1->className, "Weights")) {
				unsigned char* output = nullptr;
				unsigned int outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(double));
				defo->Weights = new double[outSize];
				ConvertUCHARtoDouble(output, defo->Weights, outSize);
				aDELETE(output);
			}

			//Transform
			if (!strcmp(n1->className, "Transform")) {
				unsigned char* output = nullptr;
				unsigned int outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(double));
				ConvertUCHARtoDouble(output, defo->TransformMatrix, outSize);
				aDELETE(output);
			}

			//TransformLink
			if (!strcmp(n1->className, "TransformLink")) {
				unsigned char* output = nullptr;
				unsigned int outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(double));
				ConvertUCHARtoDouble(output, defo->TransformLinkMatrix, outSize);
				aDELETE(output);
			}
		}
		mesh->NumDeformer++;//Deformer数カウント
	}
}

void FbxLoader::getDeformer(NodeRecord* node, FbxMeshNode* mesh) {
	if (!strcmp(node->className, "Deformer")) {
		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			NodeRecord* n1 = node->connectionNode[i];
			//各Deformer情報取得
			getSubDeformer(n1, mesh);
		}
	}
}

void FbxLoader::getGeometry(NodeRecord* node, FbxMeshNode* mesh) {
	if (!strcmp(node->className, "Geometry")) {
		int len = (int)strlen(node->nodeName[0]);
		mesh->name = new char[len + 1];
		strcpy(mesh->name, node->nodeName[0]);
		for (unsigned int i = 0; i < node->NumChildren; i++) {
			NodeRecord* n1 = &node->nodeChildren[i];

			//頂点
			if (!strcmp(n1->className, "Vertices") && mesh->vertices == nullptr) {
				unsigned char* output = nullptr;
				unsigned int outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(double));
				mesh->NumVertices = outSize / 3;
				mesh->vertices = new double[outSize];
				ConvertUCHARtoDouble(output, mesh->vertices, outSize);
				aDELETE(output);
			}

			//頂点インデックス
			if (!strcmp(n1->className, "PolygonVertexIndex") && mesh->polygonVertices == nullptr) {
				unsigned char* output = nullptr;
				unsigned int outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(int));
				mesh->NumPolygonVertices = outSize;
				mesh->polygonVertices = new int[outSize];
				ConvertUCHARtoINT32(output, mesh->polygonVertices, outSize);
				aDELETE(output);
				for (unsigned int i1 = 0; i1 < mesh->NumPolygonVertices; i1++) {
					if (mesh->polygonVertices[i1] < 0) {
						//ポリゴン毎の最終インデックスがbit反転されてるので
						//そこでポリゴン数をカウント
						mesh->NumPolygon++;
					}
				}
				mesh->PolygonSize = new unsigned int[mesh->NumPolygon];
				unsigned int polCnt = 0;
				unsigned int PolygonSizeIndex = 0;
				for (unsigned int i1 = 0; i1 < mesh->NumPolygonVertices; i1++) {
					polCnt++;
					if (mesh->polygonVertices[i1] < 0) {
						mesh->polygonVertices[i1] = ~mesh->polygonVertices[i1];//bit反転
						mesh->PolygonSize[PolygonSizeIndex++] = polCnt;
						polCnt = 0;
					}
				}
			}

			//Normal, UV
			getLayerElement(n1, mesh);
		}

		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			NodeRecord* n1 = node->connectionNode[i];

			//ボーン関連
			getDeformer(n1, mesh);
		}
	}
}

void FbxLoader::getMaterial(NodeRecord* node, FbxMeshNode* mesh, unsigned int* materialIndex) {
	if (!strcmp(node->className, "Material")) {
		mesh->material[*materialIndex] = new FbxMaterialNode();
		int len = (int)strlen(node->nodeName[0]);
		mesh->material[*materialIndex]->MaterialName = new char[len + 1];
		strcpy(mesh->material[*materialIndex]->MaterialName, node->nodeName[0]);

		for (unsigned int i = 0; i < node->NumChildren; i++) {
			if (!strcmp(node->nodeChildren[i].className, "Properties70")) {
				NodeRecord* pro70 = &node->nodeChildren[i];
				for (unsigned int i1 = 0; i1 < pro70->NumChildren; i1++) {
					getCol(&pro70->nodeChildren[i1], mesh->material[*materialIndex]->Diffuse, "DiffuseColor");
					getCol(&pro70->nodeChildren[i1], mesh->material[*materialIndex]->Specular, "SpecularColor");
					getCol(&pro70->nodeChildren[i1], mesh->material[*materialIndex]->Ambient, "AmbientColor");
				}
			}
		}
		int difCount = 0;
		bool difCountUpflg = false;
		int norCount = 0;
		bool norCountUpflg = false;
		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			if (!strcmp(node->connectionNode[i]->className, "Texture")) {
				NodeRecord* tex = node->connectionNode[i];
				char* uvName = nullptr;
				int uvNameLen = 0;
				for (unsigned int i1 = 0; i1 < tex->NumChildren; i1++) {
					if (!strcmp(tex->nodeChildren[i1].className, "Properties70")) {
						NodeRecord* pro = &tex->nodeChildren[i1];
						for (unsigned int i2 = 0; i2 < pro->NumChildren; i2++) {
							NodeRecord* proP = &pro->nodeChildren[i2];
							if (!strcmp(proP->className, "P") && proP->nodeName[0] && !strcmp(proP->nodeName[0], "UVSet")) {
								int numCnt = 1;
								for (numCnt = 1; numCnt < NUMNODENAME; numCnt++) {
									if (proP->nodeName[numCnt] == nullptr) {
										numCnt--;
										break;
									}
								}
								//使用UV判別用文字列一時取得
								uvName = proP->nodeName[numCnt];
								if (uvName)uvNameLen = (int)strlen(uvName);
							}
						}
					}
				}

				for (unsigned int i1 = 0; i1 < tex->NumChildren; i1++) {
					if (!strcmp(tex->nodeChildren[i1].className, "FileName")) {
						NodeRecord* texN = &tex->nodeChildren[i1];
						int len = (int)strlen(texN->nodeName[0]);
						if (!tex->texType.NormalMap) {
							TextureName* texName = &mesh->material[*materialIndex]->textureDifName[difCount];
							if (!texName->name) {
								texName->name = new char[len + 1];
								strcpy(texName->name, texN->nodeName[0]);
								texName->type = tex->texType;
								if (uvName) {
									texName->UVname = new char[uvNameLen + 1];
									strcpy(texName->UVname, uvName);
								}
								difCountUpflg = true;
							}
						}
						else {
							TextureName* texName = &mesh->material[*materialIndex]->textureNorName[norCount];
							if (!texName->name) {
								texName->name = new char[len + 1];
								strcpy(texName->name, texN->nodeName[0]);
								if (uvName) {
									texName->UVname = new char[uvNameLen + 1];
									strcpy(texName->UVname, uvName);
								}
								norCountUpflg = true;
							}
						}
					}
				}
			}
			if (difCountUpflg) {
				difCount++;
				difCountUpflg = false;
				mesh->material[*materialIndex]->NumDifTexture = difCount;
			}
			if (norCountUpflg) {
				norCount++;
				norCountUpflg = false;
				mesh->material[*materialIndex]->NumNorTexture = norCount;
			}
		}
		(*materialIndex)++;
	}
}

void FbxLoader::getLcl(NodeRecord* model, Lcl& lcl) {
	for (unsigned int i = 0; i < model->NumChildren; i++) {
		NodeRecord* n1 = &model->nodeChildren[i];
		if (!strcmp(n1->className, "Properties70")) {
			for (unsigned int i1 = 0; i1 < n1->NumChildren; i1++) {
				NodeRecord* n2 = &n1->nodeChildren[i1];
				getPropertiesDouble(n2, lcl.Translation, 3, "Lcl Translation");
				getPropertiesDouble(n2, lcl.Rotation, 3, "Lcl Rotation");
				getPropertiesDouble(n2, lcl.Scaling, 3, "Lcl Scaling");
			}
		}
	}
}

void FbxLoader::checkGeometry(NodeRecord* node, bool check[2]) {
	if (!strcmp(node->className, "Geometry")) {
		for (unsigned int i = 0; i < node->NumChildren; i++) {
			NodeRecord* n1 = &node->nodeChildren[i];

			//頂点
			if (!strcmp(n1->className, "Vertices")) {
				check[0] = true;
			}

			//頂点インデックス
			if (!strcmp(n1->className, "PolygonVertexIndex")) {
				check[1] = true;
			}
		}
	}
}

void FbxLoader::checkMaterial(NodeRecord* node, bool* check) {
	if (!strcmp(node->className, "Material")) {
		*check = true;
	}
}

bool FbxLoader::checkMeshNodeRecord(NodeRecord* node) {
	bool check[3] = { false,false,false };
	for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
		NodeRecord* n = node->connectionNode[i];
		checkGeometry(n, check);
		checkMaterial(n, &check[2]);
	}
	if (check[0] && check[1] && check[2])return true;
	return false;
}

void FbxLoader::getSubDeformerCounter(NodeRecord* node, FbxMeshNode* mesh) {
	if (!strcmp(node->className, "Deformer")) {
		mesh->NumDeformer++;//Deformer数カウント
	}
}

void FbxLoader::getDeformerCounter(NodeRecord* node, FbxMeshNode* mesh) {
	if (!strcmp(node->className, "Deformer")) {
		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			NodeRecord* n1 = node->connectionNode[i];
			//各Deformer情報取得
			getSubDeformerCounter(n1, mesh);
		}
	}
}

void FbxLoader::getGeometryCounter(NodeRecord* node, FbxMeshNode* mesh) {
	if (!strcmp(node->className, "Geometry")) {
		for (unsigned int i = 0; i < node->NumChildren; i++) {
			NodeRecord* n1 = &node->nodeChildren[i];

			//Normal, UV
			getLayerElementCounter(n1, mesh);
		}

		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			NodeRecord* n1 = node->connectionNode[i];

			//ボーン関連
			getDeformerCounter(n1, mesh);
		}
	}
}

void FbxLoader::getMaterialCounter(NodeRecord* node, FbxMeshNode* mesh) {
	if (!strcmp(node->className, "Material")) {
		mesh->NumMaterial++;
	}
}

void FbxLoader::meshCount(NodeRecord* n, unsigned int& numMesh) {
	for (unsigned int i = 0; i < n->connectionNode.size(); i++) {
		NodeRecord* n1 = n->connectionNode[i];
		if (!strcmp(n1->className, "Model")) {
			if (!strcmp(n1->nodeName[1], "Mesh")) {
				if (checkMeshNodeRecord(n1))numMesh++;
			}
			else {
				meshCount(n1, numMesh);
			}
		}
	}
}

void FbxLoader::geometryCount(NodeRecord* n, FbxMeshNode* meshArr, unsigned int& meshCounter) {
	for (unsigned int i = 0; i < n->connectionNode.size(); i++) {
		NodeRecord* n1 = n->connectionNode[i];
		if (!strcmp(n1->className, "Model")) {
			if (!strcmp(n1->nodeName[1], "Mesh")) {
				if (checkMeshNodeRecord(n1)) {
					FbxMeshNode& mesh = meshArr[meshCounter];
					getLcl(n1, mesh.lcl);
					for (unsigned int i1 = 0; i1 < n1->connectionNode.size(); i1++) {
						NodeRecord* n2 = n1->connectionNode[i1];
						getGeometryCounter(n2, &mesh);
						getMaterialCounter(n2, &mesh);
					}
					meshCounter++;
				}
			}
			else {
				geometryCount(n1, meshArr, meshCounter);
			}
		}
	}
}

void FbxLoader::searchGeometry(NodeRecord* n, FbxMeshNode* meshArr, unsigned int& meshCounter, unsigned int& materialCounter) {
	for (unsigned int i = 0; i < n->connectionNode.size(); i++) {
		NodeRecord* n1 = n->connectionNode[i];
		if (!strcmp(n1->className, "Model")) {
			if (!strcmp(n1->nodeName[1], "Mesh")) {
				if (checkMeshNodeRecord(n1)) {
					FbxMeshNode& mesh = meshArr[meshCounter];
					getLcl(n1, mesh.lcl);
					for (unsigned int i1 = 0; i1 < n1->connectionNode.size(); i1++) {
						NodeRecord* n2 = n1->connectionNode[i1];
						getGeometry(n2, &mesh);
						getMaterial(n2, &mesh, &materialCounter);
					}
					meshCounter++;
					materialCounter = 0;
				}
			}
			else {
				searchGeometry(n1, meshArr, meshCounter, materialCounter);
			}
		}
	}
}

void FbxLoader::createRootDeformer(NodeRecord* n, FbxMeshNode* meshArr) {
	//rootBone生成, name登録(本来Deformerじゃないので別に生成)
	for (unsigned int i = 0; i < n->connectionNode.size(); i++) {
		NodeRecord* n1 = n->connectionNode[i];
		if (!strcmp(n1->className, "Model") && n1->nodeName[1]) {
			if (!strcmp(n1->nodeName[1], "Root") || !strcmp(n1->nodeName[1], "Limb") || !strcmp(n1->nodeName[1], "Null")) {
				for (unsigned int j = 0; j < NumMesh; j++) {
					FbxMeshNode& mesh = meshArr[j];
					if (mesh.NumDeformer > 0) {
						mesh.rootDeformer = new Deformer();
						Deformer* defo = mesh.rootDeformer;
						getLcl(n1, defo->lcl);
						int len = (int)strlen(n1->nodeName[0]);
						defo->name = new char[len + 1];
						strcpy(defo->name, n1->nodeName[0]);
						getAnimation(n1, defo);
						//子ノードのModelName登録
						for (unsigned int i1 = 0; i1 < n1->connectionNode.size(); i1++) {
							NodeRecord* n2 = n1->connectionNode[i1];
							if (!strcmp(n2->className, "Model")) {
								int ln = (int)strlen(n2->nodeName[0]);
								defo->childName[defo->NumChild] = new char[ln + 1];
								strcpy(defo->childName[defo->NumChild++], n2->nodeName[0]);
							}
						}
					}
				}
				break;
			}
			else {
				createRootDeformer(n1, meshArr);
			}
		}
	}
}

void FbxLoader::getMesh() {

	meshCount(rootNode, NumMesh);

	if (NumMesh <= 0)return;
	Mesh = new FbxMeshNode[NumMesh];

	unsigned int mecnt = 0;

	geometryCount(rootNode, Mesh, mecnt);

	for (unsigned int i = 0; i < NumMesh; i++) {
		Mesh[i].material = new FbxMaterialNode * [Mesh[i].NumMaterial];
		if (Mesh[i].NumUVObj > 0)Mesh[i].UV = new LayerElement * [Mesh[i].NumUVObj];
		Mesh[i].deformer = new Deformer * [Mesh[i].NumDeformer];
		Mesh[i].NumDeformer = 0;//この後にカウンターとして使うので0に初期化
	}

	mecnt = 0;
	unsigned int matcnt = 0;

	searchGeometry(rootNode, Mesh, mecnt, matcnt);

	createRootDeformer(rootNode, Mesh);

	for (unsigned int i = 0; i < NumMesh; i++) {
		//Normals整列
		for (int i1 = 0; i1 < Mesh[i].NumNormalsObj; i1++) {
			LayerElement* nor = Mesh[i].Normals[i1];
			if (nor == nullptr)continue;
			if (nor->NumNormalsIndex > 0) {
				nor->AlignedNormals = new double[nor->NumNormalsIndex * 3];
				unsigned int cnt = 0;
				for (unsigned int i2 = 0; i2 < nor->NumNormalsIndex; i2++) {
					//NormalsIndexはNormalsの3値を一組としてのインデックスなので×3で計算
					nor->AlignedNormals[cnt++] = nor->normals[nor->NormalsIndex[i2] * 3];
					nor->AlignedNormals[cnt++] = nor->normals[nor->NormalsIndex[i2] * 3 + 1];
					nor->AlignedNormals[cnt++] = nor->normals[nor->NormalsIndex[i2] * 3 + 2];
				}
			}
			else {
				nor->AlignedNormals = new double[nor->Numnormals];
				memcpy(nor->AlignedNormals, nor->normals, nor->Numnormals * sizeof(double));
			}
		}

		//UV整列
		if (Mesh[i].NumUVObj <= 0) {
			Mesh[i].UV = new LayerElement * [1];
			Mesh[i].NumUVObj = 1;
			Mesh[i].UV[0] = new LayerElement();
			Mesh[i].UV[0]->AlignedUV = new double[Mesh[i].NumPolygonVertices * 2];
			for (unsigned int i2 = 0; i2 < Mesh[i].NumPolygonVertices * 2; i2++) {
				Mesh[i].UV[0]->AlignedUV[i2] = 0.0;
			}
			continue;
		}
		for (int i1 = 0; i1 < Mesh[i].NumUVObj; i1++) {
			LayerElement* uv = Mesh[i].UV[i1];
			if (uv == nullptr)continue;
			if (uv->NumUVindex > 0) {
				uv->AlignedUV = new double[uv->NumUVindex * 2];
				unsigned int cnt = 0;
				for (unsigned int i2 = 0; i2 < uv->NumUVindex; i2++) {
					//UVindexはUVの2値を一組としてのインデックスなので×2で計算
					uv->AlignedUV[cnt++] = uv->UV[uv->UVindex[i2] * 2];
					uv->AlignedUV[cnt++] = uv->UV[uv->UVindex[i2] * 2 + 1];
				}
			}
			else {
				uv->AlignedUV = new double[uv->NumUV];
				memcpy(uv->AlignedUV, uv->UV, uv->NumUV * sizeof(double));
			}
		}
		//Deformer親ノード登録
		setParentPointerOfSubDeformer(&Mesh[i]);
	}
}

void FbxLoader::setParentPointerOfNoneMeshSubDeformer() {
	if (NumDeformer <= 0)return;
	for (unsigned int i = 0; i < NumDeformer + 1; i++) {
		Deformer* defo = nullptr;
		if (i < NumDeformer)
			defo = deformer[i];
		else
			defo = rootDeformer;

		for (unsigned int i1 = 0; i1 < defo->NumChild; i1++) {
			for (unsigned int i2 = 0; i2 < NumDeformer; i2++) {
				//登録した子Deformer名と一致するDeformerに自身のポインタを登録
				if (nameComparison(defo->childName[i1], deformer[i2]->name)) {
					deformer[i2]->parentNode = defo;
				}
			}
		}
	}
}

void FbxLoader::getNoneMeshSubDeformerCounter(NodeRecord* node) {
	if (!strcmp(node->className, "Model")) {
		NumDeformer++;
		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			NodeRecord* n1 = node->connectionNode[i];
			if (!strcmp(n1->className, "Model")) {
				getNoneMeshSubDeformerCounter(n1);
			}
		}
	}
}

void FbxLoader::getNoneMeshDeformerCounter() {
	for (unsigned int i = 0; i < rootNode->connectionNode.size(); i++) {
		NodeRecord* n1 = rootNode->connectionNode[i];
		if (!strcmp(n1->className, "Model") && n1->nodeName[1]) {
			if (!strcmp(n1->nodeName[1], "Root") || !strcmp(n1->nodeName[1], "Limb") || !strcmp(n1->nodeName[1], "Null")) {
				for (unsigned int i1 = 0; i1 < n1->connectionNode.size(); i1++) {
					NodeRecord* n2 = n1->connectionNode[i1];
					if (!strcmp(n2->className, "Model")) {
						getNoneMeshSubDeformerCounter(n2);
					}
				}
				break;
			}
		}
	}
}

void FbxLoader::getNoneMeshSubDeformer(NodeRecord* node) {
	if (!strcmp(node->className, "Model")) {
		deformer[NumDeformer] = new Deformer();
		getLcl(node, deformer[NumDeformer]->lcl);
		Deformer* defo = deformer[NumDeformer];
		NumDeformer++;
		int len = (int)strlen(node->nodeName[0]);
		defo->name = new char[len + 1];
		strcpy(defo->name, node->nodeName[0]);
		getAnimation(node, defo);
		//子ノードのModelName登録
		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			NodeRecord* n1 = node->connectionNode[i];
			if (!strcmp(n1->className, "Model")) {
				int ln = (int)strlen(n1->nodeName[0]);
				defo->childName[defo->NumChild] = new char[ln + 1];
				strcpy(defo->childName[defo->NumChild++], n1->nodeName[0]);
				getNoneMeshSubDeformer(n1);
			}
		}
	}
}

void FbxLoader::GetNoneMeshDeformer() {
	for (unsigned int i = 0; i < rootNode->connectionNode.size(); i++) {
		NodeRecord* n1 = rootNode->connectionNode[i];
		if (!strcmp(n1->className, "Model") && n1->nodeName[1]) {
			if (!strcmp(n1->nodeName[1], "Root") || !strcmp(n1->nodeName[1], "Limb") || !strcmp(n1->nodeName[1], "Null")) {
				rootDeformer = new Deformer();
				getLcl(n1, rootDeformer->lcl);
				int len = (int)strlen(n1->nodeName[0]);
				rootDeformer->name = new char[len + 1];
				strcpy(rootDeformer->name, n1->nodeName[0]);
				getAnimation(n1, rootDeformer);
				//子ノードのModelName登録
				for (unsigned int i1 = 0; i1 < n1->connectionNode.size(); i1++) {
					NodeRecord* n2 = n1->connectionNode[i1];
					if (!strcmp(n2->className, "Model")) {
						int ln = (int)strlen(n2->nodeName[0]);
						rootDeformer->childName[rootDeformer->NumChild] = new char[ln + 1];
						strcpy(rootDeformer->childName[rootDeformer->NumChild++], n2->nodeName[0]);
						//ついでに子ノードのDeformer生成
						getNoneMeshSubDeformer(n2);
					}
				}
				break;
			}
		}
	}
	setParentPointerOfNoneMeshSubDeformer();
}

void FbxLoader::getCol(NodeRecord *pro70Child, double Col[3], char *ColStr) {
	if (!strcmp(pro70Child->className, "P") &&
		!strcmp(pro70Child->nodeName[0], ColStr)) {
		unsigned int proInd = 1;
		for (unsigned int i = 0; i < 4; i++) {
			proInd += convertUCHARtoUINT(&pro70Child->Property[proInd]) + 1 + 4;
			int k = 0;
		}
		for (unsigned int i = 0; i < 3; i++) {
			ConvertUCHARtoDouble(&pro70Child->Property[proInd], &Col[i], 1);
			proInd += 9;
			int f = 0;
		}
	}
}

void FbxLoader::getPropertiesInt(NodeRecord* pro70Child, int& pi, char* pName) {
	if (!strcmp(pro70Child->className, "P") &&
		!strcmp(pro70Child->nodeName[0], pName)) {
		unsigned int proInd = 1;
		for (unsigned int i = 0; i < 4; i++) {
			proInd += convertUCHARtoUINT(&pro70Child->Property[proInd]) + 1 + 4;
		}
		pi = convertUCHARtoINT32(&pro70Child->Property[proInd]);
	}
}

void FbxLoader::getPropertiesint64(NodeRecord* pro70Child, int64_t& pi, char* pName) {
	if (!strcmp(pro70Child->className, "P") &&
		!strcmp(pro70Child->nodeName[0], pName)) {
		unsigned int proInd = 1;
		for (unsigned int i = 0; i < 4; i++) {
			proInd += convertUCHARtoUINT(&pro70Child->Property[proInd]) + 1 + 4;
		}
		pi = convertUCHARtoint64(&pro70Child->Property[proInd]);
	}
}

void FbxLoader::getPropertiesDouble(NodeRecord* pro70Child, double* piArr, int num, char* pName) {
	if (!strcmp(pro70Child->className, "P") &&
		!strcmp(pro70Child->nodeName[0], pName)) {
		unsigned int proInd = 1;
		for (unsigned int i = 0; i < 4; i++) {
			proInd += convertUCHARtoUINT(&pro70Child->Property[proInd]) + 1 + 4;
		}
		for (int i = 0; i < num; i++) {
			piArr[i] = convertUCHARtoDouble(&pro70Child->Property[proInd]);
			proInd += 9;
		}
	}
}

void FbxLoader::getAnimationCurve(unsigned int& animInd, NodeRecord* animNode, AnimationCurve* anim, char* Lcl) {
	if (!strcmp(animNode->className, "AnimationCurveNode") &&
		!strcmp(animNode->nodeName[0], Lcl)) {
		for (unsigned int i = 0; i < animNode->connectionNode.size(); i++) {
			if (!strcmp(animNode->connectionNode[i]->className, "AnimationCurve")) {
				NodeRecord* animCurve = animNode->connectionNode[i];
				for (unsigned int i1 = 0; i1 < animCurve->NumChildren; i1++) {
					if (!strcmp(animCurve->nodeChildren[i1].className, "Default")) {
						if (anim[animInd].def)continue;
						anim[animInd].Default = convertUCHARtoDouble(&animCurve->nodeChildren[i1].Property[1]);
						anim[animInd].def = true;
					}
					if (!strcmp(animCurve->nodeChildren[i1].className, "KeyTime")) {
						if (anim[animInd].KeyTime)continue;
						unsigned char* output = nullptr;
						unsigned int outSize = 0;
						if (Decompress(&animCurve->nodeChildren[i1], &output, &outSize, sizeof(int64_t))) {
							anim[animInd].NumKey = outSize;
							anim[animInd].KeyTime = new int64_t[outSize];
							ConvertUCHARtoint64_t(output, anim[animInd].KeyTime, outSize);
						}
						aDELETE(output);
					}
					if (!strcmp(animCurve->nodeChildren[i1].className, "KeyValueFloat")) {
						if (anim[animInd].KeyValueFloat)continue;
						unsigned char* output = nullptr;
						unsigned int outSize = 0;
						if (Decompress(&animCurve->nodeChildren[i1], &output, &outSize, sizeof(float))) {
							anim[animInd].NumKey = outSize;
							anim[animInd].KeyValueFloat = new float[outSize];
							ConvertUCHARtofloat(output, anim[animInd].KeyValueFloat, outSize);
						}
						aDELETE(output);
						animInd++;
					}
				}
			}
		}
	}
}

void FbxLoader::getAnimation(NodeRecord* model, Deformer* defo) {
	//Animation関連
	unsigned int animInd[3] = {};
	for (unsigned int i = 0; i < model->connectionNode.size(); i++) {
		getAnimationCurve(animInd[0], model->connectionNode[i], defo->Translation, "T");
		getAnimationCurve(animInd[1], model->connectionNode[i], defo->Rotation, "R");
		getAnimationCurve(animInd[2], model->connectionNode[i], defo->Scaling, "S");
	}
}

void FbxLoader::ConvertUCHARtoDouble(unsigned char *arr, double *outArr, unsigned int outsize) {
	for (unsigned int i = 0; i < outsize; i++) {
		int addInd = i * 8;
		unsigned long long tmp = (((unsigned long long)arr[7 + addInd] << 56) | ((unsigned long long)arr[6 + addInd] << 48) |
			((unsigned long long)arr[5 + addInd] << 40) | ((unsigned long long)arr[4 + addInd] << 32) |
			((unsigned long long)arr[3 + addInd] << 24) | ((unsigned long long)arr[2 + addInd] << 16) |
			((unsigned long long)arr[1 + addInd] << 8) | ((unsigned long long)arr[addInd]));
		//byte列そのままで型変換する
		double *dp = reinterpret_cast<double*>(&tmp);
		outArr[i] = *dp;
	}
}

void FbxLoader::ConvertUCHARtoINT32(unsigned char *arr, int *outArr, unsigned int outsize) {
	for (unsigned int i = 0; i < outsize; i++) {
		int addInd = i * 4;
		outArr[i] = (((int)arr[3 + addInd] << 24) | ((int)arr[2 + addInd] << 16) |
			((int)arr[1 + addInd] << 8) | ((int)arr[addInd]));
	}
}

void FbxLoader::ConvertUCHARtoint64_t(unsigned char *arr, int64_t *outArr, unsigned int outsize) {
	for (unsigned int i = 0; i < outsize; i++) {
		int addInd = i * 8;
		outArr[i] = (((int64_t)arr[7 + addInd] << 56) | ((int64_t)arr[6 + addInd] << 48) |
			((int64_t)arr[5 + addInd] << 40) | ((int64_t)arr[4 + addInd]) << 32 |
			((int64_t)arr[3 + addInd] << 24) | ((int64_t)arr[2 + addInd] << 16) |
			((int64_t)arr[1 + addInd] << 8) | ((int64_t)arr[addInd]));
	}
}

void FbxLoader::ConvertUCHARtofloat(unsigned char *arr, float *outArr, unsigned int outsize) {
	for (unsigned int i = 0; i < outsize; i++) {
		int addInd = i * 4;
		unsigned int  tmp = (((unsigned int )arr[3 + addInd] << 24) | ((unsigned int )arr[2 + addInd] << 16) |
			((unsigned int )arr[1 + addInd] << 8) | ((unsigned int )arr[addInd]));
		//byte列そのままで型変換する
		float *fp = reinterpret_cast<float*>(&tmp);
		outArr[i] = *fp;
	}
}

void FbxLoader::drawname(NodeRecord* node, bool cnNode) {
	static unsigned int level = 0;
	for (unsigned int j = 0; j < level; j++) {
		std::cout << "    " << std::flush;
	}
	std::cout << level << " " << node->className << ":" << std::flush;
	if (node->thisConnectionID != -1)std::cout << node->thisConnectionID << std::flush;
	std::cout << std::endl;
	for (int i = 0; i < NUMNODENAME; i++) {
		if (node->nodeName[i]) {
			for (unsigned int j = 0; j < level; j++) {
				std::cout << "    " << std::flush;
			}
			std::cout << " " << level << " " << node->nodeName[i] << std::endl;
		}
	}
	level++;
	for (unsigned int i = 0; i < node->NumChildren; i++) {
		drawname(&node->nodeChildren[i], cnNode);
	}
	if (cnNode) {
		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			drawname(node->connectionNode[i], cnNode);
		}
	}
	level--;
	getchar();
}

FbxLoader::~FbxLoader() {
	aDELETE(Mesh);
	sDELETE(singleMesh);
	std::vector<ConnectionNo>().swap(cnNo);//解放
	std::vector<ConnectionList>().swap(cnLi);//解放
	rootNode = nullptr;
	for (unsigned int i = 0; i < NumDeformer; i++)sDELETE(deformer[i]);
	aDELETE(deformer);
	sDELETE(rootDeformer);
}

bool FbxLoader::setFbxFile(char* pass) {
	FilePointer fp;
	if (!fp.setFile(pass))return false;
	if (!fileCheck(&fp))return false;
	searchVersion(&fp);
	readFBX(&fp);
	getMesh();
	if (NumMesh <= 0) {
		getNoneMeshDeformerCounter();
		deformer = new Deformer * [NumDeformer];
		NumDeformer = 0;
		GetNoneMeshDeformer();
	}
	return true;
}

bool FbxLoader::setBinaryInFbxFile(char* strArray, int size) {
	FilePointer fp;
	fp.setCharArray(strArray, size);
	if (!fileCheck(&fp))return false;
	searchVersion(&fp);
	readFBX(&fp);
	getMesh();
	if (NumMesh <= 0) {
		getNoneMeshDeformerCounter();
		deformer = new Deformer * [NumDeformer];
		NumDeformer = 0;
		GetNoneMeshDeformer();
	}
	return true;
}

NodeRecord *FbxLoader::getFbxRecord() {
	return &FbxRecord;
}

NodeRecord *FbxLoader::getRootNode() {
	return rootNode;
}

unsigned int FbxLoader::getNumFbxMeshNode() {
	return NumMesh;
}

FbxMeshNode* FbxLoader::getFbxMeshNode(unsigned int index) {
	if (Mesh)return &Mesh[index];
	if (singleMesh)return singleMesh;
	return nullptr;
}

unsigned int FbxLoader::getNumNoneMeshDeformer() {
	return NumDeformer;
}

Deformer* FbxLoader::getNoneMeshDeformer(unsigned int index) {
	return deformer[index];
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

void FbxLoader::createFbxSingleMeshNode() {
	singleMesh = new FbxMeshNode();
	int maxNumMaterial = 0;
	for (unsigned int i = 0; i < NumMesh; i++) {
		singleMesh->NumVertices += Mesh[i].NumVertices;
		singleMesh->NumPolygonVertices += Mesh[i].NumPolygonVertices;
		singleMesh->NumPolygon += Mesh[i].NumPolygon;
		maxNumMaterial += Mesh[i].NumMaterial;
	}
	singleMesh->NumNormalsObj = Mesh[0].NumNormalsObj;
	singleMesh->NumUVObj = Mesh[0].NumUVObj;
	singleMesh->NumDeformer = Mesh[0].NumDeformer;

	singleMesh->vertices = new double[singleMesh->NumVertices * 3];
	singleMesh->polygonVertices = new int[singleMesh->NumPolygonVertices];
	singleMesh->PolygonSize = new unsigned int[singleMesh->NumPolygon];

	int verSizeCnt = 0;
	int verCnt = 0;
	int poCnt = 0;
	int indCnt = 0;
	for (unsigned int i = 0; i < NumMesh; i++) {
		FbxMeshNode& m = Mesh[i];
		memcpy(&singleMesh->vertices[verSizeCnt], m.vertices, m.NumVertices * 3 * sizeof(double));
		memcpy(&singleMesh->PolygonSize[poCnt], m.PolygonSize, m.NumPolygon * sizeof(unsigned int));
		for (unsigned int i1 = 0; i1 < m.NumPolygonVertices; i1++) {
			singleMesh->polygonVertices[indCnt + i1] = m.polygonVertices[i1] + verCnt;
		}
		verSizeCnt += m.NumVertices * 3;
		verCnt += m.NumVertices;
		poCnt += m.NumPolygon;
		indCnt += m.NumPolygonVertices;
	}

	//materialNameList生成
	int matNameCnt = 0;
	char** matNameList = new char* [maxNumMaterial];
	FbxMaterialNode** matPList = new FbxMaterialNode * [maxNumMaterial];
	for (int i = 0; i < maxNumMaterial; i++)matNameList[i] = nullptr;
	for (unsigned int i = 0; i < NumMesh; i++) {
		FbxMeshNode& mesh = Mesh[i];
		for (int i1 = 0; i1 < mesh.NumMaterial; i1++) {
			FbxMaterialNode& material = *(mesh.material[i1]);
			bool hit = false;
			//Listに名前があるか検索
			for (int i2 = 0; i2 < matNameCnt; i2++) {
				if (!material.MaterialName) {
					hit = true;
					break;
				}
				if (!strcmp(matNameList[i2], material.MaterialName)) {
					hit = true;
					break;
				}
			}
			if (!hit) {
				int ln = (int)strlen(material.MaterialName) + 1;
				matNameList[matNameCnt] = new char[ln];
				strcpy(matNameList[matNameCnt], material.MaterialName);
				matPList[matNameCnt] = mesh.material[i1];
				matNameCnt++;
			}
		}
	}
	//マテリアルポインタ配列コピー
	singleMesh->NumMaterial = matNameCnt;
	singleMesh->material = new FbxMaterialNode * [singleMesh->NumMaterial];
	memcpy(singleMesh->material, matPList, sizeof(FbxMaterialNode*) * singleMesh->NumMaterial);
	aDELETE(matPList);

	//ポリゴン毎のマテリアル番号
	singleMesh->Material[0] = new LayerElement();
	LayerElement& smat = *(singleMesh->Material[0]);
	int ln = (int)strlen("ByPolygon");
	smat.MappingInformationType = new char[ln + 1];
	strcpy(smat.MappingInformationType, "ByPolygon");
	smat.MappingInformationType[ln] = '\0';
	smat.Nummaterialarr = singleMesh->NumPolygon;
	smat.materials = new int[smat.Nummaterialarr];
	int PolygonCnt = 0;
	for (unsigned int i = 0; i < NumMesh; i++) {
		FbxMeshNode& m = Mesh[i];
		bool allSame = false;
		if (!strcmp(m.Material[0]->MappingInformationType, "AllSame"))allSame = true;
		//AllSameの場合materials配列は1個
		for (unsigned int i1 = 0; i1 < m.NumPolygon; i1++) {
			int materialsIndex = i1;
			if (allSame)materialsIndex = 0;
			int mNo = m.Material[0]->materials[materialsIndex];
			char* mName = m.material[mNo]->MaterialName;
			int smNo = 0;
			for (int i2 = 0; i2 < matNameCnt; i2++) {
				if (!strcmp(matNameList[i2], mName)) {
					smNo = i2;
					break;
				}
			}
			smat.materials[PolygonCnt + i1] = smNo;
		}
		PolygonCnt += m.NumPolygon;
	}

	for (unsigned int i = 0; i < NumMesh; i++) {
		FbxMeshNode& mesh = Mesh[i];
		for (int i1 = 0; i1 < mesh.NumMaterial; i1++) {
			bool hit = false;
			for (int i2 = 0; i2 < singleMesh->NumMaterial; i2++) {
				//singleMesh内に含まれるmaterialを検索
				if (singleMesh->material[i2] == mesh.material[i1]) {
					hit = true;
					break;
				}
			}
			if (hit) {
				//含まれるmaterialはsingleMeshから解放されるので
				//ポインタにNULLを設定しておく(解放時エラー対策)
				mesh.material[i1] = nullptr;
			}
		}
	}
	for (int i = 0; i < maxNumMaterial; i++)aDELETE(matNameList[i]);
	aDELETE(matNameList);

	//法線
	for (int i = 0; i < singleMesh->NumNormalsObj; i++) {
		singleMesh->Normals[i] = new LayerElement();
		LayerElement& nor = *(singleMesh->Normals[i]);
		for (unsigned int i1 = 0; i1 < NumMesh; i1++) {
			nor.Numnormals += Mesh[i1].Normals[i]->Numnormals;
		}
		if (nor.Numnormals > 0) {
			nor.normals = new double[nor.Numnormals];
			int norCnt = 0;
			for (unsigned int i1 = 0; i1 < NumMesh; i1++) {
				memcpy(&nor.normals[norCnt], Mesh[i1].Normals[i]->normals,
					Mesh[i1].Normals[i]->Numnormals * sizeof(double));
				norCnt += Mesh[i1].Normals[i]->Numnormals;
			}
		}
	}

	//UV
	if (singleMesh->NumUVObj > 0) {
		singleMesh->UV = new LayerElement * [singleMesh->NumUVObj];
		for (int i = 0; i < singleMesh->NumUVObj; i++) {
			singleMesh->UV[i] = new LayerElement();
			LayerElement& uv = *(singleMesh->UV[i]);
			for (unsigned int i1 = 0; i1 < NumMesh; i1++) {
				uv.NumUV += Mesh[i1].UV[i]->NumUV;
				uv.NumUVindex += Mesh[i1].UV[i]->NumUVindex;
			}
			if (Mesh[0].UV[i]->name) {
				int ln = (int)strlen(Mesh[0].UV[i]->name);
				uv.name = new char[ln + 1];
				strcpy(uv.name, Mesh[0].UV[i]->name);
				uv.name[ln] = '\0';
			}
			if (uv.NumUV > 0) {
				uv.UV = new double[uv.NumUV];
			}
			if (uv.NumUVindex > 0) {
				uv.UVindex = new int[uv.NumUVindex];
				uv.AlignedUV = new double[uv.NumUVindex * 2];
			}
			else {
				uv.AlignedUV = new double[uv.NumUV];
			}
			int uvCnt = 0;
			int uvIndCnt = 0;
			int aUvCnt = 0;
			for (unsigned int i1 = 0; i1 < NumMesh; i1++) {
				LayerElement& u = *(Mesh[i1].UV[i]);
				if (u.NumUV > 0) {
					memcpy(&uv.UV[uvCnt], u.UV, u.NumUV * sizeof(double));
				}
				if (uv.NumUVindex > 0) {
					for (unsigned int i2 = 0; i2 < u.NumUVindex; i2++) {
						uv.UVindex[uvIndCnt + i2] = u.UVindex[i2] + uvCnt;
					}
					memcpy(&uv.AlignedUV[aUvCnt], u.AlignedUV, u.NumUVindex * 2 * sizeof(double));
				}
				else {
					memcpy(&uv.AlignedUV[uvCnt], u.AlignedUV, u.NumUV * sizeof(double));
				}
				uvCnt += u.NumUV;
				uvIndCnt += u.NumUVindex;
				aUvCnt += u.NumUVindex * 2;
			}
		}
	}

	//Deformer合成
	if (Mesh[0].NumDeformer > 0) {
		singleMesh->NumDeformer = Mesh[0].NumDeformer;
		unsigned int numDeformer = Mesh[0].NumDeformer;
		int* IndicesCount = nullptr;//ボーンに影響を受ける頂点インデックス数
		int** Indices = nullptr;//ボーンに影響を受ける頂点のインデックス配列
		double** Weights = nullptr;//ボーンに影響を受ける頂点のウエイト配列
		IndicesCount = new int[numDeformer];
		Indices = new int* [numDeformer];
		Weights = new double* [numDeformer];
		for (unsigned int i = 0; i < numDeformer; i++) {
			IndicesCount[i] = 0;
			for (unsigned int i1 = 0; i1 < NumMesh; i1++) {
				IndicesCount[i] += Mesh[i1].deformer[i]->IndicesCount;
			}
			Indices[i] = new int[IndicesCount[i]];
			Weights[i] = new double[IndicesCount[i]];
			int iCnt = 0;
			int vCnt = 0;
			for (unsigned int i1 = 0; i1 < NumMesh; i1++) {
				Deformer& de = *(Mesh[i1].deformer[i]);
				for (int i2 = 0; i2 < de.IndicesCount; i2++) {
					Indices[i][i2 + iCnt] = de.Indices[i2] + vCnt;
				}
				memcpy(&Weights[i][iCnt], de.Weights, de.IndicesCount * sizeof(double));
				iCnt += de.IndicesCount;
				vCnt += Mesh[i1].NumVertices;
			}
		}

		//ポインタをコピー
		singleMesh->deformer = Mesh[0].deformer;
		Mesh[0].deformer = nullptr;
		singleMesh->rootDeformer = Mesh[0].rootDeformer;
		Mesh[0].rootDeformer = nullptr;

		//合成したボーン情報をコピー
		for (unsigned int i = 0; i < numDeformer; i++) {
			Deformer& de = *(singleMesh->deformer[i]);
			de.IndicesCount = IndicesCount[i];
			aDELETE(de.Indices);
			aDELETE(de.Weights);
			de.Indices = new int[IndicesCount[i]];
			de.Weights = new double[IndicesCount[i]];
			memcpy(de.Indices, Indices[i], IndicesCount[i] * sizeof(int));
			memcpy(de.Weights, Weights[i], IndicesCount[i] * sizeof(double));
		}

		for (unsigned int i = 0; i < numDeformer; i++) {
			aDELETE(Indices[i]);
			aDELETE(Weights[i]);
		}
		aDELETE(Indices);
		aDELETE(Weights);
		aDELETE(IndicesCount);
	}
	singleMesh->lcl = Mesh[0].lcl;
	aDELETE(Mesh);
	NumMesh = 1;
}
