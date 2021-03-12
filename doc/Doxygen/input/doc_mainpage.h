/**
 * \file 		doc_mainpage.h
 *
 * \brief 		ezDevSDK Doxygen 文档主页面
  *
 * \copyright	HangZhou Hikvision System Technology Co.,Ltd. All Right Reserved.
 *
 * \author		xurongjun
 *
 * \date		2018/07/27
 */
 
/*!\mainpage 萤石ezDevSDK使用说明 v1.0.0
 *
 * \section brief_sec  SDK概述
 *
 * 萤石ezDevSDK，也称设备SDK，目前支持Windows、Linux、Android、CentOS、Realtek-RTOS等多个平台。ezDevSDK主要由微内核和领域模块组成，精简灵活，用户可以根据自身业务的需求，
 * 利用已有的领域模块组合自己的SDK，也可以在已有的框架上，开发自己的领域模块，满足定制的特殊需求。
 * \image html ezDevSDK_Architecture.JPG SDK整体框架
 *
 * \subsection microkernel_subsec 微内核
 * 
 * 负责设备接入萤石云，业务消息通道，领域模块管理。微内核以轻量、易拓展为目标，只提供简单的基础服务，通过封装与平台的交互的消息收发机制，为上层和萤石云平台之间创建消息通道，具体的业务逻辑由上层程序实现，微内核不关心业务实现。 
 *
 * \subsection sdkboot_subsec SDKboot
 *
 * 对微内核的简单封装，提供接口对微内核进行简单管理，如初始化，启停、注销等。负责线程、时间、文件等和系统相关性较大操作，辅助微内核工作，是ezDevSDK不可缺少的一部分。
 *
 * \subsection domain_subsec 领域模块
 *
 * 具体业务的实现。ezDevSDK业务模块按领域的方式进行功能的划分，每个领域都有唯一的领域ID，如果需要开发新的领域应该和服务器事先约定，分配相应的领域ID。用户可以根据自己的需求，选择一个或多个领域模块组成SDK，目前已有的领域有：
 * \image html domain_Existed.JPG
 *
 * \subsection common_subsec 通用模块
 *
 * 特殊的领域模块，负责通用指令重定向，通常在开发领域模块的过程中会用到。有些指令会在多个领域中使用，被划分到通用领域模块。当发生指令交互时，微内核需要知道将这条指令交给哪个领域模块处理，这时就需要根据具体指令消息体中的Type字段来判断。Type字段由领域开发者和服务器事先约定，在向微内核注册领域模块的同时注册重定向信息，这样微内核才会将这条指令路由个给该领域模块处理。目前需要重定向的指令有：
 * \image html common_Command.JPG
 *
 * \section instructions_sec SDK使用说明
 *
 * 为了方便嵌入式开发者熟悉SDK，快速接入萤石云平台，ezDevSDK按功能进行划分，为不同需求的开发者以源码形式提供对应的Demo，开发者只要在此基础上，根据Demo源码注释的提示，在相应的地方填入自己的业务逻辑，就可以相应的功能。 
 * 
 * \subsection directory_subsec SDK目录
 *
 \verbatim
 |----inc
 |    |----base_typedef.h
 |    |----ezdev_sdk_kernel.h
 |    |----ezdev_sdk_kernel_error.h
 |    |----ezdev_sdk_kernel_struct.h
 |    |----ezDevSDK_boot.h
 |    |----ezDevSDK_Common_Module.h
 |    |----ezDevSDK_Common_Module_Def.h
 |
 |----lib
 |    |----libezDevSDK_boot.a
 |    |----libezDevSDK_Common_Module.a
 |    |----libmicrokernel.a
 |
 |----example
 |    |----fileDepens
 |    |    |----dev_info
 |    |
 |    |----hello_microkernel.c
 |    |----domain_abc.c
 |    |----thirdparty_domain_demo.c
 \endverbatim
 *
 * \subsection dir_detail_subsec 目录和文件说明
 *
 * - inc目录：ezDevSDK的头文件
 * - lib目录：ezDevSDK的库文件
 * - dev_info：设备信息文件，用于设备接入使用，一般采用json的格式存储在设备Flash中，字段的定义详见 \c ezdev_sdk_kernel_init()
 * - hello_microkernel.c：示例代码，演示设备上线。用于展示微内核的最基础功能和调用方式
 * - domain_abc.c：示例代码，演示领域模块的开发和注册，另外利用通用模块进行指令重定向的用法
 * - thirdparty_domain_demo.c：通过对已有的开放平台领域ISAPI功能的使用，演示了怎么使用已有的领域模块和怎么利用该模块透明通道功能进行第三方APP和设备之间进行数据传输
 */