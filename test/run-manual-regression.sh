#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage: run-manual-regression.sh [options]

Options:
  --reference-ref <ref>   Git ref (commit/tag) for reference build (default: HEAD~1)
  --snr-min <value>       Minimum SNR threshold (default: 60)
  --rms-max <value>       Maximum RMS difference (default: 0.0001)
  --abs-max <value>       Maximum absolute difference (default: 0.01)
  --build-type <type>     CMake build type (default: RelWithDebInfo)
  --cmake-flags <flags>   Extra flags passed to both CMake configure steps
  --keep-worktree         Keep the reference worktree after running
  --help                  Show this help message

Environment variables:
  REFERENCE_REF, SNR_MIN, RMS_MAX, ABS_MAX, BUILD_TYPE, REGRESSION_CMAKE_FLAGS
USAGE
}

REFERENCE_REF=${REFERENCE_REF:-HEAD~1}
SNR_MIN=${SNR_MIN:-60}
RMS_MAX=${RMS_MAX:-0.0001}
ABS_MAX=${ABS_MAX:-0.01}
BUILD_TYPE=${BUILD_TYPE:-RelWithDebInfo}
REGRESSION_CMAKE_FLAGS=${REGRESSION_CMAKE_FLAGS:-}
KEEP_WORKTREE=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --reference-ref)
      REFERENCE_REF="$2"
      shift 2
      ;;
    --snr-min)
      SNR_MIN="$2"
      shift 2
      ;;
    --rms-max)
      RMS_MAX="$2"
      shift 2
      ;;
    --abs-max)
      ABS_MAX="$2"
      shift 2
      ;;
    --build-type)
      BUILD_TYPE="$2"
      shift 2
      ;;
    --cmake-flags)
      REGRESSION_CMAKE_FLAGS="$2"
      shift 2
      ;;
    --keep-worktree)
      KEEP_WORKTREE=1
      shift 1
      ;;
    --help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 1
      ;;
  esac
done

if ! command -v sox >/dev/null 2>&1; then
  echo "sox is required to compare audio files." >&2
  exit 1
fi
if ! command -v cmake >/dev/null 2>&1; then
  echo "cmake is required to build fluidsynth." >&2
  exit 1
fi
if ! command -v git >/dev/null 2>&1; then
  echo "git is required to check out the reference revision." >&2
  exit 1
fi

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT_DIR=$(cd "${SCRIPT_DIR}/.." && pwd)

if ! git -C "$ROOT_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "${ROOT_DIR} is not a git repository." >&2
  exit 1
fi

if ! git -C "$ROOT_DIR" rev-parse --verify "$REFERENCE_REF" >/dev/null 2>&1; then
  echo "Reference ref ${REFERENCE_REF} not found. Fetch full history or set REFERENCE_REF." >&2
  exit 1
fi

CMAKE_GENERATOR_ARGS=()
if [[ -n "${CMAKE_GENERATOR:-}" ]]; then
  CMAKE_GENERATOR_ARGS=(-G "$CMAKE_GENERATOR")
elif command -v ninja >/dev/null 2>&1; then
  CMAKE_GENERATOR_ARGS=(-G Ninja)
fi

CMAKE_FLAGS=()
if [[ -n "$REGRESSION_CMAKE_FLAGS" ]]; then
  read -r -a CMAKE_FLAGS <<< "$REGRESSION_CMAKE_FLAGS"
fi

CURRENT_BUILD_DIR="${CURRENT_BUILD_DIR:-${ROOT_DIR}/build/regression-current}"
REFERENCE_TEST_BUILD_DIR="${REFERENCE_TEST_BUILD_DIR:-${ROOT_DIR}/build/regression-reference}"

REF_WORKTREE=$(mktemp -d "${TMPDIR:-/tmp}/fluidsynth-reference-XXXXXX")
cleanup() {
  if [[ $KEEP_WORKTREE -eq 0 ]]; then
    git -C "$ROOT_DIR" worktree remove --force "$REF_WORKTREE" >/dev/null 2>&1 || true
    rm -rf "$REF_WORKTREE"
    git -C "$ROOT_DIR" worktree prune >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT

echo "Checking out reference revision ${REFERENCE_REF}..."
git -C "$ROOT_DIR" worktree add --detach "$REF_WORKTREE" "$REFERENCE_REF" >/dev/null

REF_BUILD_DIR="${REF_WORKTREE}/build-regression"
cmake -S "$REF_WORKTREE" -B "$REF_BUILD_DIR" "${CMAKE_GENERATOR_ARGS[@]}" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  "${CMAKE_FLAGS[@]}"
cmake --build "$REF_BUILD_DIR" --target fluidsynth

REF_FLUIDSYNTH=$(find "$REF_BUILD_DIR" -type f -name fluidsynth -perm -111 | head -n 1)
if [[ -z "$REF_FLUIDSYNTH" ]]; then
  echo "Could not locate reference fluidsynth binary." >&2
  exit 1
fi

cmake -S "$ROOT_DIR" -B "$CURRENT_BUILD_DIR" "${CMAKE_GENERATOR_ARGS[@]}" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  "${CMAKE_FLAGS[@]}"
cmake --build "$CURRENT_BUILD_DIR" --target check_manual

cmake -S "$ROOT_DIR" -B "$REFERENCE_TEST_BUILD_DIR" "${CMAKE_GENERATOR_ARGS[@]}" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DMANUAL_TEST_FLUIDSYNTH="$REF_FLUIDSYNTH" \
  "${CMAKE_FLAGS[@]}"
cmake --build "$REFERENCE_TEST_BUILD_DIR" --target check_manual

CURRENT_OUTPUT_DIR="${CURRENT_BUILD_DIR}/test/manual"
REFERENCE_OUTPUT_DIR="${REFERENCE_TEST_BUILD_DIR}/test/manual"

if [[ ! -d "$CURRENT_OUTPUT_DIR" || ! -d "$REFERENCE_OUTPUT_DIR" ]]; then
  echo "Manual test output directories were not created." >&2
  exit 1
fi

mapfile -d '' current_files < <(find "$CURRENT_OUTPUT_DIR" -type f \( -name "*.wav" -o -name "*.raw" \) -print0 | LC_ALL=C sort -z)
mapfile -d '' reference_files < <(find "$REFERENCE_OUTPUT_DIR" -type f \( -name "*.wav" -o -name "*.raw" \) -print0 | LC_ALL=C sort -z)

if [[ ${#current_files[@]} -eq 0 ]]; then
  echo "No rendered audio files found in ${CURRENT_OUTPUT_DIR}." >&2
  exit 1
fi

missing=0
for ref_file in "${reference_files[@]}"; do
  rel_path=${ref_file#"$REFERENCE_OUTPUT_DIR/"}
  if [[ ! -f "$CURRENT_OUTPUT_DIR/$rel_path" ]]; then
    echo "Missing current render for ${rel_path}" >&2
    missing=1
  fi
done

for current_file in "${current_files[@]}"; do
  rel_path=${current_file#"$CURRENT_OUTPUT_DIR/"}
  if [[ ! -f "$REFERENCE_OUTPUT_DIR/$rel_path" ]]; then
    echo "Missing reference render for ${rel_path}" >&2
    missing=1
  fi
done

if [[ $missing -ne 0 ]]; then
  exit 1
fi

extract_stat() {
  local pattern="$1"
  awk -F: -v pat="$pattern" '$1 ~ pat { gsub(/^[ \t]+/, "", $2); print $2; exit }'
}

failures=0
printf "%-70s %12s %12s %12s\n" "File" "SNR" "RMS" "ABS"
for current_file in "${current_files[@]}"; do
  rel_path=${current_file#"$CURRENT_OUTPUT_DIR/"}
  reference_file="$REFERENCE_OUTPUT_DIR/$rel_path"

  signal_stats=$(sox "$reference_file" -n stat 2>&1)
  rms_signal=$(printf '%s\n' "$signal_stats" | extract_stat "RMS[[:space:]]+amplitude")

  diff_stats=$(sox -m -v 1 "$current_file" -v -1 "$reference_file" -n stat 2>&1)
  rms_diff=$(printf '%s\n' "$diff_stats" | extract_stat "RMS[[:space:]]+amplitude")
  abs_diff=$(printf '%s\n' "$diff_stats" | extract_stat "Maximum amplitude")

  snr_value=$(awk -v signal="$rms_signal" -v noise="$rms_diff" 'BEGIN { if (noise == 0) { print 1e9; } else { print 20*log(signal/noise)/log(10); } }')
  snr_display=$(awk -v value="$snr_value" 'BEGIN { if (value > 1e8) { print "inf"; } else { printf "%.2f", value; } }')

  printf "%-70s %12s %12s %12s\n" "$rel_path" "$snr_display" "$rms_diff" "$abs_diff"

  if ! awk -v value="$snr_value" -v min="$SNR_MIN" 'BEGIN { exit !(value >= min) }'; then
    echo "  SNR below threshold (${SNR_MIN})" >&2
    failures=$((failures + 1))
  fi
  if ! awk -v value="$rms_diff" -v max="$RMS_MAX" 'BEGIN { exit !(value <= max) }'; then
    echo "  RMS above threshold (${RMS_MAX})" >&2
    failures=$((failures + 1))
  fi
  if ! awk -v value="$abs_diff" -v max="$ABS_MAX" 'BEGIN { exit !(value <= max) }'; then
    echo "  ABS above threshold (${ABS_MAX})" >&2
    failures=$((failures + 1))
  fi
done

if [[ $failures -ne 0 ]]; then
  echo "Audio regression check failed with ${failures} threshold violations." >&2
  exit 1
fi

echo "Audio regression check passed."
