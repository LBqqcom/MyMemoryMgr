
#include"CMyMemoryMgr.h"

class A
{
public:
	A(int _a)
	{
		a = _a;
	}
private:
	int a;
};
int main()
{
	int *pint = new int(3);
	A *pa = new A(3);
	delete pint;
	delete pa;

	return 0;
}