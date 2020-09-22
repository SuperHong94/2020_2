#include <stdio.h>
#include <iostream>
bool IsLittleEndian();

bool IsBigEndian();

int main()
{


	if (IsLittleEndian())
		std::cout << "�� ��ǻ�ʹ� ��Ʋ ����� ����Դϴ�.\n";
	else
		std::cout << "�� ��ǻ�ʹ� ��Ʋ ����� ����� �ƴմϴ�.\n";

	std::cout << "\n\n";
	if (IsBigEndian())
		std::cout << "�� ��ǻ�ʹ� �� ����� ����Դϴ�.\n";
	else
		std::cout << "�� ��ǻ�ʹ� �� ����� ����� �ƴմϴ�.\n";

}

bool IsLittleEndian()
{
	std::cout << "�� ��ǻ�Ͱ� ��Ʋ ����� ������� �˻��մϴ�.\n";

	unsigned short data = 0x1234;

	printf("�� ��ǻ�ʹ� 0x%x������(unsigned short)�� 0x%x%x �̷��� �����մϴ�.\n", data, ((char*)&data)[0], ((char*)&data)[1]);

	if (((char*)&data)[0] == 0x34)
		return true;
	else
		return false;
}

bool IsBigEndian()
{
	std::cout << "�� ��ǻ�Ͱ� �� ����� ������� �˻��մϴ�.\n";

	unsigned short data = 0x1234;

	printf("�� ��ǻ�ʹ� 0x%x������(unsigned short)�� 0x%x%x �̷��� �����մϴ�.\n", data, ((char*)&data)[0], ((char*)&data)[1]);

	if (((char*)&data)[0] == 0x12) {
		return true;
	}
	else
		return false;
}
