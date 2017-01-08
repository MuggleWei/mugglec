#include "muggle/base/base.h"
#include "muggle/base_cpp/base_cpp.h"

int main(int argc, char *argv[])
{
	muggle::DeltaTime dt;

	dt.Start();

	MSleep(5000);

	dt.End();

	double ms = dt.GetElapsedMilliseconds();
	MUGGLE_DEBUG_INFO("delta time: %f\n", ms);

	return 0;
}