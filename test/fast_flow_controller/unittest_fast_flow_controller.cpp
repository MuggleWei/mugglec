#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

class TestFastFlowControllerFixture : public ::testing::Test {
public:
	virtual void SetUp() override
	{
		memset(&flow_ctl, 0, sizeof(flow_ctl));
		ticks_freq = muggle_rdtsc_freq_calibrate();
	}

	virtual void TearDown() override
	{
		muggle_fast_flow_ctl_destroy(&flow_ctl);
	}

public:
	double ticks_freq;
	muggle_fast_flow_controller_t flow_ctl;
};

TEST_F(TestFastFlowControllerFixture, flow_ctl_check_and_update)
{
#if MUGGLE_SUPPORT_FAST_FLOW_CONTROLLER
	int64_t time_range_sec = 1;
	uint32_t n = 5;
	ASSERT_TRUE(muggle_fast_flow_ctl_init(&flow_ctl, time_range_sec, n, 60,
										  ticks_freq));

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_TRUE(muggle_fast_flow_ctl_check_and_update(&flow_ctl));
	}
	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_fast_flow_ctl_check_and_update(&flow_ctl));
	}

	muggle_msleep(time_range_sec * (int)(1.2 * 1000));

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_TRUE(muggle_fast_flow_ctl_check_and_update(&flow_ctl));
	}
	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_fast_flow_ctl_check_and_update(&flow_ctl));
	}

	muggle_msleep(600);

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_fast_flow_ctl_check_and_update(&flow_ctl));
	}

	muggle_msleep(600);
	ASSERT_TRUE(muggle_fast_flow_ctl_check_and_update(&flow_ctl));
#endif
}

TEST_F(TestFastFlowControllerFixture, flow_ctl_check_and_force_update)
{
#if MUGGLE_SUPPORT_FAST_FLOW_CONTROLLER
	int64_t time_range_sec = 1;
	uint32_t n = 5;
	ASSERT_TRUE(muggle_fast_flow_ctl_init(&flow_ctl, time_range_sec, n, 60,
										  ticks_freq));

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_TRUE(muggle_fast_flow_ctl_check_and_force_update(&flow_ctl));
	}
	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_fast_flow_ctl_check_and_force_update(&flow_ctl));
	}

	muggle_msleep(time_range_sec * (int)(1.2 * 1000));

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_TRUE(muggle_fast_flow_ctl_check_and_force_update(&flow_ctl));
	}
	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_fast_flow_ctl_check_and_force_update(&flow_ctl));
	}

	muggle_msleep(600);

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_fast_flow_ctl_check_and_force_update(&flow_ctl));
	}

	muggle_msleep(600);
	ASSERT_FALSE(muggle_fast_flow_ctl_check_and_force_update(&flow_ctl));
#endif
}
