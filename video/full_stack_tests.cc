/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include <stdio.h>

#include "rtc_base/experiments/alr_experiment.h"
#include "rtc_base/flags.h"
#include "test/field_trial.h"
#include "test/gtest.h"
#include "video/video_quality_test.h"

namespace webrtc {
namespace flags {

DEFINE_bool(logs, false, "print logs to stderr");

DEFINE_string(rtc_event_log_name,
              "",
              "Filename for rtc event log. Two files "
              "with \"_send\" and \"_recv\" suffixes will be created.");
std::string RtcEventLogName() {
  return static_cast<std::string>(FLAG_rtc_event_log_name);
}
DEFINE_string(rtp_dump_name, "", "Filename for dumped received RTP stream.");
std::string RtpDumpName() {
  return static_cast<std::string>(FLAG_rtp_dump_name);
}
DEFINE_string(encoded_frame_path,
              "",
              "The base path for encoded frame logs. Created files will have "
              "the form <encoded_frame_path>.<n>.(recv|send.<m>).ivf");
std::string EncodedFramePath() {
  return static_cast<std::string>(FLAG_encoded_frame_path);
}
}  // namespace flags
}  // namespace webrtc

namespace webrtc {

namespace {
static const int kFullStackTestDurationSecs = 45;
}  // namespace

class FullStackTest : public VideoQualityTest {
 public:
  void RunTest(VideoQualityTest::Params params) {
    params.logging = {flags::FLAG_logs, flags::RtcEventLogName(),
                      flags::RtpDumpName(), flags::EncodedFramePath()};
    RunWithAnalyzer(params);
  }

 protected:
  const std::string kScreenshareSimulcastExperiment =
      "WebRTC-SimulcastScreenshare/Enabled/";
  const std::string kAlrProbingExperiment =
      std::string(AlrExperimentSettings::kScreenshareProbingBweExperimentName) +
      "/1.1,2875,85,20,-20,0/";
  const std::string kRoundRobinPacingQueueExperiment =
      "WebRTC-RoundRobinPacing/Enabled/";
  const std::string kPacerPushBackExperiment =
      "WebRTC-PacerPushbackExperiment/Enabled/";
};

// VideoQualityTest::Params params = {
//   { ... },      // Common.
//   { ... },      // Video-specific settings.
//   { ... },      // Screenshare-specific settings.
//   { ... },      // Analyzer settings.
//   pipe,         // FakeNetworkPipe::Config
//   { ... },      // Spatial scalability.
//   logs          // bool
// };

#if !defined(RTC_DISABLE_VP9)
TEST_F(FullStackTest, ForemanCifWithoutPacketLossVp9) {
  // TODO(pbos): Decide on psnr/ssim thresholds for foreman_cif.
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,    288,   30,    700000,
                          700000, 700000, false, "VP9", 1,
                          0,      0,      false, false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_net_delay_0_0_plr_0_VP9", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCifPlr5Vp9) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,    30000,
                          500000, 2000000, false, "VP9", 1,
                          0,      0,       false, false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_delay_50_0_plr_5_VP9", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.loss_percent = 5;
  foreman_cif.pipe.queue_delay_ms = 50;
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCifWithoutPacketLossMultiplexI420Frame) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,    288,   30,          700000,
                          700000, 700000, false, "multiplex", 1,
                          0,      0,      false, false,       "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_net_delay_0_0_plr_0_Multiplex", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, GeneratorWithoutPacketLossMultiplexI420AFrame) {
  VideoQualityTest::Params generator;
  generator.call.send_side_bwe = true;
  generator.video[0] = {true,   352,    288,   30,          700000,
                        700000, 700000, false, "multiplex", 1,
                        0,      0,      false, false,       "GeneratorI420A"};
  generator.analyzer = {"generator_net_delay_0_0_plr_0_Multiplex", 0.0, 0.0,
                        kFullStackTestDurationSecs};
  RunTest(generator);
}

#endif  // !defined(RTC_DISABLE_VP9)

#if defined(WEBRTC_LINUX)
// Crashes on the linux trusty perf bot: bugs.webrtc.org/9129.
#define MAYBE_ParisQcifWithoutPacketLoss DISABLED_ParisQcifWithoutPacketLoss
#else
#define MAYBE_ParisQcifWithoutPacketLoss ParisQcifWithoutPacketLoss
#endif
TEST_F(FullStackTest, MAYBE_ParisQcifWithoutPacketLoss) {
  VideoQualityTest::Params paris_qcif;
  paris_qcif.call.send_side_bwe = true;
  paris_qcif.video[0] = {true,   176,    144,   30,    300000,
                         300000, 300000, false, "VP8", 1,
                         0,      0,      false, false, "paris_qcif"};
  paris_qcif.analyzer = {"net_delay_0_0_plr_0", 36.0, 0.96,
                         kFullStackTestDurationSecs};
  RunTest(paris_qcif);
}

TEST_F(FullStackTest, ForemanCifWithoutPacketLoss) {
  // TODO(pbos): Decide on psnr/ssim thresholds for foreman_cif.
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,    288,   30,    700000,
                          700000, 700000, false, "VP8", 1,
                          0,      0,      false, false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_net_delay_0_0_plr_0", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCif30kbpsWithoutPacketLoss) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,  352,   288,   10,    30000,
                          30000, 30000, false, "VP8", 1,
                          0,     0,     false, false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_30kbps_net_delay_0_0_plr_0", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCifPlr5) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,    30000,
                          500000, 2000000, false, "VP8", 1,
                          0,      0,       false, false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_delay_50_0_plr_5", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.loss_percent = 5;
  foreman_cif.pipe.queue_delay_ms = 50;
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCifPlr5Ulpfec) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,    30000,
                          500000, 2000000, false, "VP8", 1,
                          0,      0,       true,  false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_delay_50_0_plr_5_ulpfec", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.loss_percent = 5;
  foreman_cif.pipe.queue_delay_ms = 50;
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCifPlr5Flexfec) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,    30000,
                          500000, 2000000, false, "VP8", 1,
                          0,      0,       false, true,  "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_delay_50_0_plr_5_flexfec", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.loss_percent = 5;
  foreman_cif.pipe.queue_delay_ms = 50;
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCif500kbpsPlr3Flexfec) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,    30000,
                          500000, 2000000, false, "VP8", 1,
                          0,      0,       false, true,  "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_500kbps_delay_50_0_plr_3_flexfec", 0.0,
                          0.0, kFullStackTestDurationSecs};
  foreman_cif.pipe.loss_percent = 3;
  foreman_cif.pipe.link_capacity_kbps = 500;
  foreman_cif.pipe.queue_delay_ms = 50;
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCif500kbpsPlr3Ulpfec) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,    30000,
                          500000, 2000000, false, "VP8", 1,
                          0,      0,       true,  false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_500kbps_delay_50_0_plr_3_ulpfec", 0.0,
                          0.0, kFullStackTestDurationSecs};
  foreman_cif.pipe.loss_percent = 3;
  foreman_cif.pipe.link_capacity_kbps = 500;
  foreman_cif.pipe.queue_delay_ms = 50;
  RunTest(foreman_cif);
}

#if defined(WEBRTC_USE_H264)
TEST_F(FullStackTest, ForemanCifWithoutPacketlossH264) {
  // TODO(pbos): Decide on psnr/ssim thresholds for foreman_cif.
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,    288,   30,     700000,
                          700000, 700000, false, "H264", 1,
                          0,      0,      false, false,  "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_net_delay_0_0_plr_0_H264", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCif30kbpsWithoutPacketlossH264) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,  352,   288,   10,     30000,
                          30000, 30000, false, "H264", 1,
                          0,     0,     false, false,  "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_30kbps_net_delay_0_0_plr_0_H264", 0.0,
                          0.0, kFullStackTestDurationSecs};
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCifPlr5H264) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,     30000,
                          500000, 2000000, false, "H264", 1,
                          0,      0,       false, false,  "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_delay_50_0_plr_5_H264", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.loss_percent = 5;
  foreman_cif.pipe.queue_delay_ms = 50;
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCifPlr5H264SpsPpsIdrIsKeyframe) {
  test::ScopedFieldTrials override_field_trials(
      "WebRTC-SpsPpsIdrIsH264Keyframe/Enabled/");

  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,     30000,
                          500000, 2000000, false, "H264", 1,
                          0,      0,       false, false,  "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_delay_50_0_plr_5_H264_sps_pps_idr", 0.0,
                          0.0, kFullStackTestDurationSecs};
  foreman_cif.pipe.loss_percent = 5;
  foreman_cif.pipe.queue_delay_ms = 50;
  RunTest(foreman_cif);
}

// Verify that this is worth the bot time, before enabling.
TEST_F(FullStackTest, ForemanCifPlr5H264Flexfec) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,     30000,
                          500000, 2000000, false, "H264", 1,
                          0,      0,       false, true,   "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_delay_50_0_plr_5_H264_flexfec", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.loss_percent = 5;
  foreman_cif.pipe.queue_delay_ms = 50;
  RunTest(foreman_cif);
}

// Ulpfec with H264 is an unsupported combination, so this test is only useful
// for debugging. It is therefore disabled by default.
TEST_F(FullStackTest, DISABLED_ForemanCifPlr5H264Ulpfec) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,     30000,
                          500000, 2000000, false, "H264", 1,
                          0,      0,       true,  false,  "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_delay_50_0_plr_5_H264_ulpfec", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.loss_percent = 5;
  foreman_cif.pipe.queue_delay_ms = 50;
  RunTest(foreman_cif);
}
#endif  // defined(WEBRTC_USE_H264)

TEST_F(FullStackTest, ForemanCif500kbps) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,    30000,
                          500000, 2000000, false, "VP8", 1,
                          0,      0,       false, false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_500kbps", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.queue_length_packets = 0;
  foreman_cif.pipe.queue_delay_ms = 0;
  foreman_cif.pipe.link_capacity_kbps = 500;
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCif500kbpsLimitedQueue) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,    30000,
                          500000, 2000000, false, "VP8", 1,
                          0,      0,       false, false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_500kbps_32pkts_queue", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.queue_length_packets = 32;
  foreman_cif.pipe.queue_delay_ms = 0;
  foreman_cif.pipe.link_capacity_kbps = 500;
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCif500kbps100ms) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,    30000,
                          500000, 2000000, false, "VP8", 1,
                          0,      0,       false, false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_500kbps_100ms", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.queue_length_packets = 0;
  foreman_cif.pipe.queue_delay_ms = 100;
  foreman_cif.pipe.link_capacity_kbps = 500;
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCif500kbps100msLimitedQueue) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,   352,     288,   30,    30000,
                          500000, 2000000, false, "VP8", 1,
                          0,      0,       false, false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_500kbps_100ms_32pkts_queue", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.queue_length_packets = 32;
  foreman_cif.pipe.queue_delay_ms = 100;
  foreman_cif.pipe.link_capacity_kbps = 500;
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCif500kbps100msLimitedQueueRecvBwe) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = false;
  foreman_cif.video[0] = {true,   352,     288,   30,    30000,
                          500000, 2000000, false, "VP8", 1,
                          0,      0,       false, false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_500kbps_100ms_32pkts_queue_recv_bwe",
                          0.0, 0.0, kFullStackTestDurationSecs};
  foreman_cif.pipe.queue_length_packets = 32;
  foreman_cif.pipe.queue_delay_ms = 100;
  foreman_cif.pipe.link_capacity_kbps = 500;
  RunTest(foreman_cif);
}

TEST_F(FullStackTest, ForemanCif1000kbps100msLimitedQueue) {
  VideoQualityTest::Params foreman_cif;
  foreman_cif.call.send_side_bwe = true;
  foreman_cif.video[0] = {true,    352,     288,   30,    30000,
                          2000000, 2000000, false, "VP8", 1,
                          0,       0,       false, false, "foreman_cif"};
  foreman_cif.analyzer = {"foreman_cif_1000kbps_100ms_32pkts_queue", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  foreman_cif.pipe.queue_length_packets = 32;
  foreman_cif.pipe.queue_delay_ms = 100;
  foreman_cif.pipe.link_capacity_kbps = 1000;
  RunTest(foreman_cif);
}

// TODO(sprang): Remove this if we have the similar ModerateLimits below?
TEST_F(FullStackTest, ConferenceMotionHd2000kbps100msLimitedQueue) {
  VideoQualityTest::Params conf_motion_hd;
  conf_motion_hd.call.send_side_bwe = true;
  conf_motion_hd.video[0] = {
      true,    1280,    720,   50,    30000,
      3000000, 3000000, false, "VP8", 1,
      0,       0,       false, false, "ConferenceMotion_1280_720_50"};
  conf_motion_hd.analyzer = {"conference_motion_hd_2000kbps_100ms_32pkts_queue",
                             0.0, 0.0, kFullStackTestDurationSecs};
  conf_motion_hd.pipe.queue_length_packets = 32;
  conf_motion_hd.pipe.queue_delay_ms = 100;
  conf_motion_hd.pipe.link_capacity_kbps = 2000;
  RunTest(conf_motion_hd);
}

TEST_F(FullStackTest, ConferenceMotionHd1TLModerateLimits) {
  VideoQualityTest::Params conf_motion_hd;
  conf_motion_hd.call.send_side_bwe = true;
  conf_motion_hd.video[0] = {
      true,    1280,    720,   50,    30000,
      3000000, 3000000, false, "VP8", 1,
      -1,      0,       false, false, "ConferenceMotion_1280_720_50"};
  conf_motion_hd.analyzer = {"conference_motion_hd_1tl_moderate_limits", 0.0,
                             0.0, kFullStackTestDurationSecs};
  conf_motion_hd.pipe.queue_length_packets = 50;
  conf_motion_hd.pipe.loss_percent = 3;
  conf_motion_hd.pipe.queue_delay_ms = 100;
  conf_motion_hd.pipe.link_capacity_kbps = 2000;
  RunTest(conf_motion_hd);
}

TEST_F(FullStackTest, ConferenceMotionHd2TLModerateLimits) {
  VideoQualityTest::Params conf_motion_hd;
  conf_motion_hd.call.send_side_bwe = true;
  conf_motion_hd.video[0] = {
      true,    1280,    720,   50,    30000,
      3000000, 3000000, false, "VP8", 2,
      -1,      0,       false, false, "ConferenceMotion_1280_720_50"};
  conf_motion_hd.analyzer = {"conference_motion_hd_2tl_moderate_limits", 0.0,
                             0.0, kFullStackTestDurationSecs};
  conf_motion_hd.pipe.queue_length_packets = 50;
  conf_motion_hd.pipe.loss_percent = 3;
  conf_motion_hd.pipe.queue_delay_ms = 100;
  conf_motion_hd.pipe.link_capacity_kbps = 2000;
  RunTest(conf_motion_hd);
}

TEST_F(FullStackTest, ConferenceMotionHd3TLModerateLimits) {
  VideoQualityTest::Params conf_motion_hd;
  conf_motion_hd.call.send_side_bwe = true;
  conf_motion_hd.video[0] = {
      true,    1280,    720,   50,    30000,
      3000000, 3000000, false, "VP8", 3,
      -1,      0,       false, false, "ConferenceMotion_1280_720_50"};
  conf_motion_hd.analyzer = {"conference_motion_hd_3tl_moderate_limits", 0.0,
                             0.0, kFullStackTestDurationSecs};
  conf_motion_hd.pipe.queue_length_packets = 50;
  conf_motion_hd.pipe.loss_percent = 3;
  conf_motion_hd.pipe.queue_delay_ms = 100;
  conf_motion_hd.pipe.link_capacity_kbps = 2000;
  RunTest(conf_motion_hd);
}

TEST_F(FullStackTest, ConferenceMotionHd4TLModerateLimits) {
  VideoQualityTest::Params conf_motion_hd;
  conf_motion_hd.call.send_side_bwe = true;
  conf_motion_hd.video[0] = {
      true,    1280,    720,   50,    30000,
      3000000, 3000000, false, "VP8", 4,
      -1,      0,       false, false, "ConferenceMotion_1280_720_50"};
  conf_motion_hd.analyzer = {"conference_motion_hd_4tl_moderate_limits", 0.0,
                             0.0, kFullStackTestDurationSecs};
  conf_motion_hd.pipe.queue_length_packets = 50;
  conf_motion_hd.pipe.loss_percent = 3;
  conf_motion_hd.pipe.queue_delay_ms = 100;
  conf_motion_hd.pipe.link_capacity_kbps = 2000;
  RunTest(conf_motion_hd);
}

TEST_F(FullStackTest, ConferenceMotionHd3TLModerateLimitsAltTLPattern) {
  test::ScopedFieldTrials field_trial("WebRTC-UseShortVP8TL3Pattern/Enabled/");
  VideoQualityTest::Params conf_motion_hd;
  conf_motion_hd.call.send_side_bwe = true;
  conf_motion_hd.video[0] = {
      true,    1280,    720,   50,    30000,
      3000000, 3000000, false, "VP8", 3,
      -1,      0,       false, false, "ConferenceMotion_1280_720_50"};
  conf_motion_hd.analyzer = {"conference_motion_hd_3tl_alt_moderate_limits",
                             0.0, 0.0, kFullStackTestDurationSecs};
  conf_motion_hd.pipe.queue_length_packets = 50;
  conf_motion_hd.pipe.loss_percent = 3;
  conf_motion_hd.pipe.queue_delay_ms = 100;
  conf_motion_hd.pipe.link_capacity_kbps = 2000;
  RunTest(conf_motion_hd);
}

#if !defined(RTC_DISABLE_VP9)
TEST_F(FullStackTest, ConferenceMotionHd2000kbps100msLimitedQueueVP9) {
  VideoQualityTest::Params conf_motion_hd;
  conf_motion_hd.call.send_side_bwe = true;
  conf_motion_hd.video[0] = {
      true,    1280,    720,   50,    30000,
      3000000, 3000000, false, "VP9", 1,
      0,       0,       false, false, "ConferenceMotion_1280_720_50"};
  conf_motion_hd.analyzer = {
      "conference_motion_hd_2000kbps_100ms_32pkts_queue_vp9", 0.0, 0.0,
      kFullStackTestDurationSecs};
  conf_motion_hd.pipe.queue_length_packets = 32;
  conf_motion_hd.pipe.queue_delay_ms = 100;
  conf_motion_hd.pipe.link_capacity_kbps = 2000;
  RunTest(conf_motion_hd);
}
#endif

TEST_F(FullStackTest, ScreenshareSlidesVP8_2TL) {
  VideoQualityTest::Params screenshare;
  screenshare.call.send_side_bwe = true;
  screenshare.video[0] = {true,   1850,    1110,  5,     50000,
                          200000, 2000000, false, "VP8", 2,
                          1,      400000,  false, false, ""};
  screenshare.screenshare[0] = {true, false, 10};
  screenshare.analyzer = {"screenshare_slides", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  RunTest(screenshare);
}

TEST_F(FullStackTest, ScreenshareSlidesVP8_3TL_Simulcast) {
  test::ScopedFieldTrials field_trial(kScreenshareSimulcastExperiment);
  VideoQualityTest::Params screenshare;
  screenshare.call.send_side_bwe = true;
  screenshare.screenshare[0] = {true, false, 10};
  screenshare.video[0] = {true,    1850,    1110,  5,     800000,
                          2500000, 2500000, false, "VP8", 3,
                          2,       400000,  false, false, ""};
  screenshare.analyzer = {"screenshare_slides_simulcast", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  VideoQualityTest::Params screenshare_params_high;
  screenshare_params_high.video[0] = {true,    1850,    1110,  5,     800000,
                                      2500000, 2500000, false, "VP8", 3,
                                      0,       400000,  false, false, ""};
  VideoQualityTest::Params screenshare_params_low;
  screenshare_params_low.video[0] = {true,   1850,    1110,  5,     50000,
                                     200000, 2000000, false, "VP8", 2,
                                     0,      400000,  false, false, ""};

  std::vector<VideoStream> streams = {
      DefaultVideoStream(screenshare_params_low, 0),
      DefaultVideoStream(screenshare_params_high, 0)};
  screenshare.ss[0] = {
      streams, 1, 1, 0, InterLayerPredMode::kOn, std::vector<SpatialLayer>(),
      false};
  RunTest(screenshare);
}

TEST_F(FullStackTest, ScreenshareSlidesVP8_2TL_Scroll) {
  VideoQualityTest::Params config;
  config.call.send_side_bwe = true;
  config.video[0] = {true,   1850,    1110 / 2, 5,     50000,
                     200000, 2000000, false,    "VP8", 2,
                     1,      400000,  false,    false, ""};
  config.screenshare[0] = {true, false, 10, 2};
  config.analyzer = {"screenshare_slides_scrolling", 0.0, 0.0,
                     kFullStackTestDurationSecs};
  RunTest(config);
}

TEST_F(FullStackTest, ScreenshareSlidesVP8_2TL_LossyNet) {
  VideoQualityTest::Params screenshare;
  screenshare.call.send_side_bwe = true;
  screenshare.video[0] = {true,   1850,    1110,  5,     50000,
                          200000, 2000000, false, "VP8", 2,
                          1,      400000,  false, false, ""};
  screenshare.screenshare[0] = {true, false, 10};
  screenshare.analyzer = {"screenshare_slides_lossy_net", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  screenshare.pipe.loss_percent = 5;
  screenshare.pipe.queue_delay_ms = 200;
  screenshare.pipe.link_capacity_kbps = 500;
  RunTest(screenshare);
}

TEST_F(FullStackTest, ScreenshareSlidesVP8_2TL_VeryLossyNet) {
  VideoQualityTest::Params screenshare;
  screenshare.call.send_side_bwe = true;
  screenshare.video[0] = {true,   1850,    1110,  5,     50000,
                          200000, 2000000, false, "VP8", 2,
                          1,      400000,  false, false, ""};
  screenshare.screenshare[0] = {true, false, 10};
  screenshare.analyzer = {"screenshare_slides_very_lossy", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  screenshare.pipe.loss_percent = 10;
  screenshare.pipe.queue_delay_ms = 200;
  screenshare.pipe.link_capacity_kbps = 500;
  RunTest(screenshare);
}

TEST_F(FullStackTest, ScreenshareSlidesVP8_2TL_LossyNetRestrictedQueue) {
  VideoQualityTest::Params screenshare;
  screenshare.call.send_side_bwe = true;
  screenshare.video[0] = {true,   1850,    1110,  5,     50000,
                          200000, 2000000, false, "VP8", 2,
                          1,      400000,  false, false, ""};
  screenshare.screenshare[0] = {true, false, 10};
  screenshare.analyzer = {"screenshare_slides_lossy_limited", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  screenshare.pipe.loss_percent = 5;
  screenshare.pipe.link_capacity_kbps = 200;
  screenshare.pipe.queue_length_packets = 30;

  RunTest(screenshare);
}

TEST_F(FullStackTest, ScreenshareSlidesVP8_2TL_ModeratelyRestricted) {
  VideoQualityTest::Params screenshare;
  screenshare.call.send_side_bwe = true;
  screenshare.video[0] = {true,   1850,    1110,  5,     50000,
                          200000, 2000000, false, "VP8", 2,
                          1,      400000,  false, false, ""};
  screenshare.screenshare[0] = {true, false, 10};
  screenshare.analyzer = {"screenshare_slides_moderately_restricted", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  screenshare.pipe.loss_percent = 1;
  screenshare.pipe.link_capacity_kbps = 1200;
  screenshare.pipe.queue_length_packets = 30;

  RunTest(screenshare);
}

// TODO(sprang): Retire these tests once experiment is removed.
TEST_F(FullStackTest, ScreenshareSlidesVP8_2TL_LossyNetRestrictedQueue_ALR) {
  test::ScopedFieldTrials field_trial(kAlrProbingExperiment);
  VideoQualityTest::Params screenshare;
  screenshare.call.send_side_bwe = true;
  screenshare.video[0] = {true,   1850,    1110,  5,     50000,
                          200000, 2000000, false, "VP8", 2,
                          1,      400000,  false, false, ""};
  screenshare.screenshare[0] = {true, false, 10};
  screenshare.analyzer = {"screenshare_slides_lossy_limited_ALR", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  screenshare.pipe.loss_percent = 5;
  screenshare.pipe.link_capacity_kbps = 200;
  screenshare.pipe.queue_length_packets = 30;

  RunTest(screenshare);
}

TEST_F(FullStackTest, ScreenshareSlidesVP8_2TL_ALR) {
  test::ScopedFieldTrials field_trial(kAlrProbingExperiment);
  VideoQualityTest::Params screenshare;
  screenshare.call.send_side_bwe = true;
  screenshare.video[0] = {true,   1850,    1110,  5,     50000,
                          200000, 2000000, false, "VP8", 2,
                          1,      400000,  false, false, ""};
  screenshare.screenshare[0] = {true, false, 10};
  screenshare.analyzer = {"screenshare_slides_ALR", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  RunTest(screenshare);
}

TEST_F(FullStackTest, ScreenshareSlidesVP8_2TL_ModeratelyRestricted_ALR) {
  test::ScopedFieldTrials field_trial(kAlrProbingExperiment);
  VideoQualityTest::Params screenshare;
  screenshare.call.send_side_bwe = true;
  screenshare.video[0] = {true,   1850,    1110,  5,     50000,
                          200000, 2000000, false, "VP8", 2,
                          1,      400000,  false, false, ""};
  screenshare.screenshare[0] = {true, false, 10};
  screenshare.analyzer = {"screenshare_slides_moderately_restricted_ALR", 0.0,
                          0.0, kFullStackTestDurationSecs};
  screenshare.pipe.loss_percent = 1;
  screenshare.pipe.link_capacity_kbps = 1200;
  screenshare.pipe.queue_length_packets = 30;

  RunTest(screenshare);
}

TEST_F(FullStackTest, ScreenshareSlidesVP8_3TL_Simulcast_ALR) {
  test::ScopedFieldTrials field_trial(kScreenshareSimulcastExperiment +
                                      kAlrProbingExperiment);
  VideoQualityTest::Params screenshare;
  screenshare.call.send_side_bwe = true;
  screenshare.screenshare[0] = {true, false, 10};
  screenshare.video[0] = {true,    1850,    1110,  5,     800000,
                          2500000, 2500000, false, "VP8", 3,
                          2,       400000,  false, false, ""};
  screenshare.analyzer = {"screenshare_slides_simulcast_alr", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  VideoQualityTest::Params screenshare_params_high;
  screenshare_params_high.video[0] = {true,    1850,    1110,  5,     800000,
                                      2500000, 2500000, false, "VP8", 3,
                                      0,       400000,  false, false, ""};
  VideoQualityTest::Params screenshare_params_low;
  screenshare_params_low.video[0] = {true,   1850,    1110,  5,     50000,
                                     200000, 2000000, false, "VP8", 2,
                                     0,      400000,  false, false, ""};

  std::vector<VideoStream> streams = {
      DefaultVideoStream(screenshare_params_low, 0),
      DefaultVideoStream(screenshare_params_high, 0)};
  screenshare.ss[0] = {
      streams, 1, 1, 0, InterLayerPredMode::kOn, std::vector<SpatialLayer>(),
      false};
  RunTest(screenshare);
}

const VideoQualityTest::Params::Video kSvcVp9Video = {
    true,    1280,    720,   30,    800000,
    2500000, 2500000, false, "VP9", 3,
    2,       400000,  false, false, "ConferenceMotion_1280_720_50"};

const VideoQualityTest::Params::Video kSimulcastVp8VideoHigh = {
    true,    1280,    720,   30,    800000,
    2500000, 2500000, false, "VP8", 3,
    2,       400000,  false, false, "ConferenceMotion_1280_720_50"};

const VideoQualityTest::Params::Video kSimulcastVp8VideoMedium = {
    true,   640,    360,   30,    150000,
    500000, 700000, false, "VP8", 3,
    2,      400000, false, false, "ConferenceMotion_1280_720_50"};

const VideoQualityTest::Params::Video kSimulcastVp8VideoLow = {
    true,   320,    180,   30,    30000,
    150000, 200000, false, "VP8", 3,
    2,      400000, false, false, "ConferenceMotion_1280_720_50"};

#if !defined(RTC_DISABLE_VP9)
TEST_F(FullStackTest, ScreenshareSlidesVP9_2SL) {
  VideoQualityTest::Params screenshare;
  screenshare.call.send_side_bwe = true;
  screenshare.video[0] = {true,   1850,    1110,  5,     50000,
                          200000, 2000000, false, "VP9", 1,
                          0,      400000,  false, false, ""};
  screenshare.screenshare[0] = {true, false, 10};
  screenshare.analyzer = {"screenshare_slides_vp9_2sl", 0.0, 0.0,
                          kFullStackTestDurationSecs};
  screenshare.ss[0] = {
      std::vector<VideoStream>(),  0,    2, 1, InterLayerPredMode::kOn,
      std::vector<SpatialLayer>(), false};
  RunTest(screenshare);
}

TEST_F(FullStackTest, VP9SVC_3SL_High) {
  VideoQualityTest::Params simulcast;
  simulcast.call.send_side_bwe = true;
  simulcast.video[0] = kSvcVp9Video;
  simulcast.analyzer = {"vp9svc_3sl_high", 0.0, 0.0,
                        kFullStackTestDurationSecs};
  simulcast.ss[0] = {
      std::vector<VideoStream>(),  0,    3, 2, InterLayerPredMode::kOn,
      std::vector<SpatialLayer>(), false};
  RunTest(simulcast);
}

TEST_F(FullStackTest, VP9SVC_3SL_Medium) {
  VideoQualityTest::Params simulcast;
  simulcast.call.send_side_bwe = true;
  simulcast.video[0] = kSvcVp9Video;
  simulcast.analyzer = {"vp9svc_3sl_medium", 0.0, 0.0,
                        kFullStackTestDurationSecs};
  simulcast.ss[0] = {
      std::vector<VideoStream>(),  0,    3, 1, InterLayerPredMode::kOn,
      std::vector<SpatialLayer>(), false};
  RunTest(simulcast);
}

TEST_F(FullStackTest, VP9SVC_3SL_Low) {
  VideoQualityTest::Params simulcast;
  simulcast.call.send_side_bwe = true;
  simulcast.video[0] = kSvcVp9Video;
  simulcast.analyzer = {"vp9svc_3sl_low", 0.0, 0.0, kFullStackTestDurationSecs};
  simulcast.ss[0] = {
      std::vector<VideoStream>(),  0,    3, 0, InterLayerPredMode::kOn,
      std::vector<SpatialLayer>(), false};
  RunTest(simulcast);
}

TEST_F(FullStackTest, VP9KSVC_3SL_High) {
  VideoQualityTest::Params simulcast;
  simulcast.call.send_side_bwe = true;
  simulcast.video[0] = kSvcVp9Video;
  simulcast.analyzer = {"vp9ksvc_3sl_high", 0.0, 0.0,
                        kFullStackTestDurationSecs};
  simulcast.ss[0] = {
      std::vector<VideoStream>(),  0,    3, 2, InterLayerPredMode::kOnKeyPic,
      std::vector<SpatialLayer>(), false};
  RunTest(simulcast);
}

TEST_F(FullStackTest, VP9KSVC_3SL_Medium) {
  VideoQualityTest::Params simulcast;
  simulcast.call.send_side_bwe = true;
  simulcast.video[0] = kSvcVp9Video;
  simulcast.analyzer = {"vp9ksvc_3sl_medium", 0.0, 0.0,
                        kFullStackTestDurationSecs};
  simulcast.ss[0] = {
      std::vector<VideoStream>(),  0,    3, 1, InterLayerPredMode::kOnKeyPic,
      std::vector<SpatialLayer>(), false};
  RunTest(simulcast);
}

TEST_F(FullStackTest, VP9KSVC_3SL_Low) {
  VideoQualityTest::Params simulcast;
  simulcast.call.send_side_bwe = true;
  simulcast.video[0] = kSvcVp9Video;
  simulcast.analyzer = {"vp9ksvc_3sl_low", 0.0, 0.0,
                        kFullStackTestDurationSecs};
  simulcast.ss[0] = {
      std::vector<VideoStream>(),  0,    3, 0, InterLayerPredMode::kOnKeyPic,
      std::vector<SpatialLayer>(), false};
  RunTest(simulcast);
}

TEST_F(FullStackTest, VP9KSVC_3SL_Medium_Network_Restricted) {
  VideoQualityTest::Params simulcast;
  simulcast.call.send_side_bwe = true;
  simulcast.video[0] = kSvcVp9Video;
  simulcast.analyzer = {"vp9ksvc_3sl_medium_network_restricted", 0.0, 0.0,
                        kFullStackTestDurationSecs};
  simulcast.ss[0] = {
      std::vector<VideoStream>(),  0,    3, 1, InterLayerPredMode::kOnKeyPic,
      std::vector<SpatialLayer>(), false};
  simulcast.pipe.link_capacity_kbps = 1000;
  RunTest(simulcast);
}

#endif  // !defined(RTC_DISABLE_VP9)

// Android bots can't handle FullHD, so disable the test.
// TODO(bugs.webrtc.org/9220): Investigate source of flakiness on Mac.
#if defined(WEBRTC_ANDROID) || defined(WEBRTC_MAC)
#define MAYBE_SimulcastFullHdOveruse DISABLED_SimulcastFullHdOveruse
#else
#define MAYBE_SimulcastFullHdOveruse SimulcastFullHdOveruse
#endif

TEST_F(FullStackTest, MAYBE_SimulcastFullHdOveruse) {
  VideoQualityTest::Params simulcast;
  simulcast.call.send_side_bwe = true;
  simulcast.video[0] = {true,    1920,    1080,  30,    800000,
                        2500000, 2500000, false, "VP8", 3,
                        2,       400000,  false, false, "Generator"};
  simulcast.analyzer = {"simulcast_HD_high", 0.0, 0.0,
                        kFullStackTestDurationSecs};
  simulcast.pipe.loss_percent = 0;
  simulcast.pipe.queue_delay_ms = 100;
  std::vector<VideoStream> streams = {DefaultVideoStream(simulcast, 0),
                                      DefaultVideoStream(simulcast, 0),
                                      DefaultVideoStream(simulcast, 0)};
  simulcast.ss[0] = {
      streams, 2, 1, 0, InterLayerPredMode::kOn, std::vector<SpatialLayer>(),
      true};
  webrtc::test::ScopedFieldTrials override_trials(
      "WebRTC-ForceSimulatedOveruseIntervalMs/1000-50000-300/");
  RunTest(simulcast);
}

TEST_F(FullStackTest, SimulcastVP8_3SL_High) {
  VideoQualityTest::Params simulcast;
  simulcast.call.send_side_bwe = true;
  simulcast.video[0] = kSimulcastVp8VideoHigh;
  simulcast.analyzer = {"simulcast_vp8_3sl_high", 0.0, 0.0,
                        kFullStackTestDurationSecs};
  simulcast.pipe.loss_percent = 0;
  simulcast.pipe.queue_delay_ms = 100;
  VideoQualityTest::Params video_params_high;
  video_params_high.video[0] = kSimulcastVp8VideoHigh;
  VideoQualityTest::Params video_params_medium;
  video_params_medium.video[0] = kSimulcastVp8VideoMedium;
  VideoQualityTest::Params video_params_low;
  video_params_low.video[0] = kSimulcastVp8VideoLow;

  std::vector<VideoStream> streams = {
      DefaultVideoStream(video_params_low, 0),
      DefaultVideoStream(video_params_medium, 0),
      DefaultVideoStream(video_params_high, 0)};
  simulcast.ss[0] = {
      streams, 2, 1, 0, InterLayerPredMode::kOn, std::vector<SpatialLayer>(),
      false};
  RunTest(simulcast);
}

TEST_F(FullStackTest, SimulcastVP8_3SL_Medium) {
  VideoQualityTest::Params simulcast;
  simulcast.call.send_side_bwe = true;
  simulcast.video[0] = kSimulcastVp8VideoHigh;
  simulcast.analyzer = {"simulcast_vp8_3sl_medium", 0.0, 0.0,
                        kFullStackTestDurationSecs};
  simulcast.pipe.loss_percent = 0;
  simulcast.pipe.queue_delay_ms = 100;
  VideoQualityTest::Params video_params_high;
  video_params_high.video[0] = kSimulcastVp8VideoHigh;
  VideoQualityTest::Params video_params_medium;
  video_params_medium.video[0] = kSimulcastVp8VideoMedium;
  VideoQualityTest::Params video_params_low;
  video_params_low.video[0] = kSimulcastVp8VideoLow;

  std::vector<VideoStream> streams = {
      DefaultVideoStream(video_params_low, 0),
      DefaultVideoStream(video_params_medium, 0),
      DefaultVideoStream(video_params_high, 0)};
  simulcast.ss[0] = {
      streams, 1, 1, 0, InterLayerPredMode::kOn, std::vector<SpatialLayer>(),
      false};
  RunTest(simulcast);
}

TEST_F(FullStackTest, SimulcastVP8_3SL_Low) {
  VideoQualityTest::Params simulcast;
  simulcast.call.send_side_bwe = true;
  simulcast.video[0] = kSimulcastVp8VideoHigh;
  simulcast.analyzer = {"simulcast_vp8_3sl_low", 0.0, 0.0,
                        kFullStackTestDurationSecs};
  simulcast.pipe.loss_percent = 0;
  simulcast.pipe.queue_delay_ms = 100;
  VideoQualityTest::Params video_params_high;
  video_params_high.video[0] = kSimulcastVp8VideoHigh;
  VideoQualityTest::Params video_params_medium;
  video_params_medium.video[0] = kSimulcastVp8VideoMedium;
  VideoQualityTest::Params video_params_low;
  video_params_low.video[0] = kSimulcastVp8VideoLow;

  std::vector<VideoStream> streams = {
      DefaultVideoStream(video_params_low, 0),
      DefaultVideoStream(video_params_medium, 0),
      DefaultVideoStream(video_params_high, 0)};
  simulcast.ss[0] = {
      streams, 0, 1, 0, InterLayerPredMode::kOn, std::vector<SpatialLayer>(),
      false};
  RunTest(simulcast);
}

TEST_F(FullStackTest, LargeRoomVP8_5thumb) {
  VideoQualityTest::Params large_room;
  large_room.call.send_side_bwe = true;
  large_room.video[0] = kSimulcastVp8VideoHigh;
  large_room.analyzer = {"largeroom_5thumb", 0.0, 0.0,
                         kFullStackTestDurationSecs};
  large_room.pipe.loss_percent = 0;
  large_room.pipe.queue_delay_ms = 100;
  VideoQualityTest::Params video_params_high;
  video_params_high.video[0] = kSimulcastVp8VideoHigh;
  VideoQualityTest::Params video_params_medium;
  video_params_medium.video[0] = kSimulcastVp8VideoMedium;
  VideoQualityTest::Params video_params_low;
  video_params_low.video[0] = kSimulcastVp8VideoLow;

  std::vector<VideoStream> streams = {
      DefaultVideoStream(video_params_low, 0),
      DefaultVideoStream(video_params_medium, 0),
      DefaultVideoStream(video_params_high, 0)};
  large_room.call.num_thumbnails = 5;
  large_room.ss[0] = {
      streams, 2, 1, 0, InterLayerPredMode::kOn, std::vector<SpatialLayer>(),
      false};
  RunTest(large_room);
}

#if defined(WEBRTC_ANDROID) || defined(WEBRTC_IOS)
// Fails on mobile devices:
// https://bugs.chromium.org/p/webrtc/issues/detail?id=7301
#define MAYBE_LargeRoomVP8_50thumb DISABLED_LargeRoomVP8_50thumb
#define MAYBE_LargeRoomVP8_15thumb DISABLED_LargeRoomVP8_15thumb
#else
#define MAYBE_LargeRoomVP8_50thumb LargeRoomVP8_50thumb
#define MAYBE_LargeRoomVP8_15thumb LargeRoomVP8_15thumb
#endif

TEST_F(FullStackTest, MAYBE_LargeRoomVP8_15thumb) {
  VideoQualityTest::Params large_room;
  large_room.call.send_side_bwe = true;
  large_room.video[0] = kSimulcastVp8VideoHigh;
  large_room.analyzer = {"largeroom_15thumb", 0.0, 0.0,
                         kFullStackTestDurationSecs};
  large_room.pipe.loss_percent = 0;
  large_room.pipe.queue_delay_ms = 100;
  VideoQualityTest::Params video_params_high;
  video_params_high.video[0] = kSimulcastVp8VideoHigh;
  VideoQualityTest::Params video_params_medium;
  video_params_medium.video[0] = kSimulcastVp8VideoMedium;
  VideoQualityTest::Params video_params_low;
  video_params_low.video[0] = kSimulcastVp8VideoLow;

  std::vector<VideoStream> streams = {
      DefaultVideoStream(video_params_low, 0),
      DefaultVideoStream(video_params_medium, 0),
      DefaultVideoStream(video_params_high, 0)};
  large_room.call.num_thumbnails = 15;
  large_room.ss[0] = {
      streams, 2, 1, 0, InterLayerPredMode::kOn, std::vector<SpatialLayer>(),
      false};
  RunTest(large_room);
}

TEST_F(FullStackTest, MAYBE_LargeRoomVP8_50thumb) {
  VideoQualityTest::Params large_room;
  large_room.call.send_side_bwe = true;
  large_room.video[0] = kSimulcastVp8VideoHigh;
  large_room.analyzer = {"largeroom_50thumb", 0.0, 0.0,
                         kFullStackTestDurationSecs};
  large_room.pipe.loss_percent = 0;
  large_room.pipe.queue_delay_ms = 100;
  VideoQualityTest::Params video_params_high;
  video_params_high.video[0] = kSimulcastVp8VideoHigh;
  VideoQualityTest::Params video_params_medium;
  video_params_medium.video[0] = kSimulcastVp8VideoMedium;
  VideoQualityTest::Params video_params_low;
  video_params_low.video[0] = kSimulcastVp8VideoLow;

  std::vector<VideoStream> streams = {
      DefaultVideoStream(video_params_low, 0),
      DefaultVideoStream(video_params_medium, 0),
      DefaultVideoStream(video_params_high, 0)};
  large_room.call.num_thumbnails = 50;
  large_room.ss[0] = {
      streams, 2, 1, 0, InterLayerPredMode::kOn, std::vector<SpatialLayer>(),
      false};
  RunTest(large_room);
}

class DualStreamsTest : public FullStackTest,
                        public ::testing::WithParamInterface<int> {};

// Disable dual video test on mobile device becuase it's too heavy.
#if !defined(WEBRTC_ANDROID) && !defined(WEBRTC_IOS)
TEST_P(DualStreamsTest,
       ModeratelyRestricted_SlidesVp8_3TL_Simulcast_Video_Simulcast_High) {
  test::ScopedFieldTrials field_trial(
      kScreenshareSimulcastExperiment + kAlrProbingExperiment +
      kRoundRobinPacingQueueExperiment + kPacerPushBackExperiment);
  const int first_stream = GetParam();
  VideoQualityTest::Params dual_streams;

  // Screenshare Settings.
  dual_streams.screenshare[first_stream] = {true, false, 10};
  dual_streams.video[first_stream] = {true,    1850,    1110,  5,     800000,
                                      2500000, 2500000, false, "VP8", 3,
                                      2,       400000,  false, false, ""};

  VideoQualityTest::Params screenshare_params_high;
  screenshare_params_high.video[0] = {true,    1850,    1110,  5,     800000,
                                      2500000, 2500000, false, "VP8", 3,
                                      0,       400000,  false, false, ""};
  VideoQualityTest::Params screenshare_params_low;
  screenshare_params_low.video[0] = {true,   1850,    1110,  5,     50000,
                                     200000, 2000000, false, "VP8", 2,
                                     0,      400000,  false, false, ""};
  std::vector<VideoStream> screenhsare_streams = {
      DefaultVideoStream(screenshare_params_low, 0),
      DefaultVideoStream(screenshare_params_high, 0)};

  dual_streams.ss[first_stream] = {
      screenhsare_streams,         1,    1, 0, InterLayerPredMode::kOn,
      std::vector<SpatialLayer>(), false};

  // Video settings.
  dual_streams.video[1 - first_stream] = kSimulcastVp8VideoHigh;

  VideoQualityTest::Params video_params_high;
  video_params_high.video[0] = kSimulcastVp8VideoHigh;
  VideoQualityTest::Params video_params_medium;
  video_params_medium.video[0] = kSimulcastVp8VideoMedium;
  VideoQualityTest::Params video_params_low;
  video_params_low.video[0] = kSimulcastVp8VideoLow;
  std::vector<VideoStream> streams = {
      DefaultVideoStream(video_params_low, 0),
      DefaultVideoStream(video_params_medium, 0),
      DefaultVideoStream(video_params_high, 0)};

  dual_streams.ss[1 - first_stream] = {
      streams, 2, 1, 0, InterLayerPredMode::kOn, std::vector<SpatialLayer>(),
      false};

  // Call settings.
  dual_streams.call.send_side_bwe = true;
  dual_streams.call.dual_video = true;
  std::string test_label = "dualstreams_moderately_restricted_screenshare_" +
                           std::to_string(first_stream);
  dual_streams.analyzer = {test_label, 0.0, 0.0, kFullStackTestDurationSecs};
  dual_streams.pipe.loss_percent = 1;
  dual_streams.pipe.link_capacity_kbps = 7500;
  dual_streams.pipe.queue_length_packets = 30;
  dual_streams.pipe.queue_delay_ms = 100;

  RunTest(dual_streams);
}
#endif  // !defined(WEBRTC_ANDROID) && !defined(WEBRTC_IOS)

TEST_P(DualStreamsTest, Conference_Restricted) {
  test::ScopedFieldTrials field_trial(kRoundRobinPacingQueueExperiment +
                                      kPacerPushBackExperiment);
  const int first_stream = GetParam();
  VideoQualityTest::Params dual_streams;

  // Screenshare Settings.
  dual_streams.screenshare[first_stream] = {true, false, 10};
  dual_streams.video[first_stream] = {true,    1850,    1110,  5,     800000,
                                      2500000, 2500000, false, "VP8", 3,
                                      2,       400000,  false, false, ""};
  // Video settings.
  dual_streams.video[1 - first_stream] = {
      true,   1280,   720,   30,    150000,
      500000, 700000, false, "VP8", 3,
      2,      400000, false, false, "ConferenceMotion_1280_720_50"};

  // Call settings.
  dual_streams.call.send_side_bwe = true;
  dual_streams.call.dual_video = true;
  std::string test_label = "dualstreams_conference_restricted_screenshare_" +
                           std::to_string(first_stream);
  dual_streams.analyzer = {test_label, 0.0, 0.0, kFullStackTestDurationSecs};
  dual_streams.pipe.loss_percent = 1;
  dual_streams.pipe.link_capacity_kbps = 5000;
  dual_streams.pipe.queue_length_packets = 30;
  dual_streams.pipe.queue_delay_ms = 100;

  RunTest(dual_streams);
}

INSTANTIATE_TEST_CASE_P(FullStackTest,
                        DualStreamsTest,
                        ::testing::Values(0, 1));

}  // namespace webrtc
