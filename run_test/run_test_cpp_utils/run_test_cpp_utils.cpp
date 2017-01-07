#include "muggle/base/base.h"
#include "muggle/cpp_utils/cpp_utils.h"

int main(int argc, char *argv[])
{
	muggle::DeltaTimer dt;

	dt.Start();

	MSleep(5000);

	dt.End();

	double ms = dt.GetElapsedMilliseconds();
	MUGGLE_DEBUG_INFO("delta time: %f\n", ms);

	return 0;
}