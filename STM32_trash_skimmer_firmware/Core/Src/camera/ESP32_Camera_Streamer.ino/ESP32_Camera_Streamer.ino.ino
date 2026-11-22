#include "esp_camera.h"
#include "esp_http_server.h"
#include <WiFi.h>

static const char* AP_SSID = "SkimmerCam";
static const char* AP_PASS = "skimmer123"; 
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// MJPEG multipart boundary string
static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=123456789000000000000987654321";
static const char* STREAM_BOUNDARY     = "\r\n--123456789000000000000987654321\r\n";
static const char* STREAM_PART         = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static httpd_handle_t stream_httpd = NULL;

// HTTP GET handler for /stream on port 81
static esp_err_t stream_handler(httpd_req_t* req) {
  camera_fb_t* fb = NULL;
  esp_err_t res = ESP_OK;
  char part_buf[64];

  res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  while (true) {
    // Capture one frame buffer from the OV2640 sensor
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
      break;
    }

    // Send boundary marker
    res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));

    // Send JPEG part header with exact byte length
    if (res == ESP_OK) {
      size_t hlen = snprintf(part_buf, sizeof(part_buf), STREAM_PART, fb->len);
      res = httpd_resp_send_chunk(req, part_buf, hlen);
    }

    // Send raw JPEG image payload
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char*)fb->buf, fb->len);
    }

    // Always return the frame buffer to the driver pool
    esp_camera_fb_return(fb);
    fb = NULL;

    if (res != ESP_OK) {
      // Client disconnected or socket closed
      break;
    }
  }

  return res;
}

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Low-latency QVGA (320x240) configuration
  config.frame_size   = FRAMESIZE_QVGA;
  config.jpeg_quality = 12; // 10-63 (lower = higher quality, larger size)
  config.fb_count     = 2;  // Double buffer for smoother capture/send overlap

  // Use PSRAM if present; fall back to internal DRAM if not
  if (psramFound()) {
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode   = CAMERA_GRAB_LATEST;
  } else {
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.grab_mode   = CAMERA_GRAB_WHEN_EMPTY;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }

  // Optional sensor tuning for outdoor water glare / exposure stability
  sensor_t* s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 1);     // Slightly boosted color for trash detection
    s->set_whitebal(s, 1);       // Auto white balance enabled
    s->set_exposure_ctrl(s, 1);  // Auto exposure enabled
    s->set_gain_ctrl(s, 1);      // Auto gain enabled
  }

  return true;
}

void startStreamServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 81;
  config.ctrl_port   = 32768;

  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
    Serial.println("MJPEG stream server started on port 81 (/stream)");
  } else {
    Serial.println("Failed to start HTTP stream server");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  Serial.println("\n--- SkimmerCam Stage 1 Boot ---");

  if (!initCamera()) {
    Serial.println("Halting: camera initialization failed.");
    while (true) {
      delay(1000);
    }
  }

  // Start Wi-Fi SoftAP (default IP is 192.168.4.1)
  WiFi.mode(WIFI_AP);
  bool ap_ok = WiFi.softAP(AP_SSID, AP_PASS);
  if (!ap_ok) {
    Serial.println("Failed to start SoftAP!");
    while (true) {
      delay(1000);
    }
  }

  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP IP Address: ");
  Serial.println(ip);

  startStreamServer();
  Serial.print("Open stream at: http://");
  Serial.print(ip);
  Serial.println(":81/stream");
}

void loop() {
  // HTTP server runs asynchronously in its own FreeRTOS task
  delay(1000);
}
