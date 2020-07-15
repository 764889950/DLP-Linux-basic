1、编译条件是需要程序安装opencv-3.4.5及以上版本
#将文件下载到本地
$ git clone https://github.com/764889950/opencv-3.4.5.git

#根据下载的版本而定
$ cd opencv-3.4.5
#创建release文件夹
$ mkdir release
#进入release目录下
$ cd release
#cmake读入所有源文件之后，自动生成makefile
$ cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local ..
#编译
$ sudo make -j4
#安装
$ sudo make install
#更新动态链接库
$ sudo ldconfig
#查看linux下的opencv安装版本，若提示版本号，说明安装成功
pkg-config opencv --modversion

2、编译DLP-Linux-basic(server)
在DLP-Linux-basic目录下的Makefile文件所在目录执行make命令即可
3、编译DLP-Linux-basic/client(client)
在DLP-Linux-basic/client目录下的Makefile文件所在目录执行make命令即可
4、使用方式
先运行DLP-basic-server,再运行client目录下的control-client，control-client后接控制指令