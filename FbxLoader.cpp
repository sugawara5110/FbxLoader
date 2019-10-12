//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　         FbxLoader                                          **//
//**                                                                                     **//
//*****************************************************************************************//

#define _CRT_SECURE_NO_WARNINGS
#include "FbxLoader.h"
#include "iostream"
#include "DecompressDeflate.h"
#define Kaydara_FBX_binary 18

FilePointer::~FilePointer() {
	if (fileStr) {
		delete[] fileStr;
		fileStr = nullptr;
	}
}

bool FilePointer::setFile(char *pass) {
	FILE *fp = fopen(pass, "rb");
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

unsigned int convertUCHARtoUINT(unsigned char *arr) {
	return ((unsigned int)arr[3] << 24) | ((unsigned int)arr[2] << 16) |
		((unsigned int)arr[1] << 8) | ((unsigned int)arr[0]);
}

int convertUCHARtoINT32(unsigned char *arr) {
	return ((int)arr[3] << 24) | ((int)arr[2] << 16) |
		((int)arr[1] << 8) | ((int)arr[0]);
}

int64_t convertUCHARtoint64(unsigned char *arr) {
	return ((int64_t)arr[7] << 56) | ((int64_t)arr[6] << 48) |
		((int64_t)arr[5] << 40) | ((int64_t)arr[4] << 32) |
		((int64_t)arr[3] << 24) | ((int64_t)arr[2] << 16) |
		((int64_t)arr[1] << 8) | ((int64_t)arr[0]);
}

unsigned long long convertUCHARtoUINT64(unsigned char *arr) {
	return ((unsigned long long)arr[7] << 56) | ((unsigned long long)arr[6] << 48) |
		((unsigned long long)arr[5] << 40) | ((unsigned long long)arr[4] << 32) |
		((unsigned long long)arr[3] << 24) | ((unsigned long long)arr[2] << 16) |
		((unsigned long long)arr[1] << 8) | ((unsigned long long)arr[0]);
}

double convertUCHARtoDouble(unsigned char *arr) {
	unsigned long long tmp = ((unsigned long long)arr[7] << 56) | ((unsigned long long)arr[6] << 48) |
		((unsigned long long)arr[5] << 40) | ((unsigned long long)arr[4] << 32) |
		((unsigned long long)arr[3] << 24) | ((unsigned long long)arr[2] << 16) |
		((unsigned long long)arr[1] << 8) | ((unsigned long long)arr[0]);
	//byte列そのままで型変換する
	double *dp = reinterpret_cast<double*>(&tmp);
	return *dp;
}

void NodeRecord::searchName_Type(std::vector<ConnectionNo>& cn) {
	int swt = 0;
	unsigned int ln = 0;
	int nameNo = 0;
	int addInd = 0;
	unsigned char *pr = nullptr;
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
	EndOffset = fp->convertBYTEtoUINT();
	NumProperties = fp->convertBYTEtoUINT();
	PropertyListLen = fp->convertBYTEtoUINT();
	classNameLen = fp->getByte();
	className = new char[classNameLen + 1];
	fp->fRead(className, classNameLen);
	className[classNameLen] = '\0';
	if (PropertyListLen > 0) {
		Property = new unsigned char[PropertyListLen];
		fp->fRead((char*)Property, PropertyListLen);
		searchName_Type(cn);
		if (!strcmp(className, "C") && (!strcmp(nodeName[0], "OO") || !strcmp(nodeName[0], "OP"))) {
			createConnectionList(cnLi, nodeName[1]);
		}
	}

	unsigned int curpos = fp->getPos();
	//現在のファイルポインタがEndOffsetより手前,かつ
	//現ファイルポインタから4byteが全て0ではない場合, 子ノード有り
	if (EndOffset > curpos && fp->convertBYTEtoUINT() != 0) {
		unsigned int topChildPointer = curpos;
		unsigned int childEndOffset = 0;
		//子ノードEndOffsetをたどり,個数カウント
		do {
			fp->seekPointer(fp->getPos() - 4);//"convertBYTEtoUINT() != 0"の分戻す
			NumChildren++;
			childEndOffset = fp->convertBYTEtoUINT();
			fp->seekPointer(childEndOffset);
		} while (EndOffset > childEndOffset && fp->convertBYTEtoUINT() != 0);
		//カウントが終わったので最初の子ノードのファイルポインタに戻す
		fp->seekPointer(topChildPointer);
		nodeChildren = new NodeRecord[NumChildren];
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

void FbxLoader::searchVersion(FilePointer *fp) {
	//バージョンは23-26バイトまで(4バイト分)リトルエンディアン(下の位から読んでいく)
	version = fp->convertBYTEtoUINT();
}

void FbxLoader::readFBX(FilePointer* fp) {
	unsigned int curpos = fp->getPos();

	unsigned int nodeCount = 0;
	unsigned int endoffset = 0;

	while (fp->convertBYTEtoUINT() != 0) {
		fp->seekPointer(fp->getPos() - 4);//"convertBYTEtoUINT() != 0"の分戻す
		nodeCount++;
		endoffset = fp->convertBYTEtoUINT();
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

void FbxLoader::getLayerElementSub(NodeRecord *node, LayerElement *le) {

	for (unsigned int i = 0; i < node->NumChildren; i++) {
		NodeRecord *n1 = &node->nodeChildren[i];

		if (!strcmp(n1->className, "Name")) {
			unsigned int size = convertUCHARtoUINT(&n1->Property[1]);
			if (size > 0) {
				le->name = new char[size + 1];
				unsigned char *pr = &n1->Property[5];
				memcpy(le->name, (char*)pr, size);
				le->name[size] = '\0';
			}
		}

		if (!strcmp(n1->className, "MappingInformationType")) {
			unsigned int size = convertUCHARtoUINT(&n1->Property[1]);
			if (size > 0) {
				le->MappingInformationType = new char[size + 1];
				unsigned char *pr = &n1->Property[5];
				memcpy(le->MappingInformationType, (char*)pr, size);
				le->MappingInformationType[size] = '\0';
			}
		}

		if (!strcmp(n1->className, "Materials")) {
			unsigned char *output = nullptr;
			unsigned int outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(int));
			le->Nummaterialarr = outSize;
			le->materials = new int[outSize];
			ConvertUCHARtoINT32(output, le->materials, outSize);
			aDELETE(output);
		}

		if (!strcmp(n1->className, "Normals")) {
			unsigned char *output = nullptr;
			unsigned int outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(double));
			le->Numnormals = outSize;
			le->normals = new double[outSize];
			ConvertUCHARtoDouble(output, le->normals, outSize);
			aDELETE(output);
		}

		if (!strcmp(n1->className, "UV")) {
			unsigned char *output = nullptr;
			unsigned int outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(double));
			le->NumUV = outSize;
			le->UV = new double[outSize];
			ConvertUCHARtoDouble(output, le->UV, outSize);
			aDELETE(output);
		}

		if (!strcmp(n1->className, "UVIndex")) {
			unsigned char *output = nullptr;
			unsigned int outSize = 0;
			Decompress(n1, &output, &outSize, sizeof(int));
			le->NumUVindex = outSize;
			le->UVindex = new int[outSize];
			ConvertUCHARtoINT32(output, le->UVindex, outSize);
			aDELETE(output);
		}
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

void FbxLoader::setParentPointerOfSubDeformer(FbxMeshNode *mesh) {
	for (unsigned int i = 0; i < mesh->NumDeformer + 1; i++) {
		Deformer *defo = nullptr;
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
		//Deformer名
		int len = (int)strlen(node->nodeName[0]);
		defo->name = new char[len + 1];
		strcpy(defo->name, node->nodeName[0]);

		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			NodeRecord* n1 = node->connectionNode[i];
			if (!strcmp(n1->className, "Model")) {//自身のModel
				getAnimation(n1, defo);
			}
		}

		//子ノードName登録
		for (unsigned int i = 0; i < node->connectionNode.size(); i++) {
			NodeRecord* n1 = node->connectionNode[i];
			if (!strcmp(n1->className, "Model")) {//自身のModel
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
			}
			if (norCountUpflg) {
				norCount++;
				norCountUpflg = false;
			}
		}
		(*materialIndex)++;
	}
}

void FbxLoader::getMesh() {
	for (unsigned int i = 0; i < rootNode->connectionNode.size(); i++) {
		if (!strcmp(rootNode->connectionNode[i]->className, "Model") &&
			!strcmp(rootNode->connectionNode[i]->nodeName[1], "Mesh")) {
			NumMesh++;
		}
	}
	if (NumMesh <= 0)return;
	Mesh = new FbxMeshNode[NumMesh];

	unsigned int mecnt = 0;
	unsigned int matcnt = 0;
	for (unsigned int i = 0; i < rootNode->connectionNode.size(); i++) {
		NodeRecord* n1 = rootNode->connectionNode[i];
		if (!strcmp(n1->className, "Model") &&
			!strcmp(n1->nodeName[1], "Mesh")) {
			for (unsigned int i1 = 0; i1 < n1->connectionNode.size(); i1++) {
				NodeRecord* n2 = n1->connectionNode[i1];
				getGeometry(n2, &Mesh[mecnt]);
				getMaterial(n2, &Mesh[mecnt], &matcnt);
			}
			Mesh[mecnt].NumMaterial = matcnt;
			mecnt++;
			matcnt = 0;
		}
	}

	//rootBone生成, name登録(本来Deformerじゃないので別に生成)
	for (unsigned int i = 0; i < rootNode->connectionNode.size(); i++) {
		NodeRecord* n1 = rootNode->connectionNode[i];
		if (!strcmp(n1->className, "Model") && n1->nodeName[1]) {
			if (!strcmp(n1->nodeName[1], "Root") || !strcmp(n1->nodeName[1], "Limb") || !strcmp(n1->nodeName[1], "Null")) {
				for (unsigned int j = 0; j < NumMesh; j++) {
					Mesh[j].rootDeformer = new Deformer();
					Deformer* defo = Mesh[j].rootDeformer;
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
				break;
			}
		}
	}

	//UV整列
	for (unsigned int i = 0; i < NumMesh; i++) {
		for (int i1 = 0; i1 < Mesh[i].NumMaterial; i1++) {
			LayerElement* uv = Mesh[i].UV[i1];
			if (uv == nullptr)break;
			uv->AlignedUV = new double[uv->NumUVindex * 2];
			unsigned int cnt = 0;
			for (unsigned int i2 = 0; i2 < uv->NumUVindex; i2++) {
				uv->AlignedUV[cnt++] = uv->UV[uv->UVindex[i2] * 2];//UVindexはUVの2値を一組としてのインデックスなので×2で計算
				uv->AlignedUV[cnt++] = uv->UV[uv->UVindex[i2] * 2 + 1];
			}
		}
		setParentPointerOfSubDeformer(&Mesh[i]);
	}
}

void FbxLoader::setParentPointerOfNoneMeshSubDeformer() {
	for (unsigned int i = 0; i < NumDeformer + 1; i++) {
		Deformer *defo = nullptr;
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

void FbxLoader::getNoneMeshSubDeformer(NodeRecord* node) {
	if (!strcmp(node->className, "Model")) {
		deformer[NumDeformer] = new Deformer();
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

void FbxLoader::getNoneMeshDeformer() {
	for (unsigned int i = 0; i < rootNode->connectionNode.size(); i++) {
		NodeRecord* n1 = rootNode->connectionNode[i];
		if (!strcmp(n1->className, "Model") && n1->nodeName[1]) {
			if (!strcmp(n1->nodeName[1], "Root") || !strcmp(n1->nodeName[1], "Limb") || !strcmp(n1->nodeName[1], "Null")) {
				rootDeformer = new Deformer();
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

void FbxLoader::getLcl(NodeRecord *pro70Child, AnimationCurve anim[3], char *LclStr) {
	if (!strcmp(pro70Child->className, "P") &&
		!strcmp(pro70Child->nodeName[0], LclStr)) {
		unsigned int proInd = 1;
		for (unsigned int i = 0; i < 4; i++) {
			proInd += convertUCHARtoUINT(&pro70Child->Property[proInd]) + 1 + 4;
		}
		for (unsigned int i = 0; i < 3; i++) {
			ConvertUCHARtoDouble(&pro70Child->Property[proInd], &anim[i].Lcl, 1);
			proInd += 9;
		}
	}
}

void FbxLoader::getAnimationCurve(NodeRecord* animNode, AnimationCurve anim[3], char* Lcl) {
	unsigned int animInd = 0;
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
	//Lcl Translation, Lcl Rotation, Lcl Scaling取得
	for (unsigned int i = 0; i < model->NumChildren; i++) {
		if (!strcmp(model->nodeChildren[i].className, "Properties70")) {
			NodeRecord* pro70 = &model->nodeChildren[i];
			for (unsigned int i1 = 0; i1 < pro70->NumChildren; i1++) {
				getLcl(&pro70->nodeChildren[i1], defo->Translation, "Lcl Translation");
				getLcl(&pro70->nodeChildren[i1], defo->Rotation, "Lcl Rotation");
				getLcl(&pro70->nodeChildren[i1], defo->Scaling, "Lcl Scaling");
			}
		}
	}
	//Animation関連
	for (unsigned int i = 0; i < model->connectionNode.size(); i++) {
		getAnimationCurve(model->connectionNode[i], defo->Translation, "T");
		getAnimationCurve(model->connectionNode[i], defo->Rotation, "R");
		getAnimationCurve(model->connectionNode[i], defo->Scaling, "S");
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
	std::vector<ConnectionNo>().swap(cnNo);//解放
	std::vector<ConnectionList>().swap(cnLi);//解放
	rootNode = nullptr;
	for (unsigned int i = 0; i < NumDeformer; i++)sDELETE(deformer[i]);
	sDELETE(rootDeformer);
}

bool FbxLoader::setFbxFile(char *pass) {
	FilePointer fp;
	if (!fp.setFile(pass))return false;
	if (!fileCheck(&fp))return false;
	searchVersion(&fp);
	readFBX(&fp);
	getMesh();
	if (NumMesh <= 0)getNoneMeshDeformer();
	return true;
}

bool FbxLoader::setBinaryInFbxFile(char *strArray, int size) {
	FilePointer fp;
	fp.setCharArray(strArray, size);
	if (!fileCheck(&fp))return false;
	searchVersion(&fp);
	readFBX(&fp);
	getMesh();
	if (NumMesh <= 0)getNoneMeshDeformer();
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

FbxMeshNode *FbxLoader::getFbxMeshNode(unsigned int index) {
	return &Mesh[index];
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