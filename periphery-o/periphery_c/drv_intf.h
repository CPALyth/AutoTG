#pragma once
#include <iostream>
#include <string>
#include <windows.h>
#include <fflua.h>


class DrvIntf
{
public:
	DrvIntf();
	~DrvIntf();
public:
	// ����ͨѶ�ٶȣ���֧���¿�Ӳ����
	int32_t setSpeed(unsigned char Keyboard, unsigned char Mouse, unsigned char Control);
	// �����豸
	int32_t restart();
	// ���豸��ָ���豸ID��
	int32_t openDevice(int32_t venderID, int32_t productID);
	// �ر��豸
	int32_t closeDevice();
	// �����Զ����豸ID������ID+��ƷID��
	int32_t setDeviceID(int32_t venderID, int32_t productID);


	// ������
	int32_t keyDown(std::string key);
	// ������
	int32_t keyUp(std::string  key);
	// һ�ΰ���
	int32_t keyPress(std::string  key, int32_t count);
	// ��ϼ�����
	int32_t combinationKeyDown(std::string  key1, std::string  key2, std::string  key3, std::string  key4, std::string  key5, std::string  key6);
	// ��ϼ�����
	int32_t combinationKeyUp(std::string  key1, std::string  key2, std::string  key3, std::string  key4, std::string  key5, std::string  key6);
	// ��ϰ���
	int32_t combinationKeyPress(std::string  key1, std::string  key2, std::string  key3, std::string  key4, std::string  key5, std::string  key6, int32_t count);
	// �ͷ����а���
	int32_t keyUpAll();
	// ģ�ⰴ������
	int32_t simulationPressKey(std::string keys);
	// ��ȡ��д��״̬
	int32_t getCapsLock();
	// ��ȡNumLock��״̬
	int32_t getNumLock();


	// ����������
	int32_t leftDown();
	// ����������
	int32_t leftUp();
	// ����������
	int32_t leftClick(int32_t count);
	// ������˫��
	int32_t leftDoubleClick(int32_t count);
	// ����Ҽ�����
	int32_t rightDown();
	// ����Ҽ�����
	int32_t rightUp();
	// ����Ҽ�����
	int32_t rightClick(int32_t count);
	// ����Ҽ�˫��
	int32_t rightDoubleClick(int32_t count);
	// ����м�����
	int32_t middleDown();
	// ����м�����
	int32_t middleUp();
	// ����м�����
	int32_t middleClick(int32_t count);
	// ����м�˫��
	int32_t middleDoubleClick(int32_t count);
	// �����ֹ���
	int32_t wheelsMove(int32_t y);
	// �ͷ�������갴��
	int32_t mouseUpAll();
	// ģ������ƶ�
	int32_t moveTo(int32_t x, int32_t y);
	// ����ƶ����(X��Y���ܴ���255)
	int32_t moveToR(int32_t x, int32_t y);
	// ��λ�ƶ����
	int32_t resetMouse();
private:
	HINSTANCE m_hdllInst;
	bool m_open;
};

void reg_drv_intf_lapi(lua_State *ls);
