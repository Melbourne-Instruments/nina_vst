#include "NinaUnitTests.h"

int main() {
    int passes = 0;
    int fails = 0;
    auto start = std::chrono::steady_clock::now();
    // start tests here

    // run_test(test_coarse_pitch(), passes, fails);
    // run_test(simple_poly_note_test(), passes, fails);
    // run_test(voice_efficiency_test(), passes, fails);
    // run_test(load_timbre_voice(), passes, fails);
    // run_test(run_timbre_check_filter(), passes, fails);
    // run_test(run_bunch_of_buffers(), passes, fails);
    // run_test(run_osc_dump_to_file(), passes, fails);
    // run_test(run_osc_dump_to_file(), passes, fails);
    // run_test(run_voice_like_factory_vst(), passes, fails);
    // run_test(load_analog_voices(), passes, fails);
    // run_test(analog_efficiency_test(), passes, fails);
    // run_test(voice_efficiency_test(), passes, fails);
    // run_test(num_test(), passes, fails);
    // run_test(run_layer(), passes, fails);
    // run_test(run_layer_manager(), passes, fails);
    // run_test(simple_poly_note_test(), passes, fails);
    // run_test(simple_mono_note_test(), passes, fails);
    // run_test(velocity_test(), passes, fails);
    // run_test(test_coarse_pitch(), passes, fails);
    // run_test(test_smoothing(), passes, fails);
    // run_test(env_test(), passes, fails);
    // run_test(lfo_test(), passes, fails);
    // run_test(glide_test(), passes, fails);
    // run_test(wt_load_run_test(), passes, fails);
    // run_test(pitchbendTest(), passes, fails);
    // run_test(wt_load_run_test(), passes, fails);
    // run_test(test_od_comp(), passes, fails);
    // run_test(test_analog_osc(), passes, fails);
    // run_test(test_filter(), passes, fails);
    // code_test(11);
    // run_test(lfo_test(), passes, fails);
    // run_test(test_semitone_transpose(), passes, fails);
    // run_test(test_calibration_routine(), passes, fails);
    run_test(run_single_wavetable_save_output(), passes, fails);
    // run_test(wt_alloc_test(), passes, fails);
    //  run_test(filter_gen_test(), passes, fails);

    // run_test(delay_test(), passes, fails);
    // run_test(reverb_test(), passes, fails);
    auto end = std::chrono::steady_clock::now();
    tests_finish(passes, fails, (float)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    return 0;
}
