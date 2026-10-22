# Dual DRM backport build instructions

## Scope and acceptance target

This repository builds the combined AMDGPU + i915 DRM kernel modules for Synology DSM platforms. The target for the current kernel-4 test line is `4.4.302`.

The functional acceptance target is AMD hardware transcoding **inside a Docker container on the DSM host**. Do not treat native Plex/Jellyfin playback or transcoding on kernel 4.4 DSM as a required success case, and do not claim that a successful module build alone proves hardware transcoding works.

Docker changes the userspace environment, not the DSM host kernel. The host must still boot a compatible `amdgpu.ko`, load matching firmware, and expose a working `/dev/dri/renderD*` node. The container must receive the render node with usable permissions and have a compatible AMD VA-API userspace driver and FFmpeg build. Validate host module/device readiness and container transcoding separately.

## All-platform DSM 7.2/7.3/7.4 kernel-4.4.302 build

Dispatch this exact workflow for the full all-platform run:

```text
.github/workflows/step6-dual-drm-all-platforms-44302.yml
Workflow: [STEP6] Quad DRM KMS 빌드 (ALL platforms 4.4.302, DSM 7.2/7.3/7.4)
```

This workflow has no inputs: dispatch builds ten kernel-4 platforms across DSM 7.2, 7.3, and 7.4 (30 matrix jobs). Do not substitute the separate DSM-7.4-only STEP6 workflow unless the user explicitly changes the requested scope.

The build jobs download each platform/DSM toolchain and kernel source, compile the combined modules, and upload per-target artifacts. The single-writer `push` job collects those artifacts and updates the `module-output` branch. The build now fails on a nonzero `make` result or unresolved MODPOST symbol. After the run completes, inspect the run conclusion, every matrix job, uploaded artifacts, and the `module-output` commit. A green run summary alone does not establish a successful hardware-transcoding test.

## Temporary repository visibility sequence

For a build requiring the repository to be public during GitHub Actions execution, use this exact order on **this repository**:

1. Immediately before dispatch, verify the current visibility. If private, change it to **public** and verify the public state before starting the run.
2. Dispatch the full workflow above with both inputs blank and monitor the exact run to a terminal state.
3. Whether the run succeeds or fails, after it is terminal change the repository back to **private** and verify the private state. Report the workflow outcome and the visibility restoration only after confirming it.

If workflow dispatch itself fails, do not leave the repository public: restore and verify private visibility. Do not change a different repository's visibility as a substitute.

Visibility changes require owner/admin authority; do not assume the default `GITHUB_TOKEN` can perform them. If automating the transition, use a narrowly scoped credential with repository-administration permission and a finalization path for both success and failure, plus a manual recovery path for workflow cancellation, runner loss, and finalizer failure. Never put credentials in source, logs, or artifacts.

**Exposure warning:** while public, repository source, issues, Actions metadata/logs, and public release assets can be exposed. Making the repository private later does not undo that exposure or preserve public access to release assets. Do not publish or advertise artifacts during the temporary public interval unless that exposure is intended.

## Validation and reporting

After module installation and reboot on the DSM host:

1. Confirm the expected platform/kernel modules load without kernel errors and `/dev/dri/renderD*` exists.
2. Pass the render device and permissions into the Docker container.
3. Confirm the container's AMD VA-API driver initializes.
4. Complete an FFmpeg hardware transcode and verify that video is processed by hardware—not copied unchanged and not encoded in software.

Record the module workflow outcome separately from the Docker transcode result. A Docker-only pass does not establish native-app compatibility, and build success without this end-to-end test must not be described as verified hardware transcoding.

## Change safety

- K4 AST source/API/provenance and compile-vs-runtime status are recorded in
  `docs/k4-ast-quad-drm-design.md`. Re-read it before AST work after a handoff.
  Use the Linux 5.4 AST baseline, not the K5 tree's newer AST unchanged. AST2600
  is not supported by this baseline. Runtime needs the matching `i2c-algo-bit`
  provider as well as the bundled DRM/TTM stack; MODPOST alone is not load proof.

- Before dispatch, check the selected workflow path, target branch, full matrix inputs, and current repository visibility.
- Preserve existing user changes, workflow artifacts, and diagnostic logs. Do not replace or discard module outputs without checking their provenance.
- Never commit or publish credentials, NAS passwords, or other secrets.
