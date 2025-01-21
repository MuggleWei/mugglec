#include "gtest/gtest.h"
#include "muggle/c/muggle_c.h"

class TestFlowControllerFixture : public ::testing::Test {
public:
	virtual void SetUp() override
	{
		memset(&flow_ctl, 0, sizeof(flow_ctl));
	}

	virtual void TearDown() override
	{
		muggle_flow_ctl_destroy(&flow_ctl);
	}

public:
	muggle_flow_controller_t flow_ctl;
};

TEST_F(TestFlowControllerFixture, flow_ctl_check_and_update)
{
	int64_t time_range_sec = 1;
	uint32_t n = 5;
	ASSERT_TRUE(muggle_flow_ctl_init(&flow_ctl, time_range_sec, n, 60));

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_TRUE(muggle_flow_ctl_check_and_update(&flow_ctl));
	}
	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_flow_ctl_check_and_update(&flow_ctl));
	}

	muggle_msleep(time_range_sec * (int)(1.2 * 1000));

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_TRUE(muggle_flow_ctl_check_and_update(&flow_ctl));
	}
	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_flow_ctl_check_and_update(&flow_ctl));
	}

	muggle_msleep(600);

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_flow_ctl_check_and_update(&flow_ctl));
	}

	muggle_msleep(600);
	ASSERT_TRUE(muggle_flow_ctl_check_and_update(&flow_ctl));
}

TEST_F(TestFlowControllerFixture, flow_ctl_check_and_force_update)
{
	int64_t time_range_sec = 1;
	uint32_t n = 5;
	ASSERT_TRUE(muggle_flow_ctl_init(&flow_ctl, time_range_sec, n, 60));

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_TRUE(muggle_flow_ctl_check_and_force_update(&flow_ctl));
	}
	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_flow_ctl_check_and_force_update(&flow_ctl));
	}

	muggle_msleep(time_range_sec * (int)(1.2 * 1000));

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_TRUE(muggle_flow_ctl_check_and_force_update(&flow_ctl));
	}
	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_flow_ctl_check_and_force_update(&flow_ctl));
	}

	muggle_msleep(600);

	for (uint32_t i = 0; i < n; ++i) {
		ASSERT_FALSE(muggle_flow_ctl_check_and_force_update(&flow_ctl));
	}

	muggle_msleep(600);
	ASSERT_FALSE(muggle_flow_ctl_check_and_force_update(&flow_ctl));
}
