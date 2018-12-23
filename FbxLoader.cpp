//*****************************************************************************************//
//**                                                                                     **//
//**                   　　　         FbxLoader                                          **//
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

double convertUCHARtoDouble(UCHAR *arr) {
	UINT64 tmp = ((UINT64)arr[7] << 56) | ((UINT64)arr[6] << 48) |
		((UINT64)arr[5] << 40) | ((UINT64)arr[4] << 32) |
		((UINT64)arr[3] << 24) | ((UINT64)arr[2] << 16) |
		((UINT64)arr[1] << 8) | ((UINT64)arr[0]);
	//byte列そのままで型変換する
	double *dp = reinterpret_cast<double*>(&tmp);
	return *dp;
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

void NodeRecord::createConnectionList(std::vector<ConnectionList>& cnLi) {
	ConnectionList cl;
	//S len "OO" L 計8byteの次にChildID
	cl.ChildID = convertUCHARtoint64(&Property[8]);
	//L 1byteの次にParentID
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
	//現在のファイルポインタがEndOffsetより手前,かつ
	//現ファイルポインタから4byteが全て0ではない場合, 子ノード有り
	if (EndOffset > UINT(curpos) && convertBYTEtoUINT(fp) != 0) {
		UINT topChildPointer = UINT(curpos);
		UINT childEndOffset = 0;
		//子ノードEndOffsetをたどり,個数カウント
		do {
			fseek(fp, sizeof(char) * -4, SEEK_CUR);//"convertBYTEtoUINT(fp) != 0"の分戻す
			NumChildren++;
			childEndOffset = convertBYTEtoUINT(fp);
			fseek(fp, sizeof(char) * childEndOffset, SEEK_SET);
		} while (EndOffset > childEndOffset && convertBYTEtoUINT(fp) != 0);
		//カウントが終わったので最初の子ノードのファイルポインタに戻す
		fseek(fp, sizeof(char) * topChildPointer, SEEK_SET);
		nodeChildren = new NodeRecord[NumChildren];
		for (UINT i = 0; i < NumChildren; i++) {
			nodeChildren[i].set(fp, cn, cnLi);
		}
	}
	//読み込みが終了したのでEndOffsetへポインタ移動
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

bool FbxLoader::fileCheck(FILE *fp) {

	char *str2 = "Kaydata FBX binary";

	int missCnt = 0;
	for (int i = 0; i < Kaydata_FBX_binary + 1; i++) {
		if ((char)fgetc(fp) != *str2) {
			missCnt++;
			if (missCnt > 3) {
				//ソフトによってスペルミスが有るのでとりあえず3文字以上異なる場合falseにする
				//別ファイルの場合ほぼ全数当てはまらないはず
				MessageBoxA(0, "バイナリファイルではありません", 0, MB_OK);
				return false;
			}
		}
		str2++;
	}
	//0-20バイト 『Kaydata FBX binary  [null]』
	//21-22バイト 0x1a, 0x00
	//23-26バイトまで(4バイト分): 符号なし整数,バージョンを表す
	fseek(fp, sizeof(char) * 23, SEEK_SET);//バイナリではSEEK_ENDは不定

	return true;
}

void FbxLoader::searchVersion(FILE *fp) {
	//バージョンは23-26バイトまで(4バイト分)リトルエンディアン(下の位から読んでいく)
	version = convertBYTEtoUINT(fp);
}

void FbxLoader::readFBX(FILE *fp) {
	fpos_t curpos = 0;
	fgetpos(fp, &curpos);

	UINT nodeCount = 0;
	UINT endoffset = 0;

	while (convertBYTEtoUINT(fp) != 0) {
		fseek(fp, sizeof(char) * -4, SEEK_CUR);//"convertBYTEtoUINT(fp) != 0"の分戻す
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

	//ノードIDの通りにノードを繋げる
	for (int i = 0; i < cnLi.size(); i++) {
		for (int j = 0; j < cnNo.size(); j++) {
			if (cnLi.data()[i].ParentID == cnNo.data()[j].ConnectionID) {//親ノードのポインタ検索
				for (int j1 = 0; j1 < cnNo.size(); j1++) {
					if (cnLi.data()[i].ChildID == cnNo.data()[j1].ConnectionID) {//子ノードのポインタ検索
						//子ノードポインタ抽出
						NodeRecord *childP = cnNo.data()[j1].ConnectionIDPointer;
						//親ノードポインタから親ノードへアクセス,
						//親ノード内connectionNodeへ子ノードポインタ追加
						NodeRecord *cp = cnNo.data()[j].ConnectionIDPointer;
						cp->connectionNode[cp->NumConnectionNode++] = childP;
					}
				}
			}
		}
	}
	//rootNode設定
	for (int i = 0; i < cnNo.size(); i++) {
		if (cnNo.data()[i].ConnectionID == 0) {
			rootNode = cnNo.data()[i].ConnectionIDPointer;
			break;
		}
	}
}

void FbxLoader::Decompress(NodeRecord *node, UCHAR **output, UINT *outSize, UINT typeSize) {
	//型1byte, 配列数4byte, 圧縮有無4byte, サイズ4byte, メタdata2byte 計15byte後data
	UINT comp = convertUCHARtoUINT(&node->Property[5]);//圧縮有無
	UINT meta = 0;
	if (comp == 1)meta = 2;
	UINT inSize = convertUCHARtoUINT(&node->Property[9]) - meta;//メタdata分引く
	*outSize = convertUCHARtoUINT(&node->Property[1]);
	*output = new UCHAR[(*outSize) * typeSize];
	if (comp == 1) {
		dd.getDecompressArray(&node->Property[15], inSize, *output);//解凍
	}
	else {
		memcpy(*output, &node->Property[13], (*outSize) * typeSize);//解凍無しの場合メタdata2byte無し
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
		INT32 No = convertUCHARtoINT32(&node->Property[1]);//たぶんレイヤーNo取得
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

void FbxLoader::getSubDeformer(NodeRecord *node, FbxMeshNode *mesh) {
	//各Deformer情報取得
	if (!strcmp(node->className, "Deformer")) {
		mesh->deformer[mesh->NumDeformer] = new Deformer();
		Deformer *defo = mesh->deformer[mesh->NumDeformer];
		//Deformer名
		int len = strlen(node->nodeName[0]);
		defo->name = new char[len + 1];
		strcpy_s(defo->name, len + 1, node->nodeName[0]);

		for (UINT i = 0; i < node->NumChildren; i++) {
			NodeRecord *n1 = &node->nodeChildren[i];

			//インデックス配列,数
			if (!strcmp(n1->className, "Indexes")) {
				UCHAR *output = nullptr;
				UINT outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(INT32));
				defo->IndicesCount = outSize;
				defo->Indices = new INT32[outSize];
				ConvertUCHARtoINT32(output, defo->Indices, outSize);
				aDELETE(output);
			}

			//ウエイト
			if (!strcmp(n1->className, "Weights")) {
				UCHAR *output = nullptr;
				UINT outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(double));
				defo->Weights = new double[outSize];
				ConvertUCHARtoDouble(output, defo->Weights, outSize);
				aDELETE(output);
			}

			//TransformLink
			if (!strcmp(n1->className, "TransformLink")) {
				UCHAR *output = nullptr;
				UINT outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(double));
				ConvertUCHARtoDouble(output, defo->TransformLinkMatrix, outSize);
				aDELETE(output);
			}
		}
		mesh->NumDeformer++;//Deformer数カウント
	}
}

void FbxLoader::getDeformer(NodeRecord *node, FbxMeshNode *mesh) {
	if (!strcmp(node->className, "Deformer")) {
		if (!Skeleton)Skeleton = node;
		for (UINT i = 0; i < node->NumConnectionNode; i++) {
			NodeRecord *n1 = node->connectionNode[i];
			//各Deformer情報取得
			getSubDeformer(n1, mesh);
		}
	}
}

void FbxLoader::getGeometry(NodeRecord *node, FbxMeshNode *mesh) {
	if (!strcmp(node->className, "Geometry")) {
		int len = strlen(node->nodeName[0]);
		mesh->name = new char[len + 1];
		strcpy_s(mesh->name, len + 1, node->nodeName[0]);
		for (UINT i = 0; i < node->NumChildren; i++) {
			NodeRecord *n1 = &node->nodeChildren[i];

			//頂点
			if (!strcmp(n1->className, "Vertices") && mesh->vertices == nullptr) {
				UCHAR *output = nullptr;
				UINT outSize = 0;
				Decompress(n1, &output, &outSize, sizeof(double));
				mesh->NumVertices = outSize / 3;
				mesh->vertices = new double[outSize];
				ConvertUCHARtoDouble(output, mesh->vertices, outSize);
				aDELETE(output);
			}

			//頂点インデックス
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
						//ポリゴン毎の最終インデックスがbit反転されてるので
						//そこでポリゴン数をカウント
						mesh->NumPolygon++;
					}
				}
				mesh->PolygonSize = new UINT[mesh->NumPolygon];
				UINT polCnt = 0;
				UINT PolygonSizeIndex = 0;
				for (UINT i1 = 0; i1 < mesh->NumPolygonVertices; i1++) {
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

		for (UINT i = 0; i < node->NumConnectionNode; i++) {
			NodeRecord *n1 = node->connectionNode[i];

			//ボーン関連
			getDeformer(n1, mesh);
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
	Mesh = new FbxMeshNode[NumMesh];

	UINT cnt = 0;
	for (UINT i = 0; i < rootNode->NumConnectionNode; i++) {
		NodeRecord *n1 = rootNode->connectionNode[i];
		if (!strcmp(n1->className, "Model") &&
			!strcmp(n1->nodeName[1], "Mesh")) {
			for (UINT i1 = 0; i1 < n1->NumConnectionNode; i1++) {
				NodeRecord *n2 = n1->connectionNode[i1];
				getGeometry(n2, &Mesh[cnt]);
			}
			cnt++;
		}
	}
	//UV整列
	for (UINT i = 0; i < NumMesh; i++) {
		for (INT32 i1 = 0; i1 < Mesh[i].NumMaterial; i1++) {
			LayerElement *uv = Mesh[i].UV[i1];
			uv->AlignedUV = new double[uv->NumUVindex * 2];
			UINT cnt = 0;
			for (UINT i2 = 0; i2 < uv->NumUVindex; i2++) {
				uv->AlignedUV[cnt++] = uv->UV[uv->UVindex[i2] * 2];//UVindexはUVの2値を一組としてのインデックスなので×2で計算
				uv->AlignedUV[cnt++] = uv->UV[uv->UVindex[i2] * 2 + 1];
			}
		}
	}
}

void FbxLoader::getLcl(NodeRecord *pro70Child, AnimationCurve anim[3], char *LclStr) {
	if (!strcmp(pro70Child->className, "P") &&
		!strcmp(pro70Child->nodeName[0], LclStr)) {
		UINT proInd = 1;
		for (UINT i = 0; i < 4; i++) {
			proInd += convertUCHARtoUINT(&pro70Child->Property[proInd]) + 1 + 4;
		}
		for (UINT i = 0; i < 3; i++) {
			ConvertUCHARtoDouble(&pro70Child->Property[proInd], &anim[i].Lcl, 1);
			proInd += 9;
		}
	}
}

void FbxLoader::getAnimationCurve(NodeRecord *animNode, AnimationCurve anim[3], char *Lcl) {
	UINT animInd = 0;
	if (!strcmp(animNode->className, "AnimationCurveNode") &&
		!strcmp(animNode->nodeName[0], Lcl)) {
		for (UINT i = 0; i < animNode->NumConnectionNode; i++) {
			if (!strcmp(animNode->connectionNode[i]->className, "AnimationCurve")) {
				NodeRecord *animCurve = animNode->connectionNode[i];
				for (UINT i1 = 0; i1 < animCurve->NumChildren; i1++) {
					if (!strcmp(animCurve->nodeChildren[i1].className, "Default")) {
						if (anim[animInd].def)continue;
						anim[animInd].Default = convertUCHARtoDouble(&animCurve->nodeChildren[i1].Property[1]);
						anim[animInd].def = true;
					}
					if (!strcmp(animCurve->nodeChildren[i1].className, "KeyTime")) {
						if (anim[animInd].KeyTime)continue;
						UCHAR *output = nullptr;
						UINT outSize = 0;
						Decompress(&animCurve->nodeChildren[i1], &output, &outSize, sizeof(int64_t));
						anim[animInd].NumKey = outSize;
						anim[animInd].KeyTime = new int64_t[outSize];
						ConvertUCHARtoint64_t(output, anim[animInd].KeyTime, outSize);
						aDELETE(output);
					}
					if (!strcmp(animCurve->nodeChildren[i1].className, "KeyValueFloat")) {
						if (anim[animInd].KeyValueFloat)continue;
						UCHAR *output = nullptr;
						UINT outSize = 0;
						Decompress(&animCurve->nodeChildren[i1], &output, &outSize, sizeof(float));
						anim[animInd].NumKey = outSize;
						anim[animInd].KeyValueFloat = new float[outSize];
						ConvertUCHARtofloat(output, anim[animInd].KeyValueFloat, outSize);
						aDELETE(output);
						animInd++;
					}
				}
			}
		}
	}
}

void FbxLoader::getPoseSub2(int64_t cnId, NodeRecord *node, FbxMeshNode *mesh) {
	//接続先探索
	NodeRecord *model = nullptr;
	for (int i = 0; i < cnNo.size(); i++) {
		if (cnNo.data()[i].ConnectionID == cnId) {//取り出した接続先IDからポインタを割り出し(ModelのID)
			model = cnNo.data()[i].ConnectionIDPointer;
			break;
		}
	}

	char *linkName = model->nodeName[0];//この名前から, 対応ボーンを割り出す
	UINT Numdefo = mesh->NumDeformer;

	for (UINT i = 0; i < Numdefo; i++) {
		Deformer *defo = mesh->deformer[i];
		char *deName = defo->name;
		//名前文字列に空白文字が有る場合,空白文字以前を取り除く
		char *linkNameTmp = linkName;
		do {
			while (*linkNameTmp != ' ' && *linkNameTmp != '\0') {
				linkNameTmp++;
			}
			if (*linkNameTmp == '\0') {
				break;
			}
			else {
				linkNameTmp++;
				linkName = linkNameTmp;
			}
		} while (1);

		char *deNameTmp = deName;
		do {
			while (*deNameTmp != ' ' && *deNameTmp != '\0') {
				deNameTmp++;
			}
			if (*deNameTmp == '\0') {
				break;
			}
			else {
				deNameTmp++;
				deName = deNameTmp;
			}
		} while (1);

		//名前が一致してる場合はそのDeformerにPose行列を格納する
		int llen = strlen(linkName);
		int dlen = strlen(deName);
		if (llen == dlen && !strcmp(linkName, deName)) {
			//一致したのでPose行列格納
			for (UINT i1 = 0; i1 < node->NumChildren; i1++) {
				if (!strcmp(node->nodeChildren[i1].className, "Matrix")) {
					UCHAR *output = nullptr;
					UINT outSize = 0;
					Decompress(&node->nodeChildren[i1], &output, &outSize, sizeof(double));
					ConvertUCHARtoDouble(output, defo->Pose, outSize);
					aDELETE(output);
					//Lcl Translation, Lcl Rotation, Lcl Scaling取得
					for (UINT i2 = 0; i2 < model->NumChildren; i2++) {
						if (!strcmp(model->nodeChildren[i2].className, "Properties70")) {
							NodeRecord *pro70 = &model->nodeChildren[i2];
							for (UINT i3 = 0; i3 < pro70->NumChildren; i3++) {
								getLcl(&pro70->nodeChildren[i3], defo->Translation, "Lcl Translation");
								getLcl(&pro70->nodeChildren[i3], defo->Rotation, "Lcl Rotation");
								getLcl(&pro70->nodeChildren[i3], defo->Scaling, "Lcl Scaling");
							}
						}
					}
					//Animation関連
					for (UINT i2 = 0; i2 < model->NumConnectionNode; i2++) {
						getAnimationCurve(model->connectionNode[i2], defo->Translation, "T");
						getAnimationCurve(model->connectionNode[i2], defo->Rotation, "R");
						getAnimationCurve(model->connectionNode[i2], defo->Scaling, "S");
					}
					return;
				}
			}
		}
	}
}

void FbxLoader::getPoseSub(NodeRecord *node, FbxMeshNode *mesh) {
	for (UINT i = 0; i < node->NumChildren; i++) {
		if (!strcmp(node->nodeChildren[i].className, "PoseNode")) {
			NodeRecord *n1 = &node->nodeChildren[i];
			for (UINT i2 = 0; i2 < n1->NumChildren; i2++) {
				if (!strcmp(n1->nodeChildren[i2].className, "Node")) {
					int64_t cnId = convertUCHARtoint64(&n1->nodeChildren[i2].Property[1]);
					getPoseSub2(cnId, &node->nodeChildren[i], mesh);//PoseNodeを渡す
					break;
				}
			}
		}
	}
}

void FbxLoader::getPose() {
	for (UINT i = 0; i < FbxRecord.NumChildren; i++) {
		if (!strcmp(FbxRecord.nodeChildren[i].className, "Objects")) {
			NodeRecord *n1 = &FbxRecord.nodeChildren[i];
			for (UINT i2 = 0; i2 < n1->NumChildren; i2++) {
				UINT mcnt = 0;
				if (!strcmp(n1->nodeChildren[i2].className, "Pose")) {//Poseの数とMeshの数が同じと仮定・・
					getPoseSub(&n1->nodeChildren[i2], &Mesh[mcnt++]);
				}
			}
			break;
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
		//byte列そのままで型変換する
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

void FbxLoader::ConvertUCHARtoint64_t(UCHAR *arr, int64_t *outArr, UINT outsize) {
	for (UINT i = 0; i < outsize; i++) {
		int addInd = i * 8;
		outArr[i] = (((int64_t)arr[7 + addInd] << 56) | ((int64_t)arr[6 + addInd] << 48) |
			((int64_t)arr[5 + addInd] << 40) | ((int64_t)arr[4 + addInd]) << 32 |
			((int64_t)arr[3 + addInd] << 24) | ((int64_t)arr[2 + addInd] << 16) |
			((int64_t)arr[1 + addInd] << 8) | ((int64_t)arr[addInd]));
	}
}

void FbxLoader::ConvertUCHARtofloat(UCHAR *arr, float *outArr, UINT outsize) {
	for (UINT i = 0; i < outsize; i++) {
		int addInd = i * 4;
		UINT32 tmp = (((UINT32)arr[3 + addInd] << 24) | ((UINT32)arr[2 + addInd] << 16) |
			((UINT32)arr[1 + addInd] << 8) | ((UINT32)arr[addInd]));
		//byte列そのままで型変換する
		float *fp = reinterpret_cast<float*>(&tmp);
		outArr[i] = *fp;
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
	aDELETE(Mesh);
	std::vector<ConnectionNo>().swap(cnNo);//解放
	std::vector<ConnectionList>().swap(cnLi);//解放
	rootNode = nullptr;
	Skeleton = nullptr;
}

bool FbxLoader::setFbxFile(char *pass) {
	FILE *fp;
	if (fopen_s(&fp, pass, "rb") != 0) {
		MessageBoxA(0, "ファイル読み込みエラー", 0, MB_OK);
		return false;
	}

	if (!fileCheck(fp))return false;
	searchVersion(fp);
	readFBX(fp);
	fclose(fp);
	getMesh();
	getPose();
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

FbxMeshNode *FbxLoader::getFbxMeshNode(UINT index) {
	return &Mesh[index];
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