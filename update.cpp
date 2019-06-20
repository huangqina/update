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

//ssh-keygen -f "/root/.ssh/known_hosts" -R 192.168.3.131

string pass_zip ="123456";
string loc_name ="yc-pc"; //the user on LB
string loc_ip ="192.168.3.131"; //the ip of LB
char pass_pc[]="123456\n"; //the password of LB
char pass_ai[]="123456\n"; //the password of server
char pass_disk[]="123123\n"; //the password of encrypted disk on server
char pass_loc_disk[]="123456\n"; //the password of encrypted disk on USB disk

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
// int daemon_init()
// {
// 	pid_t pid;
// 	struct sigaction sact;
// 	int i,maxfd;
// 	struct rlimit rlp;

// 	if((pid=fork())<0)
// 		return -1;
// 	else if (pid!=0)
// 		exit(0);


// 	if(setsid()<0)
// 	return -1 ;

// 	sact.sa_handler=SIG_IGN;
// 	sigemptyset(&sact.sa_mask);
// 	sact.sa_flags=0;
// 	sigaction(SIGHUP,&sact,NULL);


// 	if((pid=fork())<0)
// 		return -1;
// 	else if (pid!=0)
// 		exit(0);


// 	maxfd=sysconf(_SC_OPEN_MAX);

// 	/*
// 		rlp.rlim_cur = rlp.rlim_max = RLIM_INFINITY;
// 		if (getrlimit(RLIMIT_NOFILE, &rlp))
// 	return 0;
// 		maxfd=rlp.rlim_cur;
// 		/*maxfd=getdtablesize();*/
		


// 	for(i=0;i<maxfd;i++)
// 	close(i);

// 	open("/dev/null",O_RDWR);
// 	dup(0);
// 	dup(1);
// 	dup(2);

// 	return 0;

// }

vector<string> readfile(char* filename) //read config file 
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
    return file; 
}  



int rm(vector<string> filename) //remove the file after copy
{
	printf("-----------------------start rm----------------------------\n");
	string name;
	name += "su ";
	name += loc_name;
	name += " -c 'cd ~/;sudo -S rm -f ";
	for (int i = 0; i<filename.size();i++){
	
        name+=filename[i];
        name+=" ";
	}
	name +="filename test_db.py test_wd.py test_ai.py pic.png\}'";
	extern int exp_timeout;
	exp_timeout = 9999;
    Tcl_Interp *tcl;
	tcl = Tcl_CreateInterp();
	if (Expect_Init(tcl) != TCL_OK)
	{
		puts("failure");
		return 1;
	}
	int fd = exp_spawnl("bash","bash","-c",name.c_str(),(char *)0);
    if(fd <0)
    {
        cout<<"Fail to scp"<<endl;
    }
    else
    {
        int loop =1;
        int result;
        while(loop)
        {
            result = exp_expectl(fd,exp_glob,"*password: ",1,
                    exp_exact, "Permission denied, please try again.", 2,
                    exp_regexp, "(The authenticity of host)(.)*(Are you sure you want to continue connecting (yes/no)?)", 3,exp_glob,"*sudo*", 4,exp_glob,"*Password*", 5,exp_end);
            //char pas[]="123456\n";
            switch(result)
            {
                case 1:
                    write(fd,pass_pc,sizeof(pass_pc)-1);
                    break;
                case 2:
                    cout <<"wrong password"<<endl;
                    loop=0;
                    break;
                case 3:
                    cout<<"connect security"<<endl;
                    write(fd,"yes\n",4);
                    break;
				case 4:
                    write(fd,pass_pc,sizeof(pass_pc)-1);
                    //write(fd,"yes\n",4);
                    break;
				case 5:
                    write(fd,pass_pc,sizeof(pass_pc)-1);
                    //write(fd,"yes\n",4);
                    break;
                case EXP_EOF:
                    cout <<"EOF\n";
                    loop=0;
                    break;
                case EXP_TIMEOUT:
                    cout<<"Time out\n";
                    loop=0;
                    break;
                default:
                    cout<<"logged in "<<result<<endl;
                    loop=0;
                    break;
            }
        }
    } 
	Tcl_DeleteInterp(tcl);
}
int copy(vector<string> filename) //copy the file to ~/
{
	printf("-----------------------start copy----------------------------\n");
	string name;
	name += "su ";
	name += loc_name;
	name += " -c 'sudo -S cp -rf /mnt/";
	for (int i = 0; i<filename.size();i++){
	
        name+=filename[i];
        name+=" ";
	}
	name +="~/;sudo  -S chmod 777 ~/";
	//name +="filename test_db.py test_wd.py ~/;cd ~/;sudo chmod 777 ";
	for (int i = 0; i<filename.size();i++){
        name+=filename[i];
        name+=" ";
	}
	name+="'";
	//name +="filename test_db.py test_wd.py";
	extern int exp_timeout;
	exp_timeout = 9999;
    Tcl_Interp *tcl;
	tcl = Tcl_CreateInterp();
	if (Expect_Init(tcl) != TCL_OK)
	{
		puts("failure");
		return 1;
	}
	int fd = exp_spawnl("bash","bash","-c",name.c_str(),(char *)0);
    if(fd <0)
    {
        cout<<"Fail to scp"<<endl;
    }
    else
    {
        int loop =1;
        int result;
        while(loop)
        {
            result = exp_expectl(fd,exp_glob,"*password: ",1,
                    exp_exact, "Permission denied, please try again.", 2,
                    exp_regexp, "(The authenticity of host)(.)*(Are you sure you want to continue connecting (yes/no)?)", 3,exp_glob,"*sudo*", 4,exp_glob,"*Password*", 5,
                    exp_end);
            //char pas[]="123456\n";
            switch(result)
            {
                case 1:
                    write(fd,pass_pc,sizeof(pass_pc)-1);
                    break;
                case 2:
                    cout <<"wrong password"<<endl;
                    loop=0;
                    break;
                case 3:
                    cout<<"connect security"<<endl;
                    write(fd,"yes\n",4);
                    break;
				case 4:
                    write(fd,pass_pc,sizeof(pass_pc)-1);
                    //write(fd,"yes\n",4);
                    break;
				case 5:
                    write(fd,pass_pc,sizeof(pass_pc)-1);
                   // write(fd,"yes\n",4);
                    break;
                case EXP_EOF:
                    cout <<"EOF\n";
                    loop=0;
                    break;
                case EXP_TIMEOUT:
                    cout<<"Time out\n";
                    loop=0;
                    break;
                default:
                    cout<<"logged in "<<result<<endl;
                    loop=0;
                    break;
            }
        }
    } 
	Tcl_DeleteInterp(tcl);
}
int ssh_restart() 
{
	printf("-----------------------restart ssh----------------------------\n");
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
	string code = "";
	code += "sudo ssh-keygen -f '/root/.ssh/known_hosts'  -R ";
	code += loc_ip;
	fd = exp_spawnl("bash","bash","-c",code.c_str(), (char *)0);
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
		result = exp_expectl(fd, exp_glob, "*password*", 1, exp_exact, "Permission denied, please try again.", 2, exp_regexp, "(The authenticity of host)(.)*(Are you sure you want to continue connecting (yes/no)?)", 3, exp_glob, "*sudo*", 4,exp_glob, "*assphrase*", 5,exp_glob, "*elect*: ", 6, exp_glob,"*Password*", 7,exp_end);
		//char pas[] = "123456\n";
		switch(result)
		{
			case 1:
				write(fd, pass_pc, sizeof(pass_pc) - 1);
				break;
			case 2:
				cout <<"wrong password"<<endl;
				break;
			case 3:
				cout<<"connect security"<<endl;
				write(fd, "yes\n", 4);
				break;
			case 4:
				write(fd, pass_pc, sizeof(pass_pc) - 1);
				break;
			case 5:
				write(fd, pass_loc_disk, sizeof(pass_loc_disk) - 1);
				break;
			case 6:
				write(fd, "\r\n", 2);
				break;
			case 7:
				write(fd, pass_pc, sizeof(pass_pc) - 1);
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

int mount_copy(string ip, vector<string> filename) //mount the encrypted disk on server and copy file to it
{
	printf("-----------------------start mount_copy----------------------------\n");
	string  ip_name;
    ip_name += "yc-pc@";
	ip_name += ip;
	string name = "";
	string mv = "";
	for (int i = 0; i<filename.size();i++){
		mv += "mv -f /mnt/";
		mv +=filename[i];
		mv +=" /mnt/";
		mv +=filename[i];
		mv +="_copy";
		mv +=";";
	}
	
	name+=" sudo rsync --progress ";
	name+=loc_name;
	name+= "@";
	name+=loc_ip;
	name+=":/mnt/";
	name+="\{";
	for (int i = 0; i<filename.size();i++){
	
        name+=filename[i];
        name+=",";
	}
	//name +="test_db.py,test_wd.py\} ";
	name +="\} ";
	name +="/mnt;";
	mv+=name;
	// mv+=" rsync --progress yc-pc@";
	// mv+=loc_ip;
	// mv+=":~/update/\{test,filename,test_db.py,test_wd.py,test_ai.py,pic.png\} ~/";
	// cout<<mv;
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
	fd = exp_spawnl("ssh", "ssh", "-t", ip_name.c_str(), mv.c_str(), (char *)0);
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
		result = exp_expectl(fd, exp_glob, "*assword: ", 1, exp_exact, "Permission denied, please try again.", 2, exp_regexp, "(The authenticity of host)(.)*(Are you sure you want to continue connecting (yes/no)?)", 3, exp_glob, "*sudo*", 4,exp_glob, "*assphrase*", 5,exp_glob, "*elect*: ", 6,exp_glob,"WARNING",7, exp_end);
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
			case 7:
				ssh_restart();
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
int mount_loc(string disk) //mount the USB disk on LB
{
	printf("-----------------------start mount_loc----------------------------\n");
	string  loc="";
	loc += "su ";
	loc += loc_name;
	loc += " -c 'sudo -S cryptsetup   open ";
	loc += disk;
	loc += " p;sudo mount /dev/mapper/p /mnt'";
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
	fd = exp_spawnl("bash","bash","-c",loc.c_str(), (char *)0);
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
		result = exp_expectl(fd, exp_glob, "*password*", 1, exp_exact, "Permission denied, please try again.", 2, exp_regexp, "(The authenticity of host)(.)*(Are you sure you want to continue connecting (yes/no)?)", 3, exp_glob, "*sudo*", 4,exp_glob, "*assphrase*", 5,exp_glob, "*elect*: ", 6, exp_glob,"*Password*", 7,exp_end);
		//char pas[] = "123456\n";
		switch(result)
		{
			case 1:
				write(fd, pass_pc, sizeof(pass_pc) - 1);
				break;
			case 2:
				cout <<"wrong password"<<endl;
				break;
			case 3:
				cout<<"connect security"<<endl;
				write(fd, "yes\n", 4);
				break;
			case 4:
				write(fd, pass_pc, sizeof(pass_pc) - 1);
				break;
			case 5:
				write(fd, pass_loc_disk, sizeof(pass_loc_disk) - 1);
				break;
			case 6:
				write(fd, "\r\n", 2);
				break;
			case 7:
				write(fd, pass_pc, sizeof(pass_pc) - 1);
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


int umount_loc() //umount the USB disk on LB
{
	printf("-----------------------start umount_loc----------------------------\n");
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
	int loop = 1;
	int result;
	string  loc="";
	loc += "su ";
	loc += loc_name;
	loc +=  " -c 'sudo -S umount  /mnt;sudo cryptsetup  close   p'";
	fd = exp_spawnl("bash","bash","-c",loc.c_str(), (char *)0);
	if(fd < 0)
	{
		cout<<"Fail to sh"<<endl;
		return -1;
	}

	while(loop)
	{
		//predefine some expected responses
		result = exp_expectl(fd, exp_glob, "*password*", 1, exp_exact, "Permission denied, please try again.", 2, exp_regexp, "(The authenticity of host)(.)*(Are you sure you want to continue connecting (yes/no)?)", 3, exp_glob, "*sudo*", 4,exp_glob,"*busy*", 5, exp_glob,"*Password*", 6,exp_end);
		//char pas[] = "123456\n";
		switch(result)
		{
			case 1:
				write(fd, pass_pc, sizeof(pass_pc) - 1);
				break;
			case 2:
				cout <<"wrong password"<<endl;
				break;
			case 3:
				cout<<"connect security"<<endl;
				write(fd, "yes\n", 4);
				break;
			case 4:
				write(fd, pass_pc, sizeof(pass_pc) - 1);
				break;
			case 5:
				umount_loc();
				break;
			case 6:
				write(fd, pass_pc, sizeof(pass_pc) - 1);
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
int run(string ip,string cmd) //run 
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
		result = exp_expectl(fd, exp_glob, "*assword*", 1, exp_exact, "Permission denied, please try again.", 2, exp_regexp, "(The authenticity of host)(.)*(Are you sure you want to continue connecting (yes/no)?)", 3, exp_glob, "*sudo*", 4,exp_glob, "*assphrase: ", 5,exp_glob, "*Running*", 6,exp_glob,"*elect*: ", 7, exp_end);
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
{	//daemon_init(); //to run the code on the background
	//close(0);
	//close(1);
    //close(2);
	vector<string> filename = readfile("./filename");
	vector<string> ip = readfile("./ip");
	vector<string> disk = readfile("./disk");
	if(filename[0] == "test"){
		        printf("---------------------------------------");
				string cmd = "";
				char s[100];
				int port = 5002;
				sprintf(s, "sudo -S lsof -i:%d | awk 'NR == 1 {next}{print $2}'|xargs kill -9;", port);
				cmd += s;
				cmd += "export PATH='~/anaconda3/bin:$PATH';source activate pos;python ~/qwe.py >/dev/null 2>&1 &";
				run(ip[0],cmd);
				return 0;
			}
	umount_loc();
	mount_loc(disk[0]);
	//copy(filename);
	try{
		for (int i = 0; i<ip.size();i++){
   		mount("/dev/sda4",ip[i]);
		mount_copy(ip[i],filename);
		umount(ip[i]);
		for (int i = 0; i<filename.size();i++){
			if(filename[i] == "db.zip"){
				mount("/dev/sda4",ip[i]);
				string cmd = "";
				// cmd += "mv -f /mnt/main.py /mnt/main.py;";
				cmd += "unzip -oP ";
				cmd += pass_zip;
				cmd += " /mnt/db.zip -d ~/;cd ~/db;./start";
				// cmd += "cp -f /mnt/main.py ~/DB";
				run(ip[i],cmd);
				umount(ip[i]);	
				}
			else if(filename[i] == "wd.zip"){
				mount("/dev/sda4",ip[i]);
				string cmd = "";
				char s[100];
				int port = 8091;
				sprintf(s, "sudo -S lsof -i:%d | awk 'NR == 1 {next}{print $2}'|xargs kill -9;", port);
				// cmd += "mv -f /mnt/wd /mnt/wd_copy;";
				cmd += "unzip -oP ";
				cmd += pass_zip;
				cmd += " /mnt/wd.zip -d ~/;cd ~/wd;";
				// cmd += "cp -rf /mnt/wd ~/;";
				cmd += s;
				cmd +=  "export PATH='~/anaconda3/bin:$PATH';source activate pos; python ~/wd/main.py";
				run(ip[i],cmd);
				cmd = "sudo -S rm -rf ~/wd";
				run(ip[i],cmd);
				umount(ip[i]);	
			}
			else if(filename[i] == "ai.zip"){

				mount("/dev/sda4",ip[i]);
				string cmd = "";
				char s[100];
				int port = 5003;
				sprintf(s, "sudo -S lsof -i:%d | awk 'NR == 1 {next}{print $2}'|xargs kill -9;", port);
				//cmd += "mv -f /mnt/ai /mnt/ai_copy;";
				cmd += "unzip -oP ";
				cmd += pass_zip;
				cmd += " /mnt/ai.zip -d ~/;cd ~/ai;";
				//cmd += "cp -f /mnt/ai ~/;";
				cmd += s;
				cmd += "export PATH='~/anaconda3/bin:$PATH';source activate pos; python ~/ai/combined_ai.py";
				run(ip[i],cmd);
				cmd = "sudo -S rm -rf ~/ai";
				run(ip[i],cmd);
				umount(ip[i]);	
			}
	 	}
		rm(filename);
	}
	}
	catch(...){
		rm(filename);
		for (int i = 0; i<ip.size();i++){
			umount(ip[i]);	
			}
		umount_loc();
	}
	umount_loc();
}
