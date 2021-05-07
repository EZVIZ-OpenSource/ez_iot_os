#!/bin/bash
#by shenhongyin

PRJ_PWD=$(dirname "$PWD")
#BUILD_PWD=$(dirname "$PWD")
LOC_PWD=${PRJ_PWD}
SUBDIRS='eziot/core/link'
VERSION='V1.0.0'
TOOL_STRIP=""
DYNAMIC_SWITCH=int0
EXPORT_PRE="ez_iot_os-"

function usage()
{
	echo -e "***********************************************************"
	echo -e "USAGE:" 
	echo -e "	./build.sh \${Tool_Chain}"
	echo -e ""
	echo -e "Tool_Chain:" 
	echo -e "            gcc"
	echo -e "            mipsel-linux-492"
	echo -e "            arm-hisiv500-linux"
	echo -e "SYSTEM:"
	echo -e "            Linux rt-thread"
	echo -e "EGG:"
	echo -e "***********************************************************"
}

function ln_ToolChain_all(){
	filelist=${SUBDIRS}
	for file in $filelist
	do
		echo "---ln ${LOC_PWD}/${file} now---"
        cd ${PRJ_PWD}/${file}/
        rm ToolChain.cmake
        cp ${PRJ_PWD}/build/ToolChain.cmake ToolChain.cmake
		echo "---ln ${LOC_PWD}/${file} end---"
	done
}

function cleanall() {
	filelist=${SUBDIRS}
	for file in $filelist
	do
		echo "---clean ${file} now---"
        cd ${PRJ_PWD}/${file}/build
        make clean
        cd ..
        rm -rf build
		echo "---clean ${file} end---"
	done
}

function buildone() {
	echo "build ${LOC_Name} now"
    if [ -e ${LOC_PWD}/${LOC_Name}/build ];then
        echo "---------"
    else
		echo "show  ${LOC_PWD}/${LOC_Name}"  
        mkdir ${LOC_PWD}/${LOC_Name}/build
        cd ${LOC_PWD}/${LOC_Name}/build
        cmake ..
    fi

    cd ${LOC_PWD}/${LOC_Name}/build
        make
        
    echo "---------"
    echo "build ${LOC_Name} end"
	
	# 更新头文件和库文件
	cp ${LOC_PWD}/${LOC_Name}/../inc/*.h ${LOC_PWD}/inc
	cp ${LOC_PWD}/${LOC_Name}/../lib/linux/*.a ${LOC_PWD}/lib/$ToolChain
	if [ $DYNAMIC_SWITCH -eq 1 ]; then
		cp ${LOC_PWD}/${LOC_Name}/../lib/linux/*.so ${LOC_PWD}/lib/$ToolChain/dynamic
		$TOOL_STRIP ${LOC_PWD}/lib/$ToolChain/dynamic/*.so
	fi

	if [ 0 != "$?" ];then
		exit 1
	fi
}

function buildall() {
	rm -rf ${LOC_PWD}/inc
	rm -rf ${LOC_PWD}/lib/$ToolChain
	mkdir -p ${LOC_PWD}/inc
	mkdir -p ${LOC_PWD}/lib/$ToolChain
	if [ $DYNAMIC_SWITCH -eq 1 ]; then
		mkdir -p ${LOC_PWD}/lib/$ToolChain/dynamic
	fi
	
	filelist=${SUBDIRS}
	echo $filelist
	for file in $filelist
	do
		LOC_Name=$file
		buildone
	done
	
	# 导出SDK的所有文件
	export_ezDevSDK
}

function export_ezDevSDK()
{
	# 删除旧目录，创建新目录
	rm -rf ${LOC_PWD}/export/$EXPORT_PRE$ToolChain
	mkdir -p ${LOC_PWD}/export/$EXPORT_PRE$ToolChain
	cp -rf ${LOC_PWD}/inc/. ${LOC_PWD}/export/$EXPORT_PRE$ToolChain/inc
	cp -rf ${LOC_PWD}/lib/$ToolChain/. ${LOC_PWD}/export/$EXPORT_PRE$ToolChain/lib
}

# 生成ToolChain.cmake文件
function generate_config()
{
	echo -e "\033[32m in func generate_config \033[0m"
	echo "input param 1:$1"
	echo "input param 2:$2"
	echo "input param 3:$3"
	echo "input param 4:$4"
	echo "input param 5:$5"
	echo "input param 6:$6"

	if [ -f $1 ]; then
		if [ "ANDROID" == $4 ]; then
			echo "SET(CMAKE_SYSTEM_NAME Linux)" >> $1
			echo "SET(ANDROID Y)" >> $1
		else
			echo "SET(CMAKE_SYSTEM_NAME Linux)" >> $1
		fi

		if [ -n "$7" ]; then
			echo "SET(PRECOMPILE_MACRO \"$7\")" >> $1
		fi
		
		# 根据ToolChain名指定对应的编译工具，有符号链接和全路径链接的方式
		if [ "gcc" == "$3" ]; then
				echo "SET(CMAKE_C_COMPILER \"gcc\")" >> $1
				echo "SET(CMAKE_CXX_COMPILER \"g++\")" >> $1	
		else
				echo "SET(CMAKE_C_COMPILER \"$3-gcc\")" >> $1
				echo "SET(CMAKE_CXX_COMPILER \"$3-g++\")" >> $1
		fi

		if [ -n "$5" ]; then
			echo "SET(PLATFORM_C_FLAGS \"$5\")" >> $1
		fi
		
		if [ -n "$6" ]; then
			echo "SET(PLATFORM_CXX_FLAGS \"$6\")" >> $1
		fi
		
		echo "SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)" >> $1
		echo "SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)" >> $1
		echo "SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)" >> $1
		
		# 由外部开关"DYNAMIC"判断是否编译动态库
		for i in $* ; do  
			if [ "DYNAMIC" == "$i" ]; then
				echo "SET(DYNAMIC \"ON\")" >> $1
				DYNAMIC_SWITCH=1
			fi
		done
		
		# 如果编译动态库需要制定strip工具
		if [ $DYNAMIC_SWITCH -eq 1 ]; then
			if [ "gcc" == "$3" ]; then
				TOOL_STRIP="strip"
			else
				TOOL_STRIP="$3-strip"
			fi
			
			echo "strip tool:$TOOL_STRIP"
		fi
		
		echo "Generate Toolchain File Done!"
	else
		echo "The Tool Chain file is not exist!"
		exit 1
	fi
}

function modify_cmake_file()
{
	if [ "" != "$1" ]; then
		echo "The input tool chain is $1 ."
	else
		echo "The input tool chain is empty!"
		exit 1
	fi
    
    rm ToolChain.cmake
    touch ToolChain.cmake

	#不需要编译动态库第二参数传入""
	generate_config './ToolChain.cmake' 'DYNAMIC' "$@"
}

function build_sdk()
{
	echo "Start building ..."
	rm CMakeCache.txt cmake_install.cmake Makefile CMakeFiles -rf
	cmake $1
	if [ -e 'Makefile' ]; then
		make clean
		make
	else
		echo "The makefile build script wasn't been generated successfully!"
		exit 1
	fi
}

if [ $# == 0 ]; then
	echo "Please input the correct parameters!"
	usage
	exit 1
else
	echo "This is release mode!"
	echo "input param 1:$1"
	echo "input param 2:$2"
	echo "input param 3:$3"
	echo "input param 4:$4"
	
	svn_version=`svn info | cat -n | awk '{if($1==6)print $3}'`
	echo $svn_version

	lib_version=`svn info | cat -n | awk '{if($1==3)print $3}' |  awk -F'/' '{print $10}'`
	echo $lib_version
	
	# 编译动态库版默认开启，以后可以根据'DYNAMIC'控制关闭编译动态库
	ToolChain=$1
	modify_cmake_file "$@"
    ln_ToolChain_all
    cleanall
    buildall
fi
