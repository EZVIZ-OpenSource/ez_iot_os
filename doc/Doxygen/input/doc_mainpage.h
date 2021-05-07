
 /*!\mainpage өʯezDevSDKʹ��˵�� v1.0.0
 *
 * \section brief_sec  SDK����
 *
 * өʯezDevSDK��Ҳ���豸SDK��Ŀǰ֧��Windows��Linux��Android��CentOS��Realtek-RTOS�ȶ��ƽ̨��ezDevSDK��Ҫ��΢�ں˺�����ģ����ɣ��������û����Ը�������ҵ�������
 * �������е�����ģ������Լ���SDK��Ҳ���������еĿ���ϣ������Լ�������ģ�飬���㶨�Ƶ���������
 * \image html ezDevSDK_Architecture.JPG SDK������
 *
 * \subsection microkernel_subsec ΢�ں�
 * 
 * �����豸����өʯ�ƣ�ҵ����Ϣͨ��������ģ�������΢�ں�������������չΪĿ�ֻ꣬�ṩ�򵥵Ļ�������ͨ����װ��ƽ̨�Ľ�������Ϣ�շ����ƣ�Ϊ�ϲ��өʯ��ƽ̨֮�䴴����Ϣͨ���������ҵ���߼����ϲ����ʵ�֣�΢�ں˲�����ҵ��ʵ�֡� 
 *
 * \subsection sdkboot_subsec SDKboot
 *
 * ��΢�ں˵ļ򵥷�װ���ṩ�ӿڶ�΢�ں˽��м򵥹��������ʼ������ͣ��ע���ȡ������̡߳�ʱ�䡢�ļ��Ⱥ�ϵͳ����Խϴ����������΢�ں˹�������ezDevSDK����ȱ�ٵ�һ���֡�
 *
 * \subsection domain_subsec ����ģ��
 *
 * ����ҵ���ʵ�֡�ezDevSDKҵ��ģ�鰴����ķ�ʽ���й��ܵĻ��֣�ÿ��������Ψһ������ID�������Ҫ�����µ�����Ӧ�úͷ���������Լ����������Ӧ������ID���û����Ը����Լ�������ѡ��һ����������ģ�����SDK��Ŀǰ���е������У�
 * \image html domain_Existed.JPG
 *
 * \subsection common_subsec ͨ��ģ��
 *
 * ���������ģ�飬����ͨ��ָ���ض���ͨ���ڿ�������ģ��Ĺ����л��õ�����Щָ����ڶ��������ʹ�ã������ֵ�ͨ������ģ�顣������ָ���ʱ��΢�ں���Ҫ֪��������ָ����ĸ�����ģ�鴦������ʱ����Ҫ���ݾ���ָ����Ϣ���е�Type�ֶ����жϡ�Type�ֶ������򿪷��ߺͷ���������Լ��������΢�ں�ע������ģ���ͬʱע���ض�����Ϣ������΢�ں˲ŻὫ����ָ��·�ɸ���������ģ�鴦����Ŀǰ��Ҫ�ض����ָ���У�
 * \image html common_Command.JPG
 *
 * \section instructions_sec SDKʹ��˵��
 *
 * Ϊ�˷���Ƕ��ʽ��������ϤSDK�����ٽ���өʯ��ƽ̨��ezDevSDK�����ܽ��л��֣�Ϊ��ͬ����Ŀ�������Դ����ʽ�ṩ��Ӧ��Demo��������ֻҪ�ڴ˻����ϣ�����DemoԴ��ע�͵���ʾ������Ӧ�ĵط������Լ���ҵ���߼����Ϳ�����Ӧ�Ĺ��ܡ� 
 * 
 * \subsection directory_subsec SDKĿ¼
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
 * \subsection dir_detail_subsec Ŀ¼���ļ�˵��
 *
 * - incĿ¼��ezDevSDK��ͷ�ļ�
 * - libĿ¼��ezDevSDK�Ŀ��ļ�
 * - dev_info���豸��Ϣ�ļ��������豸����ʹ�ã�һ�����json�ĸ�ʽ�洢���豸Flash�У��ֶεĶ������ \c ezdev_sdk_kernel_init()
 * - hello_microkernel.c��ʾ�����룬��ʾ�豸���ߡ�����չʾ΢�ں˵���������ܺ͵��÷�ʽ
 * - domain_abc.c��ʾ�����룬��ʾ����ģ��Ŀ�����ע�ᣬ��������ͨ��ģ�����ָ���ض�����÷�
 * - thirdparty_domain_demo.c��ͨ�������еĿ���ƽ̨����ISAPI���ܵ�ʹ�ã���ʾ����ôʹ�����е�����ģ�����ô���ø�ģ��͸��ͨ�����ܽ��е�����APP���豸֮��������ݴ���
 */