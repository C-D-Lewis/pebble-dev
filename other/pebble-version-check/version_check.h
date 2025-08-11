/**
 * Along with VersionCheck.java, a basic version checking mechanism
 * Author: Chris Lewis
 * Version 1.2.0
 */
#pragma once

#include <pebble.h>

//Keys, spelt out on phone dialpad
#define KEY_VERSION_CHECK_VERSION 58283
#define KEY_VERSION_CHECK_SUCCESS 58278
#define KEY_VERSION_CHECK_FAILURE 58232

typedef void(VersionCheckCallback)(bool succeeded);

/**
 * Check the version number string, with a callback for response
 * During the intervening time, AppMessage will not be available as normal.
 * REMEMBER TO REGISTER NORMAL CALLBACK AFTER SUCCESS!
 */
void version_check(char *version, VersionCheckCallback *callback);
