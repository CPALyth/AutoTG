#include "pch.h"

#include <windows.h>
#include <random>
#include <cstdio>
#include <iostream>  
#include <fstream>  
#include <cassert>
#include <string>  
#include <vector>  
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/des.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include "reg.h"
#include "tlv_box.h"
#include "utils.h"


using namespace std;
using namespace tlv;

#define BUFFER_SIZE 4096
#define REG_CODE_BUFFER_SIZE 21
#define MACHINE_CODE_BUFFER_SIZE 21
#define CODE_BUFFER_SIZE 11
// ---- rsa�ǶԳƼӽ��� ---- //  
#define KEY_LENGTH  2048               // ��Կ����
#define PUB_KEY_FILE "pubkey.pem"    // ��Կ·��
#define PRI_KEY_FILE "prikey.pem"    // ˽Կ·��






// ---- md5ժҪ��ϣ ---- //  
static int md5File(const char* inFileName, std::string &encodedStr)
{
#define MAXDATABUFF 1024
#define MD5LENTH 17
	ifstream ifile(inFileName, ios::in | ios::binary);    //���ļ�
	unsigned char MD5result[MD5LENTH] = { 0 };
	do
	{
		if (ifile.fail())   //��ʧ�ܲ����ļ�MD5
		{
			break;
		}
		MD5_CTX md5_ctx;
		MD5_Init(&md5_ctx);

		char DataBuff[MAXDATABUFF];
		while (!ifile.eof())
		{
			ifile.read(DataBuff, MAXDATABUFF);   //���ļ�
			int length = (int)ifile.gcount();
			if (length)
			{
				MD5_Update(&md5_ctx, DataBuff, length);   //����ǰ�ļ�����벢����MD5
			}
		}
		MD5_Final(MD5result, &md5_ctx);  //��ȡMD5
	// ��ϣ����ַ���  
		// ��ϣ���ʮ�����ƴ� 32�ֽ�  
		char buf[65] = { 0 };
		char tmp[3] = { 0 };
		for (int i = 0; i < MD5LENTH-1; i++)
		{
			sprintf_s(tmp, "%02x", MD5result[i]);
			strcat_s(buf, tmp);
		}
		buf[32] = '\0'; // ���涼��0����32�ֽڽض�  
		encodedStr = std::string(buf);
		return 1;
	} while (false);
	return 0;
}


// ---- md5ժҪ��ϣ ---- //  
static void md5(const std::string &srcStr, std::string &encodedStr)
{
	// ����md5��ϣ  
	unsigned char mdStr[33] = { 0 };
	MD5((const unsigned char *)srcStr.c_str(), srcStr.length(), mdStr);

	// ��ϣ���ʮ�����ƴ� 32�ֽ�  
	char buf[65] = { 0 };
	char tmp[3] = { 0 };
	for (int i = 0; i < 32; i++)
	{
		sprintf_s(tmp, "%02x", mdStr[i]);
		strcat_s(buf, tmp);
	}
	buf[32] = '\0'; // ���涼��0����32�ֽڽض�  
	encodedStr = std::string(buf);
}
// ---- sha256ժҪ��ϣ ---- //  
static void sha256(const std::string &srcStr, std::string &encodedStr)
{
	// ����sha256��ϣ  
	unsigned char mdStr[33] = { 0 };
	SHA256((const unsigned char *)srcStr.c_str(), srcStr.length(), mdStr);

	// ��ϣ���ʮ�����ƴ� 32�ֽ�  
	char buf[65] = { 0 };
	char tmp[3] = { 0 };
	for (int i = 0; i < 32; i++)
	{
		sprintf_s(tmp, "%02x", mdStr[i]);
		strcat_s(buf, tmp);
	}
	buf[32] = '\0'; // ���涼��0����32�ֽڽض�  
	encodedStr = std::string(buf);
}

// ---- des�ԳƼӽ��� ---- //  
// ���� ecbģʽ  
static std::string des_encrypt(const std::string &clearText, const std::string &key)
{
	std::string cipherText; // ����  

	DES_cblock keyEncrypt;
	memset(keyEncrypt, 0, 8);

	// ���첹������Կ  
	if (key.length() <= 8)
		memcpy(keyEncrypt, key.c_str(), key.length());
	else
		memcpy(keyEncrypt, key.c_str(), 8);

	// ��Կ�û�  
	DES_key_schedule keySchedule;
	DES_set_key_unchecked(&keyEncrypt, &keySchedule);

	// ѭ�����ܣ�ÿ8�ֽ�һ��  
	const_DES_cblock inputText;
	DES_cblock outputText;
	std::vector<unsigned char> vecCiphertext;
	unsigned char tmp[8];

	for (int i = 0; i < (int)clearText.length() / 8; i++)
	{
		memcpy(inputText, clearText.c_str() + i * 8, 8);
		DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_ENCRYPT);
		memcpy(tmp, outputText, 8);

		for (int j = 0; j < 8; j++)
			vecCiphertext.push_back(tmp[j]);
	}

	if (clearText.length() % 8 != 0)
	{
		int tmp1 = clearText.length() / 8 * 8;
		int tmp2 = clearText.length() - tmp1;
		memset(inputText, 0, 8);
		memcpy(inputText, clearText.c_str() + tmp1, tmp2);
		// ���ܺ���  
		DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_ENCRYPT);
		memcpy(tmp, outputText, 8);

		for (int j = 0; j < 8; j++)
			vecCiphertext.push_back(tmp[j]);
	}

	cipherText.clear();
	cipherText.assign(vecCiphertext.begin(), vecCiphertext.end());

	return cipherText;
}

// ���� ecbģʽ  
static std::string des_decrypt(const std::string &cipherText, const std::string &key)
{
	std::string clearText; // ����  

	DES_cblock keyEncrypt;
	memset(keyEncrypt, 0, 8);

	if (key.length() <= 8)
		memcpy(keyEncrypt, key.c_str(), key.length());
	else
		memcpy(keyEncrypt, key.c_str(), 8);

	DES_key_schedule keySchedule;
	DES_set_key_unchecked(&keyEncrypt, &keySchedule);

	const_DES_cblock inputText;
	DES_cblock outputText;
	std::vector<unsigned char> vecCleartext;
	unsigned char tmp[8];

	for (int i = 0; i < (int)cipherText.length() / 8; i++)
	{
		memcpy(inputText, cipherText.c_str() + i * 8, 8);
		DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_DECRYPT);
		memcpy(tmp, outputText, 8);

		for (int j = 0; j < 8; j++)
			vecCleartext.push_back(tmp[j]);
	}

	if (cipherText.length() % 8 != 0)
	{
		int tmp1 = cipherText.length() / 8 * 8;
		int tmp2 = cipherText.length() - tmp1;
		memset(inputText, 0, 8);
		memcpy(inputText, cipherText.c_str() + tmp1, tmp2);
		// ���ܺ���  
		DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_DECRYPT);
		memcpy(tmp, outputText, 8);

		for (int j = 0; j < 8; j++)
			vecCleartext.push_back(tmp[j]);
	}

	clearText.clear();
	clearText.assign(vecCleartext.begin(), vecCleartext.end());

	return clearText;
}







// ��Կ����  
static int rsa_pub_encrypt(const unsigned char* in, int inLen, unsigned char* out, int outLen, const std::string &key)
{
	RSA *rsa = NULL;
	BIO *keybio = BIO_new_mem_buf((unsigned char *)key.c_str(), -1);
	// �˴������ַ���
	// 1, ��ȡ�ڴ������ɵ���Կ�ԣ��ٴ��ڴ�����rsa
	// 2, ��ȡ���������ɵ���Կ���ı��ļ����ڴ��ڴ�����rsa
	// 3��ֱ�ӴӶ�ȡ�ļ�ָ������rsa
	rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	//int nLen = RSA_size(rsa);
	//char* pEncode = new char[nLen + 1];
	// ���ܺ���
	int ret = RSA_public_encrypt(inLen, in, out, rsa, RSA_PKCS1_PADDING);

	// �ͷ��ڴ�
	BIO_free_all(keybio);
	RSA_free(rsa);

	return ret;
}
// ��Կ����  
static int rsa_pub_decrypt(const unsigned char* in, int inLen, unsigned char* out, int outLen, const string &key)
{
	RSA *rsa = NULL;
	BIO *keybio;
	keybio = BIO_new_mem_buf((unsigned char *)key.c_str(), -1);

	// �˴������ַ���
	// 1, ��ȡ�ڴ������ɵ���Կ�ԣ��ٴ��ڴ�����rsa
	// 2, ��ȡ���������ɵ���Կ���ı��ļ����ڴ��ڴ�����rsa
	// 3��ֱ�ӴӶ�ȡ�ļ�ָ������rsa
	rsa = PEM_read_bio_RSA_PUBKEY(keybio, NULL, NULL, NULL);

	// ���ܺ���
	int ret = RSA_public_decrypt(inLen, in, out, rsa, RSA_PKCS1_PADDING);

	// �ͷ��ڴ�
	BIO_free_all(keybio);
	RSA_free(rsa);

	return ret;
}
// ˽Կ����  
static int rsa_pri_encrypt(const unsigned char* in, int inLen, unsigned char* out, int outLen, const std::string &key)
{
	std::string strRet;
	RSA *rsa = NULL;
	BIO *keybio = BIO_new_mem_buf((unsigned char *)key.c_str(), -1);
	// �˴������ַ���
	// 1, ��ȡ�ڴ������ɵ���Կ�ԣ��ٴ��ڴ�����rsa
	// 2, ��ȡ���������ɵ���Կ���ı��ļ����ڴ��ڴ�����rsa
	// 3��ֱ�ӴӶ�ȡ�ļ�ָ������rsa
	rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);

	// ���ܺ���
	int ret = RSA_private_encrypt(inLen, in, out, rsa, RSA_PKCS1_PADDING);

	// �ͷ��ڴ�
	BIO_free_all(keybio);
	RSA_free(rsa);

	return ret;
}
// ˽Կ����  
static int rsa_pri_decrypt(const unsigned char* in, int inLen, unsigned char* out, int outLen, const std::string &key)
{
	RSA *rsa = NULL;
	BIO *keybio;
	keybio = BIO_new_mem_buf((unsigned char *)key.c_str(), -1);

	// �˴������ַ���
	// 1, ��ȡ�ڴ������ɵ���Կ�ԣ��ٴ��ڴ�����rsa
	// 2, ��ȡ���������ɵ���Կ���ı��ļ����ڴ��ڴ�����rsa
	// 3��ֱ�ӴӶ�ȡ�ļ�ָ������rsa
	rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);

	// ���ܺ���
	int ret = RSA_private_decrypt(inLen, in,out, rsa, RSA_PKCS1_PADDING);

	// �ͷ��ڴ�
	BIO_free_all(keybio);
	RSA_free(rsa);

	return ret;
}

// �ļ�����  
int file_encrypt2file(const char* inFileName, const char* outFileName)
{
	ifstream inFile;
	ofstream outFile;

	inFile.open(inFileName, ofstream::binary);
	outFile.open(outFileName, ofstream::binary | ofstream::trunc);
	if (inFile.is_open() && outFile.is_open())
	{

		inFile.seekg(0, inFile.end);
		int length = (int)inFile.tellg();
		inFile.seekg(0, inFile.beg);
		char * buffer = new char[length];
		if (buffer)
		{
			inFile.read(buffer, length);

			for (int i = 0; i < length; i++)
			{
				buffer[i] ^= 0xa;
			}
			outFile.write(buffer, length);
			delete[] buffer;
			inFile.close();
			outFile.close();
			return 1;
		}
		inFile.close();
		outFile.close();
	}

	return 0;
}
// �ļ�����  
int file_decrypt2buf(const char* inFileName, char** outbuf, int *len)
{
	ifstream inFile;

	inFile.open(inFileName, ofstream::binary);
	if (inFile.is_open())
	{

		inFile.seekg(0, inFile.end);
		int length = (int)inFile.tellg();
		inFile.seekg(0, inFile.beg);
		char * buffer = new char[length];
		if (buffer)
		{
			inFile.read(buffer, length);

			for (int i = 0; i < length; i++)
			{
				buffer[i] ^= 0xa;
			}
			*outbuf =  buffer;
			inFile.close();
			*len = length;
			return 1;
		}

	}
	*outbuf = nullptr;
	inFile.close();
	*len = 0;
	return 0;
}
// �ļ���MD5
int file_MD5toFile(const char* inFileName, const char* outFileName)
{
	ofstream outFile;
	string encodedStr;
	outFile.open(outFileName,ofstream::trunc);
	if (outFile.is_open())
	{
		if (1 == md5File(inFileName, encodedStr))
		{
			outFile << encodedStr.c_str();
			outFile.close();
			return 1;
		}
		outFile.close();
	}

	return 0;
}
// �ļ���MD5
int file_MD5toString(const char* inFileName, std::string &encodedStr)
{
	return md5File(inFileName, encodedStr);
}


static string genRandString()
{
	char buf[CODE_BUFFER_SIZE];
	random_device rd;
	uniform_int_distribution<int> uid('A', 'Z');
	for (int i = 0; i < (CODE_BUFFER_SIZE - 1); i++)
	{
		buf[i] = uid(rd);
	}
	buf[CODE_BUFFER_SIZE-1] = 0;
	return buf;

}
string  genCode()
{
	//��ȡCpuID
	unsigned long s1, s2;
	char sel;
	sel = '1';
	char CPUID[33], CPUID1[17], CPUID2[17];
	__asm {
		mov eax, 01h
		xor edx, edx
		cpuid
		mov s1, edx
		mov s2, eax
	}
	sprintf_s(CPUID1, "%08X%08X", s1, s2);
	__asm {
		mov eax, 03h
		xor ecx, ecx
		xor edx, edx
		cpuid
		mov s1, edx
		mov s2, ecx
	}
	sprintf_s(CPUID2, "%08X%08X", s1, s2);
	sprintf_s(CPUID, "%s%s", CPUID1, CPUID2);
	//��ȡ�����Ϣ
	DWORD ser;
	char namebuf[256];
	char filebuf[256];
	GetVolumeInformationA("c:\\", namebuf, 256, &ser, 0, 0, filebuf, 256);
	char DiskInfo[64];
	sprintf_s(DiskInfo, "%d", ser);
	char code[CODE_BUFFER_SIZE];
	memcpy(code, CPUID + 12, 5);//��MyCpuID�ĵ�13λ��ʼȡ5��
	memcpy(code + 5, DiskInfo + 1, 5);//��strdisk�ĵ�2λ��ʼȡ5��,�ϲ����ɻ�����
	code[CODE_BUFFER_SIZE-1] = 0;
	//���ػ�е��

	return code;

}
string  genMachine()
{
	//���ɴ���
	string code = genCode();
	//���������
	string rand = genRandString();
	char machine[MACHINE_CODE_BUFFER_SIZE];
	for (int i = 0,j = 0; i < (MACHINE_CODE_BUFFER_SIZE-1); i++,j++)
	{
		machine[i] = rand[j];
		machine[++i] = code[j];
	}
	machine[MACHINE_CODE_BUFFER_SIZE - 1] = 0;
	//���ػ�е��
	return machine;

}
string  getCodeFromMachine(string machine)
{
	if (machine.length() != (MACHINE_CODE_BUFFER_SIZE - 1))
	{
		return "";
	}
	char code[CODE_BUFFER_SIZE];
	for (int i = 0, j = 0; i < (MACHINE_CODE_BUFFER_SIZE-1) && i < (int)machine.length() ; i++,j++)
	{
		code[j] = machine[++i];
	}
	code[CODE_BUFFER_SIZE - 1] = 0;
	return code;

}
int32_t  setMachineToFile(const string &fileName)
{
	ofstream file;

	file.open(fileName, ofstream::trunc);
	if (file.is_open())
	{
		string  machine = genMachine();

		file << machine.c_str();
		file.close();
		return 1;
	}
	return 0;
}
int32_t  setRegCodeToFile(const string &fileName, const string &key, const string &machine, const string &srv, const string &port, const string &genDate, const string &validDate, int32_t version, int32_t type)
{
	unsigned char enbuf[BUFFER_SIZE] = { 0 };
	TlvBox tlv;
	tlv.PutStringValue(1, machine);
	tlv.PutStringValue(2, srv);
	tlv.PutStringValue(3, genDate);
	tlv.PutStringValue(4, validDate);
	tlv.PutIntValue(5, version);
	tlv.PutIntValue(6, type);
	tlv.PutStringValue(7, port);

	tlv.Serialize();
	ofstream file;

	file.open(fileName, ofstream::binary | ofstream::trunc);
	if (file.is_open())
	{
		int ret = rsa_pri_encrypt(tlv.GetSerializedBuffer(), tlv.GetSerializedBytes(), enbuf, sizeof(enbuf), key);
		if (ret > 0)
		{
			file.write((char*)enbuf, ret);
			file.close();
			return 1;
		}
	}
	return 0;
}
int32_t  getRegCodeFromFile(const string &fileName, const string &key, string &machine, string &srv, string &port, string &genDate, string &validDate, int32_t &version, int32_t &type)
{
	int ret = 0;
	unsigned char desbuf[BUFFER_SIZE] = { 0 };
	ifstream file;
	file.open(fileName, ifstream::binary);
	if (file.is_open())
	{
		file.seekg(0, ifstream::end);
		int fileLen = (int)file.tellg();
		// allocate memory:
		unsigned char * buf = new unsigned char[fileLen];
		file.seekg(0, ifstream::beg);
		file.read((char*)buf, fileLen);
		file.close();
		int len = rsa_pub_decrypt(buf, fileLen, desbuf, sizeof(desbuf), key);
		if (len > 0)
		{
			TlvBox tlv;
			if (true == tlv.Parse(desbuf, len))
			{
				tlv.GetStringValue(1, machine);
				tlv.GetStringValue(2, srv);
				tlv.GetStringValue(3, genDate);
				tlv.GetStringValue(4, validDate);
				tlv.GetIntValue(5, version);
				tlv.GetIntValue(6, type);
				tlv.GetStringValue(7, port);
				ret = 1;
			}
		}
		delete[] buf;

	}

	return ret;
}
// �����з������ɹ�˽Կ�ԣ�begin public key/ begin private key��
// �ҵ�openssl�����й��ߣ���������
// openssl genrsa -out prikey.pem 2048 
// openssl rsa -in prikey.pem -pubout -out pubkey.pem
// ��������������Կ�� 
void generateRSAKey(string strKey[2])
{
	// ��˽��Կ��  
	size_t pri_len;
	size_t pub_len;
	char *pri_key = NULL;
	char *pub_key = NULL;

	// ������Կ��  
	RSA *keypair = RSA_generate_key(KEY_LENGTH, RSA_3, NULL, NULL);

	BIO *pri = BIO_new(BIO_s_mem());
	BIO *pub = BIO_new(BIO_s_mem());

	PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);
	PEM_write_bio_RSAPublicKey(pub, keypair);

	// ��ȡ����  
	pri_len = BIO_pending(pri);
	pub_len = BIO_pending(pub);

	// ��Կ�Զ�ȡ���ַ���  
	pri_key = (char *)malloc(pri_len + 1);
	pub_key = (char *)malloc(pub_len + 1);

	BIO_read(pri, pri_key, pri_len);
	BIO_read(pub, pub_key, pub_len);

	pri_key[pri_len] = '\0';
	pub_key[pub_len] = '\0';

	// �洢��Կ��  
	strKey[0] = pub_key;
	strKey[1] = pri_key;

	// �ڴ��ͷ�
	RSA_free(keypair);
	BIO_free_all(pub);
	BIO_free_all(pri);

	free(pri_key);
	free(pub_key);
}
void setRSAKeyToFile()
{
	// ��˽��Կ��  
	size_t pri_len;
	size_t pub_len;
	char *pri_key = NULL;
	char *pub_key = NULL;

	// ������Կ��  
	RSA *keypair = RSA_generate_key(KEY_LENGTH, RSA_3, NULL, NULL);

	BIO *pri = BIO_new(BIO_s_mem());
	BIO *pub = BIO_new(BIO_s_mem());

	PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);
	PEM_write_bio_RSAPublicKey(pub, keypair);

	// ��ȡ����  
	pri_len = BIO_pending(pri);
	pub_len = BIO_pending(pub);

	// ��Կ�Զ�ȡ���ַ���  
	pri_key = (char *)malloc(pri_len + 1);
	pub_key = (char *)malloc(pub_len + 1);

	BIO_read(pri, pri_key, pri_len);
	BIO_read(pub, pub_key, pub_len);

	pri_key[pri_len] = '\0';
	pub_key[pub_len] = '\0';

	// �洢�����̣����ַ�ʽ�洢����begin rsa public key/ begin rsa private key��ͷ�ģ�
	FILE *pubFile;
	fopen_s(&pubFile, PUB_KEY_FILE, "w");
	if (pubFile == NULL)
	{
		assert(false);
		return;
	}
	fputs(pub_key, pubFile);
	fclose(pubFile);

	FILE *priFile;
	fopen_s(&priFile, PRI_KEY_FILE, "w");
	if (priFile == NULL)
	{
		assert(false);
		return;
	}
	fputs(pri_key, priFile);
	fclose(priFile);

	// �ڴ��ͷ�
	RSA_free(keypair);
	BIO_free_all(pub);
	BIO_free_all(pri);

	free(pri_key);
	free(pub_key);
}

string getPcName()
{
	const int MAX_BUFFER_LEN = 500;
	TCHAR  szBuffer[MAX_BUFFER_LEN];
	DWORD dwNameLen;
	string pcName;
	string userName;

	//get pc name
	dwNameLen = MAX_BUFFER_LEN;
	if (GetComputerName((LPWSTR)szBuffer, &dwNameLen))
	{
		wstring ws;
		ws.append(szBuffer);
		pcName = WStringToString(ws);
	}
	//get user name
	dwNameLen = MAX_BUFFER_LEN;
	if (GetUserName((LPWSTR)szBuffer, &dwNameLen))
	{
		wstring ws;
		ws.append(szBuffer);
		userName = WStringToString(ws);
	}

	return pcName + "-" + userName;
}

