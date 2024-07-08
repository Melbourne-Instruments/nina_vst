
set(synthia_vst_sources
    source/NinaProcessor.cpp
    source/NinaProcessor.h
    source/NinaController.cpp
    source/NinaController.h
    source/WavetableOsc.cpp
    source/WavetableOsc.h
    source/factory.cpp
    source/version.h
    source/NinaParameters.h
    source/NinaParameters.cpp
    source/GuiMsgQueue.cpp
    source/SynthMath.h
    source/NoiseOscillator.h
    source/common.h
    source/NinaLogger.h
    source/NinaLogger.cpp
    source/EffectsController.cpp
    source/EffectsProcessor.cpp
    source/EffectsController.h
    source/EffectsProcessor.h
    source/OnePoleLP.h

    source/Matrix.h
    source/DsVca.h
    source/AnalogOscGen.h
    source/AnalogOscGen.cpp
    source/AnalogFiltGen.h
    source/AnalogFiltGen.cpp
    source/AnalogVoice.h
    source/AnalogVoice.cpp
    source/NinaCalSequencer.cpp

    source/FactoryTestNinaProcessor.h
    source/FactoryTestNinaProcessor.cpp
    source/FactoryTestNinaController.h
    source/FactoryTestNinaController.cpp
    source/FactorySamplerProcessor.h
    source/FactorySamplerProcessor.cpp
    source/FactorySamplerController.h
    source/FactorySamplerController.cpp
    source/FactorySampleMsgQueue.cpp
    source/FactorySampleMsgQueue.h
    source/NinaEnvelope.h
    source/NinaEnvelope.cpp
    source/NinaLfo.h
    source/NinaLfo.cpp
    source/NinaMatrix.h
    source/NinaMatrix.cpp
    source/NinaOutputPanner.h
    source/NinaOutputPanner.cpp
    source/NinaVoice.h
    source/NinaVoice.cpp
    source/NinaOscMix.h
    source/NinaXorMix.h
    source/LayerManager.h
    source/LayerManager.cpp
    source/Layer.h
    source/Layer.cpp
    source/NinaDriveCompensator.h
    source/NinaCalSequencer.h
    source/NinaReverb.h
    source/NinaDelay.h
    source/NinaExpander.h
    source/NinaReverb.cpp
    source/freeverb/allpass.cpp
    source/freeverb/irmodel3p.cpp
    source/freeverb/allpass.hpp
    source/freeverb/irmodel3p.hpp
    source/freeverb/allpass_t.hpp
    source/freeverb/irmodel3p_t.hpp
    source/freeverb/irmodel3_t.hpp
    source/freeverb/biquad.cpp

    # source/freeverb/irmodel3w.cpp
    source/freeverb/biquad.hpp

    # source/freeverb/irmodel3w.hpp
    source/freeverb/biquad_t.hpp
    source/freeverb/irmodel3w_t.hpp
    source/freeverb/blockDelay.cpp
    source/freeverb/irmodels.cpp
    source/freeverb/blockDelay.hpp
    source/freeverb/irmodels.hpp
    source/freeverb/blockDelay_t.hpp
    source/freeverb/irmodels_t.hpp
    source/freeverb/limitmodel.cpp
    source/freeverb/comb.cpp
    source/freeverb/limitmodel.hpp
    source/freeverb/comb.hpp
    source/freeverb/limitmodel_t.hpp
    source/freeverb/comb_t.hpp
    source/freeverb/mls.cpp
    source/freeverb/compmodel.cpp
    source/freeverb/mls.hpp
    source/freeverb/compmodel.hpp
    source/freeverb/mls_t.hpp
    source/freeverb/compmodel_t.hpp
    source/freeverb/nrevb.cpp
    source/freeverb/nrevb.hpp
    source/freeverb/delay.cpp
    source/freeverb/nrevb_t.hpp
    source/freeverb/delay.hpp
    source/freeverb/nrev.cpp
    source/freeverb/delayline.cpp
    source/freeverb/nrev.hpp
    source/freeverb/delayline.hpp
    source/freeverb/nrev_t.hpp
    source/freeverb/delayline_t.hpp
    source/freeverb/progenitor2.cpp
    source/freeverb/delay_t.hpp
    source/freeverb/progenitor2.hpp
    source/freeverb/dl_gardner.cpp
    source/freeverb/progenitor2_t.hpp
    source/freeverb/dl_gardner.hpp
    source/freeverb/progenitor.cpp
    source/freeverb/dl_gardner_t.hpp
    source/freeverb/progenitor.hpp
    source/freeverb/earlyref.cpp
    source/freeverb/progenitor_t.hpp
    source/freeverb/earlyref.hpp
    source/freeverb/earlyref_t.hpp
    source/freeverb/revbase.cpp
    source/freeverb/efilter.cpp
    source/freeverb/revbase.hpp
    source/freeverb/efilter.hpp
    source/freeverb/revbase_t.hpp
    source/freeverb/efilter_t.hpp

    # source/freeverb/revmodel.cpp
    source/freeverb/fir3bandsplit.cpp

    # source/freeverb/revmodel.hpp
    source/freeverb/fir3bandsplit.hpp

    # source/freeverb/revmodel_t.hpp
    source/freeverb/fir3bandsplit_t.hpp
    source/freeverb/rms.cpp
    source/freeverb/firfilter.cpp
    source/freeverb/rms.hpp
    source/freeverb/firfilter.hpp
    source/freeverb/rms_t.hpp
    source/freeverb/firfilter_t.hpp
    source/freeverb/scomp.cpp
    source/freeverb/firwindow.cpp
    source/freeverb/scomp.hpp
    source/freeverb/firwindow.hpp
    source/freeverb/scomp_t.hpp
    source/freeverb/firwindow_t.hpp
    source/freeverb/slimit.cpp

    # source/freeverb/frag.cpp
    source/freeverb/slimit.hpp

    # source/freeverb/frag.hpp
    source/freeverb/slimit_t.hpp
    source/freeverb/frag_t.hpp
    source/freeverb/slot.cpp
    source/freeverb/fv3_ch_tool.hpp
    source/freeverb/slot.hpp
    source/freeverb/fv3_defs.h
    source/freeverb/slot_t.hpp
    source/freeverb/fv3_ns_end.h
    source/freeverb/stenh.cpp
    source/freeverb/fv3_ns_start.h
    source/freeverb/stenh.hpp
    source/freeverb/fv3_pthread_tool.hpp
    source/freeverb/stenh_t.hpp
    source/freeverb/fv3_type_float.h
    source/freeverb/strev.cpp
    source/freeverb/irbase.cpp
    source/freeverb/strev.hpp
    source/freeverb/irbase.hpp
    source/freeverb/strev_t.hpp
    source/freeverb/irbase_t.hpp
    source/freeverb/sweep.cpp
    source/freeverb/irmodel1.cpp
    source/freeverb/sweep.hpp
    source/freeverb/irmodel1.hpp
    source/freeverb/sweep_t.hpp
    source/freeverb/irmodel1_t.hpp
    source/freeverb/utils.cpp
    source/freeverb/irmodel2.cpp
    source/freeverb/utils.hpp
    source/freeverb/irmodel2.hpp
    source/freeverb/utils_t.hpp
    source/freeverb/irmodel2_t.hpp
    source/freeverb/zrev2.cpp
    source/freeverb/irmodel2zl.cpp
    source/freeverb/zrev2.hpp
    source/freeverb/irmodel2zl.hpp
    source/freeverb/zrev2_t.hpp
    source/freeverb/irmodel2zl_t.hpp
    source/freeverb/zrev.cpp
    source/freeverb/irmodel3.cpp
    source/freeverb/zrev.hpp
    source/freeverb/irmodel3.hpp
    source/freeverb/zrev_t.hpp
    source/NinaEffectsLimiter.h
)

set(nina_unit_testing_sources
    source/NinaParameters.h
    source/NinaParameters.cpp
    source/NinaParameters.h
    source/NinaParameters.cpp
    source/NinaEffectsLimiter.h
    source/NinaDelay.h
    source/NinaExpander.h
    source/AnalogVoice.h
    source/AnalogVoice.cpp
    source/LayerManager.h
    source/LayerManager.cpp
    source/Layer.h
    source/Layer.cpp
    source/NinaUnitTests.h
    source/NinaOscMix.h
    source/NinaUnitTests.cpp
    source/NinaEnvelope.h
    source/NinaEnvelope.cpp
    source/NinaLfo.h
    source/NinaLfo.cpp
    source/NinaMatrix.h
    source/NinaMatrix.cpp
    source/NinaOutputPanner.h
    source/NinaOutputPanner.cpp
    source/NinaVoice.h
    source/NinaVoice.cpp
    source/Matrix.h
    source/DsVca.h
    source/NinaXorMix.h
    source/WavetableOsc.h
    source/NinaDriveCompensator.h
    source/WavetableOsc.cpp
    source/NinaCalSequencer.h
    source/NinaCalSequencer.cpp
    source/AnalogOscGen.cpp
    source/AnalogOscGen.h
    source/AnalogFiltGen.h
    source/AnalogFiltGen.cpp
    source/NinaReverb.h
    source/NinaDelay.h
    source/NinaReverb.cpp
    source/freeverb/allpass.cpp
    source/freeverb/irmodel3p.cpp
    source/freeverb/allpass.hpp
    source/freeverb/irmodel3p.hpp
    source/freeverb/allpass_t.hpp
    source/freeverb/irmodel3p_t.hpp
    source/freeverb/irmodel3_t.hpp
    source/freeverb/biquad.cpp

    # source/freeverb/irmodel3w.cpp
    source/freeverb/biquad.hpp

    # source/freeverb/irmodel3w.hpp
    source/freeverb/biquad_t.hpp
    source/freeverb/irmodel3w_t.hpp
    source/freeverb/blockDelay.cpp
    source/freeverb/irmodels.cpp
    source/freeverb/blockDelay.hpp
    source/freeverb/irmodels.hpp
    source/freeverb/blockDelay_t.hpp
    source/freeverb/irmodels_t.hpp
    source/freeverb/limitmodel.cpp
    source/freeverb/comb.cpp
    source/freeverb/limitmodel.hpp
    source/freeverb/comb.hpp
    source/freeverb/limitmodel_t.hpp
    source/freeverb/comb_t.hpp
    source/freeverb/mls.cpp
    source/freeverb/compmodel.cpp
    source/freeverb/mls.hpp
    source/freeverb/compmodel.hpp
    source/freeverb/mls_t.hpp
    source/freeverb/compmodel_t.hpp
    source/freeverb/nrevb.cpp
    source/freeverb/nrevb.hpp
    source/freeverb/delay.cpp
    source/freeverb/nrevb_t.hpp
    source/freeverb/delay.hpp
    source/freeverb/nrev.cpp
    source/freeverb/delayline.cpp
    source/freeverb/nrev.hpp
    source/freeverb/delayline.hpp
    source/freeverb/nrev_t.hpp
    source/freeverb/delayline_t.hpp
    source/freeverb/progenitor2.cpp
    source/freeverb/delay_t.hpp
    source/freeverb/progenitor2.hpp
    source/freeverb/dl_gardner.cpp
    source/freeverb/progenitor2_t.hpp
    source/freeverb/dl_gardner.hpp
    source/freeverb/progenitor.cpp
    source/freeverb/dl_gardner_t.hpp
    source/freeverb/progenitor.hpp
    source/freeverb/earlyref.cpp
    source/freeverb/progenitor_t.hpp
    source/freeverb/earlyref.hpp
    source/freeverb/earlyref_t.hpp
    source/freeverb/revbase.cpp
    source/freeverb/efilter.cpp
    source/freeverb/revbase.hpp
    source/freeverb/efilter.hpp
    source/freeverb/revbase_t.hpp
    source/freeverb/efilter_t.hpp

    # source/freeverb/revmodel.cpp
    source/freeverb/fir3bandsplit.cpp

    # source/freeverb/revmodel.hpp
    source/freeverb/fir3bandsplit.hpp

    # source/freeverb/revmodel_t.hpp
    source/freeverb/fir3bandsplit_t.hpp
    source/freeverb/rms.cpp
    source/freeverb/firfilter.cpp
    source/freeverb/rms.hpp
    source/freeverb/firfilter.hpp
    source/freeverb/rms_t.hpp
    source/freeverb/firfilter_t.hpp
    source/freeverb/scomp.cpp
    source/freeverb/firwindow.cpp
    source/freeverb/scomp.hpp
    source/freeverb/firwindow.hpp
    source/freeverb/scomp_t.hpp
    source/freeverb/firwindow_t.hpp
    source/freeverb/slimit.cpp

    # source/freeverb/frag.cpp
    source/freeverb/slimit.hpp

    # source/freeverb/frag.hpp
    source/freeverb/slimit_t.hpp
    source/freeverb/frag_t.hpp
    source/freeverb/slot.cpp
    source/freeverb/fv3_ch_tool.hpp
    source/freeverb/slot.hpp
    source/freeverb/fv3_defs.h
    source/freeverb/slot_t.hpp
    source/freeverb/fv3_ns_end.h
    source/freeverb/stenh.cpp
    source/freeverb/fv3_ns_start.h
    source/freeverb/stenh.hpp
    source/freeverb/fv3_pthread_tool.hpp
    source/freeverb/stenh_t.hpp
    source/freeverb/fv3_type_float.h
    source/freeverb/strev.cpp
    source/freeverb/irbase.cpp
    source/freeverb/strev.hpp
    source/freeverb/irbase.hpp
    source/freeverb/strev_t.hpp
    source/freeverb/irbase_t.hpp
    source/freeverb/sweep.cpp
    source/freeverb/irmodel1.cpp
    source/freeverb/sweep.hpp
    source/freeverb/irmodel1.hpp
    source/freeverb/sweep_t.hpp
    source/freeverb/irmodel1_t.hpp
    source/freeverb/utils.cpp
    source/freeverb/irmodel2.cpp
    source/freeverb/utils.hpp
    source/freeverb/irmodel2.hpp
    source/freeverb/utils_t.hpp
    source/freeverb/irmodel2_t.hpp
    source/freeverb/zrev2.cpp
    source/freeverb/irmodel2zl.cpp
    source/freeverb/zrev2.hpp
    source/freeverb/irmodel2zl.hpp
    source/freeverb/zrev2_t.hpp
    source/freeverb/irmodel2zl_t.hpp
    source/freeverb/zrev.cpp
    source/freeverb/irmodel3.cpp
    source/freeverb/zrev.hpp
    source/freeverb/irmodel3.hpp
    source/freeverb/zrev_t.hpp
    source/NinaEffectsLimiter.h
    source/NinaReverb.h
    source/NinaDelay.h
    source/NinaExpander.h
    source/NinaReverb.cpp
    source/freeverb/allpass.cpp
    source/freeverb/irmodel3p.cpp
    source/freeverb/allpass.hpp
    source/freeverb/irmodel3p.hpp
    source/freeverb/allpass_t.hpp
    source/freeverb/irmodel3p_t.hpp
    source/freeverb/irmodel3_t.hpp
    source/freeverb/biquad.cpp

    # source/freeverb/irmodel3w.cpp
    source/freeverb/biquad.hpp

    # source/freeverb/irmodel3w.hpp
    source/freeverb/biquad_t.hpp
    source/freeverb/irmodel3w_t.hpp
    source/freeverb/blockDelay.cpp
    source/freeverb/irmodels.cpp
    source/freeverb/blockDelay.hpp
    source/freeverb/irmodels.hpp
    source/freeverb/blockDelay_t.hpp
    source/freeverb/irmodels_t.hpp
    source/freeverb/limitmodel.cpp
    source/freeverb/comb.cpp
    source/freeverb/limitmodel.hpp
    source/freeverb/comb.hpp
    source/freeverb/limitmodel_t.hpp
    source/freeverb/comb_t.hpp
    source/freeverb/mls.cpp
    source/freeverb/compmodel.cpp
    source/freeverb/mls.hpp
    source/freeverb/compmodel.hpp
    source/freeverb/mls_t.hpp
    source/freeverb/compmodel_t.hpp
    source/freeverb/nrevb.cpp
    source/freeverb/nrevb.hpp
    source/freeverb/delay.cpp
    source/freeverb/nrevb_t.hpp
    source/freeverb/delay.hpp
    source/freeverb/nrev.cpp
    source/freeverb/delayline.cpp
    source/freeverb/nrev.hpp
    source/freeverb/delayline.hpp
    source/freeverb/nrev_t.hpp
    source/freeverb/delayline_t.hpp
    source/freeverb/progenitor2.cpp
    source/freeverb/delay_t.hpp
    source/freeverb/progenitor2.hpp
    source/freeverb/dl_gardner.cpp
    source/freeverb/progenitor2_t.hpp
    source/freeverb/dl_gardner.hpp
    source/freeverb/progenitor.cpp
    source/freeverb/dl_gardner_t.hpp
    source/freeverb/progenitor.hpp
    source/freeverb/earlyref.cpp
    source/freeverb/progenitor_t.hpp
    source/freeverb/earlyref.hpp
    source/freeverb/earlyref_t.hpp
    source/freeverb/revbase.cpp
    source/freeverb/efilter.cpp
    source/freeverb/revbase.hpp
    source/freeverb/efilter.hpp
    source/freeverb/revbase_t.hpp
    source/freeverb/efilter_t.hpp

    # source/freeverb/revmodel.cpp
    source/freeverb/fir3bandsplit.cpp

    # source/freeverb/revmodel.hpp
    source/freeverb/fir3bandsplit.hpp

    # source/freeverb/revmodel_t.hpp
    source/freeverb/fir3bandsplit_t.hpp
    source/freeverb/rms.cpp
    source/freeverb/firfilter.cpp
    source/freeverb/rms.hpp
    source/freeverb/firfilter.hpp
    source/freeverb/rms_t.hpp
    source/freeverb/firfilter_t.hpp
    source/freeverb/scomp.cpp
    source/freeverb/firwindow.cpp
    source/freeverb/scomp.hpp
    source/freeverb/firwindow.hpp
    source/freeverb/scomp_t.hpp
    source/freeverb/firwindow_t.hpp
    source/freeverb/slimit.cpp

    # source/freeverb/frag.cpp
    source/freeverb/slimit.hpp

    # source/freeverb/frag.hpp
    source/freeverb/slimit_t.hpp
    source/freeverb/frag_t.hpp
    source/freeverb/slot.cpp
    source/freeverb/fv3_ch_tool.hpp
    source/freeverb/slot.hpp
    source/freeverb/fv3_defs.h
    source/freeverb/slot_t.hpp
    source/freeverb/fv3_ns_end.h
    source/freeverb/stenh.cpp
    source/freeverb/fv3_ns_start.h
    source/freeverb/stenh.hpp
    source/freeverb/fv3_pthread_tool.hpp
    source/freeverb/stenh_t.hpp
    source/freeverb/fv3_type_float.h
    source/freeverb/strev.cpp
    source/freeverb/irbase.cpp
    source/freeverb/strev.hpp
    source/freeverb/irbase.hpp
    source/freeverb/strev_t.hpp
    source/freeverb/irbase_t.hpp
    source/freeverb/sweep.cpp
    source/freeverb/irmodel1.cpp
    source/freeverb/sweep.hpp
    source/freeverb/irmodel1.hpp
    source/freeverb/sweep_t.hpp
    source/freeverb/irmodel1_t.hpp
    source/freeverb/utils.cpp
    source/freeverb/irmodel2.cpp
    source/freeverb/utils.hpp
    source/freeverb/irmodel2.hpp
    source/freeverb/utils_t.hpp
    source/freeverb/irmodel2_t.hpp
    source/freeverb/zrev2.cpp
    source/freeverb/irmodel2zl.cpp
    source/freeverb/zrev2.hpp
    source/freeverb/irmodel2zl.hpp
    source/freeverb/zrev2_t.hpp
    source/freeverb/irmodel2zl_t.hpp
    source/freeverb/zrev.cpp
    source/freeverb/irmodel3.cpp
    source/freeverb/zrev.hpp
    source/freeverb/irmodel3.hpp
    source/freeverb/zrev_t.hpp
    source/NinaEffectsLimiter.h
)

set(INCLUDE_DIRS
    "${PROJECT_SOURCE_DIR}/submodules/AudioFile"
    "${PROJECT_SOURCE_DIR}/synthia/source/freeverb"
    "${PROJECT_SOURCE_DIR}/thirdparty/rapidjson/include")

set(target synthia_vst)

smtg_add_vst3plugin(${target} ${synthia_vst_sources})

smtg_get_linux_architecture_name() # Sets var LINUX_ARCHITECTURE_NAME

find_package(GSL REQUIRED)

if((LINUX_ARCHITECTURE_NAME STREQUAL "aarch64-linux"))
    message("set arm compile options")
    target_compile_options(synthia_vst PRIVATE -mtune=cortex-a72
        -mcpu=cortex-a72
        -funsafe-math-optimizations -std=c++20 -fno-rtti -fsched2-use-superblocks -funroll-loops -fuse-linker-plugin -fno-math-errno -fsignaling-nans -ftree-vectorize -flto -Ofast -ffast-math --param inline-unit-growth=20000)

# ### Enable Profile Optimisation ####
# target_compile_options(synthia_vst PRIVATE -fprofile-use=${PROJECT_SOURCE_DIR}/pgo )

# ### Enable Profile Generation ####
# target_compile_options(synthia_vst PRIVATE -mtune=cortex-a72 -funsafe-math-optimizations -std=c++20 -fprofile-generate=/home/root/pgo -flto -Ofast -ffast-math)
else()
    message("set 86 compile options")
    target_compile_options(synthia_vst PRIVATE -funsafe-math-optimizations -std=c++20 -O0)
endif()

target_include_directories(${target} PRIVATE ${INCLUDE_DIRS})
target_link_libraries(${target} PRIVATE base -Bstatic sdk fftw3 -lgcov --coverage AudioFile rt -mtune=cortex-a72 -std=c++20 -mcpu=cortex-a72 -funsafe-math-optimizations -fno-rtti -fsched2-use-superblocks -funroll-loops -fuse-linker-plugin -fno-math-errno -fsignaling-nans -ftree-vectorize -flto -Ofast -ffast-math --param=inline-unit-growth=20000)
smtg_dump_plugin_package_variables(${target})

# Copy the python scripts to the vst directory
add_custom_command(TARGET synthia_vst POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/melb_inst_vst3/synthia/source/osc_tuning_routine.py ${CMAKE_BINARY_DIR}/VST3/Debug/synthia_vst.vst3/Contents/)
add_custom_command(TARGET synthia_vst POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/melb_inst_vst3/synthia/source/filter_tuning_routine.py ${CMAKE_BINARY_DIR}/VST3/Debug/synthia_vst.vst3/Contents/)

add_custom_command(TARGET synthia_vst POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/melb_inst_vst3/synthia/source/RunCalibration.py ${CMAKE_BINARY_DIR}/VST3/Debug/synthia_vst.vst3/Contents/)

# Generate the vst version number and write it to the version.txt file
execute_process(
    COMMAND git log -1 --format=%H
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(SUBSTRING "${GIT_COMMIT_HASH}" 0 8 GIT_SHORT_HASH)
message("${GIT_SHORT_HASH}")
set(VST_X_VER "1")
set(VST_Y_VER "4")
set(VST_Z_VER "0")
configure_file(
    ${CMAKE_SOURCE_DIR}/version.txt.in
    ${CMAKE_BINARY_DIR}/version.txt
)
configure_file(
    ${CMAKE_SOURCE_DIR}/version.h.in
    ${CMAKE_SOURCE_DIR}/melb_inst_vst3/synthia/source/version.h
)

# copy the version.txt to the vst file
add_custom_command(TARGET synthia_vst POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/version.txt ${CMAKE_BINARY_DIR}/VST3/Debug/synthia_vst.vst3/Contents/)
file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/melb_inst_vst3/synthia/source/*.cpp
    ${PROJECT_SOURCE_DIR}/melb_inst_vst3/synthia/source/*.h
    ${PROJECT_SOURCE_DIR}/melb_inst_vst3/synthia/source/freeverb/*.h
    ${PROJECT_SOURCE_DIR}/melb_inst_vst3/synthia/source/freeverb/*.hpp
    ${PROJECT_SOURCE_DIR}/melb_inst_vst3/synthia/source/freeverb/*.cpp
)

add_custom_target(
    format
    COMMAND /usr/bin/clang-format-14
    -i
    -style=file
    ${ALL_CXX_SOURCE_FILES}
)

# make the x86 and ARM vst tests
message("make unit tests?")
message(STATUS "${LINUX_ARCHITECTURE_NAME}")

if(NOT(LINUX_ARCHITECTURE_NAME STREQUAL "aarch64-linux"))
    message("make and run native tests")
    add_executable(unitTesting ${nina_unit_testing_sources})

    target_compile_options(unitTesting PUBLIC -D_NINA_UNIT_TESTS -fsanitize=address -flto -Og)
    target_include_directories(unitTesting PRIVATE ${INCLUDE_DIRS})
    target_link_libraries(unitTesting PUBLIC base sdk AudioFile rt fftw3 -flto -fsanitize=address)
    add_custom_target(run_unit_tests
        COMMAND unitTesting
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}

        COMMENT "run generated tests in ${CMAKE_CURRENT_BINARY_DIR}"
    )
else()
    message("make aarch64 tests")
    add_executable(unitTesting ${nina_unit_testing_sources})

    target_compile_options(unitTesting PUBLIC -D_NINA_UNIT_TESTS -mtune=cortex-a72 -march=armv8-a -flto -Og)

    target_link_libraries(unitTesting PUBLIC base sdk AudioFile rt fftw3 -march=armv8-a -mtune=cortex-a72 -Og -funsafe-math-optimizations -flto -Wno-missing-profile)

    add_custom_command(TARGET synthia_vst POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/melb_inst_vst3/synthia/source/osc_tuning_routine.py ${CMAKE_BINARY_DIR}/VST3/Debug/synthia_vst.vst3/Contents/)
    add_custom_command(TARGET synthia_vst POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/melb_inst_vst3/synthia/source/filter_tuning_routine.py ${CMAKE_BINARY_DIR}/VST3/Debug/synthia_vst.vst3/Contents/)
endif()