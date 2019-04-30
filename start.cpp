#include <tcl.h>
#include <expect.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <expect_tcl.h>
#include <fstream>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector> 
#include <string> 

using namespace std;


char pass_pc[]="123456\n";
char pass_ai[]="123456\n"; //the password of server
char pass_disk[]="123123\n";


int daemon_init() 
{
	pid_t pid;
	if((pid = fork()) < 0)
		return -1;
	else if(pid != 0)
		exit(0);
	setsid();
	umask(0);
	return 0;
} 

vector<string> readfile(char* filename)
{ 
    char  name[100];
    sprintf(name, "%s",filename);
    ifstream myfile(name); 
    vector<string> file; 
    string temp; 
    if (!myfile.is_open()) 
    { 
        cout << "未成功打开文件" << endl; 
    } 
    while(getline(myfile,temp)){
        file.push_back(temp);
    };
    myfile.close(); 
    cout<<temp;
    return file; 
}  

void readTxt(string file)
{
    ifstream infile; 
    infile.open(file.data());   //将文件流对象与文件连接起来 
    assert(infile.is_open());   //若失败,则输出错误消息,并终止程序运行 
    string s;
    while(getline(infile,s))
    {
        cout<<s<<endl;
    }
    infile.close();             //关闭文件输入流 
}

int mount(string disk,string filename) //mount the encrypted disk on server
{
	printf("-----------------------start mount----------------------------\n");
    string  name="";
    name += "yc-pc@";
	name += filename;
	string  loc="";
	loc += "sudo cryptsetup   open  ";
	loc += disk;
	loc += " pri;sudo mount /dev/mapper/pri /mnt";
	extern int exp_timeout;
	exp_timeout = 9999;
    Tcl_Interp *tcl;
	tcl = Tcl_CreateInterp();
	if (Expect_Init(tcl) != TCL_OK)
	{
		puts("failure");
		return 1;
	}
	int fd;
	fd = exp_spawnl("ssh", "ssh", "-t", name.c_str(), loc.c_str(), (char *)0);
	if(fd < 0)
	{
		cout<<"Fail to ssh"<<endl;
		return -1;
	}
	int loop = 1;
	int result;
	while(loop)
	{
		//predefine some expected responses
		result = exp_expectl(fd, exp_glob, "*assword: ", 1, exp_exact, "Permission denied, please try again.", 2, exp_regexp, "(The authenticity of host)(.)*(Are you sure you want to continue connecting (yes/no)?)", 3, exp_glob, "*sudo*", 4,exp_glob, "*assphrase*", 5,exp_glob, "*elect*: ", 6, exp_end);
		//char pas[] = "123456\n";
		switch(result)
		{
			case 1:
				write(fd, pass_ai, sizeof(pass_ai) - 1);
				break;
			case 2:
				cout <<"wrong password"<<endl;
				break;
			case 3:
				cout<<"connect security"<<endl;
				write(fd, "yes\n", 4);
				break;
			case 4:
				write(fd, pass_ai, sizeof(pass_ai) - 1);
				break;
			case 5:
				write(fd, pass_disk, sizeof(pass_disk) - 1);
				break;
			case 6:
				write(fd, "\r\n", 2);
				break;
			case EXP_EOF:
				cout << "EOF\n";
				loop = 0;
				break;
			case EXP_TIMEOUT:
				cout<<"Time out\n";
				loop = 0;
				break;
			default:
				cout<<"logged in "<<result<<endl;
				loop = 0;
				break;
		}
	}
    Tcl_DeleteInterp(tcl);
}
int umount(string filename) //umount the encrypted disk on server
{
	printf("-----------------------start umount----------------------------\n");
    string  name;
    name += "yc-pc@";
	name += filename;	
	extern int exp_timeout;
	exp_timeout = 9999;
    Tcl_Interp *tcl;
	tcl = Tcl_CreateInterp();
	if (Expect_Init(tcl) != TCL_OK)
	{
		puts("failure");
		return 1;
	}
	int fd;
	int loop = 1;
	int result;
	fd = exp_spawnl("ssh", "ssh", "-t", name.c_str(), "sudo umount  /mnt;sudo cryptsetup  close   pri", (char *)0);
	if(fd < 0)
	{
		cout<<"Fail to ssh"<<endl;
		return -1;
	}

	while(loop)
	{
		//predefine some expected responses
		result = exp_expectl(fd, exp_glob, "*assword: ", 1, exp_exact, "Permission denied, please try again.", 2, exp_regexp, "(The authenticity of host)(.)*(Are you sure you want to continue connecting (yes/no)?)", 3, exp_glob, "*sudo*", 4,exp_glob,"*busy*", 5, exp_end);
		//char pas[] = "123456\n";
		switch(result)
		{
			case 1:
				write(fd, pass_ai, sizeof(pass_ai) - 1);
				break;
			case 2:
				cout <<"wrong password"<<endl;
				break;
			case 3:
				cout<<"connect security"<<endl;
				write(fd, "yes\n", 4);
				break;
			case 4:
				write(fd, pass_ai, sizeof(pass_ai) - 1);
				break;
			case 5:
				umount(filename);
				break;
			case 6:
				write(fd, "\r\n", 2);
				break;
			case EXP_EOF:
				cout << "EOF\n";
				loop = 0;
				break;
			case EXP_TIMEOUT:
				cout<<"Time out\n";
				loop = 0;
				break;
			default:
				cout<<"logged in "<<result<<endl;
				loop = 0;
				break;
		}
	}
    Tcl_DeleteInterp(tcl);
}

int run(string ip,string cmd) //run test
{
	printf("-----------------------start run----------------------------\n");
	string  ip_name;
    ip_name += "yc-pc@";
	ip_name += ip;
	extern int exp_timeout;
	exp_timeout = 100;
    Tcl_Interp *tcl;
	tcl = Tcl_CreateInterp();
	if (Expect_Init(tcl) != TCL_OK)
	{
		puts("failure");
		return 1;
	}
	int fd;
	fd = exp_spawnl("ssh", "ssh", ip_name.c_str(), cmd.c_str(), (char *)0);
	if(fd < 0)
	{
		cout<<"Fail to ssh"<<endl;
		return -1;
	}
	int loop = 1;
	int result;
	while(loop)
	{
		//predefine some expected responses
		result = exp_expectl(fd, exp_glob, "*assword*", 1, exp_exact, "Permission denied, please try again.", 2, exp_regexp, "(The authenticity of host)(.)*(Are you sure you want to continue connecting (yes/no)?)", 3, exp_glob, "*sudo*", 4,exp_glob, "*assphrase: ", 5,exp_glob, "*Debugger is active*", 6,exp_glob,"*elect*: ", 7, exp_end);
		//char pas[] = "123456\n";Running on
		switch(result)
		{
			case 1:
				write(fd, pass_ai, sizeof(pass_ai) - 1);
				break;
			case 2:
				cout <<"wrong password"<<endl;
				break;
			case 3:
				cout<<"connect security"<<endl;
				write(fd, "yes\n", 4);
				break;
			case 4:
				write(fd, pass_ai, sizeof(pass_ai) - 1);
				break;
			case 5:
				write(fd, pass_disk, sizeof(pass_disk) - 1);
				break;
            case 6:
				cout<<"start successful\n";
                return 1;
				break;
			case 7:
				write(fd, "\r\n", 2);
				break;
			case EXP_EOF:
				cout << "EOF\n";
				loop = 0;
				break;
			case EXP_TIMEOUT:
				cout<<"Time out\n";
				loop = 0;
				break;
			default:
				cout<<"logged in "<<result<<endl;
				loop = 0;
				break;
		}
	}
    Tcl_DeleteInterp(tcl);
}

int main()
{   
	vector<string> filename = readfile("./filename");
    vector<string> ip = readfile("./ip");
    if(filename[0] == "test"){
            printf("---------------------------------------");
            string cmd = "";
            cmd += "sudo -S docker-compose -f ~/DB/docker-compose.yaml up";
            run(ip[0],cmd);
            return 0;
        }
    try{
		for (int i = 0; i<sizeof(ip);i++){
            for (int i = 0; i<filename.size();i++){
                if(filename[i] == "DB.zip"){
                    string cmd = "";
                    mount("/dev/sda4",ip[i]);
                    cmd += "sudo -S docker-compose -f ~/DB/docker-compose.yaml up";
                    run(ip[i],cmd);
                    umount(ip[i]);
                }
                else if(filename[i] == "wd.zip"){
                    mount("/dev/sda4",ip[i]);
                    string cmd = "";
                    char s[100];
                    int port = 8091;
                    sprintf(s, "sudo -S lsof -i:%d | awk 'NR == 1 {next}{print $2}'|xargs kill -9;", port);
                    cmd += "sudo -S cp -f /mnt/wd ~/wd;";
                    cmd += s;
                    cmd += "export PATH='~/anaconda3/bin:$PATH';source activate pos;python ~/wd/main.py>/dev/null 2>&1 &;";
                    cmd += "rm -rf ~/wd";
                    run(ip[i],cmd);
                    umount(ip[i]);	
                }
                else if(filename[i] == "ai.zip"){
                    mount("/dev/sda4",ip[i]);
                    string cmd = "";
                    char s[100];
                    int port = 5000;
                    sprintf(s, "sudo -S lsof -i:%d | awk 'NR == 1 {next}{print $2}'|xargs kill -9;", port);
                    cmd += "sudo -S cp -f /mnt/ai ~/ai;";
                    cmd += s;
                    cmd += "export PATH='~/anaconda3/bin:$PATH';source activate pos;python ~/ai/combined_ai.py>/dev/null 2>&1 &;";
                    cmd += "rm -rf ~/ai";
                    run(ip[i],cmd);
                    umount(ip[i]);
                }
            }
        }
    }
    catch(...){
		for (int i = 0; i<sizeof(ip);i++){
			umount(ip[i]);	
		}
	}
}