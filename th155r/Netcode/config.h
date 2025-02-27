#pragma once

#ifndef CONFIG_H
#define CONFIG_H 1

#include <stdint.h>
#include <stdlib.h>


#include "util.h"

// Increase the main version number whenever a change
// results in incompatibilites with the previous netcode.
static inline constexpr size_t PLUGIN_VERSION = 1;
// Increase the revision number for bugfixing builds and
// other sorts of changes that don't make the netcode
// incompatible. Reset to 0 whenever increasing the main version.
static inline constexpr size_t PLUGIN_REVISION = 2;

void init_config_file();

const char* get_lobby_host(const char* host);
const char* get_lobby_port(const char* port);
const char* get_lobby_pass(const char* pass);

bool get_hide_ip_enabled();
bool get_hide_name_enabled();
bool get_skip_intro_enabled();

bool get_ping_enabled();
int32_t get_ping_x();
int32_t get_ping_y();
float get_ping_scale_x();
float get_ping_scale_y();
uint32_t get_ping_color();
bool get_ping_frames();

bool get_inputp1_enabled();
int32_t get_inputp1_x();
int32_t get_inputp1_y();
float get_inputp1_scale_x();
float get_inputp1_scale_y();
int32_t get_inputp1_offset();
int32_t get_inputp1_count();
uint32_t get_inputp1_color();
bool get_inputp1_spacing();
int32_t get_inputp1_timer();
bool get_inputp1_raw_input();

bool get_hide_wip_enabled();

bool get_inputp2_enabled();
int32_t get_inputp2_x();
int32_t get_inputp2_y();
float get_inputp2_scale_x();
float get_inputp2_scale_y();
int32_t get_inputp2_offset();
int32_t get_inputp2_count();
uint32_t get_inputp2_color();
bool get_inputp2_spacing();
int32_t get_inputp2_timer();
bool get_inputp2_raw_input();

enum IPv6State : int8_t {
	IPv6NeedsTest = -1,
	IPv6Disabled = 0,
	IPv6Enabled = 1
};

IPv6State get_ipv6_state();
bool get_netplay_state();

void set_ipv6_state(bool state);

#endif