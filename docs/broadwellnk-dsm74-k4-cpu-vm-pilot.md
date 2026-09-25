# broadwellnk DSM 7.4 / kernel 4.4.302 diagnostic pilot

This is **not a general K4 release**. The pilot is built with Synology's DSM
7.4 broadwellnk toolkit and enables `SYNO_AMDGPU_PILOT_CPU_VM_UPDATE`, which
sets `amdgpu.vm_update_mode=1` (CPU graphics-VM page-table updates). It also
enables `SYNO_DRM_SCHED_DIAG` for scheduler-close logging.

## Reproduction and findings

On the broadwellnk test NAS at 192.168.45.17, a one-shot open/close of
`/dev/dri/renderD128` hung in `drm_sched_entity_flush`. The diagnostic build
logged `list_empty=0 queued=1` before that wait. With CPU graphics-VM updates,
it logged `list_empty=1 queued=0` and returned from the flush.

The next close step exposed an independent K4 compatibility bug:
`amdgpu_pasid_free_delayed` calls `dma_resv_get_fences_rcu` with a NULL
exclusive-fence output. The old `reservation_object_get_fences_rcu` dereferences
that output unconditionally. The compatibility wrapper now provides a real
output pointer and, when the caller passed NULL, appends the exclusive fence to
the returned array as the newer API requires.

The pilot built from commit `2ac5de0` (GitHub Actions run `36159498975`) was
packed as **test-only** SPK `0.5.2-4` and installed. After a clean reboot,
all three one-shot probes completed: DRM open/close, libdrm AMDGPU
initialize/deinitialize, and GPU-info query/close (ASIC ID `0x1636`). No new
Oops or D-state process appeared in those tests.

This establishes only the minimal close path. It does **not** establish
`amdgpu_top`, VA-API, transcoding, long-running load, or other K4 platform
compatibility. Keep the CPU-VM override in the isolated pilot until those
paths are tested separately; do not promote the test SPK as a release.
