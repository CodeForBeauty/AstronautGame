cmake_minimum_required (VERSION 3.12...3.26)

# MSVC runtime library flags are selected by an abstraction, CMake >= 3.15
# This policy still need to be set even with cmake_minimum_required() command above.
if (POLICY CMP0091)
	if ((DEFINED ENABLE_STATIC_RUNTIME) AND (DEFINED CMAKE_MSVC_RUNTIME_LIBRARY))
		message (FATAL_ERROR "Both ENABLE_STATIC_RUNTIME and CMAKE_MSVC_RUNTIME_LIBRARY are set.")
		return ()
	endif ()

	if (CMAKE_VERSION VERSION_LESS 3.18)
		if (DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
			cmake_policy (SET CMP0091 NEW)
		else ()
			cmake_policy (SET CMP0091 OLD)
		endif ()
	endif ()
endif ()

option (ENABLE_EXTERNAL_LIBS "Enable FLAC, Vorbis, and Opus codecs" ON)
if (ENABLE_EXTERNAL_LIBS)
	list (APPEND VCPKG_MANIFEST_FEATURES "external-libs")
endif ()

option (ENABLE_MPEG "Enable MPEG codecs" ON)
if (ENABLE_MPEG)
	list (APPEND VCPKG_MANIFEST_FEATURES "mpeg")
endif ()

option (ENABLE_EXPERIMENTAL "Enable experimental code" OFF)
if (ENABLE_EXPERIMENTAL)
	list (APPEND VCPKG_MANIFEST_FEATURES "speex")
endif ()

option (BUILD_REGTEST "Build regtest" OFF)
if (BUILD_REGTEST)
	list (APPEND VCPKG_MANIFEST_FEATURES "regtest")
endif ()

project(libsndfile VERSION 1.2.2)

#
# Variables
#

set (CMAKE_C_STANDARD 99)
set (CMAKE_C_STANDARD_REQUIRED TRUE)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set (PACKAGE_NAME ${PROJECT_NAME})
set (CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set (CPACK_PACKAGE_VERSION_STAGE "")
set (CPACK_PACKAGE_VERSION_FULL "${PROJECT_VERSION}${CPACK_PACKAGE_VERSION_STAGE}")

#
# System-wide includes
#

include (GNUInstallDirs)
include (FeatureSummary)
include (CMakeDependentOption)
include (CTest)
include (CheckCCompilerFlag)

#
# Options
#

option (BUILD_SHARED_LIBS "Build shared libraries" OFF)
if (BUILD_SHARED_LIBS AND BUILD_TESTING)
	set (BUILD_TESTING OFF)
	message ("Build testing required static libraries. To prevent build errors BUILD_TESTING disabled.")
endif ()
option (BUILD_PROGRAMS "Build programs" ON)
option (BUILD_EXAMPLES "Build examples" ON)
option (ENABLE_CPACK "Enable CPack support" ON)
option (ENABLE_BOW_DOCS "Enable black-on-white html docs" OFF)
if (MSVC AND (DEFINED ENABLE_STATIC_RUNTIME))
	option (ENABLE_STATIC_RUNTIME "Enable static runtime" ${ENABLE_STATIC_RUNTIME})
elseif (MINGW)
	option (ENABLE_STATIC_RUNTIME "Enable static runtime" OFF)
endif ()
option (ENABLE_PACKAGE_CONFIG "Generate and install package config file" ON)
option (INSTALL_PKGCONFIG_MODULE "Generate and install pkg-config module" ON)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
if (CMAKE_VERSION VERSION_LESS 3.14)
	list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/sqlite")
endif ()

#
# Setup definitions
#

include(SndFileChecks)

if (ENABLE_EXTERNAL_LIBS AND NOT (Vorbis_FOUND OR FLAC_FOUND OR OPUS_FOUND))
	set (ENABLE_EXTERNAL_LIBS OFF)
endif()
if(ENABLE_MPEG AND (NOT HAVE_MPEG_LIBS))
	set (ENABLE_MPEG OFF)
endif()
if (BUILD_REGTEST AND (NOT SQLITE3_FOUND))
	set (BUILD_REGTEST OFF)
endif()

cmake_dependent_option (ENABLE_COMPATIBLE_LIBSNDFILE_NAME "Set DLL name to libsndfile-1.dll (canonical name), sndfile.dll otherwise" OFF "WIN32;BUILD_SHARED_LIBS" OFF)
cmake_dependent_option (INSTALL_MANPAGES "Install man pages for programs" ON "BUILD_PROGRAMS" OFF)

if (NOT MSVC)
	if (CPU_IS_X86)
		check_c_compiler_flag (-msse2 HAVE_MSSE2_COMPILER_FLAG)
		if (HAVE_MSSE2_COMPILER_FLAG)
			cmake_dependent_option (ENABLE_SSE2 "Add SSE2 compiler flag" ON "HAVE_MSSE2_COMPILER_FLAG" OFF)
		endif ()
	endif ()
endif ()
if (ENABLE_SSE2)
	add_compile_options (-msse2)
endif ()

set (HAVE_EXTERNAL_XIPH_LIBS ${ENABLE_EXTERNAL_LIBS})
set (HAVE_SQLITE3 ${BUILD_REGTEST})
set (HAVE_ALSA_ASOUNDLIB_H ${ALSA_FOUND})
set (HAVE_SNDIO_H ${SNDIO_FOUND})

set (ENABLE_EXPERIMENTAL_CODE ${ENABLE_EXPERIMENTAL})
set (HAVE_MPEG ${ENABLE_MPEG})
set (HAVE_SPEEX ${ENABLE_EXPERIMENTAL})

add_feature_info (BUILD_SHARED_LIBS BUILD_SHARED_LIBS "build shared libraries")
add_feature_info (ENABLE_EXTERNAL_LIBS ENABLE_EXTERNAL_LIBS "enable FLAC, Vorbis, and Opus codecs")
add_feature_info (ENABLE_MPEG ENABLE_MPEG "enable MPEG audio (including mp3) codecs")
add_feature_info (ENABLE_EXPERIMENTAL ENABLE_EXPERIMENTAL "enable experimental code")
add_feature_info (BUILD_TESTING BUILD_TESTING "build tests")
add_feature_info (BUILD_REGTEST BUILD_REGTEST "build regtest")
add_feature_info (ENABLE_CPACK ENABLE_CPACK "enable CPack support")
add_feature_info (ENABLE_BOW_DOCS ENABLE_BOW_DOCS "enable black-on-white html docs")
add_feature_info (ENABLE_PACKAGE_CONFIG ENABLE_PACKAGE_CONFIG "generate and install package config file")
add_feature_info (INSTALL_PKGCONFIG_MODULE INSTALL_PKGCONFIG_MODULE "generate and install pkg-config module")
add_feature_info (INSTALL_MANPAGES INSTALL_MANPAGES "install man pages for programs")
if (WIN32 AND BUILD_SHARED_LIBS)
	add_feature_info (ENABLE_COMPATIBLE_LIBSNDFILE_NAME ENABLE_COMPATIBLE_LIBSNDFILE_NAME "Set DLL name to libsndfile-1.dll (canonical name), sndfile.dll otherwise")
endif ()

if (HAVE_MSSE2_COMPILER_FLAG)
	add_feature_info (ENABLE_SSE2 ENABLE_SSE2 "add SSE2 compiler flag")
endif ()




set_package_properties (Ogg PROPERTIES
	TYPE RECOMMENDED
	URL "www.xiph.org/ogg/"
	DESCRIPTION "library for manipulating ogg bitstreams"
	PURPOSE "Required to enable Vorbis, Speex, and Opus support"
	)
set_package_properties (Vorbis PROPERTIES
	TYPE RECOMMENDED
	URL "www.vorbis.com/"
	DESCRIPTION "open source lossy audio codec"
	PURPOSE "Enables Vorbis support"
	)
set_package_properties (FLAC PROPERTIES
	TYPE RECOMMENDED
	URL "www.xiph.org/flac/"
	DESCRIPTION "Free Lossless Audio Codec Library"
	PURPOSE "Enables FLAC support"
	)
set_package_properties (mp3lame PROPERTIES
	TYPE RECOMMENDED
	URL "https://lame.sourceforge.io/"
	DESCRIPTION "High quality MPEG Audio Layer III (MP3) encoder"
	PURPOSE "Enables MPEG layer III (MP3) writing support"
	)
set_package_properties (mpg123 PROPERTIES
	TYPE RECOMMENDED
	URL "https://www.mpg123.de/"
	DESCRIPTION "MPEG Audio Layer I/II/III decoder"
	PURPOSE "Enables MPEG Audio reading support"
	)
set_package_properties(Opus PROPERTIES
	TYPE RECOMMENDED
	URL	"www.opus-codec.org/"
	DESCRIPTION "Standardized open source low-latency fullband codec"
	PURPOSE	"Enables experimental Opus support"
	)
set_package_properties(Speex PROPERTIES TYPE OPTIONAL
	URL "www.speex.org/"
	DESCRIPTION "an audio codec tuned for speech"
	PURPOSE "Enables experimental Speex support"
	)
set_package_properties (SQLite3 PROPERTIES
	TYPE OPTIONAL
	URL "www.sqlite.org/"
	DESCRIPTION "light weight SQL database engine."
	PURPOSE "Enables regtest"
	)
if (BUILD_SHARED_LIBS)
	set_package_properties (PythonInterp PROPERTIES
		TYPE REQUIRED
		URL "www.python.org/"
		DESCRIPTION "Python is a widely used high-level programming language."
		PURPOSE "Required to build shared libraries"
		)
endif()

feature_summary (WHAT ALL)

#
# Setup configuration
#

configure_file (src/config.h.cmake src/config.h)

if (INSTALL_PKGCONFIG_MODULE)

	set (prefix ${CMAKE_INSTALL_PREFIX})
	set (exec_prefix "\$\{prefix\}")
	set (libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
	set (includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
	set (VERSION ${PROJECT_VERSION})
	if (ENABLE_EXTERNAL_LIBS)
		set (EXTERNAL_XIPH_REQUIRE "flac ogg vorbis vorbisenc opus")
		if (ENABLE_EXPERIMENTAL)
			set (EXTERNAL_XIPH_REQUIRE "${EXTERNAL_XIPH_REQUIRE} speex")
		endif ()
	endif ()
	if (ENABLE_MPEG)
		set (EXTERNAL_MPEG_REQUIRE "libmpg123")
		set (EXTERNAL_MPEG_LIBS "-lmp3lame")
	endif ()

	configure_file (sndfile.pc.in sndfile.pc @ONLY)

endif ()

#
# libsndfile
#

# Public libsndfile headers
set (sndfile_HDRS
	include/sndfile.h
	include/sndfile.hh
	)

#
# libsndfile static library
#

add_library (sndfile
	src/sfconfig.h
	src/sfendian.h
	src/sf_unistd.h
	src/common.h
	src/common.c
	src/file_io.c
	src/command.c
	src/pcm.c
	src/ulaw.c
	src/alaw.c
	src/float32.c
	src/double64.c
	src/ima_adpcm.c
	src/ms_adpcm.c
	src/gsm610.c
	src/dwvw.c
	src/vox_adpcm.c
	src/interleave.c
	src/strings.c
	src/dither.c
	src/cart.c
	src/broadcast.c
	src/audio_detect.c
 	src/ima_oki_adpcm.c
	src/ima_oki_adpcm.h
	src/alac.c
	src/chunk.c
	src/ogg.h
	src/ogg.c
	src/chanmap.h
	src/chanmap.c
	src/id3.h
	src/id3.c
	$<$<BOOL:${WIN32}>:src/windows.c>
	src/sndfile.c
	src/aiff.c
	src/au.c
	src/avr.c
	src/caf.c
	src/dwd.c
	src/flac.c
	src/g72x.c
	src/htk.c
	src/ircam.c
	src/macos.c
	src/mat4.c
	src/mat5.c
	src/nist.c
	src/paf.c
	src/pvf.c
	src/raw.c
	src/rx2.c
	src/sd2.c
	src/sds.c
	src/svx.c
	src/txw.c
	src/voc.c
	src/wve.c
	src/w64.c
	src/wavlike.h
	src/wavlike.c
	src/wav.c
	src/xi.c
	src/mpc2k.c
	src/rf64.c
	src/ogg_vorbis.c
	src/ogg_speex.c
	src/ogg_pcm.c
	src/ogg_opus.c
	src/ogg_vcomment.h
	src/ogg_vcomment.c
	src/nms_adpcm.c
	src/mpeg.c
	src/mpeg_decode.c
	src/mpeg_l3_encode.c
	src/GSM610/config.h
	src/GSM610/gsm.h
	src/GSM610/gsm610_priv.h
	src/GSM610/add.c
	src/GSM610/code.c
	src/GSM610/decode.c
	src/GSM610/gsm_create.c
	src/GSM610/gsm_decode.c
	src/GSM610/gsm_destroy.c
	src/GSM610/gsm_encode.c
	src/GSM610/gsm_option.c
	src/GSM610/long_term.c
	src/GSM610/lpc.c
	src/GSM610/preprocess.c
	src/GSM610/rpe.c
	src/GSM610/short_term.c
	src/GSM610/table.c
	src/G72x/g72x.h
	src/G72x/g72x_priv.h
	src/G72x/g721.c
	src/G72x/g723_16.c
	src/G72x/g723_24.c
	src/G72x/g723_40.c
	src/G72x/g72x.c
	src/ALAC/ALACAudioTypes.h
	src/ALAC/ALACBitUtilities.h
	src/ALAC/EndianPortable.h
	src/ALAC/aglib.h
	src/ALAC/dplib.h
	src/ALAC/matrixlib.h
	src/ALAC/alac_codec.h
	src/ALAC/shift.h
	src/ALAC/ALACBitUtilities.c
	src/ALAC/ag_dec.c
	src/ALAC/ag_enc.c
	src/ALAC/dp_dec.c
	src/ALAC/dp_enc.c
	src/ALAC/matrix_dec.c
	src/ALAC/matrix_enc.c
	src/ALAC/alac_decoder.c
	src/ALAC/alac_encoder.c
	${sndfile_HDRS}
	${CMAKE_CURRENT_BINARY_DIR}/src/config.h
	)

add_library (SndFile::sndfile ALIAS sndfile)

target_include_directories (sndfile
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		src
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
	)
target_link_libraries (sndfile
	PRIVATE
		$<$<BOOL:${LIBM_REQUIRED}>:m>
		$<$<BOOL:${HAVE_EXTERNAL_XIPH_LIBS}>:Ogg::ogg>
		$<$<BOOL:${HAVE_EXTERNAL_XIPH_LIBS}>:Vorbis::vorbisenc>
		$<$<BOOL:${HAVE_EXTERNAL_XIPH_LIBS}>:FLAC::FLAC>
		$<$<AND:$<BOOL:${ENABLE_EXPERIMENTAL}>,$<BOOL:${HAVE_EXTERNAL_XIPH_LIBS}>,$<BOOL:${HAVE_SPEEX}>>:Speex::Speex>
		$<$<BOOL:${HAVE_EXTERNAL_XIPH_LIBS}>:Opus::opus>
		$<$<BOOL:${HAVE_MPEG}>:MPG123::libmpg123>
		$<$<BOOL:${HAVE_MPEG}>:mp3lame::mp3lame>
	)
set_target_properties (sndfile PROPERTIES
	PUBLIC_HEADER "${sndfile_HDRS}"
	)

if (ENABLE_COMPATIBLE_LIBSNDFILE_NAME)
	if (MINGW OR CYGWIN)
		set_target_properties (sndfile PROPERTIES
			RUNTIME_OUTPUT_NAME "sndfile-1"
			)
	else ()
		set_target_properties (sndfile PROPERTIES
			RUNTIME_OUTPUT_NAME "libsndfile-1"
			)
	endif ()
endif ()

if (BUILD_SHARED_LIBS)

	#
	# ABI version of library.
	#

	#
	# Read libtool version from `configure.ac` and set libsndfile ABI version:
	#
	#   SNDFILE_ABI_VERSION_MAJOR
	#   SNDFILE_ABI_VERSION_MINOR
	#   SNDFILE_ABI_VERSION_PATCH
	#   SNDFILE_ABI_VERSION
	#
	# and Mach-O current and compatibility versions:
	#
	#   SNDFILE_MACHO_CURRENT_VERSION
	#   SNDFILE_MACHO_COMPATIBILITY_VERSION
	#

	include (SetupABIVersions)

	setup_abi_versions()

	if (WIN32)
		set (VERSION_MAJOR ${CPACK_PACKAGE_VERSION_MAJOR})
		set (GEN_TOOL cmake)
		
		set (WIN_RC_VERSION "${CPACK_PACKAGE_VERSION_MAJOR},${CPACK_PACKAGE_VERSION_MINOR},${CPACK_PACKAGE_VERSION_PATCH}")
		set (CLEAN_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
		set (PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})

		configure_file (src/version-metadata.rc.in src/version-metadata.rc @ONLY)
		target_sources (sndfile PRIVATE ${PROJECT_BINARY_DIR}/src/version-metadata.rc)
	endif ()


	set_target_properties (sndfile PROPERTIES
		SOVERSION ${SNDFILE_ABI_VERSION_MAJOR}
		VERSION ${SNDFILE_ABI_VERSION}
		)

	if (APPLE)
		if (NOT (CMAKE_VERSION VERSION_LESS 3.17))
			set_target_properties (sndfile PROPERTIES
				MACHO_CURRENT_VERSION ${SNDFILE_MACHO_CURRENT_VERSION}
				MACHO_COMPATIBILITY_VERSION ${SNDFILE_MACHO_COMPATIBILITY_VERSION}
				)
		else ()
			message (FATAL_ERROR "Apple platform requires cmake >= 3.17 to build dylib.")
		endif ()
	endif ()

	# Symbol files generation

	if (WIN32)
		set (SYMBOL_FILENAME "sndfile.def")
		set (SYMBOL_OS "win32")
	elseif ((CMAKE_SYSTEM_NAME MATCHES "Darwin") OR (CMAKE_SYSTEM_NAME MATCHES "Rhapsody"))
		set (SYMBOL_FILENAME "Symbols.darwin")
		set (SYMBOL_OS "darwin")
	elseif (CMAKE_SYSTEM_NAME MATCHES "OS2")
		set (SYMBOL_FILENAME "Symbols.os2")
		set (SYMBOL_OS "os2")
	elseif (UNIX)
		set (SYMBOL_FILENAME "Symbols.gnu-binutils")
		set (SYMBOL_OS "linux")
	endif ()

	if (DEFINED SYMBOL_OS)
		add_custom_command (
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/src/${SYMBOL_FILENAME}
			COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/create_symbols_file.py ${SYMBOL_OS} ${SNDFILE_ABI_VERSION} > ${CMAKE_CURRENT_BINARY_DIR}/src/${SYMBOL_FILENAME}
			COMMENT "Generating ${SYMBOL_FILENAME}..."
			)

		add_custom_target (GENFILES DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/src/${SYMBOL_FILENAME})
		if (SYMBOL_OS MATCHES "win32")
			target_sources (sndfile
				PRIVATE
					${CMAKE_CURRENT_BINARY_DIR}/src/${SYMBOL_FILENAME}
				)
		elseif (SYMBOL_OS MATCHES "darwin")
			add_dependencies (sndfile GENFILES)
			if (CMAKE_VERSION VERSION_LESS 3.13)
				set_property (TARGET sndfile APPEND_STRING PROPERTY
					LINK_FLAGS "-Wl,-exported_symbols_list -Wl,${CMAKE_CURRENT_BINARY_DIR}/src/${SYMBOL_FILENAME}"
					)
			else ()
				target_link_options (sndfile PRIVATE "LINKER:-exported_symbols_list,${CMAKE_CURRENT_BINARY_DIR}/src/${SYMBOL_FILENAME}")
			endif()
		elseif (SYMBOL_OS MATCHES "os")
			add_dependencies (sndfile GENFILES)
			if (CMAKE_VERSION VERSION_LESS 3.13)
				set_property (TARGET sndfile APPEND_STRING PROPERTY
					LINK_FLAGS "-Wl,-export-symbols ${CMAKE_CURRENT_BINARY_DIR}/src/${SYMBOL_FILENAME}"
					)
			else ()
				target_link_options (sndfile PRIVATE "LINKER:-export-symbols ${CMAKE_CURRENT_BINARY_DIR}/src/${SYMBOL_FILENAME}")
			endif()
		elseif (UNIX)
			add_dependencies (sndfile GENFILES)
			if (CMAKE_VERSION VERSION_LESS 3.13)
				set_property (TARGET sndfile APPEND_STRING PROPERTY
					LINK_FLAGS "-Wl,--version-script,${CMAKE_CURRENT_BINARY_DIR}/src/${SYMBOL_FILENAME}"
					)
			else ()
				target_link_options (sndfile PRIVATE "LINKER:--version-script,${CMAKE_CURRENT_BINARY_DIR}/src/${SYMBOL_FILENAME}")
			endif()
		endif()
	endif()

endif ()

#
# Installation
#

if (ENABLE_PACKAGE_CONFIG)

	if (WIN32 AND (NOT MINGW) AND (NOT CYGWIN))
		set (CMAKE_INSTALL_PACKAGEDIR cmake) 
	 else ()
	 	set (CMAKE_INSTALL_PACKAGEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/SndFile)
	 endif()

	 install (TARGETS ${SNDFILE_PROGRAM_TARGETS}
		RUNTIME DESTINATION			${CMAKE_INSTALL_BINDIR}
 	)

	install (TARGETS sndfile
		EXPORT SndFileTargets
		RUNTIME DESTINATION			${CMAKE_INSTALL_BINDIR}
		ARCHIVE DESTINATION			${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION			${CMAKE_INSTALL_LIBDIR}
		PUBLIC_HEADER DESTINATION	${CMAKE_INSTALL_INCLUDEDIR}
	)

	export (EXPORT SndFileTargets NAMESPACE SndFile:: FILE ${PROJECT_BINARY_DIR}/SndFileTargets.cmake)

	include (CMakePackageConfigHelpers)

	if (ENABLE_EXTERNAL_LIBS)
		set (SndFile_WITH_EXTERNAL_LIBS 1)
		list (APPEND FIND_MODULES_INSTALL_LIST 
			${CMAKE_CURRENT_LIST_DIR}/cmake/FindFLAC.cmake
			${CMAKE_CURRENT_LIST_DIR}/cmake/FindOgg.cmake
			${CMAKE_CURRENT_LIST_DIR}/cmake/FindOpus.cmake
			${CMAKE_CURRENT_LIST_DIR}/cmake/FindVorbis.cmake)
	else ()
		set (SndFile_WITH_EXTERNAL_LIBS 0)
	endif ()

	if(ENABLE_MPEG)
		set (SndFile_WITH_MPEG 1)
		list (APPEND FIND_MODULES_INSTALL_LIST 
			${CMAKE_CURRENT_LIST_DIR}/cmake/Findmpg123.cmake
			${CMAKE_CURRENT_LIST_DIR}/cmake/Findmp3lame.cmake)
	else ()
		set (SndFile_WITH_MPEG 0)
	endif ()

	set (INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})
	configure_package_config_file(cmake/SndFileConfig.cmake.in SndFileConfig.cmake
		INSTALL_DESTINATION ${PROJECT_BINARY_DIR}
		INSTALL_PREFIX ${PROJECT_BINARY_DIR}
		PATH_VARS INCLUDE_INSTALL_DIR
		)
	configure_package_config_file(cmake/SndFileConfig.cmake.in SndFileConfig2.cmake
		INSTALL_DESTINATION ${CMAKE_INSTALL_PACKAGEDIR}
		PATH_VARS INCLUDE_INSTALL_DIR
		)
	write_basic_package_version_file (SndFileConfigVersion.cmake COMPATIBILITY SameMajorVersion)

	install(EXPORT SndFileTargets
		NAMESPACE SndFile::
		DESTINATION ${CMAKE_INSTALL_PACKAGEDIR}
		)
	install(
		FILES ${CMAKE_CURRENT_BINARY_DIR}/SndFileConfig2.cmake
		RENAME SndFileConfig.cmake
		DESTINATION	${CMAKE_INSTALL_PACKAGEDIR}
		)
	install(
		FILES
			${CMAKE_CURRENT_BINARY_DIR}/SndFileConfigVersion.cmake
		DESTINATION	${CMAKE_INSTALL_PACKAGEDIR}
		)

		if (NOT BUILD_SHARED_LIBS AND FIND_MODULES_INSTALL_LIST)
			file(COPY ${FIND_MODULES_INSTALL_LIST} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
			install(FILES ${FIND_MODULES_INSTALL_LIST} DESTINATION ${CMAKE_INSTALL_PACKAGEDIR})
		endif ()
else ()

	install (TARGETS sndfile ${sdnfile_PROGRAMS}
		RUNTIME DESTINATION			${CMAKE_INSTALL_BINDIR}
		ARCHIVE DESTINATION			${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION			${CMAKE_INSTALL_LIBDIR}
		PUBLIC_HEADER DESTINATION	${CMAKE_INSTALL_INCLUDEDIR})

endif ()

if (INSTALL_MANPAGES)

	set (man_MANS
		man/sndfile-info.1
		man/sndfile-play.1
		man/sndfile-convert.1
		man/sndfile-cmp.1
		man/sndfile-metadata-get.1
		man/sndfile-concat.1
		man/sndfile-interleave.1
		man/sndfile-salvage.1
		)
	install (FILES ${man_MANS} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
	install (FILES man/sndfile-metadata-get.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 RENAME sndfile-metadata-set.1)
	install (FILES man/sndfile-interleave.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 RENAME sndfile-deinterleave.1)
endif ()

if (ENABLE_BOW_DOCS)
	set (HTML_BGCOLOUR "white")
	set (HTML_FGCOLOUR "black")
else ()
	set (HTML_BGCOLOUR "black")
	set (HTML_FGCOLOUR "white")
endif ()
set (dist_doc_DATA
	docs/index.md
	docs/libsndfile.jpg
	docs/libsndfile.css
	docs/print.css
	docs/api.md
	docs/command.md
	docs/bugs.md
	docs/formats.md
	docs/sndfile_info.md
	docs/new_file_type_howto.md
	docs/win32.md
	docs/FAQ.md
	docs/lists.md
	docs/embedded_files.md
	docs/octave.md
	docs/tutorial.md
	)
install (FILES ${dist_doc_DATA} DESTINATION ${CMAKE_INSTALL_DOCDIR})

if (INSTALL_PKGCONFIG_MODULE)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/sndfile.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif ()

if (ENABLE_CPACK)
	if ((NOT CPACK_PACKAGE_VERSION) AND CPACK_PACKAGE_VERSION_STAGE)
		set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_FULL}")
	endif ()
	include (CPack)
endif ()
