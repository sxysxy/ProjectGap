// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� RGSSRUNTIMEPLUGIN_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// RGSSRUNTIMEPLUGIN_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef RGSSRUNTIMEPLUGIN_EXPORTS
#define RGSSRUNTIMEPLUGIN_API __declspec(dllexport)
#else
#define RGSSRUNTIMEPLUGIN_API __declspec(dllimport)
#endif

#include <Common\include\PluginData.h>

extern PluginData gPluginData;
