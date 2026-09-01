# The `v0.0.5-behavior1k` branch

The nvblox that `StanfordVL/curobo@78612f45` builds against, plus the minimum edits needed to
compile it on a modern CUDA toolchain. Branched 2026-09-01.

## Where the code comes from

cuRobo's dockerfiles (`docker/x86.dockerfile:113`, `docker/aarch64.dockerfile:158`,
`docker/isaac_sim.dockerfile:229`) do a bare `git clone https://github.com/valtsblukis/nvblox.git`
— no `-b`, no sha. The "pin" is whichever commit that fork's default branch happened to be at
build time. It has been `7bda93e` ("minor fix to clear layer") since 2024-01-30, so in practice
that is the version, but nothing in cuRobo enforces it.

`valtsblukis/nvblox` is a fork of this repo. Its `public` branch leaves ours at `7c76f90`, which is
tag `v0.0.5` exactly, and adds nine commits:

```
7bda93e minor fix to clear layer
ebd7b7d Merge pull request #3 from valtsblukis/bala/minor_fix
b195d2d small fix
d1e6efc Upgrade to nvblox 0.0.5
65ffd3a remove unsupported benchmark
8544cb3 minor fix with buffer capacity
fe633bf Merge branch 'nvidia-isaac-public' into bala/upgrade_0.0.5
e52caa3 Merge branch 'public' of github.com:nvidia-isaac/nvblox into nvidia-isaac-public
f3934f4 Additional accomodations for linking against PyTorch with Pre-CXX11 ABI.
        Change default TSDF site distance.
```

The substantive one is `f3934f4`: `PRE_CXX11_ABI_LINKABLE`, which is what lets the library link
against a PyTorch built with the old ABI. cuRobo passes `-DPRE_CXX11_ABI_LINKABLE=ON`.

The branch name says `v0.0.5` because that is the tag it descends from. `nvblox/CMakeLists.txt`
declares `project(nvblox VERSION 0.0.4)`, but so does tag `v0.0.5` — upstream never bumped the
CMake version at that release, so the string is not evidence of anything.

The matching Python wrapper is `NVlabs/nvblox_torch` at `e121121` (`main`, 2024-07-10, also
unpinned by the dockerfile and also unchanged since). cuRobo calls `Mapper(voxel_sizes=,
integrator_types=, free_on_destruction=, cuda_device_id=)`, `update_hashmaps`, `update_mesh`,
`get_mesh`, `decay_occupancy`, `query_sphere_sdf_cost` and `query_sphere_trajectory_sdf_cost`.
All of those exist there. None of them exists in the in-tree `nvblox_torch` of `v0.0.9`, which
renamed or dropped every one — later nvblox releases are not a drop-in for cuRobo, even though
they build on modern CUDA with none of the patches below.

## What this branch changes on top of `7bda93e`

Build system only, no behavior change. Everything needed to compile against CUDA 12.8 is now
committed here, including the two fixes that land inside stdgpu itself.

`nvblox/thirdparty/stdgpu/`:

* `stdgpu.cmake` — `GIT_TAG` moves to `71a5aef2`, the pin nvblox `v0.0.9` uses, because the 2021
  stdgpu that 0.0.4 pins does not compile against thrust 2.7 (`allocator_traits::construct` fails
  on a host/device `forward` mismatch). `STDGPU_BUILD_BENCHMARKS` is forced off — the newer stdgpu
  builds benchmarks by default and they do not get thrust's include path. A `PATCH_COMMAND` applies
  the two patches below to the fetched tree.
* `apply_patches.cmake` — the `PATCH_COMMAND` itself. Skips a patch that already applies in
  reverse, so a re-populate is safe.
* `stdgpu_thrust_version_regex.patch` — CUDA 12.8's `thrust/version.h` puts a trailing comment on
  the `#define`, so stdgpu's `Findthrust.cmake` regex hands cmake `200700 // ...` and `math()`
  fails. Strips everything after the digits.
* `stdgpu_fix_cuda12_6.patch` — `to_address`, `destroy_at`, `construct_at` and `forward` go
  ambiguous against CUDA 12's own; qualifies them with `stdgpu::`. Same fix upstream carries under
  this name in a later release.

`nvblox/CMakeLists.txt`: five `FILE_SET stdgpu_*` clauses on the `install(TARGETS ...)`. The newer
stdgpu exports its headers as file sets, which 0.0.4's install rule does not know about.

## What still has to be patched outside this repo

One thing, in `nvblox_torch`, in `src/nvblox_torch/cpp/CMakeLists.txt`, before
`find_package(Torch)`: CUDA 12.8 dropped the `CUDA::nvToolsExt` target that torch's
`Caffe2/public/cuda.cmake` still asks for, so declare it as a header-only interface target
pointing at nvtx3's includes. Carried on `chahyon-ku/nvblox_torch`, branch `main-behavior1k`.

## The toolchain

The 2023 code does not build against system CUDA 13.0, and the torch it links against is cu128, so
the build uses a `cu128` conda env (nvcc 12.8, cmake 3.26, gcc 11) and its libraries. No sudo
anywhere.

```
mamba install -n cu128 -c conda-forge glog=0.6.0 gflags sqlite gtest benchmark
```

**glog must be 0.6.** glog 0.7 fails to compile in nvblox's includes: `GLOG_EXPORT` is undefined
without `GLOG_USE_GLOG_EXPORT`, and `MakeCheckOpValueString` no longer resolves.

## The flags that are not optional

With `CONDA=~/miniforge3/envs/cu128`, an install prefix `PREFIX`, and
`NVTX_INC=$CONDA/nsight-compute-2025.1.1/host/target-linux-x64/nvtx/include/nvtx3`:

* `-DCMAKE_CUDA_ARCHITECTURES=<arch>`. 89 for the RTX 4090 Laptop this was built on.
* `-I$NVTX_INC` and `-DCUDA_nvToolsExt_LIBRARY=` — CUDA 12 has no `nvToolsExt.h` at the top level
  and no library to link; nvtx3 is header only.
* `-DTHRUST_INCLUDE_DIR=$CONDA/targets/x86_64-linux/include`.
* `-L` and `-Wl,-rpath-link` for `$CONDA/lib` and `$CONDA/targets/x86_64-linux/lib`, or the
  executables fail to link against libsqlite3 and the npp libraries.
* `-DPRE_CXX11_ABI_LINKABLE=ON`, as cuRobo's dockerfile does. Linking still works against a torch
  reporting `_GLIBCXX_USE_CXX11_ABI = True`.
* `-DCMAKE_INSTALL_RPATH="$CONDA/lib;$CONDA/targets/x86_64-linux/lib;$PREFIX/lib"` with
  `-DCMAKE_BUILD_WITH_INSTALL_RPATH=ON`, or nothing loads outside the build tree.

`make install` lands `bin`, `lib`, `include` and `share/nvblox/cmake` under `$PREFIX`, which is
what nvblox_torch's build then points `-DCMAKE_PREFIX_PATH` at.

## Sanity check

Integrate a flat 2 m depth image, render from the same pose, read 2.0 m back. A ray that hits
nothing returns **-1.0**, not 0.

On an RTX 4090 Laptop, one 320x320 depth frame at a time at 5 cm voxels, through nvblox_torch:
integration 2.8 ms, sphere-traced prediction 2.0 ms, the first call of each an order slower while
the kernels load.
