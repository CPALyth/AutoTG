// periphery_c.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include <atomic> 
#include <signal.h>
#include <iostream>
#include <string>

#include <windows.h>
#undef ERROR
#include <tlhelp32.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <glog/logging.h>
#include <gflags/gflags.h>

#include "capture_screen.h"
#include "drv_intf.h"
#include "win_intf.h"
#include "cap_intf.h"
#include "utils_intf.h"
#include "reg.h"
#include "conf.h"
#include "utils.h"
#include "client.h"

using namespace std;
using namespace cv;
using namespace ff;


static string pubkey = "-----BEGIN PUBLIC KEY-----\n"\
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzj9+ba4GBJoTXQZNm9+6\n"\
"AHri/R6dOjHEFrr9mXukg/Y0MHhLZ9v5zID6UcrTVltcVZ/b/5UfiypNIBqqcn8T\n"\
"VjRza9fCLuIZeSjClRXeFjPJX8RG110y6IaI2RX9QGMLfmB7GHWuSlOFrf8z+9rC\n"\
"ANZRZUtGfgvay+q20Xp0yxW9mEwC5ZgYKrt5WcanollJbvCd5JbcoTWfW9sxkkuc\n"\
"we/A7C+rQouHmCJp33j/3JV4RhajVBJIER+iKINUyh21nU7g4yAUOr/337qpPeKV\n"\
"Id6BycUBMi8w+vSoTGuR0DppEiFBWLbzANp7JBQYqI2tn+ZoFj40tu/WNV9JxCMj\n"\
"JwIDAQAB\n"\
"-----END PUBLIC KEY-----\n";


#ifdef PRIVATE_VERSION
static string prikey = "-----BEGIN RSA PRIVATE KEY-----\n"\
"MIIEpQIBAAKCAQEAzj9+ba4GBJoTXQZNm9+6AHri/R6dOjHEFrr9mXukg/Y0MHhL\n"\
"Z9v5zID6UcrTVltcVZ/b/5UfiypNIBqqcn8TVjRza9fCLuIZeSjClRXeFjPJX8RG\n"\
"110y6IaI2RX9QGMLfmB7GHWuSlOFrf8z+9rCANZRZUtGfgvay+q20Xp0yxW9mEwC\n"\
"5ZgYKrt5WcanollJbvCd5JbcoTWfW9sxkkucwe/A7C+rQouHmCJp33j/3JV4Rhaj\n"\
"VBJIER+iKINUyh21nU7g4yAUOr/337qpPeKVId6BycUBMi8w+vSoTGuR0DppEiFB\n"\
"WLbzANp7JBQYqI2tn+ZoFj40tu/WNV9JxCMjJwIDAQABAoIBAQDM5ELjDuinlbD6\n"\
"zxI2cbHEXFA3iTxE3b+hnS3mVfB/sAz0weyXRu8H/HGw47/DoIs//Ml/RFPL2sA3\n"\
"zoXZrOg20XCAiB0+mhsYRCfaF5lDkTrSSpjdxpaWbWBx2Oh4GG0IpSxkhjQZXpQE\n"\
"4OxxnUbxsfe2m2tyOLCpt4ja3b49KJlGuG34yusUZTzrU114unPYH/WsuifMxP9B\n"\
"Zpe4ihgZoeJwCunDvIa1csrR3CU7hm58qwfmNTAAmANeSColTTMAXD9fs1SZ++tW\n"\
"8D9Npx1PlkGBuY6XR0Q+0zHyXnj57aimjLCV8HUUEwynwR4fZRoVF13mwub1lWdI\n"\
"Y0bbPeABAoGBAOqFYNo5ugzZe/4PnzccuTXNkMU0rgH1BAdmgUXjR66QiX5dfBlj\n"\
"Z36tt24HgzRlw+9iyBRbpTcGVpiImVj3YH87G0a2sYFrlAOzQh4VoLQg0f8w4CVa\n"\
"mzjN5FzeewhbAu4/O5FO7k6WKALVz85FnAiI4VdGPfEKU1GYl8qaawsnAoGBAOEj\n"\
"ONZ0jDwHwbi8zbsKs638EF9lfga1O/th9H4x+hcGQc9DHUGVokaD6U3A5BSeBbiI\n"\
"PBFYLM7l5xIb3qhfGaFQQDOaMj0p//8D0q5RaphucrMmd5f+x/X7ST5QWGiVmTxx\n"\
"6XE1nzvYhM7g4MdcWVfK/enOWrHZLwXUB73UdigBAoGBANA2VLPKtqS73jrFEd0z\n"\
"UjgzZsm/KJIyPz52wmUTEUwppPic9RXxTCyK/KTiZJl7lEaPlAWHDBrbNLy94i+7\n"\
"iV/MUT4p61R093eFijj9iK0Dyo1fjlF7rP30xj+xgtf7PZHab/9lbWw2IhXKUHHZ\n"\
"GVv2cuIyScIcIbb63CETkd/XAoGAPbRTnaNv0/wkQietEziJqoPNUPgWJGyDthBQ\n"\
"3E2CR9E1NEMmv5TtVvpMdT0KxgPReZQNMovCLCqivDnbOrZl5eqCziHS5ySdvx0j\n"\
"Hxe9aW3MDgHbAVOZJnfqySNoN7HYv9JQsvCJZfOf4ruLn9hVvFQsZ2phPSFTW/uS\n"\
"65pRkAECgYEApKEkkUSWza4U6EpiWG0CgjRgPXm5yKSu0aw3ezerIaIewzG7D+ux\n"\
"r6DX94KjRzaTi0aAtspyW3Ng0OzszWTEDlYHtRxIptAcW5DGW+Ey8oyvl4tESzzh\n"\
"gCm01AUo+02eeV44hI4J0paNf+IBqX/QflEwaHT1MDW9WREQx+IUDUo=\n"\
"-----END RSA PRIVATE KEY-----\n";


DEFINE_bool(genlic, false, "program generate license");
DEFINE_string(lconfig, "gpl.conf", "program license config file name");
DEFINE_string(licfileout, "", "program lic out file");
DEFINE_bool(genenc, false, "program generate encrypt file");
DEFINE_string(encfilein, "", "program encrypt in file");
DEFINE_string(encfileout, "", "program encrypt out file");
DEFINE_bool(genmd5, false, "program generate md5 by file");
DEFINE_string(md5filein, "", "program md5 from file");
DEFINE_string(md5fileout, "", "program md5 out file");

#endif // PRIVATE_VERSION
DEFINE_bool(genmc, false, "program generate host machine code");
DEFINE_string(mcfileout, "gp.mc", "program machine file name");
DEFINE_bool(run, false, "program run script");
DEFINE_bool(wait, false, "program not exit");
DEFINE_bool(direct, false, "program direct");
DEFINE_bool(enc, false, "program encrypt mode");
DEFINE_bool(debug, false, "program debug mode");
DEFINE_bool(control, true, "program control mode");
DEFINE_string(lic, "gp.lic", "program license file name");
DEFINE_uint32(step, 1, "program direct step");
DEFINE_uint32(role, 1, "program direct role");
DEFINE_string(script, "gp.script", "program script file name");
DEFINE_string(sconfig, "gps.conf", "program script config file name");
DEFINE_string(srv_addr, "gp.dtflavour.com", "program server addr");
DEFINE_string(srv_port, "8888", "program server port");
DEFINE_string(ntp_addr, "time.nist.gov", "program ntp server addr");
DEFINE_string(uconfig, "gpu.conf", "program account config file name");

//global variable
HANDLE g_hwt[2] = { INVALID_HANDLE_VALUE };
#define GPATC_VERSION "100"
string g_user;
string g_pwd;
string g_uuid;
string g_price;
string g_ver;
string g_srv;
string g_port;
ClientInfo g_client_info;
atomic_long g_stop(0);
atomic_long g_script_run(0);
bool g_suspend = false;
bool g_first = true;




static BOOL resume_thread(DWORD dwOwnerPID)
{
	HANDLE        hThreadSnap = NULL;
	BOOL          bRet = FALSE;
	THREADENTRY32 te32 = { 0 };

	// Take a snapshot of all threads currently in the system. 

	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return (FALSE);

	// Fill in the size of the structure before using it. 

	te32.dwSize = sizeof(THREADENTRY32);

	// Walk the thread snapshot to find all threads of the process. 
	// If the thread belongs to the process, add its information 
	// to the display list.

	if (Thread32First(hThreadSnap, &te32))
	{
		do
		{
			if (te32.th32OwnerProcessID == dwOwnerPID)
			{
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
				if (g_suspend)
				{
					ResumeThread(hThread);
				}
				else
				{
					SuspendThread(hThread);
				}

				CloseHandle(hThread);
			}
		} while (Thread32Next(hThreadSnap, &te32));
		bRet = TRUE;
		if (g_suspend)
		{
			g_suspend = false;
		}
		else
		{
			g_suspend = true;
		}
	}
	else
		bRet = FALSE;          // could not walk the list of threads 

	// Do not forget to clean up the snapshot object. 
	CloseHandle(hThreadSnap);

	return (bRet);
}





static void check_lic()
{
	char *msg;
	string machine;
	string srv;
	string port;
	string genDate;
	string validDate;
	int32_t version;
	int32_t type;
	int32_t ret = getRegCodeFromFile(FLAGS_lic, pubkey, machine, srv, port, genDate, validDate, version, type);
	if (!ret)
	{
		msg = "00 failed";
		throw msg;
	}
	//get info
	g_ver = std::to_string(version);
	if (!srv.empty())
	{
		g_srv = srv;
	}
	if (!port.empty())
	{
		g_port = port;
	}
	//check
	if (type == 0)
	{

		ULONGLONG checkTime = StrTime2LongTime(validDate);
		ULONGLONG curTime = GetNetTime("");
		if (curTime > checkTime)
		{
			msg = "11 failed";
			throw msg;
		}
	}
	else if (type == 1)
	{
		string curCode = genCode();
		string checkCode = getCodeFromMachine(machine);
		if (curCode != checkCode)
		{
			msg = "21 failed";
			throw msg;
		}
		ULONGLONG checkTime = StrTime2LongTime(validDate);
		ULONGLONG curTime = GetNetTime("");
		if (curTime > checkTime)
		{
			msg = "22 failed";
			throw msg;
		}
	}

}

static void check_user()
{
	char *msg;

	if (!FLAGS_uconfig.empty())
	{
		conf conf_file(FLAGS_uconfig);
		if (conf_file.isLoaded())
		{
			g_user = conf_file.get<string>("ui.user", "");
			g_pwd = conf_file.get<string>("ui.pwd", "");
			g_uuid = conf_file.get<string>("ui.uuid", "");
			g_price = conf_file.get<string>("ui.price", "");
			int ret_login = 0;
			if (g_srv.empty() || g_port.empty())
			{
				g_client_info.srv = FLAGS_srv_addr;
				g_client_info.port = FLAGS_srv_port;
				g_client_info.user = g_user;
				g_client_info.pwd = g_pwd;
				g_client_info.uuid = g_uuid;
				g_client_info.ver = g_ver;
				g_client_info.price = g_price;

				ret_login = client_login(FLAGS_srv_addr, FLAGS_srv_port, g_user, g_pwd, g_uuid, g_ver, g_price);
			}
			else
			{
				g_client_info.srv = g_srv;
				g_client_info.port = g_port;
				g_client_info.user = g_user;
				g_client_info.pwd = g_pwd;
				g_client_info.uuid = g_uuid;
				g_client_info.ver = g_ver;
				g_client_info.price = g_price;

				ret_login = client_login(g_srv, g_port, g_user, g_pwd, g_uuid, g_ver, g_price);
			}
			if (1 != ret_login)
			{
				msg = "33 failed";
				throw msg;
			}
		}
		else
		{
			msg = "32 failed";
			throw msg;
		}
	}
	else
	{
		msg = "31 failed";
		throw msg;
	}

}

static void lua_reg(fflua_t *fflua, lua_State* ls)
{
	reg_drv_intf_lapi(ls);
	reg_win_intf_lapi(ls);
	reg_cap_intf_lapi(ls);
	//misc
	reg_utils_intf_lapi(fflua);

}

static int genmc_proc()
{
	int ret;
	ret = setMachineToFile(FLAGS_mcfileout);
	return ret;
}

#ifdef PRIVATE_VERSION

static int genlic_proc()
{
	int ret = 0;
	if (!FLAGS_lconfig.empty())
	{
		conf conf_file(FLAGS_lconfig);
		if (conf_file.isLoaded())
		{
			string machine = conf_file.get<string>("mc.machine", "");
			string srv = conf_file.get<string>("mc.srv", "");
			string port = conf_file.get<string>("mc.port", "");
			string genDate = conf_file.get<string>("mc.gen_date", "1970-01-01 00:00:00");
			string validDate = conf_file.get<string>("mc.valid_date", "1970-01-01 00:00:00");
			int32_t version = conf_file.get<int>("mc.version", 1);
			int32_t type = conf_file.get<int>("mc.type", 1);
			ret = setRegCodeToFile(FLAGS_licfileout, prikey, machine, srv, port, genDate, validDate, version, type);
		}
	}
	return ret;
}
static int genenc_proc()
{
	int ret = 0;
	cout << FLAGS_encfilein << endl;
	cout << FLAGS_encfileout << endl;

	if (!FLAGS_encfilein.empty() && !FLAGS_encfileout.empty())
	{
		ret = file_encrypt2file(FLAGS_encfilein.c_str(), FLAGS_encfileout.c_str());

	}
	return ret;
}
static int genmd5_proc()
{
	int ret = 0;
	cout << FLAGS_md5filein << endl;
	cout << FLAGS_md5fileout << endl;

	if (!FLAGS_md5filein.empty() && !FLAGS_md5fileout.empty())
	{
		ret = file_MD5toFile(FLAGS_md5filein.c_str(), FLAGS_md5fileout.c_str());

	}
	return ret;
}
#endif

static DWORD WINAPI run_proc(LPVOID lpParameter)
//static int run_proc()
{

	fflua_t fflua;
	try
	{
		fflua.setModFuncFlag(true);
		//! 注册C++ 对象到lua中
		fflua.reg(lua_reg);
		//设置全局开关
		if (FLAGS_direct)
		{
			 fflua.set_global_variable("gc_direct", true);
		}
		else
		{
			fflua.set_global_variable("gc_direct", false);
		}
		fflua.set_global_variable("gc_step", FLAGS_step);
		fflua.set_global_variable("gc_role", FLAGS_role);
		fflua.set_global_variable("gc_debug", FLAGS_debug);



		//! 载入lua文件
		fflua.add_package_path("./");
		if (!FLAGS_script.empty() && !FLAGS_sconfig.empty())
		{
			if (FLAGS_enc)
			{
				char* outbuf = nullptr;
				int len;
				int iRet;
				iRet = file_decrypt2buf(FLAGS_script.c_str(), &outbuf, &len);
				if (iRet > 0)
				{
					bool exist = false;
					LOG(INFO) << "encrypt" << endl;
					fflua.load_file(FLAGS_sconfig.c_str());
					luaL_loadbuffer(fflua.get_lua_state(), outbuf, len, "script") || lua_pcall(fflua.get_lua_state(), 0, 0, 0);
					//fflua.get_global_variable("init_task_to_run", exist);
					//if (true == exist)
					//{
					fflua.call<int>("init_task_to_run");
					//}
				}
			}
			else
			{
				bool exist = false;
				LOG(INFO) << "normal" << endl;
				fflua.load_file(FLAGS_sconfig.c_str());
				fflua.load_file(FLAGS_script.c_str());
				//fflua.get_global_variable("init_task_to_run", exist);
				//if (true == exist)
				//{
				fflua.call<int>("init_task_to_run");
				//}
			}
		}
	}
	catch (exception& e)
	{
		LOG(WARNING) << "script run exception:" << e.what() << endl;
		g_script_run--;
		return 0;

	}
	LOG(INFO) << "script run sucessful" << endl;
	g_script_run--;
	return 1;
}


static int cleanup_proc(bool normal)
{

	fflua_t fflua;
	try
	{
		if (!normal)
		{
			fflua.setModFuncFlag(true);
			//! 注册C++ 对象到lua中
			fflua.reg(lua_reg);
			//! 载入lua文件
			fflua.add_package_path("./");
			if (!FLAGS_script.empty() && !FLAGS_sconfig.empty())
			{
				if (FLAGS_enc)
				{
					char* outbuf = nullptr;
					int len;
					int iRet;
					iRet = file_decrypt2buf(FLAGS_script.c_str(), &outbuf, &len);
					if (iRet > 0)
					{
						bool exist = false;
						fflua.load_file(FLAGS_sconfig.c_str());
						luaL_loadbuffer(fflua.get_lua_state(), outbuf, len, "script") || lua_pcall(fflua.get_lua_state(), 0, 0, 0);
						//fflua.get_global_variable("fini_task_to_stop", exist);
						//if (true == exist)
						//{
						fflua.call<int>("fini_task_to_stop");
						//}
					}
				}
				else
				{
					bool exist = false;
					fflua.load_file(FLAGS_sconfig.c_str());
					fflua.load_file(FLAGS_script.c_str());
					//fflua.get_global_variable("fini_task_to_stop", exist);
					//if (true == exist)
					//{
					fflua.call<int>("fini_task_to_stop");
					//}
				}

			}
		}

		//
		if (g_srv.empty() || g_port.empty())
		{
			client_logout(FLAGS_srv_addr, FLAGS_srv_port, g_user, g_pwd, g_uuid, g_ver);
		}
		else
		{
			client_logout(g_srv, g_port, g_user, g_pwd, g_uuid, g_ver);
		}
	}
	catch (exception& e)
	{
		LOG(WARNING) << "cleanup run exception:" << e.what() << endl;
		return 0;
	}
	LOG(INFO) << "cleanup run sucessful" << endl;
	return 1;
}

static BOOL CALLBACK CosonleHandler(DWORD ev)
{
	BOOL bRet = TRUE;
	switch (ev)
	{
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
	{
		//if (g_hwt[1] != INVALID_HANDLE_VALUE && g_stop == 0)
		//{
		//	TerminateThread(g_hwt[1], 0);
		//}
		//if (g_hwt[0] != INVALID_HANDLE_VALUE)
		//{
		//	TerminateThread(g_hwt[0], 0);
		//}
		if (g_stop == 0)
		{
			if (g_script_run > 0)
			{
				//TerminateThread(g_hwt[0], 0);
				cleanup_proc(false);
			}
			else
			{
				cleanup_proc(true);
			}
			LOG(WARNING) << "process exit by event:" << ev << endl;
		}
		bRet = FALSE;

		break;
	}
	case CTRL_BREAK_EVENT:
	{

		if (g_first)
		{
			FLAGS_control = false;
			g_first = false;
			LOG(WARNING) << "process continue by event" << endl;
		}
		else
		{
			DWORD processId = GetCurrentProcessId();
			BOOL result = resume_thread(processId);
			if (g_suspend)
			{
				LOG(WARNING) << "process suspend by event" << endl;
			}
			else
			{
				LOG(WARNING) << "process resume by event" << endl;
			}
		}

		bRet = TRUE;
		break;
	}
	default:
		break;
	}
	return bRet;
}
static void SignalHandler(int signal)
{
	if (signal == SIGTERM && signal == SIGINT && signal == SIGBREAK)
	{
		LOG(WARNING) << "process exit by signal" << endl;
		ExitProcess(1);
	}

}


static bool validate_lic_file(const char* flag_name, const string &value)
{
	return true;
}



//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char* argv[])
{
	char *task = "none";
	int ret = 0;
	//DWORD wt_id;//pid

	//设置库的搜索路径
	//SetDllDirectory(L"lib");
	//控制台事件和信号注册
	SetConsoleCtrlHandler(CosonleHandler, TRUE);
	//signal(SIGTERM, SignalHandler);
	//signal(SIGINT, SignalHandler);
	//signal(SIGBREAK, SignalHandler);
	//日志和命令行参数初始化
	google::RegisterFlagValidator(&FLAGS_lic, &validate_lic_file);
	FLAGS_log_dir = ".";
	FLAGS_logtostderr = true;
	google::ParseCommandLineFlags(&argc, &argv, true);
	google::InitGoogleLogging(argv[0]);

	//execute generate machine code
	if (FLAGS_genmc)
	{
		task = "generate machine code";
		ret = genmc_proc();
		goto error_tag;
	}
#ifdef PRIVATE_VERSION
	//execute generate license file
	if (FLAGS_genlic)
	{
		task = "generate license";
		ret = genlic_proc();
		goto error_tag;

	}
	//execute generate encrypt file
	if (FLAGS_genenc)
	{
		task = "generate encrypt file";
		ret = genenc_proc();
		goto error_tag;

	}
	//execute generate encrypt file
	if (FLAGS_genmd5)
	{
		task = "generate md5 file";
		ret = genmd5_proc();
		goto error_tag;

	}
#endif
	//execute script
	if (FLAGS_run)
	{
		while (FLAGS_control)
		{
			Sleep(2000);
			cout << "please input ctrl + break to run script" << endl;
		}

		task = "run script";
		try
		{
			cout << "start" << endl;
			check_lic();
			check_user();
			g_script_run++;
			//g_hwt[0] = CreateThread(NULL, 0, run_proc, NULL, CREATE_SUSPENDED, &wt_id);//创建脚本工作线程
			//g_hwt[1] = CreateThread(NULL, 0, client_thr, (LPVOID)&g_client_info, 0, &wt_id);//创建网络工作线程
			//if (g_hwt[0] != INVALID_HANDLE_VALUE && g_hwt[1] != INVALID_HANDLE_VALUE)
			//{
			//	ret = 1;
			//}
			LPVOID lpParameter = 0;
			ret = run_proc(lpParameter);
			//ret = client_thr((LPVOID)&g_client_info);

		}
		catch (const char* msg)
		{
			LOG(WARNING) << "run script exception1:" << msg << endl;
			ret = 0;
		}
		catch (exception& e)
		{
			LOG(WARNING) << "run script exception2:" << e.what() << endl;
			ret = 0;
		}
		goto error_tag;
	}
	else
	{
		task = "stop script";
		try
		{
			ret = cleanup_proc(false);
		}
		catch (const char* msg)
		{
			LOG(WARNING) << "stop script exception1:" << msg << endl;
			ret = 0;
		}
		catch (exception& e)
		{
			LOG(WARNING) << "stop script exception2:" << e.what() << endl;
			ret = 0;
		}
		goto error_tag;
	}
	//error process
error_tag:
	if (ret == 0)
	{
		LOG(WARNING) << task << " failed" << endl;
	}
	else
	{
		LOG(INFO) << task << " sucessful" << endl;
	}
	//normal exit

	//if (g_hwt[0] != INVALID_HANDLE_VALUE && g_hwt[1] != INVALID_HANDLE_VALUE)
	//g_stop++;
	//if (g_script_run > 0 && g_hwt[0] != INVALID_HANDLE_VALUE)
	//{
	//	g_script_run--;
		//WaitForMultipleObjects(2, g_hwt, FALSE, INFINITE);
		//over script thread
		//TerminateThread(g_hwt[0], 0);
		//CloseHandle(g_hwt[0]);
		//g_hwt[0] = INVALID_HANDLE_VALUE;
		//over net thread
		//TerminateThread(g_hwt[1], 0);
		//CloseHandle(g_hwt[1]);
		//g_hwt[1] = INVALID_HANDLE_VALUE;
	//	cleanup_proc(false);
	//}
	//else
	//{
		cleanup_proc(true);
	//}
	LOG(INFO) << "process exit by normal" << endl;
	//wait if  config
	if (FLAGS_wait)
	{
		waitKey(0);
	}
	google::ShutdownGoogleLogging();
	google::ShutDownCommandLineFlags();
	return ret;

}

