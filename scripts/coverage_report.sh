#!/usr/bin/env bash

set -euo pipefail

minimum="${1:-}"
report_file="$(mktemp)"
trap 'rm -f "$report_file"' EXIT

gcov -b password-core-tests-password_core.gcno >"$report_file"

extract_metric() {
    local metric="$1"
    awk -v file="password_core.cpp" -v metric="$metric" '
        $0 == "File \047" file "\047" { in_file = 1; next }
        in_file && index($0, metric ":") == 1 { print; exit }
        in_file && /^File / { exit }
    ' "$report_file"
}

line_summary="$(extract_metric "Lines executed")"
branch_summary="$(extract_metric "Branches executed")"
taken_summary="$(extract_metric "Taken at least once")"
call_summary="$(extract_metric "Calls executed")"

printf '%s\n' "$line_summary"
printf '%s\n' "$branch_summary"
printf '%s\n' "$taken_summary"
printf '%s\n' "$call_summary"

if [[ -n "$minimum" ]]; then
    line_coverage="$(printf '%s\n' "$line_summary" | awk -F '[:%]' '{ print $2 }')"
    if ! awk -v actual="$line_coverage" -v minimum="$minimum" 'BEGIN { exit !(actual + 0 >= minimum + 0) }'; then
        printf 'Coverage check failed: password_core.cpp line coverage %.2f%% is below %.2f%%\n' \
            "$line_coverage" "$minimum" >&2
        exit 1
    fi

    printf 'Coverage check passed: password_core.cpp line coverage %.2f%% >= %.2f%%\n' \
        "$line_coverage" "$minimum"
fi
