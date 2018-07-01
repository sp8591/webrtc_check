#include <memory>
#include <utility>
#include <vector>

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/test/fakeconstraints.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "examples/peerconnection/client/defaults.h"
#include "media/engine/webrtcvideocapturerfactory.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture_factory.h"
#include "rtc_base/checks.h"
#include "rtc_base/json.h"
#include "rtc_base/logging.h"

//from conductor.h
#include "api/mediastreaminterface.h"
#include "api/peerconnectioninterface.h"
#include "iostream"
#include "examples/peerconnection/client/linux/main_wndcalldirect.h"
// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";
std::unique_ptr<cricket::VideoCapturer> OpenVideoCaptureDevice() {
	std::vector<std::string> device_names;
	{
		std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(
			webrtc::VideoCaptureFactory::CreateDeviceInfo());
		if (!info) {
			return nullptr;
		}
		int num_devices = info->NumberOfDevices();
		for (int i = 0; i < num_devices; ++i) {
			const uint32_t kSize = 256;
			char name[kSize] = {0};
			char id[kSize] = {0};
			if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
				device_names.push_back(name);
			}
		}
	}

	cricket::WebRtcVideoDeviceCapturerFactory factory;
	std::unique_ptr<cricket::VideoCapturer> capturer;
	for (const auto& name : device_names) {
		capturer = factory.Create(cricket::Device(name, 0));
		if (capturer) {
			break;
		}
	}
	return capturer;
}
void initialize(GtkMainWnd* main_wnd_) {
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
	peer_connection_factory_;
	peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
      nullptr /* network_thread */, nullptr /* worker_thread */,
      nullptr /* signaling_thread */, nullptr /* default_adm */,
		webrtc::CreateBuiltinAudioEncoderFactory(),
		webrtc::CreateBuiltinAudioDecoderFactory(),
		webrtc::CreateBuiltinVideoEncoderFactory(),
      webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
      nullptr /* audio_processing */);
		if(! peer_connection_factory_) {
			std::cout << "initialize peer_connection_factory_ failed!" << std::endl;
		} else {
			std::cout << "initialize peer_connection_factory_ success!" << std::endl;
		}
		
		webrtc::PeerConnectionObserver* observer = NULL;
		webrtc::PeerConnectionInterface::RTCConfiguration config;
		config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
		config.enable_dtls_srtp = false;
		webrtc::PeerConnectionInterface::IceServer server;
		server.uri = GetPeerConnectionString();
		config.servers.push_back(server);
		peer_connection_ = peer_connection_factory_->
		CreatePeerConnection(config, nullptr, nullptr, observer);

		//addtracks
		//for local render
		std::unique_ptr<cricket::VideoCapturer> video_device = OpenVideoCaptureDevice();
		if(video_device) {
			rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_(
				peer_connection_factory_->CreateVideoTrack(
					kVideoLabel, peer_connection_factory_->CreateVideoSource(
						std::move(video_device), nullptr)));
			main_wnd_->StartLocalRenderer(video_track_);
		}
		//for peerconnection track, create tranceivers
		auto result_or_error = peer_connection_->AddTrack(video_track_, {kStreamId});
		if (!result_or_error.ok()) {
			std::cout << "Failed to add video track to PeerConnection: "
                        << result_or_error.error().message();
		}
		
  		//main_wnd_->SwitchToConnectUI();
	}
	
