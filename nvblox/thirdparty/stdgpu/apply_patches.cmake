# Applied by stdgpu.cmake's FetchContent PATCH_COMMAND, because both fixes land inside the stdgpu
# that FetchContent pulls and so cannot be committed to this repository directly.
#
# Re-running is safe: a patch that already applies in reverse is already in the tree and is skipped.
# Invoked as `cmake -DGIT_EXECUTABLE= -DSRC_DIR= -DPATCH_1= -DPATCH_2= -P apply_patches.cmake`.

foreach(patch "${PATCH_1}" "${PATCH_2}")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" apply --reverse --check "${patch}"
        WORKING_DIRECTORY "${SRC_DIR}"
        RESULT_VARIABLE reverse_applies
        OUTPUT_QUIET ERROR_QUIET
    )
    if(reverse_applies EQUAL 0)
        message(STATUS "stdgpu: already patched, skipping ${patch}")
    else()
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" apply "${patch}"
            WORKING_DIRECTORY "${SRC_DIR}"
            RESULT_VARIABLE apply_failed
        )
        if(NOT apply_failed EQUAL 0)
            message(FATAL_ERROR "stdgpu: failed to apply ${patch}")
        endif()
        message(STATUS "stdgpu: applied ${patch}")
    endif()
endforeach()
