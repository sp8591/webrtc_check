/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef EXAMPLES_PEERCONNECTION_CLIENT_LINUX_MAIN_WND_H_
#define EXAMPLES_PEERCONNECTION_CLIENT_LINUX_MAIN_WND_H_

#include <memory>
#include <string>

#include "examples/peerconnection/client/main_wnd.h"
#include "examples/peerconnection/client/peer_connection_client.h"

// Forward declarations.
typedef struct _GtkWidget GtkWidget;
typedef union _GdkEvent GdkEvent;
typedef struct _GdkEventKey GdkEventKey;
typedef struct _GtkTreeView GtkTreeView;
typedef struct _GtkTreePath GtkTreePath;
typedef struct _GtkTreeViewColumn GtkTreeViewColumn;
typedef struct _cairo cairo_t;

// Implements the main UI of the peer connection client.
// This is functionally equivalent to the MainWnd class in the Windows
// implementation.


class GtkMainWnd  {
public:
    GtkWidget* window_;
    GtkWidget* window_control;
    GtkWidget* draw_area_;
    GtkWidget* vbox_;  // Container for the Connect UI.
    GtkWidget* server_edit_;
    GtkWidget* port_edit_;
    GtkWidget* peer_list_;  // The list of peers.
    int width_;
    int height_;
    std::unique_ptr<uint8_t[]> draw_buffer_;
    int draw_buffer_size_;
    bool Create();
    void SwitchToConnectUI();
    GtkMainWnd();
    void SwitchToStreamingUI();
    void Draw(GtkWidget* widget, cairo_t* cr);
    void OnRedraw();
    virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);

    void OnClicked(GtkWidget* widget);
    class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
    public:
        VideoRenderer(GtkMainWnd* main_wnd,
          webrtc::VideoTrackInterface* track_to_render);
        virtual ~VideoRenderer();

    // VideoSinkInterface implementation
        void OnFrame(const webrtc::VideoFrame& frame) override;

        const uint8_t* image() const { return image_.get(); }

        int width() const {
          return width_;
      }

      int height() const {
          return height_;
      }

  protected:
    void SetSize(int width, int height);
    std::unique_ptr<uint8_t[]> image_;
    int width_;
    int height_;
    GtkMainWnd* main_wnd_;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
};

std::unique_ptr<VideoRenderer> local_renderer_;
std::unique_ptr<VideoRenderer> remote_renderer_;

};

#endif  // EXAMPLES_PEERCONNECTION_CLIENT_LINUX_MAIN_WND_H_
