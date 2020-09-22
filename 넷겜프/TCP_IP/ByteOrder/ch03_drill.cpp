#include <stdio.h>
#include <iostream>
bool IsLittleEndian();

bool IsBigEndian();

int main()
{


	if (IsLittleEndian())
		std::cout << "이 컴퓨터는 리틀 엔디안 방식입니다.\n";
	else
		std::cout << "이 컴퓨터는 리틀 엔디안 방식이 아닙니다.\n";

	std::cout << "\n\n";
	if (IsBigEndian())
		std::cout << "이 컴퓨터는 빅 엔디안 방식입니다.\n";
	else
		std::cout << "이 컴퓨터는 빅 엔디안 방식이 아닙니다.\n";

}

bool IsLittleEndian()
{
	std::cout << "이 컴퓨터가 리틀 엔디안 방식인지 검사합니다.\n";

	unsigned short data = 0x1234;

	printf("이 컴퓨터는 0x%x데이터(unsigned short)를 0x%x%x 이렇게 저장합니다.\n", data, ((char*)&data)[0], ((char*)&data)[1]);

	if (((char*)&data)[0] == 0x34)
		return true;
	else
		return false;
}

bool IsBigEndian()
{
	std::cout << "이 컴퓨터가 빅 엔디안 방식인지 검사합니다.\n";

	unsigned short data = 0x1234;

	printf("이 컴퓨터는 0x%x데이터(unsigned short)를 0x%x%x 이렇게 저장합니다.\n", data, ((char*)&data)[0], ((char*)&data)[1]);

	if (((char*)&data)[0] == 0x12) {
		return true;
	}
	else
		return false;
}
