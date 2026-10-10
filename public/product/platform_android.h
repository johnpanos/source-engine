//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: product.platform.android (RFC 0027 L7): the Android platform
//			providers that replace build-android-apk.sh.
//
//			android-ndk (ITargetToolchain)
//			  Fetches and verifies the profile's pinned archives (NDK, SDL3,
//			  KTX-Software, SDK platform and build-tools) into
//			  dependencies/android-ndk/, cross-builds the native dependencies
//			  (SDL3, the KTX reader, freetype, libpng, libjpeg, curl) for the
//			  flavor's ABI under a key covering every pin, and gives Waf the
//			  `--android` option and the pkg-config environment. The flavor
//			  names the ABI and must be one of android.abis.
//			android-native-libs (IProductStage)
//			  Consumes engine-install, strips every shared object and adds
//			  libSDL3.so and the NDK's libc++_shared.so; publishes the
//			  `android-libs` directory (lib/<abi>/*.so) whose facts carry the
//			  SDK paths the packager needs and an unstripped symbols/ copy.
//			android-apk (IPackager)
//			  aapt2 link, javac + d8 for SDLActivity, zipalign -P 16 and
//			  apksigner; then the independent verifier
//			  (tools/quality/android_apk.py check). Debug-signed unless the
//			  request carries release credentials (keystore, alias,
//			  keystore_pass, key_pass; never persisted or logged).
//			adb (IDeployTransport)
//			  install, content-sync (against a manifest recorded in the
//			  content root), launch (`am start -n <arguments[0]>`),
//			  device-facts.
//
//=============================================================================//

#ifndef PUBLIC_PRODUCT_PLATFORM_ANDROID_H
#define PUBLIC_PRODUCT_PLATFORM_ANDROID_H

#include "product/contracts.h"

#include <memory>

namespace product
{

inline constexpr std::string_view kAndroidNdkToolchain = "android-ndk";
inline constexpr std::string_view kAndroidNativeLibsStage = "android-native-libs";
inline constexpr std::string_view kAndroidLibsArtifact = "android-libs";
inline constexpr std::string_view kAndroidApkPackager = "android-apk";
inline constexpr std::string_view kAdbTransport = "adb";

// The process provider is borrowed and must outlive the provider.
std::unique_ptr<ITargetToolchain> CreateAndroidNdkToolchain( platform::IToolProcessProvider &processes );
std::unique_ptr<IProductStage> CreateAndroidNativeLibsStage();
std::unique_ptr<IPackager> CreateAndroidApkPackager( platform::IToolProcessProvider &processes );
std::unique_ptr<IDeployTransport> CreateAdbTransport( platform::IToolProcessProvider &processes );

} // namespace product

#endif // PUBLIC_PRODUCT_PLATFORM_ANDROID_H
