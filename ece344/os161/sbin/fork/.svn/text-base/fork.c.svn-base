#include <unistd.h>

int
main()
{
	fork();
	printf("1");
	fork();
	printf("2");
	fork();
	printf("3");
	fork();
	printf("4");
	printf("\nthe pid from get_pid syscall is %d", getpid());

	return 0;
}
